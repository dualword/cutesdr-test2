/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** Modified by  Moe Wheatley 2015
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include <QDateTime>
#include "wavefilewriter.h"


#define MAX_WAVE_BUF 16384

struct chunk
{
	char        id[4];
	quint32     size;
}__attribute__((gcc_struct,packed));


struct RIFFHeader
{
	chunk       descriptor;     // "RIFF"
	char        type[4];        // "WAVE"
}__attribute__((gcc_struct,packed));


struct WAVEHeader
{
	chunk       descriptor;
	quint16     audioFormat;
	quint16     numChannels;
	quint32     sampleRate;
	quint32     byteRate;
	quint16     blockAlign;
	quint16     bitsPerSample;
	quint16		cbSize;
}__attribute__((gcc_struct,packed));

struct AUXINFO
{	//custom chunk used by Spectravue for additional file information
	chunk       descriptor;
	sSYSTEMTIME StartTime;
	sSYSTEMTIME StopTime;
	quint32 CenterFreq;
	quint32 ADFrequency;
	quint32 IFFrequency;
	quint32 Bandwidth;
	quint32 IQOffset;
	quint32 DBOffset;
	quint32 MaxVal;
	quint32 Unused4;
	quint32 Unused5;
	quint32 Unused6;
}__attribute__((gcc_struct,packed));


struct DATAHeader
{
	chunk       descriptor;
}__attribute__((gcc_struct,packed));


struct CombinedHeader
{
	RIFFHeader  riff;
	WAVEHeader  wave;
	AUXINFO		auxi;
	DATAHeader  data;
}__attribute__((gcc_struct,packed));


///////////////////////////////////////////////////////////////////////////////////////
/// Constructor/Destructor
///////////////////////////////////////////////////////////////////////////////////////
CWaveFileWriter::CWaveFileWriter(QObject *parent)
	: QObject(parent)
{
	m_HeaderLength = sizeof(CombinedHeader);
	qDebug()<<"Header length = "<<m_HeaderLength;
}

CWaveFileWriter::~CWaveFileWriter()
{
}

///////////////////////////////////////////////////////////////////////////////////////
/// Open wave file for writing
/// returns trus if opened ok
///////////////////////////////////////////////////////////////////////////////////////
bool CWaveFileWriter::Open( QString fileName, bool complex, int Rate, bool Data24Bit, qint64 CenterFreq)
{
	if (m_File.isOpen())
		return false; // file already open
	QDateTime datetime = QDateTime::currentDateTimeUtc();
	QString Time = datetime.toString("yyyy-MM-dd_HH-mm-ss");
	fileName.remove(".wav", Qt::CaseInsensitive);
	m_DataLength = 0;
	m_CenterFrequency = CenterFreq;
	if(complex)
		m_Format.setChannelCount(2);
	else
		m_Format.setChannelCount(1);
	if(Data24Bit)
		m_Format.setSampleSize(24);
	else
		m_Format.setSampleSize(16);
	m_Format.setSampleType(QAudioFormat::SignedInt);
	m_Format.setByteOrder(QAudioFormat::LittleEndian);
	m_Format.setSampleRate(Rate);
	m_File.setFileName(fileName+Time+".wav");
	if (!m_File.open(QIODevice::WriteOnly))
		return false; // unable to open file for writing

	if (!WriteHeader(m_Format))
		return false;

   return true;
}

