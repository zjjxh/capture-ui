#include <stdio.h>

typedef struct st_MWNVENCHANDLE { int data; } *HMWNVENC;
typedef enum en_CODEC_TYPE{
    EN_CODEC_TYPE_H264 = 1,
    EN_CODEC_TYPE_H265 = 2,
}EN_CODEC_TYPE;
typedef enum en_CODEC_PROFILE{
    EN_CODEC_PROFILE_BASELINE = 1,
    EN_CODEC_PROFILE_MAIN = 2,
    EN_CODEC_PROFILE_HIGHT = 3,
}EN_CODEC_PROFILE;

typedef enum _en_PIXEL_FORMAT
{
    EN_PIXEL_FORMAT_UNDEFINED     = 0x00000000,
    EN_PIXEL_FORMAT_NV12          = 0x00000001,
    EN_PIXEL_FORMAT_YV12          = 0x00000010,
    EN_PIXEL_FORMAT_IYUV          = 0x00000100,
    EN_PIXEL_FORMAT_YUV444        = 0x00001000,
    EN_PIXEL_FORMAT_YUV420_10BIT  = 0x00010000,
    EN_PIXEL_FORMAT_YUV444_10BIT  = 0x00100000,
    EN_PIXEL_FORMAT_ARGB          = 0x01000000,
    EN_PIXEL_FORMAT_ARGB10        = 0x02000000,
    EN_PIXEL_FORMAT_AYUV          = 0x04000000,
    EN_PIXEL_FORMAT_ABGR          = 0x10000000,
    EN_PIXEL_FORMAT_ABGR10        = 0x20000000,
} EN_PIXEL_FORMAT;

HMWNVENC MWNVOpenEncoder(int iDeviceId, int iWidth, int iHeight, int iFpsNum, int iFpsDen, EN_PIXEL_FORMAT enPixelFormat,
    int iBitRate, int iKeyInterval, EN_CODEC_TYPE enCodeType, EN_CODEC_PROFILE enCodecProfile, unsigned int uiIfAsyncEncodec);

int MWNVCloseEncoder(HMWNVENC hMWNVEnc);


int MWNVEncodeFrameSync(HMWNVENC hMWNVEnc, unsigned char *pucInFrame, unsigned char **ppucOutFrame, unsigned int *puiOutFrameLen);


int MWNVEncodeFrameAsync(HMWNVENC hMWNVEnc, unsigned char *pucInFrame);

int MWNVGetBitStream(HMWNVENC hMWNVEnc, unsigned char **ppucOutFrame, unsigned int *puiOutFrameLen);
