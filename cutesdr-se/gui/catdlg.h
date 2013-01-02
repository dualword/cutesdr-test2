//////////////////////////////////////////////////////////////////////
// catdlg.h: interface for the CCatdlg class.
//
// History:
//	2012-12-11  Initial creation
/////////////////////////////////////////////////////////////////////
#ifndef CATDLG_H
#define CATDLG_H

#include <QDialog>
#include <QtCore>

//#include "gui/mainwindow.h"
//#include <ui_catdlg.h>
//#include "gui/testbench.h"

namespace Ui {
	class CCatDlg;
}

class CCatDlg : public QDialog
{
    Q_OBJECT

public:
	explicit CCatDlg(QWidget *parent = 0);
	~CCatDlg();

	void InitDlg(const QStringList &);
	QString getComPort();

public slots:
	void accept();

private:
	Ui::CCatDlg *ui;

};

#endif // CATDLG_H
