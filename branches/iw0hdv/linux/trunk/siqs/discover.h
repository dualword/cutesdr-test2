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

#if !defined __DISCOVER_H__
#define      __DISCOVER_H__


#include <stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>

#include <string>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

//
// Excerpted from the sdrxx original code
//
typedef struct  __attribute__((__packed__)) _COMMON__DISCOVER_MSG_SDRXX
{	
    // 56 fixed common byte fields
	unsigned char length[2]; 	//length of total message in bytes (little endian byte order)
	unsigned char key[2];		//fixed key key[0]==0x5A  key[1]==0xA5
	unsigned char op; 		    //0==Request(to device)  1==Response(from device) 2 ==Set(to device)
	char name[16];			    //Device name string null terminated
	char sn[16];			    //Serial number string null terminated
	unsigned char ipaddr[16];	//device IP address (little endian byte order)
	unsigned char port[2];		//device Port number (little endian byte order)
	unsigned char customfield;	//Specify a custom data field for a particular device

} COMMON_DISCOVER_MSG_SDRXX;

typedef struct  __attribute__((__packed__)) _EXTENDED__DISCOVER_MSG_SDRXX
{	
    COMMON_DISCOVER_MSG_SDRXX c;

	// start of optional variable custom byte fields
	unsigned char fwver[2];		//Firmware version*100 (little endian byte order)(read only)
	unsigned char btver[2];		//Boot version*100 (little endian byte order) (read only)
	unsigned char subnet[4];	//IP subnet mask (little endian byte order)
	unsigned char gwaddr[4];	//gateway address (little endian byte order)
	char connection[32];		//interface connection string null terminated(ex: COM3, DEVTTY5, etc)
	unsigned char status;		//bit 0 == TCP connected   Bit 1 == running  Bit 2-7 not defined
	unsigned char future[15];	//future use
} EXTENDED_DISCOVER_MSG_SDRXX;

#define STATUS_BIT_CONNECTED (1)
#define STATUS_BIT_RUNNING (2)

#define KEY0     0x5A
#define KEY1     0xA5
#define MSG_REQ  0
#define MSG_RESP 1
#define MSG_SET  2

#ifdef __cplusplus
};
#endif


#include "minithread.h"
#include "netsupport.h"


class DiscoverSdrxx: public MiniThread {

public:
    DiscoverSdrxx (unsigned short p = 50000);
    ~DiscoverSdrxx () {}

    int threadproc ();
    in_addr_t get_addresses(const int domain, const char *if_name);

private:

    const static unsigned short discover_port        = 48321;
    const static unsigned short discover_client_port = 48322;

    int sock;
    unsigned int   myIP;
	unsigned short myPort;

    unsigned short myBootVer;
    unsigned short myFirmVer;

    char hostname [16];

};



#endif
