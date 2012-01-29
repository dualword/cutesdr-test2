//
//  sdriq-cute: An attempt to create a Unix server for RF-SPACE SDR-IQ 
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


///// g++ -msse3 siqs.cpp -o siqs -lpthread && ./siqs

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <values.h>

#define LENGTH_MASK  0x1FFF

#include <string>

using namespace std;

/**
 * 
 * open_sdriq_dev
 * 
 * 
 *  VMIN = 0 and VTIME = 0 
 *
 *  This is a completely non-blocking read - the 
 *  call is satisfied immediately directly from the driver's input queue.  
 *  If data are available, it's transferred to the caller's buffer up to 
 *  nbytes and returned.  Otherwise zero is immediately returned to 
 *  indicate "no data".  We'll note that this is "polling" of the serial 
 *  port, and it's almost always a bad idea.  If done repeatedly, it can 
 *  consume enormous amounts of processor time and is highly inefficient.  
 *  Don't use this mode unless you really, really know what you're doing.  
 *
 *
 *
 *  VMIN = 0 and VTIME > 0 
 *
 *  This is a pure timed read.  If data are 
 *  available in the input queue, it's transferred to the caller's buffer 
 *  up to a maximum of nbytes, and returned immediately to the caller.  
 *  Otherwise the driver blocks until data arrives, or when VTIME tenths 
 *  expire from the start of the call.  If the timer expires without data, 
 *  zero is returned.  A single byte is sufficient to satisfy this read 
 *  call, but if more is available in the input queue, it's returned to the 
 *  caller.  Note that this is an overall timer, not an intercharacter one.  
 *   
 *
 *   
 *  VMIN > 0 and VTIME > 0 
 *
 *  A read() is satisfied when either VMIN 
 *  characters have been transferred to the caller's buffer, or when VTIME 
 *  tenths expire between characters.  Since this timer is not started 
 *  until the first character arrives, this call can block indefinitely if 
 *  the serial line is idle.  This is the most common mode of operation, 
 *  and we consider VTIME to be an intercharacter timeout, not an overall 
 *  one.  This call should never return zero bytes read.  
 *
 *
 *
 *  VMIN > 0 and VTIME = 0 
 *
 *  This is a counted read that is satisfied only 
 *  when at least VMIN characters have been transferred to the caller's 
 *  buffer - there is no timing component involved.  This read can be 
 *  satisfied from the driver's input queue (where the call could return 
 *  immediately), or by waiting for new data to arrive: in this respect the 
 *  call could block indefinitely.  We believe that it's undefined behavior 
 *  if nbytes is less then VMIN.  
 *   
 *   
 */ 
int open_sdriq_dev (const string name, string &msg)
{
    struct termios newtio;
    int            sdriq_fd = -1;
    
    if (name.find("/dev/ttyUSB") != string::npos) {    // use ftdi_sio driver
        sdriq_fd = open(name.c_str(), O_RDWR | O_NOCTTY);
        if (sdriq_fd < 0) {
            msg = "Open SDR-IQ : " + string(strerror(errno));
            return -1;
        }
        bzero(&newtio, sizeof(newtio));
        newtio.c_cflag = CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        //cfsetispeed(&newtio, B230400);
        //cfsetospeed(&newtio, B230400);
        int ispeed = cfsetispeed(&newtio, B230400);
        int ospeed = cfsetospeed(&newtio, B230400);
        newtio.c_lflag = 0;
        newtio.c_cc[VTIME]    = 1;  // specify blocking read 
        newtio.c_cc[VMIN]     = 0;
        tcflush(sdriq_fd, TCIFLUSH);
        int rc = tcsetattr(sdriq_fd, TCSANOW, &newtio);
        //fprintf (stderr, "%d %d %d\n", ispeed, ospeed, rc);

        int nr;
        unsigned char ch;
        fprintf (stderr, "Flushing the serial....");
        while ((nr = read(sdriq_fd, &ch, sizeof(ch))) > 0) {
            fprintf (stderr, " %d", nr);
        }
        fprintf (stderr, "\n");
    }
    else {      // use ft245 or similar driver
        sdriq_fd = open(name.c_str(), O_RDWR | O_NONBLOCK); 
        if (sdriq_fd < 0) {
            msg = "Open SDR-IQ: " + string(strerror(errno));
        }
    }         
    
    return sdriq_fd;
}


void dump (unsigned char*buf, int len)
{
    for (int i =0; i < len; ++i) {
        fprintf (stderr, "%02X " , buf[i]);
    }
    fprintf (stderr, "\n"); fflush(stderr);
}

class MiniThread
{
public:
    MiniThread (): thread_id(-1) {}
    ~MiniThread () {}

