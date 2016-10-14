/////////////////////////////////////////////////////////////////////
// soundout.cpp: implementation of the CSoundOut class.
//
//	This class implements a class to output data to a soundcard.
// A fractional resampler is used to convert the users input rate to
// the sound card rate and also perform frequency lock between the
// two clock domains.
// Soundcard object runs in its own thread using thread event loop
//for update notifications.
// History:
//	2010-09-15  Initial creation MSW
//	2013-01-31  Changed Threading method, removed blocking mode
//	2015-10-26  Changed OutQueue routines to return number of resampled data
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
#include "interface/soundout.h"
#include "gui/testbench.h"
#include "interface/sdrinterface.h"
#include <QDebug>
#include <math.h>

#define SOUNDCARD_RATE 48000	//output soundcard sample rate
//#define SOUNDCARD_RATE 44100

#define FILTERQLEVEL_ALPHA 0.001
#define P_GAIN 2.38e-7		//Proportional gain

#define TEST_ERROR 1.0
//#define TEST_ERROR 1.001	//use to force fixed sample rate error for testing
//#define TEST_ERROR 0.999

/////////////////////////////////////////////////////////////////////
//   constructor/destructor
/////////////////////////////////////////////////////////////////////
CSoundOut::CSoundOut()
{
	m_pAudioOutput = NULL;
	m_pIODevice = NULL;
	m_UserDataRate = SOUNDCARD_RATE;
	m_OutRatio = 1.0;
	m_OutAudioFormat.setSampleRate(SOUNDCARD_RATE);
	m_OutResampler.Init(8192);
	m_RateCorrection = 0.0;
	m_Gain = 1.0;
	m_Startup = true;
//qDebug()<<"GUI Thread "<<this->thread()->currentThread();
}

CSoundOut::~CSoundOut()
{
qDebug()<<"CSoundOut destructor";
	CleanupThread();	//tell thread to cleanup after itself by calling ThreadExit()
}

//////////////////////////////////////////////////////////////////////////
//Called when worker thread starts to initialize things
//////////////////////////////////////////////////////////////////////////
void CSoundOut::ThreadInit()	//overrided funciton is called by new thread when started
{
	//use event loop to service external calls.
    connect(this,SIGNAL( StartSig(int, bool, double) ), this, SLOT(StartSlot(int, bool, double) ) );
	connect(this,SIGNAL( StopSig()), this, SLOT(StopSlot()) );
//qDebug()<<"Soundout Thread Init "<<this->thread()->currentThread();
}

/////////////////////////////////////////////////////////////////////
// Called by this worker thread to cleanup after itself
/////////////////////////////////////////////////////////////////////
void CSoundOut::ThreadExit()
{
    StopSlot();
}

/////////////////////////////////////////////////////////////////////
// Starts up soundcard output thread using soundcard at list OutDevIndx
/////////////////////////////////////////////////////////////////////
void CSoundOut::StartSlot(int OutDevIndx, bool StereoOut, double UsrDataRate)
{
QAudioDeviceInfo  DeviceInfo;
qDebug()<<"Soundout Thread "<<this->thread()->currentThread();
    m_pThread->setPriority(QThread::HighestPriority);
	m_StereoOut = StereoOut;
	//Get required soundcard from list
	m_OutDevices = DeviceInfo.availableDevices(QAudio::AudioOutput);
	m_OutDeviceInfo = m_OutDevices.at(OutDevIndx);

	//Setup fixed format for sound ouput
	m_OutAudioFormat.setCodec("audio/pcm");
	m_OutAudioFormat.setSampleRate(SOUNDCARD_RATE);
	m_OutAudioFormat.setSampleSize(16);
	m_OutAudioFormat.setSampleType(QAudioFormat::SignedInt);
	m_OutAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
	if(m_StereoOut)
		m_OutAudioFormat.setChannelCount(2);
	else
		m_OutAudioFormat.setChannelCount(1);

    if(m_pAudioOutput)
    {
        delete m_pAudioOutput;
        m_pAudioOutput = NULL;
    }
    m_pAudioOutput = new QAudioOutput(m_OutDeviceInfo, m_OutAudioFormat, this);
	if(!m_pAudioOutput)
	{
		qDebug()<<"Soundcard output error";
		return;
	}
	if(QAudio::NoError == m_pAudioOutput->error() )
	{
		//initialize the data queue variables
		m_UserDataRate = 1;	//force user data rate to be changed
		ChangeUserDataRate(UsrDataRate);
		//operate in mode where notify() slot is called periodically to
		// see how much data can be sent to soundcard output
		//connect notify signal to get more soundcard output data
		m_pAudioOutput->setBufferSize(SOUND_WRITEBUFSIZE*2);
		//connect notify event that new output data is needed
		connect(m_pAudioOutput,SIGNAL(notify()), this, SLOT(GetNewData()));
		m_pIODevice = m_pAudioOutput->start(); //start Qt AudioOutput, get pointer to its QIODevice
		m_pAudioOutput->setNotifyInterval(20);	//20mSec update interval
		GetNewData();	//fill buffer initially with zeros
        qDebug()<<"Soundcard output Opened ok";
        return;
	}
	else
	{
		qDebug()<<"Soundcard output error";
		return;
	}
}

