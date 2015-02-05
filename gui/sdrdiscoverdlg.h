//////////////////////////////////////////////////////////////////////
// sdrdiscoverdlg.h: interface for the CSdrDiscoverDlg class.
//
// History:
//	2010-09-15  Initial creation MSW
//	2011-03-27  Initial release
/////////////////////////////////////////////////////////////////////
#ifndef SDRDISCOVERDLG_H
#define SDRDISCOVERDLG_H
#include "ui_sdrdiscoverdlg.h"

#include <QDialog>
#include <QUdpSocket>
#include <QTimer>
#include <QListWidget>
#include <QKeyEvent>

#define MAX_DEVICES 32

typedef struct __attribute__ ((__packed__)) _DISCOVER_MSG_COMMON
{		//56 fixed common byte fields
	unsigned char length[2]; 	//length of total message in bytes (little endian byte order)
	unsigned char key[2];		//fixed key key[0]==0x5A  key[1]==0xA5
	unsigned char op;			//0==Request(to device)  1==Response(from device) 2 ==Set(to device)
	char name[16];				//Device name string null terminated
	char sn[16];				//Serial number string null terminated
	unsigned char ipaddr[16];	//device IP address (little endian byte order)
	unsigned char port[2];		//device Port number (little endian byte order)
	unsigned char customfield;	//Specifies a custom data field for a particular device
}tDiscover_COMMONMSG;

typedef struct __attribute__ ((__packed__)) _DISCOVER_MSG_NETSDR
{		//56 fixed common byte fields
	unsigned char length[2]; 	//length of total message in bytes (little endian byte order)
	unsigned char key[2];		//fixed key key[0]==0x5A  key[1]==0xA5
	unsigned char op;			//0==Request(to device)  1==Response(from device) 2 ==Set(to device)
	char name[16];				//Device name string null terminated
	char sn[16];				//Serial number string null terminated
	unsigned char ipaddr[16];	//device IP address (little endian byte order)
	unsigned char port[2];		//device Port number (little endian byte order)
	unsigned char customfield;	//Specifies a custom data field for a particular device
		//start of optional variable custom byte fields
	unsigned char macaddr[6];	//HW mac address (little endian byte order) (read only)
	unsigned char hwver[2];		//Hardware version*100  (little endian byte order) (read only)
	unsigned char fwver[2];		//Firmware version*100 (little endian byte order)(read only)
	unsigned char btver[2];		//Boot version*100 (little endian byte order) (read only)
	unsigned char fpgaid;		//FPGA ID (read only)
	unsigned char fpgarev;		//FPGA revision (read only)
	unsigned char opts;			//Options (read only)
	unsigned char mode;			//0 == Use DHCP 1==manual  2==manual Alternate data address
	unsigned char subnet[4];	//IP subnet mask (little endian byte order)
	unsigned char gwaddr[4];	//gateway address (little endian byte order)
	unsigned char dataipaddr[4];// Alternate data IP address for UDP data  (little endian byte order)
	unsigned char dataport[2];	// Alternate data Port address for UDP (little endian byte order)
	unsigned char fpga;			//0 == default cfg   1==custom1    2==custom2
	unsigned char status;		//bit 0 == TCP connected   Bit 1 == running  Bit 2-7 not defined
	unsigned char future[15];	//future use
}tDiscover_NETSDR;

typedef struct __attribute__ ((__packed__)) _DISCOVER_MSG_SDRXX
{		//56 fixed common byte fields
	unsigned char length[2]; 	//length of total message in bytes (little endian byte order)
	unsigned char key[2];		//fixed key key[0]==0x5A  key[1]==0xA5
	unsigned char op;			//0==Request(to device)  1==Response(from device) 2 ==Set(to device)
	char name[16];				//Device name string null terminated
	char sn[16];				//Serial number string null terminated
	unsigned char ipaddr[16];	//device IP address (little endian byte order)
	unsigned char port[2];		//device Port number (little endian byte order)
	unsigned char customfield;	//Specifies a custom data field for a particular device
		//start of optional variable custom byte fields
	unsigned char fwver[2];		//Firmware version*100 (little endian byte order)(read only)
	unsigned char btver[2];		//Boot version*100 (little endian byte order) (read only)
	unsigned char subnet[4];	//IP subnet mask (little endian byte order)
	unsigned char gwaddr[4];	//gateway address (little endian byte order)
	char connection[32];		//interface connection string null terminated(ex: COM3, DEVTTY5, etc)
	unsigned char status;		//bit 0 == TCP connected   Bit 1 == running  Bit 2-7 not defined
	unsigned char future[15];	//future use
}tDiscover_SDRxx;

