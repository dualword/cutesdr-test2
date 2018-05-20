/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
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

#include <qendian.h>
#include <QVector>
#include <QDebug>
#include "wavefilereader.h"

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


struct DATAHeader
{
    chunk       descriptor;
}__attribute__((gcc_struct,packed));


CWaveFileReader::CWaveFileReader(QObject *parent)
	: QFile(parent)
{
	m_FileInfoStr = "No Info";
}

CWaveFileReader::~CWaveFileReader()
{
	close();
}

/////////////////////////////////////////////////////////////////////
/// \brief CWaveFileReader::open and read wave header
/// \param fileName path
/// \return true if file opens and is correct wave format
/////////////////////////////////////////////////////////////////////
bool CWaveFileReader::open(const QString &fileName)
{
	close();
	setFileName(fileName);
	return QFile::open(QIODevice::ReadOnly) && readHeader();
}

/////////////////////////////////////////////////////////////////////
/// \brief CWaveFileReader::readHeader information
/// \return true if header read ok
/////////////////////////////////////////////////////////////////////
bool CWaveFileReader::readHeader()
{
	bool ret = false;
	seek(0);
	RIFFHeader riffheader;
	memset(reinterpret_cast<char *>(&m_AuxiSubChunk), 0, sizeof(sAuxiSubChunk));
	memset(reinterpret_cast<char *>(&m_FmtSubChunk), 0, sizeof(sFmtSubChunk));
	m_FileInfoStr = "No Info";
	bool result = read(reinterpret_cast<char *>(&riffheader), sizeof(RIFFHeader)) == sizeof(RIFFHeader);
	if(result )
	{
		if(	('R'==riffheader.descriptor.id[0]) &&
			('I'==riffheader.descriptor.id[1]) &&
			('F'==riffheader.descriptor.id[2]) &&
			('F'==riffheader.descriptor.id[3]) &&
			('W' == riffheader.type[0]) &&
			('A' == riffheader.type[1]) &&
			('V' == riffheader.type[2]) &&
			('E' == riffheader.type[3]) )
		{
			ret = true;
			seek(0);
			qint64 bytesread = read((char*)m_HeaderBuffer, MAX_HEADER);
			if(bytesread>0)
			{
				quint32 Start;
				quint32 Length;
				if( FindSubChunk("fmt ", &Start, &Length) )
					memcpy(reinterpret_cast<char *>(&m_FmtSubChunk), &m_HeaderBuffer[Start],Length);
				else
					ret = false;
				if( FindSubChunk("auxi", &Start, &Length) )
					memcpy(reinterpret_cast<char *>(&m_AuxiSubChunk), &m_HeaderBuffer[Start],Length);
				else
					ret = false;
				if( FindSubChunk("data", &Start, &Length) )
				{
					m_NumSamples = Length/(m_FmtSubChunk.bitsPerSample/8);
					seek(Start);	//move file pointer to start of data
				}
				else
				{
					ret = false;
				}
			}
		}
	}
	if(ret)
	{
		char Tbuf[256];
		if( 2 == m_FmtSubChunk.numChannels)
			strcpy(Tbuf, "Complex");
		else
			strcpy(Tbuf, "Real");
		m_FileInfoStr.sprintf("%d Samples of %s Data\nSampleRate = %d  Bits/Sample= %d", m_NumSamples, Tbuf, (int)m_FmtSubChunk.sampleRate, (int)m_FmtSubChunk.bitsPerSample );
	}
	else
	{
		m_FileInfoStr = "Invalid RIFF Wave File";
		qDebug()<<m_FileInfoStr;
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief CWaveFileReader::FindSubChunk
/// \param Id passed 4 character string of subchunk to find in m_HeaderBuffer
/// \param pStart  returned starting offset of chunk data from start of file header buffer
/// \param pLength returned length of subchunk data
/// \return true if sub chunk found
///////////////////////////////////////////////////////////////////////////////
bool CWaveFileReader::FindSubChunk(const char*Id, quint32* pStart, quint32* pLength)
{
	if(!pStart || !pLength)
		return false;
	bool found = false;
	*pStart = 0;
	*pLength = 0;
	for(int i=0; i<(MAX_HEADER-5); i++)
	{	//search all locations for 4 character substring ID
		if( (m_HeaderBuffer[i+0] == Id[0]) &&
			(m_HeaderBuffer[i+1] == Id[1]) &&
			(m_HeaderBuffer[i+2] == Id[2]) &&
			(m_HeaderBuffer[i+3] == Id[3]) )
		{
			found = true;
			*pStart = i+8;	//don't include header and length
			*pLength = m_HeaderBuffer[i+7]; *pLength <<= 8;
			*pLength += m_HeaderBuffer[i+6]; *pLength <<= 8;
			*pLength += m_HeaderBuffer[i+5]; *pLength <<= 8;
			*pLength += m_HeaderBuffer[i+4];
qDebug()<<Id <<"SubChunk Found"<<*pStart << *pLength;
			break;
		}
	}
	return found;
}

/////////////////////////////////////////////////////////////////////////
/// \brief CWaveFileReader::GetNextDataBlock
/// \param pData   pointer to callers complex float data buffer
/// \param NumSamples
/// \return number of sample read, 0 if reached end of file or -1 error
/////////////////////////////////////////////////////////////////////////
int CWaveFileReader::GetNextDataBlock(TYPECPX* pData, int NumSamples)
{
int ByteLength = 0;
int i;
int j = -1;
	if(m_FmtSubChunk.numChannels != 2)
	{
		qDebug()<<"Not 2 channels";
		return -1;
	}
	if(24 == m_FmtSubChunk.bitsPerSample)
	{
		tBtoL data;
		ByteLength = NumSamples*6;
		if(ByteLength < MAX_RDDATABLK)
		{
			data.all = 0;
			qint64 bytesread = read((char*)m_DataBuffer, ByteLength);
			if(bytesread <= 0)
			{
				return bytesread;
			}
			for( i=0,j=0; i<bytesread; i+=6,j++)
			{
				TYPECPX cpxtmp;
				data.bytes.b1 = m_DataBuffer[i];		//combine 3 bytes into 32 bit signed int
				data.bytes.b2 = m_DataBuffer[i+1];
				data.bytes.b3 = m_DataBuffer[i+2];
				cpxtmp.re = (TYPEREAL)data.all/2147483647.0;
				data.bytes.b1 = m_DataBuffer[i+3];		//combine 3 bytes into 32 bit signed int
				data.bytes.b2 = m_DataBuffer[i+4];
				data.bytes.b3 = m_DataBuffer[i+5];
				cpxtmp.im = (TYPEREAL)data.all/2147483647.0;
				pData[j] = cpxtmp;
			}
		}
	}
	else
	{	//here if samples are 16 bits
		tBtoS data;
		ByteLength = NumSamples*4;
		if(ByteLength < MAX_RDDATABLK)
		{
			data.all = 0;
			qint64 bytesread = read((char*)m_DataBuffer, ByteLength);
			if(bytesread <= 0)
				return 0;
			for( int i=0,j=0; i<bytesread; i+=4,j++)
			{
				TYPECPX cpxtmp;
				data.bytes.b0 = m_DataBuffer[i];		//combine 3 bytes into 32 bit signed int
				data.bytes.b1 = m_DataBuffer[i+1];
				cpxtmp.re = (TYPEREAL)data.sall/65535.0;
				data.bytes.b0 = m_DataBuffer[i+2];		//combine 3 bytes into 32 bit signed int
				data.bytes.b1 = m_DataBuffer[i+3];
				cpxtmp.im = (TYPEREAL)data.sall/65535.0;
				pData[j] = cpxtmp;
			}
		}
	}
	return j;
}

/////////////////////////////////////////////////////////////////////////
/// \brief CWaveFileReader::GetNextDataBlock
/// \param pData   pointer to callers real float data buffer
/// \param NumSamples
/// \return number of sample read, -1 if reached end of file or error
/////////////////////////////////////////////////////////////////////////
int CWaveFileReader::GetNextDataBlock(TYPEREAL* pData, int NumSamples)
{
	if(m_FmtSubChunk.numChannels != 1)
	{
		qDebug()<<"Not 2 channels";
		return -1;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////
/// \brief CWaveFileReader::GetNextDataBlock
/// \param pData   pointer to callers complex 16 bit data buffer
/// \param NumSamples
/// \return number of sample read, -1 if reached end of file or error
/////////////////////////////////////////////////////////////////////////
int CWaveFileReader::GetNextDataBlock(tStereo16* pData, int NumSamples)
{
	if(m_FmtSubChunk.numChannels != 2)
	{
		qDebug()<<"Not 2 channels";
		return -1;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////
/// \brief CWaveFileReader::GetNextDataBlock
/// \param pData   pointer to callers real 16bit data buffer
/// \param NumSamples
/// \return number of sample read, -1 if reached end of file or error
/////////////////////////////////////////////////////////////////////////
int CWaveFileReader::GetNextDataBlock(qint16* pData, int NumSamples)
{
	if(m_FmtSubChunk.numChannels != 1)
	{
		qDebug()<<"Not 2 channels";
		return -1;
	}
	return -1;
}



