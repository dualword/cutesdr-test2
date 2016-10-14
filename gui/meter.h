//////////////////////////////////////////////////////////////////////
// meter.h: interface for the CMeter class.
//
// History:
//	2013-10-02  Initial creation MSW
//	2014-07-11  Added Squelch level position MSW
/////////////////////////////////////////////////////////////////////
#ifndef METER_H
#define METER_H

#include <QtGui>
#include <QFrame>
#include <QImage>
#include "dsp/datatypes.h"

class CMeter : public QFrame
{
    Q_OBJECT
public:
	explicit CMeter(QWidget *parent = 0);
	~CMeter();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void draw();		//call to draw new fft data onto screen plot
	void UpdateOverlay(){DrawOverlay();}
    void SetSquelchPos(TYPEREAL db);		//{ m_SquelchPos = CalcPosFromdB(db);}

signals:

public slots:
    void SetdBmLevel(TYPEREAL dbm, bool Overload);

protected:
		//re-implemented widget event handlers
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent* event);

private:
	void DrawOverlay();
    int CalcPosFromdB(TYPEREAL db);
	QPixmap m_Pixmap;
	QPixmap m_OverlayPixmap;
	QSize m_Size;
	QString m_Str;
	bool m_Overload;
	int m_Slevel;
	int m_SquelchPos;
	int m_dBm;
};

#endif // METER_H
