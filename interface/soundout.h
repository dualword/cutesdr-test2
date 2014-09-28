//////////////////////////////////////////////////////////////////////
// soundout.h: interface for the CSoundOut class.
//
// History:
//	2010-09-15  Initial creation MSW
//	2013-01-31  Changed Threading method, removed blocking mode
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
//=============================================================================
#ifndef SOUNDOUT_H
#define SOUNDOUT_H

#include <QList>
#include "threadwrapper.h"
#include <QIODevice>
#include <QAudioOutput>
#include "dsp/fractresampler.h"

#define OUTQSIZE 20000	//max samples
#define SOUND_WRITEBUFSIZE (OUTQSIZE/2)

class CSoundOut : public CThreadWrapper		//subclass a general purpose thread class
{
	Q_OBJECT
public:
	explicit CSoundOut();
	virtual ~CSoundOut();

	//Exposed functions
	void Start(int OutDevIndx, bool StereoOut, TYPEREAL UsrDataRate)
					{emit StartSig(OutDevIndx, StereoOut, UsrDataRate);}
	void Stop(){emit StopSig();}	//stops soundcard output

	void PutOutQueue(int numsamples, TYPEREAL* pData );
	void PutOutQueue(int numsamples, TYPECPX* pData );
	void ChangeUserDataRate(TYPEREAL UsrDataRate);
	void SetVolume(qint32 vol);
	int GetRateError(){return (int)m_PpmError;}

signals:
	void StartSig(int OutDevIndx, bool StereoOut, double UsrDataRate);
	void StopSig();

private slots:
	void ThreadInit();	//overrided function is called by new thread when started
	void ThreadExit();	//overrided function is called by new thread when stopped
	void StartSlot(int OutDevIndx, bool StereoOut, double UsrDataRate);
	void StopSlot();	//stops soundcard output
	void GetNewData();

protected:

private:
	void GetOutQueue(int numsamples, TYPEMONO16* pData );
	void GetOutQueue(int numsamples, TYPESTEREO16* pData );
	void CalcError();

	bool m_Startup;
	bool m_StereoOut;
	bool m_UpdateToggle;
	int m_OutQHead;
	int m_OutQTail;
	int m_RateUpdateCount;
	int m_OutQLevel;
	int m_PpmError;
	TYPEREAL m_Gain;
	TYPEREAL m_UserDataRate;
	TYPEREAL m_OutRatio;
	TYPEREAL m_RateCorrection;
	TYPEREAL m_AveOutQLevel;

	char m_pData[SOUND_WRITEBUFSIZE];
	TYPEMONO16 m_OutQueueMono[OUTQSIZE];
	TYPESTEREO16 m_OutQueueStereo[OUTQSIZE];

	QList<QAudioDeviceInfo> m_OutDevices;
	QAudioDeviceInfo  m_OutDeviceInfo;
	QAudioFormat m_OutAudioFormat;
	QAudioOutput* m_pAudioOutput;
	QIODevice* m_pIODevice; // ptr to internal soundout IODevice
	CFractResampler m_OutResampler;
};
#endif // SOUNDOUT_H