typedef struct __attribute__ ((__packed__)) _DISCOVER_MSG_CLOUDSDR
{		//56 fixed common byte fields
	unsigned char length[2]; 	//length of total message in bytes (little endian byte order)
	unsigned char key[2];		//fixed key key[0]==0x5A  key[1]==0xA5
	unsigned char op;			//0==Request(to device)  1==Response(from device) 2 ==Set(to device)
	char name[16];				//Device name string null terminated
	char sn[16];				//Serial number string null terminated
	unsigned char ipaddr[16];	//device IP address (little endian byte order)
	unsigned char port[2];		//device Port number (little endian byte order)
	unsigned char customfield;	//Specifies a custom data field for a particular device
		//start of optional variable custom byte fields
	unsigned char macaddr[6];	//HW mac address (little endian byte order) (read only)
	unsigned char hwver[2];		//Hardware version*100  (little endian byte order) (read only)
	unsigned char fwver[2];		//Firmware version*100 (little endian byte order)(read only)
	unsigned char btver[2];		//Boot version*100 (little endian byte order) (read only)
	unsigned char fpgaid;		//FPGA ID (read only)
	unsigned char fpgarev;		//FPGA revision (read only)
	unsigned char opts;			//Options (read only)
	unsigned char mode;			//bit 0 (0==manual 1==Use DHCP), bit 1 (0=list client off, 1==List client On)
	unsigned char subnet[4];	//IP subnet mask (little endian byte order)
	unsigned char gwaddr[4];	//gateway address (little endian byte order)
	unsigned char dataipaddr[4];// Alternate data IP address for UDP data  (little endian byte order)
	unsigned char dataport[2];	// Alternate data Port address for UDP (little endian byte order)
	unsigned char status;		//bit 0 == TCP connected   Bit 1 == running  Bit 2-7 not defined
	unsigned char sdrmode;		//0 == Normal local SDR Mode   1 == Internet Web Mode  2 == Aux Mode (3-255 Not defined)
	char serverdescr[151];		//up to 150 character NULL terminated server description string
	char serverdescrurl[151];	//up to 150 character NULL terminated server description URL string
	char serverdomain[81];		//up to 80 character NULL terminated optional server domain string
	char listdomain[81];		//up to 80 character NULL terminated list server domain string
	char listscript[81];		//up to 80 character NULL terminated list server path/script string
	char rxpw[17];				//up to 16 character NULL terminated Rx Password string
	char txpw[17];				//up to 16 character NULL terminated Tx Password string
	char gpslat[33];			//up to 32 character NULL terminated Latitude string
	char gpslon[33];			//up to 32 character NULL terminated Longitude string
	unsigned char tcptimeout;	//TCP Connection timeout in minutes(0==no timeout)
	unsigned char uartuse;		//UART use mode 0==normal  1==Monitor   2 = Debug
	unsigned char future[128];	//future use
}tDiscover_CLOUDSDR;

#define STATUS_BIT_CONNECTED (1)
#define STATUS_BIT_RUNNING (2)

namespace Ui {
    class CSdrDiscoverDlg;
}

class CSdrDiscoverDlg : public QDialog
{
    Q_OBJECT

public:
	explicit CSdrDiscoverDlg(QWidget *parent);
    ~CSdrDiscoverDlg();

	void InitDlg();

	QString m_NameFilter;

	quint32 m_IPAdr;
	unsigned short m_Port;
	QString m_Name;

public slots:
	void accept();

private slots:
	void OnFind();
	void OnItemDoubleClick( QListWidgetItem * item );
	void ReadUDPMessages();
	void SendDiscoverRequest();
	void CloseUdp();

private:
    Ui::CSdrDiscoverDlg *ui;
	void ParseMsg(int index);
	QTimer m_Timer;
	QTimer m_CloseTimer;
	bool m_UdpOpen;
    QUdpSocket* m_pUdpDiscoverSocket;
	tDiscover_COMMONMSG m_DiscovermsgCommon[MAX_DEVICES];
	tDiscover_NETSDR m_DiscovermsgNetSDR[MAX_DEVICES];
	tDiscover_CLOUDSDR m_DiscovermsgCloudSDR[MAX_DEVICES];
	tDiscover_SDRxx m_DiscovermsgSDRxx[MAX_DEVICES];
};

#endif // SDRDISCOVERDLG_H
