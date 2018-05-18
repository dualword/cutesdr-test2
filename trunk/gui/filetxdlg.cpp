#include "filetxdlg.h"
#include "ui_filetxdlg.h"
#include <QFileDialog>

CFileTxDlg::CFileTxDlg(QWidget *parent, CSdrInterface* pSdrInterface) :
	QDialog(parent),
	m_pSdrInterface(pSdrInterface),
	ui(new Ui::CFileTxDlg)
{
	ui->setupUi(this);
	connect(ui->frameTxFreqCtrl, SIGNAL(NewFrequency(qint64)), this, SLOT(OnNewCenterFrequency(qint64)));
	ui->frameTxFreqCtrl->Setup(10, 100U, 1700000000U, 1, UNITS_MHZ );
	ui->frameTxFreqCtrl->SetBkColor(Qt::black);
	ui->frameTxFreqCtrl->SetDigitColor(Qt::yellow);
	ui->frameTxFreqCtrl->SetUnitsColor(Qt::lightGray);
	ui->frameTxFreqCtrl->SetHighlightColor(Qt::darkGray);
	ui->frameTxFreqCtrl->SetFrequency(1234567);
	m_TxFilePath = "";
	m_TxRepeat = false;

}

CFileTxDlg::~CFileTxDlg()
{
	delete ui;
}

void CFileTxDlg::Init()
{
	ui->lineEdit->setText(m_TxFilePath);
	ui->frameTxFreqCtrl->SetFrequency(m_TxFrequency);
	ui->checkBoxRepeat->setChecked(m_TxRepeat);
	m_FileReader.open(m_TxFilePath);
//	ui->labelFileInfo->setText(m_FileReader.m_FileInfoStr);
	m_FileReader.close();
}

void CFileTxDlg::done(int r)	//virtual override
{
	QDialog::done(r);	//call base class
}

/////////////////////////////////////////////////////////////////////
// Handle change event for center frequency control
/////////////////////////////////////////////////////////////////////
void CFileTxDlg::OnNewCenterFrequency(qint64 freq)
{
	m_TxFrequency = freq;
}

void CFileTxDlg::on_pushButtonFileSelect_clicked()
{
	QString str = QFileDialog::getOpenFileName(this,tr("Select .wav File Base Name"),m_TxFilePath,tr("wav files (*.wav)"));
	if(str != "")
	{
		m_TxFilePath = str;
		ui->lineEdit->setText(m_TxFilePath);
		m_FileReader.open(m_TxFilePath);
		ui->labelFileInfo->setText(m_FileReader.m_FileInfoStr);
		m_FileReader.close();
	}
}

void CFileTxDlg::on_checkBoxRepeat_clicked(bool checked)
{
	m_TxRepeat = checked;
}

void CFileTxDlg::on_pushButtonStart_clicked()
{
	if(m_FileReader.open(m_TxFilePath) )
	{
		ui->labelFileInfo->setText(m_FileReader.m_FileInfoStr);
		m_FileReader.close();
	}
	else
	{
		qDebug()<<"File read Fail";
	}
}
