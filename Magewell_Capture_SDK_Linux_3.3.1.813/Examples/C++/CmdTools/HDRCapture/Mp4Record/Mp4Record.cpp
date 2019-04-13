#include "Mp4Record.h"


Mp4Record::Mp4Record(void)
{
	m_fmtctxOut = NULL;

	m_deCodec = NULL;
	m_deParser = NULL;
	m_deCodecCtx = NULL;

	m_data = NULL;
	m_dwDataSize = 0;

	m_dwWidth = 1920;
	m_dwHeight = 1080;
}


Mp4Record::~Mp4Record(void)
{
}

bool Mp4Record::InitMp4(int dwWidth, int dwHeight, const char* pOutFileName)
{
	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;

	int ret = 0;
	AVOutputFormat *ofmt = NULL;

	//init ffmpeg
	av_register_all();

	m_fmtctxOut = avformat_alloc_context();

	//init out format
	avformat_alloc_output_context2(&m_fmtctxOut, NULL, NULL, pOutFileName);
	if (!m_fmtctxOut) {
		printf( "Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		return false;
	}

	ofmt = m_fmtctxOut->oformat;

	//Open output file
	if (!(ofmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&m_fmtctxOut->pb, pOutFileName, AVIO_FLAG_WRITE) < 0) {
			printf( "Could not open output file '%s'", pOutFileName);
			return false;
		}
	}

	//init decoder codec
	/* find the MPEG-1 video decoder */
	//m_deCodec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    m_deCodec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
	if (!m_deCodec) {
		fprintf(stderr, "Codec not found\n");
		return false;
	}

	m_deParser = av_parser_init(m_deCodec->id);
	if (!m_deParser) {
		fprintf(stderr, "parser not found\n");
        return false;
	}

	m_deCodecCtx = avcodec_alloc_context3(m_deCodec);
	if (!m_deCodecCtx) {
		fprintf(stderr, "Could not allocate video codec context\n");
        return false;
	}

	 /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(m_deCodecCtx, m_deCodec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return false;
    }

	return true;
}

void Mp4Record::WriteMp4Header(const unsigned char* pbyData, int cbSize)
{
	//Create output AVStream according to input AVStream
	//AVStream *in_stream = ifmt_ctx_v->streams[i];
	int ret = 0;
	/* read raw data from the input file */
	m_dwDataSize = cbSize;

	/* use the parser to split the data into frames */
    m_data = (unsigned char*)pbyData;

	//AVRational time_base1;
	//time_base1.num = 1;
	//time_base1.den = 1200000;

	//AVRational frame_rate;
	//frame_rate.num = 30;
	//frame_rate.den = 1;

	//init packet
	AVPacket pkt = {0};

	ret = av_parser_parse2(m_deParser, m_deCodecCtx, &pkt.data, &pkt.size,
		m_data, m_dwDataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	if (ret < 0) {
		fprintf(stderr, "Error while parsing\n");
		exit(1);
	}

	if (pkt.size == 0)
	{
		ret = av_parser_parse2(m_deParser, m_deCodecCtx, &pkt.data, &pkt.size,
			m_data, m_dwDataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	}

	m_deParser->parser->split(m_deCodecCtx, pkt.data, pkt.size);

	m_deCodecCtx->extradata = (uint8_t*)av_malloc(pkt.size + FF_INPUT_BUFFER_PADDING_SIZE);
	if (m_deCodecCtx->extradata) {
		memset(m_deCodecCtx->extradata + pkt.size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
		m_deCodecCtx->extradata_size = pkt.size;
		ret = 0;
	}

	memcpy(m_deCodecCtx->extradata, pkt.data,  
		m_deCodecCtx->extradata_size);

	AVStream *out_stream = avformat_new_stream(m_fmtctxOut, m_deCodec);
	if (!out_stream) {
		printf( "Failed allocating output stream\n");
		ret = AVERROR_UNKNOWN;
	}

	out_stream->start_time = 0;  
	m_deCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P10LE;//AV_PIX_FMT_P010LE;//
	m_deCodecCtx->width = m_dwWidth;  
	m_deCodecCtx->height = m_dwHeight; 

	//Copy the settings of AVCodecContext
	if (avcodec_copy_context(out_stream->codec, m_deCodecCtx) < 0) {
		printf( "Failed to copy context from input to output stream codec context\n");
		//goto end;
	}
	out_stream->codec->codec_tag = 0;
	if (m_fmtctxOut->oformat->flags & AVFMT_GLOBALHEADER)
		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

	AVDictionary* opt = NULL;
	av_dict_set_int(&opt, "video_track_timescale", 1000, 0);

	int nRet = avformat_write_header(m_fmtctxOut, &opt);
	if ( nRet< 0) {
		printf( "Error occurred when opening output file\n");
		//goto end;
	}

	m_out_stream = m_fmtctxOut->streams[0];
}

int Mp4Record::WriteVideoFrame(const unsigned char* pbyData, int cbSize, long long timeStamp)
{
		int ret = 0;
        /* read raw data from the input file */
        m_dwDataSize = cbSize;

        /* use the parser to split the data into frames */
        m_data = (unsigned char*)pbyData;

		//init packet
		AVPacket pkt = {0};

			do 
			{
				ret = av_parser_parse2(m_deParser, m_deCodecCtx, &pkt.data, &pkt.size,
					m_data, m_dwDataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
				if (ret < 0) {
					fprintf(stderr, "Error while parsing\n");
					exit(1);
				}
			} while (pkt.size == 0);

            if (pkt.size)
			{
				pkt.pts = pkt.dts = timeStamp;
				av_interleaved_write_frame(m_fmtctxOut, &pkt);
			}
}


void Mp4Record::DestoryMp4()
{
	//Write file trailer
	av_write_trailer(m_fmtctxOut);

	/* close output */
	int ret = 0;
	AVOutputFormat *ofmt = m_fmtctxOut->oformat;
	if (m_fmtctxOut && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(m_fmtctxOut->pb);
	avformat_free_context(m_fmtctxOut);
	if (ret < 0 && ret != AVERROR_EOF) {
		printf( "Error occurred.\n");
	}

	av_parser_close(m_deParser);
	avcodec_free_context(&m_deCodecCtx);
}