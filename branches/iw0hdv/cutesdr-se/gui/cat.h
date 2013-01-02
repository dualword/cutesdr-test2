//////////////////////////////////////////////////////////////////////
// aboutdlg.h: interface for the CAboutDlg class.
//
// History:
//	2012-12-10  Initial creation IW0HDV
/////////////////////////////////////////////////////////////////////
#ifndef CAT_H
#define CAT_H


#include <QObject>
#include <QtCore>
#include "interface/sdrinterface.h"
class QextSerialPort;
class MainWindow;

class CatListener : public QObject
{
Q_OBJECT
public:
    CatListener(const QString & portName = "", MainWindow *p = 0, CSdrInterface* pSdrInterface = 0);
int enumerator_ ();
	QStringList getPortList () { return comList_; }
	int getPorts() { return comList_.size(); }
signals:
	void NewFrequency (qint64 freq);	// emitted when frequency has changed
	void NewHighCutFreq(int freq);      // emitted when the filter has been changed on external device
	void NewLowCutFreq(int freq);       // emitted when the filter has been changed on external device

public slots:
	void setFrequency (qint64 freq) ;  // received when the frequency of remote device has to be changed
	void setComPort (const QString & portName);
	void setDemod (int demod);
	void setLowCutFreq  (int);
	void setHighCutFreq (int);

private:
    void openPort_();

    QStringList comList_;
	QString pname_;
    QextSerialPort *port_;
	char line[256];
    void processMsg ();

	qint64 newFrequency;
    char nf[256];
	int    newDemod;
    char nd[256];

	struct filterTbl {
		char * code;
		float  hf;
	} ;
	
	static struct filterTbl ft[];


private:
	CSdrInterface* m_pSdrInterface;
	MainWindow   * m_pMainWindow;
	int m_DemodMode;

private slots:
    void onReadyRead();
    void onDsrChanged(bool status);

};


#endif // CAT_H