/////////////////////////////////////////////////////////////////////
// Closes down sound card output thread
/////////////////////////////////////////////////////////////////////
void CSoundOut::StopSlot()
{
	if(m_pAudioOutput)
	{
		if( ( QAudio::ActiveState==m_pAudioOutput->state() )
				|| (QAudio::IdleState==m_pAudioOutput->state()) )
			m_pAudioOutput->stop();
	}

qDebug()<<"Soundcard output stopped";
}

/////////////////////////////////////////////////////////////////////
// Sets/changes user data input rate
/////////////////////////////////////////////////////////////////////
void CSoundOut::ChangeUserDataRate(TYPEREAL UsrDataRate)
{
	if(m_UserDataRate != UsrDataRate)
	{
		m_UserDataRate = UsrDataRate;
		for(int i=0; i<OUTQSIZE ;i++)	//zero buffer for data output
		{
			m_OutQueueMono[i] = 0;
			m_OutQueueStereo[i].re = 0;
			m_OutQueueStereo[i].im = 0;
		}
		m_OutRatio = m_UserDataRate/m_OutAudioFormat.sampleRate();
		m_OutQHead = 0;
		m_OutQTail = 0;
		m_OutQLevel = 0;
		m_AveOutQLevel = OUTQSIZE/2;
		m_Startup = true;
	}
//qDebug()<<"SoundOutRatio  Rate"<<(1.0/m_OutRatio) << m_UserDataRate;
}

/////////////////////////////////////////////////////////////////////
// Sets/changes volume control gain  0 <= vol <= 99
//range scales to attenuation(gain) of -50dB to 0dB
/////////////////////////////////////////////////////////////////////
void CSoundOut::SetVolume(qint32 vol)
{
	m_Mutex.lock();
	if(0==vol)	//if zero make infinite attenuation
		m_Gain = 0.0;
	else if(vol<=99)
		m_Gain = MPOW(10.0, ((TYPEREAL)vol-99.0)/39.2 );
	m_Mutex.unlock();
//qDebug()<<"Volume "<<vol << m_Gain;
}

/////////////////////////////////////////////////////////////////////
//Slot called by thread every "notify()" interval
//to write any new data into soundcard
/////////////////////////////////////////////////////////////////////
void CSoundOut::GetNewData()
{
	if(!m_pAudioOutput)
		return;
	if( (QAudio::IdleState == m_pAudioOutput->state() ) ||
		(QAudio::ActiveState == m_pAudioOutput->state() ) )
	{	//Process sound data while soundcard is active and no errors
		int len =  m_pAudioOutput->bytesFree();	//in bytes
		if( len>0 )
		{	//limit size to SOUND_WRITEBUFSIZE
			if(len > SOUND_WRITEBUFSIZE)
				len = SOUND_WRITEBUFSIZE;
			if(m_StereoOut)
			{
				len &= ~(0x03);	//keep on 4 byte chunks
				GetOutQueue( len/4, (TYPESTEREO16*)m_pData );
			}
			else
			{
				len &= ~(0x01);	//keep on 2 byte chunks
				GetOutQueue( len/2, (TYPEMONO16*)m_pData );
			}
			m_pIODevice->write((char*)m_pData,len);
		}
	}
	else
	{	//bail out if error occurs
		emit StopSig();
		qDebug()<<"SoundOut Error";
	}
}

