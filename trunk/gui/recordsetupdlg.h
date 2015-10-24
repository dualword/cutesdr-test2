#ifndef RECORDSETUPDLG_H
#define RECORDSETUPDLG_H

#include <QDialog>

namespace Ui {
class CRecordSetupDlg;
}

class CRecordSetupDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CRecordSetupDlg(QWidget *parent = 0);
	~CRecordSetupDlg();
	void Init();
	qint32 m_RecordMode;
	QString m_RecordFilePath;

protected:
	void done(int r);	//virtual override

private slots:
	void OnSelectFileButton();
private:
	Ui::CRecordSetupDlg *ui;
};

#endif // RECORDSETUPDLG_H
