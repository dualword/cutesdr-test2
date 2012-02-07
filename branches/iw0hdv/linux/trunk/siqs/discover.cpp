//
//  sdriq-ftdi: An attempt to create a Unix server for RF-SPACE SDR-IQ 
//
//  Copyright (C) 2012 Andrea Montefusco IW0HDV
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "logger.h"
#include "discover.h"


    DiscoverSdrxx :: DiscoverSdrxx (unsigned short p):myPort(p)
    {
        myIP   = 0;  
        myBootVer = 0;  
        myFirmVer = 0;  

        struct sockaddr_in sa;

     	if( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) == -1) {
            throw SysException (errno, strerror(errno), "Error creating UDP socket in Discover");
     	}

     	int optval = 1;
     	setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval) );
     	setsockopt( sock, SOL_SOCKET, SO_LINGER,    (const char*)&optval, sizeof(optval) );
     	setsockopt( sock, SOL_SOCKET, SO_BROADCAST, (const char*) &optval, sizeof(optval) );

     	memset( &sa, 0, sizeof(sa));
     	sa.sin_family       = AF_INET;
     	sa.sin_addr.s_addr  = htonl(INADDR_ANY );
     	sa.sin_port         = htons(discover_port);

     	if (bind(sock, (sockaddr *)&sa, sizeof(sa)) == -1)
     	{
            throw SysException (errno, strerror(errno), "Socket failed to bind");
     	}

     	// get the hostname
        gethostname(hostname, sizeof(hostname));             


     	optval = 1;
     	setsockopt( sock, SOL_SOCKET, SO_LINGER, (const char*)&optval, sizeof(optval) );
     	setsockopt( sock, SOL_SOCKET, SO_BROADCAST,(const char*) &optval, sizeof(optval) );
    }


    in_addr_t DiscoverSdrxx :: get_addresses(const int domain, const char *if_name)
    {
      int s;
      struct ifconf ifc;
      struct ifreq ifr[50];
      int ifs;
      int i;

      s = socket(domain, SOCK_STREAM, 0);
      if (s < 0) throw SysException (errno, strerror(errno), "Socket failed to create");

      ifc.ifc_buf = (char *) ifr;
      ifc.ifc_len = sizeof ifr;

      if (ioctl(s, SIOCGIFCONF, &ifc) == -1) throw SysException (errno, strerror(errno), "Socket failed to IOCTL");

      ifs = ifc.ifc_len / sizeof(ifr[0]);
      DEBUG(DBG_DEBUG, "interfaces = " << ifs);
      for (i = 0; i < ifs; i++) {
        char ip[INET_ADDRSTRLEN];
        struct sockaddr_in *s_in = (struct sockaddr_in *) &ifr[i].ifr_addr;

        if (!inet_ntop(domain, &s_in->sin_addr, ip, sizeof(ip))) {
            throw SysException (errno, strerror(errno), "inet_ntop: converting IP address");
        } else {
            DEBUG(DBG_DEBUG, ifr[i].ifr_name << " - " << ip);

            if (strcmp(if_name, ifr[i].ifr_name)==0) {
                return s_in->sin_addr.s_addr;
            }
        }

      }

      close(s);

      return 0;
    }
  
    int DiscoverSdrxx :: threadproc ()
    {
        unsigned char buf [10240];

        myIP = get_addresses ( AF_INET , "eth0" );

        XDEBUG(DBG_WARN, 
               fprintf (stderr, "SRXXX STARTED: size %d\n", sizeof(COMMON_DISCOVER_MSG_SDRXX) );
        );
        while (1) {
            int rc = read (sock, buf, sizeof(buf));

            if ( rc < 0 ) break;
            unsigned int len = rc;
            if (len
                && len >=  sizeof(COMMON_DISCOVER_MSG_SDRXX) 
                && len <= (sizeof(COMMON_DISCOVER_MSG_SDRXX) + sizeof(EXTENDED_DISCOVER_MSG_SDRXX)) 
               ) {

                COMMON_DISCOVER_MSG_SDRXX *p = (COMMON_DISCOVER_MSG_SDRXX  *)buf;

                if ( p->key[0] == KEY0 &&  p->key[1] == KEY1 ) {
                    int len = p->length[0] | p->length[1] << 8;

                    XDEBUG(DBG_WARN, 
                           fprintf (stderr, "GOT SRXX message: %d\n", len);
                           if (strlen(p->name)) {
                               fprintf (stderr, "Destination name: [%s]\n", p->name);
                           }
                           if (strlen(p->sn)) {
                               fprintf (stderr, "Destination /N: [%s]\n", p->sn);
                           }
                    );


                    //
                    // setup the answer
                    //
                    EXTENDED_DISCOVER_MSG_SDRXX  emsg;
                    COMMON_DISCOVER_MSG_SDRXX   *pa;

                    memset (&emsg, 0, sizeof(emsg));
                    memcpy (&emsg, p, sizeof(*p));

                    pa = (COMMON_DISCOVER_MSG_SDRXX *)&emsg ;

                	strcpy( pa->name, hostname);          //fill in name string field
                	strcpy( pa->sn, "NOT IMPLEMENTED");   //fill in Serial Number string field
                		

                	pa->length[0] = sizeof(emsg) & 0xFF;
                	pa->length[1] = sizeof(emsg) >> 8;
                	pa->key[0] = KEY0;
                	pa->key[1] = KEY1;
                	pa->op = MSG_RESP;
                    
                    //fill in IP adr and port
                    pa->ipaddr[3] =  myIP      & 0x000000FF;
                    pa->ipaddr[2] = (myIP>>8)  & 0x000000FF;
                    pa->ipaddr[1] = (myIP>>16) & 0x000000FF;
                    pa->ipaddr[0] = (myIP>>24) & 0x000000FF;
                    
                    pa->port[0] = myPort&0x00FF;
                    pa->port[1] = (myPort>>8)&0x00FF;
                    
                    emsg.btver[0] = myBootVer&0x00FF;
                    emsg.btver[1] = (myBootVer>>8)&0x00FF;
                         
                    emsg.fwver[0] = myFirmVer&0x00FF;
                    emsg.fwver[1] = (myFirmVer>>8)&0x00FF;
                         
                    emsg.connection[0] = 'U';
                    emsg.connection[1] = 'S';
                    emsg.connection[2] = 'B';
                    
                    if(0)
                    	emsg.status |= STATUS_BIT_CONNECTED;
                    else
                    	emsg.status &= ~STATUS_BIT_CONNECTED;

                    if( sock != -1 ) {

                        int length = pa->length[1];
                        length <<= 8;
                        length += (int)pa->length[0];

                        struct sockaddr_in satx;
                        memset( &satx, 0, sizeof(satx)-1 );
                        satx.sin_family = AF_INET;
                        satx.sin_addr.s_addr = inet_addr("255.255.255.255");
                        satx.sin_port = htons(discover_client_port);

                        int ns = sendto (sock, (const char*)&emsg, length, 0, (sockaddr *)&satx, sizeof(satx) );
                        if( ns != length ) {
                        	DEBUG(DBG_ERROR, "Send Failed");
                        } else {
                        	DEBUG(DBG_ERROR, "Send Msg");
                        }
                    }

                } else {
                    DEBUG(DBG_ERROR, "Invalid key in discover message: " << p->key[0] << p->key[1]);
                }
            }
        }
        return 0;
    }


