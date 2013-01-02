/////////////////////////////////////////////////////////////////////
// catdlg.cpp: implementation of the CCatDlg class.
//
//	This class implements a dialog to setup the CAT parameters
//
// History:
//	2012-12-11  Initial release
/////////////////////////////////////////////////////////////////////

//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
//Copyright 2010 Moe Wheatley. All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are
//permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice, this list of
//	  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice, this list
//	  of conditions and the following disclaimer in the documentation and/or other materials
//	  provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY Moe Wheatley ``AS IS'' AND ANY EXPRESS OR IMPLIED
//WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Moe Wheatley OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//The views and conclusions contained in the software and documentation are those of the
//authors and should not be interpreted as representing official policies, either expressed
//or implied, of Moe Wheatley.
//==========================================================================================
#include "catdlg.h"
#include "ui_catdlg.h"
#include <QDebug>

CCatDlg::CCatDlg(QWidget *parent) :
    QDialog(parent),
	ui(new Ui::CCatDlg)
{
    ui->setupUi(this);

}

CCatDlg::~CCatDlg()
{
    delete ui;
}

//Fill in initial data
void CCatDlg::InitDlg(const QStringList & comList)
{

	for (int i = 0; i < comList.size(); ++i) {
		ui->SerialcomboBox->addItem(comList.at(i), 1);
	}

#if 0
	ui->SerialcomboBox->addItem("COM1", 1);
	ui->SerialcomboBox->addItem("COM2", 2);
	ui->SerialcomboBox->addItem("COM3", 3);
	ui->SerialcomboBox->addItem("COM4", 4);
	ui->SerialcomboBox->addItem("COM5", 5);
	ui->SerialcomboBox->addItem("COM6", 6);
	ui->SerialcomboBox->addItem("COM7", 7);
	ui->SerialcomboBox->addItem("COM8", 8);
	ui->SerialcomboBox->addItem("COM9", 9);
	ui->SerialcomboBox->addItem("COM10", 10);

	int index = ui->fftSizecomboBox->findData(m_FftSize);
	if(index<0)
	{
		index = 1;
		m_FftSize = 4096;
	}
	ui->fftSizecomboBox->setCurrentIndex(index);
	ui->fftAvespinBox->setValue(m_FftAve);
	ui->ClickResolutionspinBox->setValue(m_ClickResolution);
	ui->MaxDisplayRatespinBox->setValue(m_MaxDisplayRate);
	ui->Screen2DSizespinBox->setValue(m_Percent2DScreen);
	ui->checkBoxTestBench->setChecked(m_UseTestBench);
	m_NeedToStop = false;
#endif
}

QString CCatDlg::getComPort()
{
	return ui->SerialcomboBox->currentText();
}



//Called when OK button pressed so get all the dialog data
void CCatDlg::accept()
{
#if 0
	m_FftSize = ui->fftSizecomboBox->itemData(ui->fftSizecomboBox->currentIndex()).toInt();
	int tmpsz = ui->Screen2DSizespinBox->value();
	if( tmpsz!=m_Percent2DScreen)
	{
		m_NeedToStop = true;
		m_Percent2DScreen = tmpsz;
	}
	m_FftAve = ui->fftAvespinBox->value();
	m_ClickResolution = ui->ClickResolutionspinBox->value();
	m_MaxDisplayRate = ui->MaxDisplayRatespinBox->value();
	m_UseTestBench = ui->checkBoxTestBench->isChecked();

#endif
	QDialog::accept();	//need to call base class
}
