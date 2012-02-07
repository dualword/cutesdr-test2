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

#if !defined __NETSUPPORT_H__
#define      __NETSUPPORT_H__

#define LISTEN_PORT 50000 

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


#include "logger.h"
#include "minithread.h"


struct SysException {

    SysException (int eno, char *pMsg, const char *pDesc = ""):
        errno_(eno), msg_(pMsg), desc_(pDesc) 
    {}

    int   errno_;
    char * msg_;
    const char *desc_;
};


template <typename T>
class Listener: public MiniThread {
public:
    
    Listener (T *p, int port = LISTEN_PORT):lis_port(port), pExec(p) {}
    int getSocket () { return sock; }

    int threadproc () {
        
        //int address_length;
        struct sockaddr_in address;
        int on=1;
        
        // create TCP socket to listen on
        s=socket(AF_INET,SOCK_STREAM,0);
        if(s < 0) {
            //perror();
            throw SysException(errno, strerror(errno), "Listen socket failed");
        }

        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        // bind to listening port
        memset(&address,0,sizeof(address));
        address.sin_family=AF_INET;
        address.sin_addr.s_addr=INADDR_ANY;
        address.sin_port=htons(lis_port);
        if(bind(s,(struct sockaddr*)&address,sizeof(address))<0) {
            throw SysException(errno, strerror(errno), "Command bind failed");
        }

        while (1) {

            fprintf(stderr,"Listening for TCP connections on port %d socket: %d\n",LISTEN_PORT, s);
            if(listen(s,5)<0) {
               throw SysException(errno, strerror(errno), "Command listen failed");
            }

            address_length=sizeof(address);
            iq_port=-1;

            if((sock=accept(s,(struct sockaddr*)&address, (socklen_t *)&address_length)) < 0) {
                throw SysException(errno, strerror(errno), "Command accept failed");
            } else {
                DEBUG(DBG_WARN, "client socket " << sock);
                DEBUG(DBG_WARN, "client connected: " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port));

                //
                // allow for one only instance at time
                //
                pExec->setSocket(sock);
                pExec->run();
                pExec->wait();
            }
        }
    }

private:    
    int address_length;
    struct sockaddr_in address;
    int iq_port;
    int lis_port;
    int s;
    pthread_t thread_id;
protected:    
    int sock;
    T *pExec;
    
};



#endif

