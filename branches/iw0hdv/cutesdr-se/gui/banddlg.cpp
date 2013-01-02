#include "gui/banddlg.h"
#include "ui_banddlg.h"
#include "gui/band.h"
#include <QTableWidget>
#include <QDebug>

CBandDlg::CBandDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CBandDlg)
{
	ui->setupUi(this);
}

//Fill in initial data
void CBandDlg::InitDlg()
{
	for (int i=0; i<ombt.getNb(); ++i) {
		ui->bandTableWidget->setItem ( i, 0, new QTableWidgetItem(QString (ombt.getName(i)))              );
		ui->bandTableWidget->setItem ( i, 1, new QTableWidgetItem(QString ("%1").arg(ombt.getFLow(i)))    );
		ui->bandTableWidget->setItem ( i, 2, new QTableWidgetItem(QString ("%1").arg(ombt.getFHigh(i)))   );
		ui->bandTableWidget->setItem ( i, 3, new QTableWidgetItem(QString ("%1").arg(ombt.getFCenter(i))) );
		ui->bandTableWidget->setItem ( i, 4, new QTableWidgetItem(QString ("%1").arg(ombt.getFLow(i)+(ombt.getFHigh(i)-ombt.getFLow(i))/2)) );
	}
}

void CBandDlg::OnBandChange(int row)
{
	// reset the selection on the table
	ui->bandTableWidget->setRangeSelected (	QTableWidgetSelectionRange ( 
												0, 0, 
												ui->bandTableWidget->rowCount()-1, 
												ui->bandTableWidget->columnCount()-1 ), 
											false );
	if (row >= 0) {
		// select the new band
		ui->bandTableWidget->setRangeSelected (	QTableWidgetSelectionRange ( row, 0, row, ui->bandTableWidget->columnCount()-1 ), true );
	}
}

CBandDlg::~CBandDlg()
{
    delete ui;
}