////////////////////////////////////////////////////////////////
//Called by application to put COMPLEX input into
// STEREO 2 channel soundcard output queue
// Returns number of resampled samples sent to sound card
////////////////////////////////////////////////////////////////
int CSoundOut::PutOutQueue(int numsamples, TYPECPX* pData )
{
int NumResamples;
	if( 0==numsamples )
		return 0;

	//Call Resampler to match sample rates between radio and sound card
	NumResamples = m_OutResampler.Resample(numsamples, TEST_ERROR*m_OutRatio *(1.0+m_RateCorrection),
										 pData, m_RData, m_Gain);

g_pTestBench->DisplayData(NumResamples, 1.0, m_RData, SOUNDCARD_RATE, PROFILE_5);

	m_Mutex.lock();
	for( int i=0; i<NumResamples; i++)
	{
		m_OutQueueStereo[m_OutQHead++] = m_RData[i];
		if(m_OutQHead >= OUTQSIZE)
			m_OutQHead = 0;
		m_OutQLevel++;
		if(m_OutQHead==m_OutQTail)	//if full
		{	//remove 1/2 a queue's worth of data
			m_OutQLevel = OUTQSIZE/2;
			m_AveOutQLevel = m_OutQLevel;
			m_OutQTail += (OUTQSIZE/2);
			if(m_OutQTail >= OUTQSIZE)
				m_OutQTail = m_OutQTail - OUTQSIZE;
			m_AveOutQLevel = m_OutQLevel;
			qDebug()<<"Snd Overflow";
			g_pTestBench->SendDebugTxt("Snd Overflow");
			i = numsamples;		//force break out of for loop
		}
	}
	//calculate average Queue fill level
	m_AveOutQLevel = (1.0-FILTERQLEVEL_ALPHA)*m_AveOutQLevel + FILTERQLEVEL_ALPHA*(TYPEREAL)m_OutQLevel;
	m_Mutex.unlock();
	return NumResamples;
}

////////////////////////////////////////////////////////////////
//Called by application to put REAL soundcard output samples
//into MONO soundcard queue
// Returns number of resampled samples sent to sound card
////////////////////////////////////////////////////////////////
int CSoundOut::PutOutQueue(int numsamples, TYPEREAL* pData )
{
int NumResamples;
	if( 0==numsamples )
		return 0;

	//Call Resampler to match sample rates between radio and sound card
	NumResamples = m_OutResampler.Resample(numsamples, TEST_ERROR*m_OutRatio *(1.0+m_RateCorrection),
										 pData, (TYPEMONO16*)m_RData, m_Gain);

g_pTestBench->DisplayData(NumResamples, 1.0, (TYPEMONO16*)m_RData, SOUNDCARD_RATE, PROFILE_5);

	m_Mutex.lock();
	for( int i=0; i<NumResamples; i++)
	{
		m_OutQueueMono[m_OutQHead++] = ((TYPEMONO16*)m_RData)[i];
		if(m_OutQHead >= OUTQSIZE)
			m_OutQHead = 0;
		m_OutQLevel++;
		if(m_OutQHead==m_OutQTail)	//if full
		{	//remove 1/2 a queue's worth of data
			m_OutQLevel = OUTQSIZE/2;
			m_AveOutQLevel = m_OutQLevel;
			m_OutQTail += (OUTQSIZE/2);
			if(m_OutQTail >= OUTQSIZE)
				m_OutQTail = m_OutQTail - OUTQSIZE;
			m_AveOutQLevel = m_OutQLevel;
			qDebug()<<"Snd Overflow";
			g_pTestBench->SendDebugTxt("Snd Overflow");
			i = numsamples;		//force break out of for loop
		}
	}
	//calculate average Queue fill level
	m_AveOutQLevel = (1.0-FILTERQLEVEL_ALPHA)*m_AveOutQLevel + FILTERQLEVEL_ALPHA*(TYPEREAL)m_OutQLevel;
	m_Mutex.unlock();
	return NumResamples;
}

