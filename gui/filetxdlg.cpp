#include "filetxdlg.h"
#include "ui_filetxdlg.h"
#include <QFileDialog>
#include  "interface/wavefilewriter.h"
#include "dsp/datamodifier.h"

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
	ui->labelFileInfo->setText(m_FileReader.m_FileInfoStr);
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
	CWaveFileWriter FileWriter;
	CDataModifier DataModifier;

	if(m_FileReader.open(m_TxFilePath) )
	{
		if( !FileWriter.open( "d:\\testwr.wav",true, m_FileReader.GetSampleRate(), true, 0) )
		{
			m_FileReader.close();
			qDebug()<<"File write open error";
			return;
		}
		int sampleswritten = 0;
		int samplesread = 0;
		DataModifier.Init(m_FileReader.GetSampleRate());
		DataModifier.SetSweepRate(1.0);
		DataModifier.SetSweepStart(-100.0);
		DataModifier.SetSweepStop(100.0);
		while(sampleswritten < m_FileReader.GetNumberSamples())
		{
			//copy in blocks of 512 samples
			samplesread = m_FileReader.GetNextDataBlock( m_TxDataBuf, 512);
			if( samplesread > 0 )
			{
				DataModifier.ProcessBlock(m_TxDataBuf, samplesread);
				if( !FileWriter.Write(m_TxDataBuf, samplesread) )
				{
					m_FileReader.close();
					FileWriter.close();
					qDebug()<<"File copy error";
					m_FileReader.close();
					FileWriter.close();
					return;
				}
				sampleswritten += samplesread;
			}
			else
			{
				if(samplesread < 0 )
					qDebug()<<"File read error";
				else
					qDebug()<<"File operation complete";
				break;
			}
		}
		m_FileReader.close();
		FileWriter.close();
	}
	else
	{
		qDebug()<<"File read open Fail";
	}
}
