#include <QtExtSerialPort/qextserialport.h>
#include <QtExtSerialPort/qextserialenumerator.h>
#include <QtDebug>

#include "gui/mainwindow.h"
#include "cat.h"



#if   defined (Q_OS_LINUX)
#define PORT_SEED "tty"
#elif defined (Q_OS_WIN)
#define PORT_SEED "COM"
#elif defined (Q_OS_MAC)
#define PORT_SEED "tty"
#else
  #error "PLATFORM NOT SUPPORTED"
#endif
struct CatListener::filterTbl CatListener::ft [] = 
	{
		{ "00", 5000.0f },
		{ "01", 4400.0f },
		{ "02", 3800.0f },
		{ "03", 3300.0f },
		{ "04", 2900.0f },
		{ "05", 2700.0f },
		{ "06", 2400.0f },
		{ "07", 2100.0f },
		{ "08", 1800.0f },
		{ "09", 1000.0f },
	};

CatListener::CatListener(const QString & portName, 
						 MainWindow *pMainWindow, 
						 CSdrInterface* pSdrInterface) :
							m_pMainWindow(pMainWindow), 
							m_pSdrInterface(pSdrInterface), 
							pname_(portName)
{
	if (enumerator_() == 0) return;

	if (pname_ == "") pname_ = comList_.last(); 

	port_ = new QextSerialPort(pname_, QextSerialPort::EventDriven);

	if (port_ == 0) return;
    qDebug() << "CAT: " << port_ ;
	
	port_->setBaudRate(BAUD19200);
    port_->setFlowControl(FLOW_OFF);
    port_->setParity(PAR_NONE);
    port_->setDataBits(DATA_8);
    port_->setStopBits(STOP_1);

    connect(port_, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(port_, SIGNAL(dsrChanged(bool)), this, SLOT(onDsrChanged(bool)));
	 
 	line[0] = 0;
	newFrequency = 0;
	nf[0] = 0;
	newDemod = -1;
	nd[0] = 0;

	openPort_();
}


int CatListener::enumerator_ ()
{
	int np = 0;
	QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
	qDebug() << "List of ports:";
	foreach (QextPortInfo info, ports) {
        qDebug() << "port name:"       << info.portName;
        qDebug() << "friendly name:"   << info.friendName;
        qDebug() << "physical name:"   << info.physName;
        qDebug() << "enumerator name:" << info.enumName;
        qDebug() << "vendor ID:"       << info.vendorID;
        qDebug() << "product ID:"      << info.productID;

        qDebug() << "===================================" << PORT_SEED ;
		
	if ( info.portName.startsWith(PORT_SEED) ) {
		qDebug() << info.portName << np ;
		comList_ <<  info.portName, ++np;
	}
    }
	return np;
}

void CatListener::setComPort (const QString & portName) 
{ 
	if (port_->isOpen()) port_->close();
	pname_ = portName; 
    port_->setPortName(pname_);
	openPort_();
}

void CatListener::openPort_()
{
   if (port_->open(QIODevice::ReadWrite) == true) {
        if (!(port_->lineStatus() & LS_DSR))
            qDebug() << "CAT: warning: device is not turned on";
        qDebug() << "CAT: listening for data on" << port_->portName();
    } else {
        qDebug() << "CAT: device failed to open:" << port_->errorString();
    }
}


void CatListener::onReadyRead()
{
	//qDebug() << "CAT: onReadyRead";

	if (port_ == 0) return;
    QByteArray bytes;
    int n = port_->bytesAvailable();
    bytes.resize(n);
    port_->read(bytes.data(), bytes.size());
    //qDebug() << "bytes read:" << bytes.size();
    //qDebug() << "bytes:" << bytes;

	for (int i=0; i<n; ++i) {
		if (bytes[i] == ';') {
			// end of message detected
			processMsg();
			line[0] = 0;
		} else {
			int l = strlen(line); 
			line [l] = bytes[i];
            line [l+1] = 0;
		}
	}
}

void CatListener::processMsg ()
{ 
	//qDebug() << "CAT: processMsg: [" << line << "]";

	if ( (strcmp(line, "FA") == 0) && strlen(nf) ) {
		qDebug() << "CAT: processMsg: " << nf;
		port_->write(nf, strlen(nf));
		nf[0] = 0;
		return;
	}

	qint64 nfx;
	if (sscanf (line, "FA%Lu", &nfx) == 1) {
		qDebug() << "CAT: new frequency: [" << (int)nfx << "]";
		emit NewFrequency ((int)nfx);
		return;
	}

	if (strcmp(line, "ZZMA1") == 0) { 
		if (m_pSdrInterface) m_pSdrInterface->Mute();
		qDebug() << "CAT ZZMA1" ; 
		return;
	}
	if (strcmp(line, "ZZMA0") == 0) {
		if (m_pSdrInterface) m_pSdrInterface->UnMute();
		qDebug() << "CAT ZZMA0" ; 
		return;
	}

	/*
		ZZMD Command

		P1 values:
			00 = LSB
			01 = USB
			02 = DSB
			03 = CWL
			04 = CWU
			05 = FM
			06 = AM
			07 = DIGU
			08 = SPEC
			09 = DIGL
			10 = SAM
			11 = DRM
	*/

	if ( (strcmp(line, "ZZMD") == 0) && strlen(nd) ) {
		qDebug() << "CAT: processMsg: " << nd;
		port_->write(nd, strlen(nd));
		nd[0] = 0;
		return;
	}

	int demod;
	if (sscanf (line, "ZZMD%d", &demod) == 1) {
		qDebug() << "CAT: new demod: [" << demod << "]";
		switch (demod) {
		case 6:
			m_DemodMode = DEMOD_AM;
			break;
		case 10:
			m_DemodMode = DEMOD_SAM;
			break;
		case 5:
			m_DemodMode = DEMOD_FM;
			break;
		case 1:
			m_DemodMode = DEMOD_USB;
			break;
		case 0:
			m_DemodMode = DEMOD_LSB;
			break;
		case 4:
			m_DemodMode = DEMOD_CWU;
			break;
		case 3:
			m_DemodMode = DEMOD_CWL;
			break;
		}

		m_pMainWindow->SetupDemod(m_DemodMode);
		//m_pDemodInfo = &(((MainWindow*)this->parent())->m_DemodSettings[m_DemodMode]);
		//UpdateDemodInfo();

		return;
	}
	/*
		ZZGT Sets or reads the AGC thumbwheel control
		P1 values:
		0 = Fixed
		1 = Long
		2 = Slow
		3 = Med
		4 = Fast
		5 = Custom
	*/
	int agc;
	if (sscanf (line, "ZZGT%d", &agc) == 1) {
		qDebug() << "CAT: new AGC [" << agc << "]";

	}

	/*
		ZZFI Command
		ZZFI Sets or reads the current RX1 DSP receive filter
		Notes
		P1 values: lsb/usb digl/digu am/sam/dsb cwl/cwu

		00 5.0K 3.0K 16K 1.0K
		01 4.4K 2.5K 12K 800
		02 3.8K 2.0K 10K 750
		03 3.3K 1.5K 8.0K 600
		04 2.9K 1.0K 6.6K 500
		05 2.7K 800 5.2K 400
		06 2.4K 600 4.0K 250
		07 2.1K 300 3.1K 100
		08 1.8K 150 2.9K 50
		09 1.0K 75 2.4K 25
		10 VAR1 VAR1 VAR1 VAR1
		11 VAR2 VAR2 VAR2 VAR2

		These are the default values for the receive filters. If you customize your filters, 
		your custom values will be displayed.
	*/
	char hc[3];
	float x = - 1.0f;
	if (sscanf (line, "ZZFI%s", hc) == 1) {
        hc[2] = 0;
		qDebug() << "CAT: new FILTER [" << hc << "]";

		for (int i=0; i < (sizeof(ft)/sizeof(ft[0])); ++i)
			if (strcmp(hc, ft[i].code) == 0) {
			x = ft[i].hf;
			break; 
		}
		if (x > 0.0) {
			qDebug() << "CAT: filter high cut: [" << x << "]";
			if (newDemod == DEMOD_USB)
			   emit NewHighCutFreq((int)x);
			if (newDemod == DEMOD_LSB)
			   emit NewLowCutFreq((int)-x);
		}
	}
}


void CatListener::setFrequency (qint64 freq) // received when the frequency of remote device has to be changed
{
	qDebug() << "CAT: setFrequency: [" << (int)freq << "]";
	if (freq != newFrequency) {
		newFrequency = freq;
		sprintf (nf, "FA%011ld;", newFrequency);
	}
}

void CatListener::setDemod (int demod) // received when the demodulator of remote device has to be changed
{
	qDebug() << "CAT: setDemod: [" << demod << "]";
	if (demod != newDemod) {
		int x = -1;
		 
		switch(demod) {
		case DEMOD_AM:
			x = 6;
			break;
		case DEMOD_SAM:
			x = 6;
			break;
		case DEMOD_USB:
			x = 1;
			break;
		case DEMOD_LSB:
			x = 0;
			break;
		case DEMOD_CWU:
			x = 4;
			break;
		case DEMOD_CWL:
			x = 3;
			break;
		default:
			qDebug() << "CAT: setDemod: unknow value [" << newDemod << "]";
			return;
		}

		newDemod = demod;
		sprintf (nd, "ZZMD%02d;", x);
		qDebug() << "CAT: setDemod string: [" << nd << "]";
	}
}

void CatListener::setLowCutFreq  (int)
{
}

void CatListener::setHighCutFreq (int hc)
{
	char *x = 0;

	for (int i=0; i < (sizeof(ft)/sizeof(ft[0])); ++i)
		if (abs(hc) > ft[i].hf) {
			x = ft[i].code;
			break; 
		}

	if (x) {
		char  ff [256];
		sprintf (ff, "ZZFI%s;", x);
		qDebug() << "CAT: setLowCutFreq string: [" << ff << "]";
		port_->write(ff, strlen(ff));
	}
}


void CatListener::onDsrChanged(bool status)
{
    if (status)
        qDebug() << "device was turned on";
    else
        qDebug() << "device was turned off";
}
