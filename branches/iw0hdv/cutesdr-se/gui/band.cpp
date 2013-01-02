#include <QDebug>
#include "band.h"



/*
	160	1.8 - 2.0		night
	80	3.5 - 4.0		night and local day
	40	7.0 - 7.3		night and local day
	30	10.1 - 10.15	CW and digital
	20	14.0 - 14.350	world wide day and night
	17	18.068 - 18.168	world wide day and night
	15	21.0 - 21.450	primarily a daytime band
	12	24.890 - 24.990	primarily a daytime band
	10	28.0 - 29.70	daytime during sunspot highs
*/

#define MID(L,U) ((L+U)/2) 

struct OMBand
OMBandTable::t[] = {
	{ "160M",	1800000,	2000000,	0 },
	{ "80M",	3500000,	4000000,	0 },
	{ "40M",	7000000,	7300000,	0 },
	{ "30M",	10100000,	10150000,	0 },
	{ "20M",	14000000,	14350000,	0 },
	{ "17M",    18068000,	18168000,	0 },
	{ "15M",    21000000,	21450000,	0 },
	{ "12M",	24890000,	24990000,	0 },
	{ "10M",	28000000,	29700000,	0 },
	{ "6M",		50000000,	54000000,	0 },
};

OMBandTable::OMBandTable (): nb(sizeof(t)/sizeof(t[0]))
{
	for (int i=0; i< nb; ++i) {
		t[i].fCenter = MID(t[i].fLow,t[i].fHigh);
		qDebug() << t[i].name << t[i].fLow << t[i].fHigh << t[i].fCenter ;
	}
}

int OMBandTable :: band (unsigned f)
{
	for (int i=0; i < nb; ++i) {
		if ( ( t[i].fLow <= f ) && ( f <= t[i].fHigh ) )
			return i;
	}
	return -1;
}

int OMBandTable :: onFreqChange (unsigned fCurCenter, unsigned fCurrent, unsigned fNew)
{
	int oldB = band (fCurrent);
	int newB = band (fNew);

	qDebug() << "***** onFreq Change: old band " << oldB << " new band " << newB ;

	if (oldB != -1 && newB != -1 && oldB != newB) {
		t[oldB].fCenter = fCurCenter;
		emit NewBand (newB);
		if (t[newB].fCenter) 
			return t[newB].fCenter;
		else
			return -1;
	}
	if (oldB == -1 && oldB != newB) {
		t[oldB].fCenter = fCurCenter;
		return -1;
	}
	return -1;
}
	
OMBandTable ombt;


