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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <values.h>
#include <errno.h>
#include <ftdi.h>



#include <string>
#include <iostream>

using namespace std;

#include "minithread.h"
#include "netsupport.h"
#include "discover.h"


struct ftdi_context * open_ftdi ()
{
    struct ftdi_context *ftdi;
    char *descstring = NULL;

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return 0;
    }

    if (ftdi_set_interface(ftdi, INTERFACE_A) < 0)
    {
        fprintf(stderr, "ftdi_set_interface failed\n");
        ftdi_free(ftdi);
        return 0;
    }

    if (ftdi_usb_open_desc(ftdi, 0x0403, 0x6001, descstring, NULL) < 0)
    {
        fprintf(stderr,"Can't open ftdi device: %s\n",ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return 0;
    }

    /* A timeout value of 1 results in may skipped blocks */
    if(ftdi_set_latency_timer(ftdi, 2))
    {
        fprintf(stderr,"Can't set latency, Error %s\n",ftdi_get_error_string(ftdi));
        ftdi_usb_close(ftdi);
        ftdi_free(ftdi);
        return 0;
    }

    if(ftdi_usb_purge_rx_buffer(ftdi) < 0)
    {
        fprintf(stderr,"Can't rx purge, error: %s\n",ftdi_get_error_string(ftdi));
        return 0;
    }

    return ftdi;
}



void dump (unsigned char*buf, int len)
{
    for (int i =0; i < len; ++i) {
        fprintf (stderr, "%02X " , buf[i]);
    }
    fprintf (stderr, "\n"); fflush(stderr);
}



    static const int LENGTH_MASK = 0x1FFF;
    
    enum MsgState {
           MSGSTATE_HDR1 = 0,
           MSGSTATE_HDR2 = 1,
           MSGSTATE_DATA = 2
    } msgState;
    
    



/**
 * Radio class
 * 
 * Run on his own thread, read the serial interface (emulated on USB)
 * Decodes the ASCP messages and send it to CuteSDR via IP
 * 
 * we need to decode each message because the data messages containing the      
 * I/Q samples are to be forwarded back to CuteSDR via UDP packets 
 * whilst the others are pushd into the TCP socket
 */ 

class Radio: public MiniThread
{
public:

    Radio (int s, struct ftdi_context *f);

    ~Radio();
    int process (unsigned char ch);
    int threadproc ();

private:
    void udp_write (unsigned char *msg, int len);
    //void SendUDPPacketToSocket( unsigned char* pData);

    int         udpSocket;
    short       seqNumber;
    sockaddr    clientTCPAddr;
    sockaddr_in clientUDPAddr;

    int         sock;
    int         usb_fd;

    struct ftdi_context *ftdi;
    unsigned char ascpMsg[10240];
    int msgLen;
    int msgIndex;

    pthread_t thread_id;

    // timing variables
    timespec  time_start, time_end, time_diff;

public:
};

Radio::Radio (int s, struct ftdi_context *f): sock(s), ftdi(f)
{
    msgState = MSGSTATE_HDR1;
    msgLen = 0;
    msgIndex = 0;
    udpSocket = -1;
    seqNumber = 0;

    fprintf (stderr, "Radio::Radio: %d %d\n", sock, usb_fd);

    socklen_t addrlen = sizeof(clientTCPAddr);
    if (getpeername(sock, &clientTCPAddr, &addrlen)) {
        perror("getpeername failed");
        throw;
    } else {

        clientUDPAddr.sin_family = AF_INET;
        clientUDPAddr.sin_addr = ((sockaddr_in *)&clientTCPAddr)->sin_addr;
        clientUDPAddr.sin_port = htons(50000);

        //
        // create the UDP Socket for I/Q sending
        //

        udpSocket = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(udpSocket < 0) {
            perror("create socket failed for I/Q socket\n");
            throw;
        } else {
            int buffsize = 1000000;
            setsockopt(udpSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&buffsize, sizeof(buffsize));
        }
    }
}

Radio :: ~Radio ()
{
    close (usb_fd);
    close (udpSocket);
    close (sock);
}