    virtual int threadproc () = 0; 

private:
    pthread_t thread_id;
    int stop_f;

    static void *thread (void *arg)
    {
        MiniThread *p = (MiniThread *) arg;
        p->threadproc ();
        return p;
    }

public:
    int have_to_stop () { return !stop_f ; }

    int run() 
    {
        stop_f = 0;
        int rc = pthread_create (&thread_id,NULL, thread,(void *)this);
        if(rc<0) {
            perror("pthread_create MiniThread failed");
            exit(1);
        }
    }
    int wait ()
    {
        void *ret;

        if (thread_id != -1) {
            int rc = pthread_join (thread_id, &ret);
        }
    }
    int stop ()
    {
        void *ret;

        if (thread_id != -1) {
            stop_f = -1;
            int rc = pthread_join (thread_id, &ret);
            if (rc) {

            }
        }
    }

};


#define LISTEN_PORT 50000 


template <typename T>
class Listener: public MiniThread {
public:
    
    Listener (T *p, int port = LISTEN_PORT):lis_port(port), pExec(p) {}
    int getSocket () { return sock; }
    int threadproc ();
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


template <typename T>
int Listener<T>::threadproc () {
    
    //int address_length;
    struct sockaddr_in address;
    int rc;
    int on=1;
    
    // create TCP socket to listen on
    s=socket(AF_INET,SOCK_STREAM,0);
    if(s < 0) {
        perror("Listen socket failed");
        exit(1);
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // bind to listening port
    memset(&address,0,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(lis_port);
    if(bind(s,(struct sockaddr*)&address,sizeof(address))<0) {
        perror("Command bind failed");
        exit(1);
    }

    while (1) {

        fprintf(stderr,"Listening for TCP connections on port %d socket: %d\n",LISTEN_PORT, s);
        if(listen(s,5)<0) {
            perror("Command listen failed");
            exit(1);          

        }

        address_length=sizeof(address);
        iq_port=-1;

        if((sock=accept(s,(struct sockaddr*)&address, (socklen_t *)&address_length)) < 0) 
        {
            perror("Command accept failed");
            exit(1);
        }
        
        fprintf(stderr,"client socket %d\n",sock);
        fprintf(stderr,"client connected: %s:%d\n",inet_ntoa(address.sin_addr),ntohs(address.sin_port));

        //
        // allow for one only instance at time
        //
        pExec->setSocket(sock);
        pExec->run();
        pExec->wait();
    }
}



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
    
    enum MsgState {
           MSGSTATE_HDR1 = 0,
           MSGSTATE_HDR2 = 1,
           MSGSTATE_DATA = 2
    } msgState;
    
    
    Radio (int s, int u);
    ~Radio();
    int process (unsigned char ch);
    int threadproc ();

private:
    void udp_write (unsigned char *msg, int len);
    //void SendUDPPacketToSocket( unsigned char* pData);

    int m_UDPSocket;
    short seqNumber;
    sockaddr    m_ClientTCPAddress;
    sockaddr_in m_ClientUDPAddress;

    int sock;
    int usb_fd;
    unsigned char ascpMsg[10240];
    int msgLen;
    int msgIndex;

    pthread_t thread_id;

    // timing variable
    timespec  time_start, time_end, time_diff;

public:
};

Radio::Radio (int s, int u): sock(s), usb_fd(u)
{
    msgState = MSGSTATE_HDR1;
    msgLen = 0;
    msgIndex = 0;
    m_UDPSocket = -1;
    seqNumber = 0;

    fprintf (stderr, "Radio::Radio: %d %d\n", sock, usb_fd);

    socklen_t addrlen = sizeof(m_ClientTCPAddress);
    if (getpeername(sock, &m_ClientTCPAddress, &addrlen)) {
        perror("getpeername failed");
        exit(1);
    } else {

        m_ClientUDPAddress.sin_family = AF_INET;
        m_ClientUDPAddress.sin_addr = ((sockaddr_in *)&m_ClientTCPAddress)->sin_addr;
        m_ClientUDPAddress.sin_port = htons(50000);

        //
        // create the UDP Socket for I/Q sending
        //

        m_UDPSocket = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(m_UDPSocket < 0) {
            perror("create socket failed for I/Q socket\n");
        } else {
            int buffsize = 1000000;
            setsockopt(m_UDPSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&buffsize, sizeof(buffsize));
        }
    }
}

Radio :: ~Radio ()
{
    close (usb_fd);
    close (m_UDPSocket);
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
 
//void Radio::SendUDPPacketToSocket( unsigned char* pData)
//{
//}

void Radio::udp_write( unsigned char *msg, int length)
{
    //break 8192 byte packet into eight 1024 ASCP packets
    for(int i=2; i<8194; i+=1024) {

        unsigned char  dataMsg [10240];
        if(m_UDPSocket == -1)
            return;
        dataMsg[0] = 0x04;    //data item0 length 0x404 = 1024+4
        dataMsg[1] = 0x84;
        dataMsg[2] = seqNumber & 0xFF;
        dataMsg[3] = (seqNumber >> 8) & 0xFF;
        memcpy(&dataMsg[4], (msg+i), 1024);
        int result = sendto(m_UDPSocket, (char*)dataMsg, 1024+4, 0,
                            (struct sockaddr *)&m_ClientUDPAddress,sizeof(m_ClientUDPAddress) );
        if (result < 0) {
            perror( "!!!!!!!!!!! unable to write into UDP socket !");
        } else if(result != (1024+4) ) {       

            fprintf (stderr, "!!!!!!!!!!! partial write into UDP socket !");

            // Something bad happened on the socket.  Deal with it.
            //StopSDRxx();
            //ShutdownConnection(m_ClientSocket);
            //m_SocketSstatus = SOCKET_WAITINGCONNECT;
            //m_USBReInit = TRUE; //signal USB port so it can reinitialize
        }
        seqNumber++;

        if ((seqNumber % 1024) == 0) {

            clock_gettime(CLOCK_REALTIME, &time_end);

            time_diff = ::diff(time_start, time_end);
            long double diff_s = time_diff.tv_sec + (time_diff.tv_nsec/1E9) ;
            //fprintf(stderr, "diff: %ds::%dns %Lf seconds\n", time_diff.tv_sec, time_diff.tv_nsec, diff_s);

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

              write ( sock, ascpMsg, msgLen ); 
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
                  write (sock, ascpMsg, msgLen );
              }
              msgIndex = 0;
           }
           break;
            
    } //end switch statement
}

int Radio :: threadproc ()
{
    unsigned char buf [64];
    fprintf (stderr, "TTT %s, entering reading loop....\n", __FUNCTION__);

    clock_gettime (CLOCK_REALTIME, &time_start);

    while (1) {
        // read from serial
        int len = read (usb_fd, buf, sizeof(buf));
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
public:
    
    enum MsgState {
           MSGSTATE_HDR1 = 0,
           MSGSTATE_HDR2 = 1,
           MSGSTATE_DATA = 2
    } msgState;
    
private:

    int usb_fd;
    int sock;
    unsigned char ascpMsg[10240];
    int msgLen;
    int msgIndex;
    int seqNumber;

    Radio *pRadio;
    string name_serial;

    int openUsb ()
    {
        string buf;

        usb_fd = open_sdriq_dev(name_serial, buf);
        if (usb_fd <0) {
            fprintf (stderr, "Fatal: error opening: %s:\n[%s]\n", name_serial.c_str(), buf.c_str());
            exit (255);
        } 
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
        openUsb();
        msgState = MSGSTATE_HDR1;
        msgLen = 0;
        msgIndex = 0;
        seqNumber = 0;
    }
    
    void setSocket (int s) 
    { 
        sock = s; 
        openUsb();
        fprintf (stderr, "creating Radio object: socket: %d usb_fd: %d\n", s, usb_fd);
        pRadio = new Radio (s, usb_fd);
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

              write ( usb_fd, ascpMsg, msgLen ); 
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

                  write (usb_fd, ascpMsg, msgLen );
                  //usleep (10000);
              }
              msgIndex = 0;
           }
           break;
            
    } //end switch statement
}


int Cute::threadproc ()
{
    string buf;
    
    if (usb_fd < 0) {
        usb_fd = open_sdriq_dev(name_serial, buf);
    }
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
            close (usb_fd);
            usb_fd = -1;
            pRadio->stop();
            delete pRadio;
            break;
        }
    }
}



const char *name = "/dev/ttyUSB0";

int main ()
{
    fprintf (stderr, "------ \n");
    fprintf (stderr, "------ SDR-IQ server for Cute SDR\n");
    fprintf (stderr, "------ \n");
    fprintf (stderr, "------ (C) 2012, Ken N9VV && Andrea IW0HDV\n");
    fprintf (stderr, "\n");

    string buf;
    
    int usb_fd = open_sdriq_dev(name, buf);
    if (usb_fd <0) {
        fprintf (stderr, "Fatal: error opening: %s:\n[%s]\n", name, buf.c_str());
        exit (255);
    } else {
        fprintf (stderr, "%s opened. \n", name);
        close (usb_fd);
    }


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
}


// obsolete code
#if 0
#endif
