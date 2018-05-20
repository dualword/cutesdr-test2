#ifndef DATAMODIFIER_H
#define DATAMODIFIER_H

#include "dsp/datatypes.h"

class CDataModifier
{
public:
	CDataModifier();
	void Init(TYPEREAL SampleRate);
	void ProcessBlock(TYPECPX* pBuf, int NumSamples);
	void SetSweepStart(int start);
	void SetSweepStop(int stop);
	void SetSweepRate( TYPEREAL rate);

private:
	TYPEREAL m_SweepFrequency;
	TYPEREAL m_SweepFreqNorm;
	TYPEREAL m_SweepAcc;
	TYPEREAL m_SweepRateInc;
	TYPEREAL m_SampleRate;
	TYPEREAL m_SweepStartFrequency;
	TYPEREAL m_SweepStopFrequency;
	TYPEREAL m_SweepRate;
	bool m_SweepDirUp;
};

#endif // DATAMODIFIER_H
