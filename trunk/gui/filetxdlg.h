#ifndef FILETXDLG_H
#define FILETXDLG_H

#include <QDialog>
#include <QTimer>
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
	bool m_UseTxFile;
	TYPEREAL m_TxSignalPower;
	TYPEREAL m_TxNoisePower;
	qint32 m_TxSweepStartFrequency;
	qint32 m_TxSweepStopFrequency;
	qint32 m_TxSweepRate;

private slots:
	void NewTxMsgSlot(int FifoPtr);
	void OnTimer();
	void OnNewCenterFrequency(qint64 freq);	//called when center frequency has changed
	void on_pushButtonFileSelect_clicked();
	void on_checkBoxRepeat_clicked(bool checked);
	void on_checkBoxUseFile_clicked(bool checked);
	void on_pushButtonStart_clicked();
	void on_pushButtonStop_clicked();

	void on_spinBoxAmp_valueChanged(int arg1);

	void on_spinBoxNoise_valueChanged(int arg1);

	void on_spinBoxStart_valueChanged(int arg1);

	void on_spinBoxStop_valueChanged(int arg1);

	void on_spinBoxSweep_valueChanged(int arg1);

protected:
	void done(int r);	//virtual override

private:
	void SetTxState(bool On);
	void SetTxFreq(quint32 freq);
	int SendIQDataBlk(tICpx32* pData, int NumSamples);
	int SendIQDataBlk(TYPECPX* pData, int NumSamples);
	void GenerateTestData(tICpx32* pBuf, int NumSamples);

	Ui::CFileTxDlg *ui;
	CSdrInterface* m_pSdrInterface;
	CWaveFileReader m_FileReader;
	CDataModifier m_DataModifier;
	QTimer *m_pTimer;
	TYPECPX m_TxDataBuf[MAX_TXBLKSAMPLES];
	tICpx32 m_TestBuf[MAX_TXBLKSAMPLES];
	bool m_CtrlLockout;
	bool m_TransmitOn;
	quint16 m_SeqNumber;
	quint32 m_FileTotalSamples;
	quint32 m_FileSamplesSent;
};

#endif // FILETXDLG_H