timespec diff(timespec start, timespec end)
{
        timespec temp;
        if ((end.tv_nsec - start.tv_nsec) < 0) {
                temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
                temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
        } else {
                temp.tv_sec  = end.tv_sec  - start.tv_sec;
                temp.tv_nsec = end.tv_nsec - start.tv_nsec;
        }
        return temp;
}


/**
 *
 * Called send a 1024 byte ASCP formated message to the Client UDP Socket 
 * Add a two byte sequence number to the header                           
 * 
 */ 
 
void Radio::udp_write( unsigned char *msg, int length)
{
    //break 8192 byte packet into eight 1024 ASCP packets
    for(int i=2; i<8194; i+=1024) {

        unsigned char  dataMsg [10240];
        if(udpSocket == -1)
            return;
        dataMsg[0] = 0x04;    //data item0 length 0x404 = 1024+4
        dataMsg[1] = 0x84;
        dataMsg[2] = seqNumber & 0xFF;
        dataMsg[3] = (seqNumber >> 8) & 0xFF;
        memcpy(&dataMsg[4], (msg+i), 1024);
        int result = sendto(udpSocket, (char*)dataMsg, 1024+4, 0,
                            (struct sockaddr *)&clientUDPAddr,sizeof(clientUDPAddr) );
        if (result < 0) {
            perror( "!!!!!!!!!!! unable to write into UDP socket !");
        } else if(result != (1024+4) ) {       

            fprintf (stderr, "!!!!!!!!!!! partial write into UDP socket !");

            // Something bad happened on the socket.  Deal with it.
        }
        seqNumber++;

        if ((seqNumber % 1024) == 0) {

            clock_gettime(CLOCK_REALTIME, &time_end);

            time_diff = ::diff(time_start, time_end);
            long double diff_s = time_diff.tv_sec + (time_diff.tv_nsec/1E9) ;

            long ts = 1024 * (1024 / 4);
            fprintf (stderr, "Samples received: %lu, %.1Lf kS/s\n", ts, ((double)ts / (diff_s)/1E3) );

            clock_gettime (CLOCK_REALTIME, &time_start);

        }

        if( 0 == seqNumber)   //if seq number rollover
            seqNumber = 1;    //dont use zero as seq number

    }
}

int Radio :: process (unsigned char ch)
{
    int rc = 0;
    //
    // reading from radio serial and decoding
    // the messages
    // we need to decode because the data messages, containing the
    // I/Q samples, are to be forwarded back to CuteSDR via UDP packets
    //
    switch(msgState)    //state machine to get generic ASCP msg from TCP data stream
    {
        case MSGSTATE_HDR1:     //get first header byte of ASCP msg
           ascpMsg[0] = ch;
           msgState = MSGSTATE_HDR2;    //go to get second header byte state
           msgLen = 0;
           msgIndex = 0;
        break;
            
        case MSGSTATE_HDR2: //here for second byte of header
           ascpMsg[1] = ch;
           msgLen = ((ascpMsg[1] << 8) | ascpMsg[0] ) & LENGTH_MASK; // 0x03ff
                
           msgIndex = 2;
           if(2 == msgLen)  {//if msg has no parameters then we are done
              msgState = MSGSTATE_HDR1; //go back to first statewrite ( sock, ascpMsg, msgLen );

              #ifdef  DEBUG
              fprintf (stderr, "%s: writing in TCP socket %d bytes: ", __FUNCTION__, msgLen);
              dump (ascpMsg, msgLen);
              #endif

              rc = write ( sock, ascpMsg, msgLen ); 
              msgLen = 0;
              msgIndex = 0;
           } else {  //there are data bytes to fetch
              if (0 == msgLen) msgLen = 8192+2; //handle special case of 8192 byte data message
                 msgState = MSGSTATE_DATA;  //go to data byte reading state
           }
           break;
    
        case MSGSTATE_DATA: //try to read the rest of the message
           ascpMsg[msgIndex++] = ch;
           if( msgIndex >= msgLen ) {
               msgState = MSGSTATE_HDR1;   //go back to first stage
                    
              //
              // analyze if the message has to be sent via UDP
              //
              if (ascpMsg[1] == 0x80 && ascpMsg[0] == 0x00) {
                  udp_write ( ascpMsg, msgLen );
              }
              else {
                  #ifdef  DEBUG
                  fprintf (stderr, "%s: writing in TCP socket %d bytes: ", __FUNCTION__, msgLen);
                  dump (ascpMsg, msgLen);
                  #endif
                  rc = write (sock, ascpMsg, msgLen );
              }
              msgIndex = 0;
           }
           break;
            
    } //end switch statement
    return rc;
}