////////////////////////////////////////////////////////////////
//Called by CSoundOut worker thread to get new samples from queue
// This routine is called from a worker thread so must be careful.
//   MONO version
////////////////////////////////////////////////////////////////
void CSoundOut::GetOutQueue(int numsamples, TYPEMONO16* pData )
{
int i;
	m_Mutex.lock();
	if(m_Startup)
	{	//if no data in queue yet just stuff in silence until something is put in queue
		for( i=0; i<numsamples; i++)
			pData[i] = 0;
		if(m_OutQLevel>OUTQSIZE/2)
		{
			m_Startup = false;
			m_RateUpdateCount = -5*SOUNDCARD_RATE;	//delay first error update to let settle
			m_PpmError = 0;
			m_AveOutQLevel = m_OutQLevel;
			m_UpdateToggle = true;
			}
		else
		{
			m_Mutex.unlock();
			return;
		}
	}

	for( i=0; i<numsamples; i++)
	{
		if(m_OutQHead!=m_OutQTail)
		{
			pData[i] = m_OutQueueMono[m_OutQTail++];
			if(m_OutQTail >= OUTQSIZE)
				m_OutQTail = 0;
			if(m_OutQLevel>0)
				m_OutQLevel--;
		}
		else	//queue went empty
		{	//backup queue ptr and use previous data in queue
			m_OutQTail -= (OUTQSIZE/2);
			if(m_OutQTail < 0)
				m_OutQTail = m_OutQTail + OUTQSIZE;
			pData[i] = m_OutQueueMono[m_OutQTail];
			if(m_OutQTail >= OUTQSIZE)
				m_OutQTail = 0;
			m_OutQLevel = OUTQSIZE/2;
			m_AveOutQLevel = m_OutQLevel;
			qDebug()<<"Snd Underflow";
			g_pTestBench->SendDebugTxt("Snd Underflow");
		}
	}

	//calculate average Queue fill level
	m_AveOutQLevel = (1.0-FILTERQLEVEL_ALPHA)*m_AveOutQLevel + FILTERQLEVEL_ALPHA*m_OutQLevel;

	// See if time to update rate error calculation routine
	m_RateUpdateCount += numsamples;
	if(m_RateUpdateCount >= SOUNDCARD_RATE)	//every second
	{
		CalcError();
		m_RateUpdateCount = 0;
	}
	m_Mutex.unlock();
}

////////////////////////////////////////////////////////////////
//Called by CSoundOut worker thread to get new samples from queue
// This routine is called from a worker thread so must be careful.
//   STEREO version
////////////////////////////////////////////////////////////////
void CSoundOut::GetOutQueue(int numsamples, TYPESTEREO16* pData )
{
int i;
	m_Mutex.lock();
	if(m_Startup)
	{	//if no data in queue yet just stuff in silence until something is put in queue
		for( i=0; i<numsamples; i++)
		{
			pData[i].re = 0;
			pData[i].im = 0;
		}
		if(m_OutQLevel>OUTQSIZE/2)
		{
			m_Startup = false;
			m_RateUpdateCount = -5*SOUNDCARD_RATE;	//delay first error update to let settle
			m_PpmError = 0;
			m_AveOutQLevel = m_OutQLevel;
			m_UpdateToggle = true;
			}
		else
		{
			m_Mutex.unlock();
			return;
		}
	}

	for( i=0; i<numsamples; i++)
	{
		if(m_OutQHead!=m_OutQTail)
		{
			pData[i] = m_OutQueueStereo[m_OutQTail++];
			if(m_OutQTail >= OUTQSIZE)
				m_OutQTail = 0;
			if(m_OutQLevel>0)
				m_OutQLevel--;
		}
		else	//queue went empty
		{	//backup queue ptr and use previous data in queue
			m_OutQTail -= (OUTQSIZE/2);
			if(m_OutQTail < 0)
				m_OutQTail = m_OutQTail + OUTQSIZE;
			pData[i] = m_OutQueueStereo[m_OutQTail++];
			if(m_OutQTail >= OUTQSIZE)
				m_OutQTail = 0;
			m_OutQLevel = OUTQSIZE/2;
			m_AveOutQLevel = m_OutQLevel;
			qDebug()<<"Snd Underflow";
			g_pTestBench->SendDebugTxt("Snd Underflow");
		}
	}
	//calculate average Queue fill level
	m_AveOutQLevel = (1.0-FILTERQLEVEL_ALPHA)*m_AveOutQLevel + FILTERQLEVEL_ALPHA*m_OutQLevel;

	// See if time to update rate error calculation routine
	m_RateUpdateCount += numsamples;
	if(m_RateUpdateCount >= SOUNDCARD_RATE)	//every second
	{
		CalcError();
		m_RateUpdateCount = 0;
	}
	m_Mutex.unlock();
}

////////////////////////////////////////////////////////////////
// Called from the Get routines to update the
// error correction process
////////////////////////////////////////////////////////////////
void CSoundOut::CalcError()
{
TYPEREAL error;
	error = (TYPEREAL)(m_AveOutQLevel - OUTQSIZE/2 );	//neg==level is too low  pos == level is to high
	error = error * P_GAIN;
	m_RateCorrection = error;
	m_PpmError = (int)( m_RateCorrection*1e6 );
	if( abs(m_PpmError) > 500)
	{
//		qDebug()<<"SoundOut "<<m_PpmError << m_AveOutQLevel;
//		g_pTestBench->SendDebugTxt("Snd error>500ppm");
	}
}
