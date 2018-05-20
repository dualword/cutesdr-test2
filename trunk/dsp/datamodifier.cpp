#include "datamodifier.h"
#include <QDebug>


CDataModifier::CDataModifier()
{
	m_SampleRate = 1;
	m_SweepRate = 1;
	m_SweepStartFrequency = 1;
	m_SweepStopFrequency = 1;

}

void CDataModifier::Init(TYPEREAL SampleRate)
{
int i;
	m_SampleRate = SampleRate;
	//initialize sweep generator values
	m_SweepFrequency = m_SweepStartFrequency;
	m_SweepFreqNorm = K_2PI/m_SampleRate;
	m_SweepAcc = 0.0;
	m_SweepRateInc = m_SweepRate/m_SampleRate;
	m_SweepDirUp = true;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void CDataModifier::SetSweepStart(int start)
{
	m_SweepStartFrequency = (TYPEREAL)start;
	m_SweepFrequency = m_SweepStartFrequency;
	m_SweepAcc = 0.0;
}

void CDataModifier::SetSweepStop(int stop)
{
	m_SweepStopFrequency = (TYPEREAL)stop;
	m_SweepFrequency = m_SweepStartFrequency;
	m_SweepAcc = 0.0;
}

void CDataModifier::SetSweepRate(TYPEREAL rate)
{
	m_SweepRate = rate; // Hz/sec
	m_SweepAcc = 0.0;
	m_SweepRateInc = m_SweepRate/m_SampleRate;
}


void CDataModifier::ProcessBlock(TYPECPX* pBuf, int NumSamples)
{
	TYPECPX dtmp;
	TYPECPX Osc;
	for(int i=0; i<NumSamples; i++)
	{
		//create complex sin/cos modulation signal
		Osc.re = 0.5*MCOS(m_SweepAcc);
		Osc.im = 0.5*MSIN(m_SweepAcc);
		//inc phase accummulator with normalized freqeuency step
		m_SweepAcc += ( m_SweepFrequency*m_SweepFreqNorm );
		if(	m_SweepDirUp )
		{
			m_SweepFrequency += m_SweepRateInc;	//inc sweep frequency
			if(m_SweepFrequency >= m_SweepStopFrequency)	//reached end of sweep?
				m_SweepDirUp = false;
		}
		else
		{
			m_SweepFrequency -= m_SweepRateInc;	//dec sweep frequency
			if(m_SweepFrequency <= m_SweepStartFrequency)	//reached start of sweep?
				m_SweepDirUp = true;
		}
		//Cpx multiply by shift frequency
		dtmp = pBuf[i];
		pBuf[i].re = ((dtmp.re * Osc.re) - (dtmp.im * Osc.im));
		pBuf[i].im = ((dtmp.re * Osc.im) + (dtmp.im * Osc.re));
	}
	m_SweepAcc = (TYPEREAL)MFMOD((TYPEREAL)m_SweepAcc, K_2PI);	//keep radian counter bounded
}
