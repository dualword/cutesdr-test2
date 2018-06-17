#ifndef FILETXDLG_H
#define FILETXDLG_H

#include <QDialog>
#include "interface/sdrinterface.h"
#include "interface/wavefilereader.h"
#include "dsp/datamodifier.h"


#define MAX_TXBLKSAMPLES 1024


namespace Ui {
class CFileTxDlg;
}

class CFileTxDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CFileTxDlg(QWidget *parent, CSdrInterface*  pSdrInterface);
	~CFileTxDlg();
	void Init();
	QString m_TxFilePath;
	qint64 m_TxFrequency;
	bool m_TxRepeat;


private slots:
	void NewTxMsgSlot(int FifoPtr);
	void OnNewCenterFrequency(qint64 freq);	//called when center frequency has changed
	void on_pushButtonFileSelect_clicked();
	void on_checkBoxRepeat_clicked(bool checked);
	void on_pushButtonStart_clicked();
	void on_pushButtonStartTest_clicked();
	void on_pushButtonStopTest_clicked();

protected:
	void done(int r);	//virtual override

private:
	void SetTxState(bool On);
	void SetTxFreq(quint32 freq);
	int SendIQDataBlk(tICpx32* pData, int NumSamples);
	void GenerateTestData(tICpx32* pBuf, int NumSamples);

	Ui::CFileTxDlg *ui;
	CSdrInterface* m_pSdrInterface;
	CWaveFileReader m_FileReader;
	CDataModifier m_DataModifier;
	TYPECPX m_TxDataBuf[MAX_TXBLKSAMPLES];
	tICpx32 m_TestBuf[MAX_TXBLKSAMPLES];
	quint16 m_SeqNumber;
};

#endif // FILETXDLG_H
