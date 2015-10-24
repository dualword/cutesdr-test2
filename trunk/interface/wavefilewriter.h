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

#ifndef WAVEFILEWRITER_H
#define WAVEFILEWRITER_H

#include <QObject>
#include <QFile>
#include <QAudioFormat>
#include "dsp/datatypes.h"


typedef union
{
	struct ss1
	{
		quint8 b0;
		quint8 b1;
		quint8 b2;
		quint8 b3;
	}bytes;
	qint32 all;
}tiTemp;

typedef union
{
	struct ss2
	{
		quint8 lsb;
		quint8 msb;
	}bytes;
	qint16 both;
}tsTemp;

struct sSYSTEMTIME
{
	quint16 wYear;
	quint16 wMonth;
	quint16 wDayOfWeek;
	quint16 wDay;
	quint16 wHour;
	quint16 wMinute;
	quint16 wSecond;
	quint16 wMilliseconds;
};

class CWaveFileWriter : public QObject
{
	Q_OBJECT
public:
	explicit CWaveFileWriter(QObject *parent = 0);
	~CWaveFileWriter();
	bool Open( QString fileName, bool complex, int Rate, bool Data24Bit, qint64 CenterFreq);
	void Close();
	bool Write( int NumSamples, qint16* buffer);
	bool Write( int NumSamples, qint8* buffer);
	bool isOpen() const { return m_File.isOpen(); }


private:

	bool WriteHeader(const QAudioFormat &format);
	bool WriteDataLength();
	void GetSytemTimeStructure(sSYSTEMTIME& systime);

	QFile m_File;
	QAudioFormat m_Format;
	qint64 m_HeaderLength;
	qint64 m_DataLength;
	qint64 m_CenterFrequency;

};
#endif // WAVEFILEWRITER_H
