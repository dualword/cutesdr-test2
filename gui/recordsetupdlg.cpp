#include "recordsetupdlg.h"
#include "ui_recordsetupdlg.h"
#include <QFileDialog>


CRecordSetupDlg::CRecordSetupDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CRecordSetupDlg)
{
	ui->setupUi(this);
	m_RecordFilePath = "";
}

CRecordSetupDlg::~CRecordSetupDlg()
{
	delete ui;
}

void CRecordSetupDlg::Init()
{
	ui->lineEdit->setText(m_RecordFilePath);
	ui->buttonGroup->addButton(ui->radioButtonAudio, 0);
	ui->buttonGroup->addButton(ui->radioButtonIQ, 1);
	ui->buttonGroup->button(m_RecordMode)->setChecked(true);
}

void CRecordSetupDlg::done(int r)	//virtual override
{
	m_RecordMode = ui->buttonGroup->checkedId();
	QDialog::done(r);	//call base class
}

void CRecordSetupDlg::OnSelectFileButton()
{
QString str = QFileDialog::getSaveFileName(this,tr("Select .wav File Base Name"),m_RecordFilePath,tr("wav files (*.wav)"));
	if(str != "")
	{
		m_RecordFilePath = str;
		ui->lineEdit->setText(m_RecordFilePath);
	}
}