int Radio :: threadproc ()
{
    unsigned char buf [64];
    fprintf (stderr, "TTT %s, entering reading loop....\n", __FUNCTION__);

    clock_gettime (CLOCK_REALTIME, &time_start);

    while (1) {
        // read from serial
        //int len = read (usb_fd, buf, sizeof(buf));

        if (ftdi == 0) break;
        
        int len = ftdi_read_data (ftdi, buf, sizeof(buf) );
        if (len > 0) {

            #ifdef  DEBUG
            fprintf (stderr, "%s: read from serial: %d: ", __FUNCTION__, len);
            dump (buf, len);
            #endif
            for (int i=0; i<len; ++i) process(buf[i]);
        } else if (len < 0) {
            fprintf (stderr, "TTT %s, exiting because read failed.\n", __FUNCTION__);
            break;
        }
    }    
    return 0;
}

/**
 * 
 *  Cute class
 * 
 *  Read from TCP socket the messages coming from CuteSDR
 *  scan the ASCP messages in order to send everything down the serial
 *  interface (emulated on USB)
 * 
 */ 


class Cute: public MiniThread
{
private:

    int usb_fd;
    struct ftdi_context *ftdi;
    int sock;
    unsigned char ascpMsg[10240];
    int msgLen;
    int msgIndex;
    int seqNumber;

    Radio *pRadio;
    string name_serial;

    int openFtdi ()
    {
        ftdi = open_ftdi();
        if (ftdi == 0) throw "Unable to get FTDI access.";
        return 0;
    }

public:
    Cute (int ufd): MiniThread(), usb_fd(ufd)
    {
        msgState = MSGSTATE_HDR1;
        msgLen = 0;
        msgIndex = 0;
        seqNumber = 0;
    }


    Cute (const char *name): MiniThread(), name_serial(name)
    {
        openFtdi();
        msgState = MSGSTATE_HDR1;
        msgLen = 0;
        msgIndex = 0;
        seqNumber = 0;
    }
    
    void setSocket (int s) 
    { 
        sock = s; 
        openFtdi ();
        fprintf (stderr, "creating Radio object: socket: %d usb_fd: %d\n", s, usb_fd);
        pRadio = new Radio (s, ftdi);
        if (pRadio) pRadio->run();
    }

    int threadproc ();
    int process (unsigned char ch);
    
};

/**
 * reading from radio serial and decoding the messages
 * 
 * 
 */
int Cute :: process (unsigned char ch)
{
    int rc = 0;

    switch(msgState)    //state machine to get generic ASCP msg from TCP data stream
    {
        case MSGSTATE_HDR1:             //get first header byte of ASCP msg
           ascpMsg[0] = ch;
           msgState = MSGSTATE_HDR2;    //go to get second header byte state
           msgLen = 0;
           msgIndex = 0;
        break;
            
        case MSGSTATE_HDR2: //here for second byte of header
           ascpMsg[1] = ch;
           msgLen = ((ascpMsg[1] << 8) | ascpMsg[0] ) & LENGTH_MASK;
                
           msgIndex = 2;
           if(2 == msgLen)  {//if msg has no parameters then we are done
              msgState = MSGSTATE_HDR1; //go back to first statewrite

              #ifdef DEBUG
              fprintf (stderr, "%s: writing in SERIAL(2) %d bytes: ", __FUNCTION__, msgLen);
              dump (ascpMsg, msgLen);
              #endif

              //write ( usb_fd, ascpMsg, msgLen ); 

              rc = ftdi_write_data (ftdi, ascpMsg, msgLen );

              msgLen = 0;
              msgIndex = 0;
           } else {  //there are data bytes to fetch
              if (0 == msgLen) msgLen = 8192+2; //handle special case of 8192 byte data message
                 msgState = MSGSTATE_DATA;  //go to data byte reading state
           }
           break;
    
        case MSGSTATE_DATA: //try to read the rest of the message
           ascpMsg[msgIndex++] = ch;
           if( msgIndex >= msgLen ) {
               msgState = MSGSTATE_HDR1;   //go back to first stage
                    
              //
              // analyze if the message has to be sent via UDP
              //
              if (ascpMsg[1] == 0x80 && ascpMsg[0] == 0x00) {
                  //udp_write ( ascpMsg, msgLen );
              }
              else {
                  #ifdef DEBUG
                  fprintf (stderr, "%s: writing in SERIAL %d bytes: ", __FUNCTION__, msgLen);
                  dump (ascpMsg, msgLen);
                  #endif

                  //write (usb_fd, ascpMsg, msgLen );
                  rc = ftdi_write_data (ftdi, ascpMsg, msgLen );
                  //usleep (10000);
              }
              msgIndex = 0;
           }
           break;
            
    } //end switch statement
    return rc;
}


