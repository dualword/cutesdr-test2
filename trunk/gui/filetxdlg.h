#ifndef FILETXDLG_H
#define FILETXDLG_H

#include <QDialog>
#include "interface/sdrinterface.h"
#include "interface/wavefilereader.h"


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
	void OnNewCenterFrequency(qint64 freq);	//called when center frequency has changed

	void on_pushButtonFileSelect_clicked();

	void on_checkBoxRepeat_clicked(bool checked);

	void on_pushButtonStart_clicked();

protected:
	void done(int r);	//virtual override

private:
	Ui::CFileTxDlg *ui;
	CSdrInterface* m_pSdrInterface;
	CWaveFileReader m_FileReader;
	TYPECPX m_TxDataBuf[MAX_TXBLKSAMPLES];
};

#endif // FILETXDLG_H
