/****************************************************************************
****************************************************************************/

#ifndef WAVEFILEREADER_H
#define WAVEFILEREADER_H

#include <QObject>
#include <QFile>
#include <QAudioFormat>
#include "dsp/datatypes.h"


class CWaveFileReader : public QFile
{
	Q_OBJECT
public:
	CWaveFileReader(QObject *parent = 0);
	~CWaveFileReader();
	using QFile::open;
	bool open(const QString &fileName);
	const QAudioFormat &fileFormat() const;
	qint64 headerLength(){return m_headerLength;}
	qint64 dataLength() {return m_DataLength;}
	qint64 centerFrequency(){return m_CenterFrequency;}

private:
	bool readHeader();
	qint64 m_headerLength;
	qint64 m_DataLength;
	qint64 m_CenterFrequency;
	QAudioFormat m_fileFormat;

};

#endif // WAVEFILEREADER_H