int Cute::threadproc ()
{
    string buf;
    
    msgState = MSGSTATE_HDR1;
    msgLen = 0;
    msgIndex = 0;
    seqNumber = 0;

    fprintf (stderr, "TTT Entering in execute, reading from TCP socket\n");
    while (1) {
        int nr;
        unsigned char buf[1024];

        nr = read (sock, buf, sizeof(buf));
        if (nr > 0) {
            #ifdef  DEBUG
            fprintf (stderr, "%s: %d bytes read from socket: ", __FUNCTION__, nr);
            dump (buf, nr);
            #endif
            for (int j=0; j<nr; ++j) {
                //
                // write to receiver
                //
                process (buf[j]);
            }
        } else {
            fprintf (stderr, "TTT exiting reading from TCP socket failed\n");
            
            ftdi_usb_close(ftdi);
            ftdi_free(ftdi);
            ftdi = 0;
            pRadio->stop();
            delete pRadio;
            break;
        }
    }
    return 0;
}



const char *name = "/dev/ttyUSB0";

int scan_ftdi ()
{
    int ret, i;
    struct ftdi_context *ftdi;
    struct ftdi_device_list *devlist, *curdev;
    char manufacturer[128], description[128];
    int retval = EXIT_SUCCESS;

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }

    if ((ret = ftdi_usb_find_all(ftdi, &devlist, 0, 0)) < 0)
    {
        fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        retval =  EXIT_FAILURE;
        goto do_deinit;
    }

    printf("Number of FTDI devices found: %d\n", ret);

    i = 0;
    for (curdev = devlist; curdev != NULL; i++)
    {
        printf("Checking device: %d\n", i);
        if ((ret = ftdi_usb_get_strings(ftdi, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0)
        {
            fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
            retval = EXIT_FAILURE;
            goto done;
        }
        printf("Manufacturer: %s, Description: %s\n\n", manufacturer, description);
        curdev = curdev->next;
    }
done:
    ftdi_list_free(&devlist);
do_deinit:
    ftdi_free(ftdi);

    return retval;
}


int main ()
{
    //struct ftdi_context *ftdi;
    //char *descstring = NULL;

    fprintf (stderr, "------ \n");
    fprintf (stderr, "------ SDR-IQ server for Cute SDR\n");
    fprintf (stderr, "------ \n");
    fprintf (stderr, "------ libftdi version\n");
    fprintf (stderr, "------ \n");
    fprintf (stderr, "------ (C) 2012, Ken N9VV && Andrea IW0HDV\n");
    fprintf (stderr, "\n");

    string buf;
    
    //scan_ftdi();

    try {
       DiscoverSdrxx sd;
       sd.run();


       Cute c(name);          // Instantiate a Cute object

       Listener<Cute> l(&c);  // Instantiate the Listener
       l.run();

       // wait for exit from user 
       puts( "Press q <ENTER> to exit." );
 
       char ch;
       while((ch = getc(stdin)) != EOF) {
           if (ch == 'q') break;
       }
       return 0;

     } catch (const char *msg) {
         std::cerr << msg << std::endl;
     } catch (SysException &le) {
         std::cerr << le.desc_ << ": " << le.msg_ << std::endl;
     } catch (...) {
         std::cerr << "Unexpected exception !" << std::endl;
     } 
     ;
}

