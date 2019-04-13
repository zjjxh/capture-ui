#pragma  once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern "C"{
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
#ifndef   UINT64_C
#define   UINT64_C(value)__CONCAT(value,ULL)
#endif
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}



class Mp4Record
{
public:
	Mp4Record(void);
	virtual ~Mp4Record(void);

	bool InitMp4(int dwWidth, int dwHeight, const char* pOutFileName);
	
	void WriteMp4Header(const unsigned char* pbyData, int cbSize);
	int WriteVideoFrame(const unsigned char* pbyData, int cbSize, long long timeStamp);

	void DestoryMp4();

protected:
	AVFormatContext *m_fmtctxOut;
	AVStream		*m_out_stream;
    unsigned char	*m_data;
	int			    m_dwDataSize;

	const AVCodec			*m_deCodec;
	AVCodecParserContext	*m_deParser;
	AVCodecContext			*m_deCodecCtx;

	int m_dwWidth;
	int m_dwHeight;
};