///////////////////////////////////////////////////////////////////////////////////////
/// update wav header and close file
///////////////////////////////////////////////////////////////////////////////////////
void CWaveFileWriter::Close()
{
	if (m_File.isOpen())
	{
		m_Mutex.lock();
		WriteDataLength();
		m_DataLength = 0;
		m_File.close();
		m_Mutex.unlock();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
/// fill in time structure with current UTC time/data
///////////////////////////////////////////////////////////////////////////////////////
void CWaveFileWriter::GetSytemTimeStructure(sSYSTEMTIME& systime)
{
	QDateTime datetime = QDateTime::currentDateTimeUtc();
	systime.wYear = datetime.date().year();
	systime.wMonth = datetime.date().month();
	systime.wDay = datetime.date().day();
	systime.wDayOfWeek = datetime.date().dayOfWeek();
	if(7 == systime.wDayOfWeek)//make 1 to 7 into SYSTIME format of 0 to 6
		systime.wDayOfWeek = 0 ;
	systime.wHour = datetime.time().hour();
	systime.wMinute = datetime.time().minute();
	systime.wSecond = datetime.time().second();
	systime.wMilliseconds = datetime.time().msec();
}

///////////////////////////////////////////////////////////////////////////////////////
/// Write wav header information to file
///////////////////////////////////////////////////////////////////////////////////////
bool CWaveFileWriter::WriteHeader(const QAudioFormat &format)
{
	// check if format is supported
	if (format.byteOrder() == QAudioFormat::BigEndian || m_Format.sampleType() != QAudioFormat::SignedInt)
		return false;

	CombinedHeader header;
	memset(&header, 0, m_HeaderLength);

	sSYSTEMTIME stime;
	GetSytemTimeStructure(stime);
	header.auxi.StartTime = stime;
	header.auxi.StopTime = stime;

	// RIFF header
	memcpy(header.riff.descriptor.id, "RIFF", 4);
	header.riff.descriptor.size = 0; // this will be updated with correct duration:
									 // m_dataLength + HeaderLength - 8
	// WAVE header
	memcpy(header.riff.type, "WAVE", 4);
	memcpy(header.wave.descriptor.id, "fmt ", 4);
	header.wave.descriptor.size = (quint32)sizeof(WAVEHeader)-sizeof(chunk);
	header.wave.audioFormat = 1;
	header.wave.numChannels = (quint16)m_Format.channelCount();
	header.wave.sampleRate = (quint32)m_Format.sampleRate();
	header.wave.byteRate = (quint32)(m_Format.sampleRate() * format.channelCount() * format.sampleSize() / 8);
	header.wave.blockAlign = (quint16)(m_Format.channelCount() * format.sampleSize() / 8);
	header.wave.bitsPerSample = (quint16)m_Format.sampleSize();
	header.wave.cbSize = 0;

	// auxi header
	memcpy(header.auxi.descriptor.id, "auxi", 4);
	header.auxi.descriptor.size = (quint32)sizeof(AUXINFO)-sizeof(chunk);
	header.auxi.ADFrequency = 122880000;
	if(2 == header.wave.numChannels)
		header.auxi.Bandwidth = header.wave.sampleRate;
	else
		header.auxi.Bandwidth = header.wave.sampleRate/2;
	header.auxi.CenterFreq = (quint32)m_CenterFrequency;
	header.auxi.IFFrequency = 0;
	header.auxi.IQOffset = 0;
	if(8 == header.wave.bitsPerSample)
		header.auxi.MaxVal = 127;
	else
		header.auxi.MaxVal = 32767;

	// DATA header
	memcpy(header.data.descriptor.id,"data", 4);
	header.data.descriptor.size = 0; // this will be updated with correct data length: m_dataLength

	return (m_File.write(reinterpret_cast<const char *>(&header), m_HeaderLength) == m_HeaderLength);
}

///////////////////////////////////////////////////////////////////////////////////////
/// Called before closing wav file to write length and stop time into header
///////////////////////////////////////////////////////////////////////////////////////
bool CWaveFileWriter::WriteDataLength()
{
	if (m_File.isSequential())
		return false;

   // seek to RIFF header size, see header.riff.descriptor.size above
	if (!m_File.seek(4))
		return false;

	quint32 length = m_DataLength + m_HeaderLength - 8;
	if (m_File.write(reinterpret_cast<const char *>(&length), 4) != 4)
		return false;

	sSYSTEMTIME StopTime;
	GetSytemTimeStructure(StopTime);

	// seek to aux header stop systime field
	if (!m_File.seek( sizeof(RIFFHeader)+sizeof(WAVEHeader)+sizeof(chunk)+sizeof(sSYSTEMTIME) ) )
		return false;
	if (m_File.write(reinterpret_cast<const char *>(&StopTime), sizeof(sSYSTEMTIME)) != sizeof(sSYSTEMTIME))
		return false;

	// seek to DATA header size
	if (!m_File.seek(sizeof(RIFFHeader)+sizeof(WAVEHeader)+sizeof(AUXINFO)+4))
		return false;
	return m_File.write(reinterpret_cast<const char *>(&m_DataLength), 4) == 4;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Direct Write of byte data into wave file. Up to caller to know data size and
///  number of channels. returns true if writes ok
///////////////////////////////////////////////////////////////////////////////////////
bool CWaveFileWriter::Write(int Length,  qint8* pbuf)
{
	if( 0 == Length)
		return true;
	if (!m_File.isOpen())
		return false; // file not open
	m_Mutex.lock();
	qint64 written = m_File.write((const char *)pbuf, Length );
	m_DataLength += written;
	m_Mutex.unlock();
	return written == Length;
}

