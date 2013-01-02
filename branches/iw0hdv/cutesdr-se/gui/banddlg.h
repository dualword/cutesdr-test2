/////////////////////////////////////////////////////////////////////
#ifndef BANDDLG_H
#define BANDDLG_H

#include <QDialog>
#include "gui/band.h"


namespace Ui {
    class CBandDlg;
}

class CBandDlg : public QDialog
{
    Q_OBJECT

public:
	explicit CBandDlg(QWidget *parent);
    ~CBandDlg();

	void InitDlg();

public slots:
	void OnBandChange(int row);

private:
	void UpdateBandInfo();
	Ui::CBandDlg *ui;
};

#endif // BANDDLG_H
