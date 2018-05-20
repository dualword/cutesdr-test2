/****************************************************************************
****************************************************************************/

#ifndef WAVEFILEREADER_H
#define WAVEFILEREADER_H

#include <QObject>
#include <QFile>
#include <QAudioFormat>
#include "dsp/datatypes.h"

struct sFmtSubChunk
{
	quint16     audioFormat;
	quint16     numChannels;
	quint32     sampleRate;
	quint32     byteRate;
	quint16     blockAlign;
	quint16     bitsPerSample;
	quint8		cbSizePad[24];
}__attribute__((gcc_struct,packed));

struct sAuxiSubChunk
{
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

#define MAX_HEADER 128	//max expected size of header
#define MAX_RDDATABLK 8192	//max bytes per data block read

class CWaveFileReader : public QFile
{
	Q_OBJECT
public:
	CWaveFileReader(QObject *parent = 0);
	~CWaveFileReader();
	using QFile::open;
	bool open(const QString &fileName);
	//overloaded read functions
	int GetNextDataBlock(tStereo16* pData, int NumSamples);	//for complex 16 bit ints
	int GetNextDataBlock(qint16* pData, int NumSamples);	//for real 16 bit ints
	int GetNextDataBlock(TYPECPX* pData, int NumSamples);	//for complex 16 bit ints
	int GetNextDataBlock(TYPEREAL* pData, int NumSamples);	//for real 16 bit ints
	qint64 GetCenterFrequency(){return m_CenterFrequency;}
	quint32 GetSampleRate(){return m_FmtSubChunk.sampleRate;}
	quint32 GetNumberSamples(){return m_NumSamples;}

	QString m_FileInfoStr;

private:
	bool readHeader();
	bool FindSubChunk(const char*Id, quint32* pStart, quint32* pLength);
	quint8 m_HeaderBuffer[MAX_HEADER];
	quint8 m_DataBuffer[MAX_RDDATABLK];
	quint32 m_NumSamples;
	quint32 m_CenterFrequency;
	sFmtSubChunk m_FmtSubChunk;
	sAuxiSubChunk m_AuxiSubChunk;
};

#endif // WAVEFILEREADER_H
