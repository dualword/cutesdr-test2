/////////////////////////////////////////////////////////////////////
// Handle plot mouse change event for demod frequency
/////////////////////////////////////////////////////////////////////
#ifndef BAND_H
#define BAND_H

#include <QObject>
#include <QtCore>

struct OMBand {
	const char *name;
	const unsigned int fLow;
	const unsigned int fHigh;
	unsigned int fCenter;
};

class OMBandTable : public QObject
{
Q_OBJECT
public:
	OMBandTable ();

	int onFreqChange (unsigned fCurrent, unsigned fNew);
	int band (unsigned f);
	int onFreqChange (unsigned fCurCenter, unsigned fCurrent, unsigned fNew);
	int getNb () { return nb; }

	const char *getName (int row) { return t[row].name ; } 
	unsigned int getFLow (int row) { return t[row].fLow ; } 
	unsigned int getFHigh (int row) { return t[row].fHigh ; } 
	unsigned int getFCenter (int row) { return t[row].fCenter ; } 

signals:
 
	void NewBand (int index);

private:
	const int nb;
	static struct OMBand t[];
};
#endif

extern OMBandTable ombt;
