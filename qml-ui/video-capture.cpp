#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/eventfd.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdarg.h>

#include "video-capture.h"
#include "MWFOURCC.h"
#include "LibMWCapture/MWCapture.h"
#include "bitmap.h"

static bool b_st352 = false;
static SMPTE_ST352_PAYLOAD_ID u_st352 = {0};

static bool isRGB = false;
static bool isCheckSDI = false;/*6G or above SDI need.*/
static bool isCheckHDMI = false;

static MWCAP_VIDEO_COLOR_FORMAT capture_colorfmt = MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN;
static MWCAP_VIDEO_QUANTIZATION_RANGE capture_range = MWCAP_VIDEO_QUANTIZATION_UNKNOWN;

static int capture_width = 0;
static int capture_height = 0;
static DWORD capture_fourcc =  MWFOURCC_UNK;
static CS_ID capture_cs_id = CS_UNKNOWN;
static char video_signal_info[8192] = {0};
static char *_s_cur = &video_signal_info[0];

const char *get_capture_fourcc()
{
    static char _s_fourcc[5];
    char *tmp = (char *)&capture_fourcc;
    sprintf(_s_fourcc, "%c%c%c%c", tmp[0], tmp[1], tmp[2], tmp[3]);
    _s_fourcc[4] = 0;
    return _s_fourcc;
}
uint32_t get_capture_width()
{
    return static_cast<uint32_t>(capture_width);
}
uint32_t get_capture_height()
{
    return static_cast<uint32_t>(capture_height);
}
uint32_t get_capture_cs_id()
{
    return capture_cs_id; 
}
const char *get_capture_inputinfo()
{
    return video_signal_info;
}

static void viprintf(const char *format_string, ...)
{
    va_list args;
    va_start(args, format_string);
    _s_cur += vsprintf(_s_cur, format_string, args);
    va_end(args);
}

static void GetVideoInputTypeString(DWORD dwInputType, char *szInputType)
{
    switch (dwInputType) {
    case MWCAP_VIDEO_INPUT_TYPE_NONE:
        strcpy(szInputType, "NONE");
        break;
    case MWCAP_VIDEO_INPUT_TYPE_HDMI:
        strcpy(szInputType, "HDMI");
        break;
    case MWCAP_VIDEO_INPUT_TYPE_VGA:
        strcpy(szInputType, "VGA");
        break;
    case MWCAP_VIDEO_INPUT_TYPE_SDI:
        strcpy(szInputType, "SDI");
        break;
    case MWCAP_VIDEO_INPUT_TYPE_COMPONENT:
        strcpy(szInputType, "COMPONENT");
        break;
    case MWCAP_VIDEO_INPUT_TYPE_CVBS:
        strcpy(szInputType, "CVBS");
        break;
    case MWCAP_VIDEO_INPUT_TYPE_YC:
        strcpy(szInputType, "YC");
        break;
    default:
        strcpy(szInputType, "N/A");
        break;
    }
}

static void GetSignalStateString(DWORD dwSignalState, char *szSignalState)
{
    switch (dwSignalState) {
    case MWCAP_VIDEO_SIGNAL_NONE:
        strcpy(szSignalState, "None");
        break;
    case MWCAP_VIDEO_SIGNAL_LOCKING:
        strcpy(szSignalState, "Locking");
        break;
    case MWCAP_VIDEO_SIGNAL_LOCKED:
        strcpy(szSignalState, "Locked");
        break;
    case MWCAP_VIDEO_SIGNAL_UNSUPPORTED:
        strcpy(szSignalState, "Unsupported");
        break;
    default:
        strcpy(szSignalState, "N/A");
        break;
    }
}

static void GetColorSpaceString(DWORD dwColorspace, char *szColorSpace)
{
    switch (dwColorspace) {
    case MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN:
        strcpy(szColorSpace, "Unknown");
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_RGB:
        strcpy(szColorSpace, "RGB");
        capture_cs_id = sRGB;
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_YUV601:
        strcpy(szColorSpace, "YUV BT.601");
        capture_cs_id = ITU601;
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_YUV709:
        strcpy(szColorSpace, "YUV BT.709");
        capture_cs_id = ITU709;
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_YUV2020:
        strcpy(szColorSpace, "YUV BT.2020");
        capture_cs_id = ITU2020;
        break;
    case MWCAP_VIDEO_COLOR_FORMAT_YUV2020C:
        strcpy(szColorSpace, "YUV BT.2020c");
        capture_cs_id = ITU2020C;
        break;
    default:
        strcpy(szColorSpace, "N/A");
        break;
    }
}

static void GetQuantizationString(DWORD dwQuantization, char *szQuantization)
{
    switch (dwQuantization) {
    case MWCAP_VIDEO_QUANTIZATION_UNKNOWN:
        strcpy(szQuantization, "Unknown");
        break;
    case MWCAP_VIDEO_QUANTIZATION_FULL:
        strcpy(szQuantization, "Full");
        break;
    case MWCAP_VIDEO_QUANTIZATION_LIMITED:
        strcpy(szQuantization, "Limited");
        break;
    default:
        strcpy(szQuantization, "N/A");
        break;
    }
}

static void GetSaturationString(DWORD dwSaturation, char *szSaturation)
{
    switch (dwSaturation) {
    case MWCAP_VIDEO_SATURATION_UNKNOWN:
        strcpy(szSaturation, "Unknown");
        break;
    case MWCAP_VIDEO_SATURATION_FULL:
        strcpy(szSaturation, "Full");
        break;
    case MWCAP_VIDEO_SATURATION_LIMITED:
        strcpy(szSaturation, "Limited");
        break;
    case MWCAP_VIDEO_SATURATION_EXTENDED_GAMUT:
        strcpy(szSaturation, "Gamut");
        break;
    default:
        strcpy(szSaturation, "N/A");
        break;
    }
}

static MW_RESULT PrintInputCommon(HCHANNEL hChannel)
{
    MW_RESULT mr;

    do {
        DWORD dwVInputSource, dwVInputType;
        char szVInputType[16];
        mr = MWGetVideoInputSource(hChannel, &dwVInputSource);
        if (mr != MW_SUCCEEDED)
            break;

        dwVInputType = INPUT_TYPE(dwVInputSource);
        GetVideoInputTypeString(dwVInputType, szVInputType);

        viprintf("Input common\n"
               "  Video input ............................ %s\n",
               szVInputType);
        viprintf("\n");
    } while (0);

    return mr;
}

static MW_RESULT PrintInputVideo(HCHANNEL hChannel)
{
    MW_RESULT mr;

    do {
        MWCAP_VIDEO_SIGNAL_STATUS signalStatus;
        mr = MWGetVideoSignalStatus(hChannel, &signalStatus);
        if (mr != MW_SUCCEEDED)
            break;
        if (signalStatus.state == MWCAP_VIDEO_SIGNAL_NONE) {
            viprintf("Input video\n"
                   "  Signal ................................. None\n\n");
            break;
        }

        char szSignalState[16];
        char szColorSpace[16];
        char szQuantization[16];
        char szSaturation[16];
        GetSignalStateString(signalStatus.state, szSignalState);
        GetColorSpaceString(signalStatus.colorFormat, szColorSpace);
        GetQuantizationString(signalStatus.quantRange, szQuantization);
        GetSaturationString(signalStatus.satRange, szSaturation);

        char szScanningType[16];
        double fFrameRate;
        if (signalStatus.bSegmentedFrame) {
            strcpy(szScanningType, "sF");
            fFrameRate = 1.0 * 10000000 / signalStatus.dwFrameDuration;
        } else if (signalStatus.bInterlaced) {
            fFrameRate = 2.0 * 10000000 / signalStatus.dwFrameDuration;
            strcpy(szScanningType, "i");
        } else {
            fFrameRate = 1.0 * 10000000 / signalStatus.dwFrameDuration;
            strcpy(szScanningType, "p");
        }

        capture_colorfmt = signalStatus.colorFormat;
        capture_range = signalStatus.quantRange;
        if (capture_colorfmt == MWCAP_VIDEO_COLOR_FORMAT_RGB) {
            capture_fourcc = MWFOURCC_BGR10;
            isRGB = true;
        } else {
            capture_fourcc = MWFOURCC_Y410;
            isRGB = false;
        }
        capture_width = signalStatus.cx;
        capture_height = signalStatus.cy;
        viprintf("Input video\n"
               "  Signal state ........................... %s\n"
               "  Resolution ............................. %dx%d%s %'.2f Hz\n"
               "  Aspect ................................. %d:%d\n"
               "  Total size ............................. %dx%d\n"
               "  X offset ............................... %d\n"
               "  Y offset ............................... %d\n"
               "  Color space ............................ %s\n"
               "  Quantization ........................... %s\n"
               "  Saturation ............................. %s\n\n",
               szSignalState,
               signalStatus.cx, signalStatus.cy, szScanningType,
               fFrameRate,
               signalStatus.nAspectX, signalStatus.nAspectY,
               signalStatus.cxTotal, signalStatus.cyTotal,
               signalStatus.x, signalStatus.y,
               szColorSpace,
               szQuantization,
               szSaturation);
    } while (0);

    return mr;
}

static void PrintInputSDI(MWCAP_INPUT_SPECIFIC_STATUS specificStatus)
{
    const char *szSdiType = "N/A";
    switch (specificStatus.sdiStatus.sdiType) {
    case SDI_TYPE_SD:
        szSdiType = "SD";
        break;
    case SDI_TYPE_HD:
        szSdiType = "HD";
        break;
    case SDI_TYPE_3GA:
        szSdiType = "3G-A";
        break;
    case SDI_TYPE_3GB_DL:
        szSdiType = "3G-B dual link";
        break;
    case SDI_TYPE_3GB_DS:
        szSdiType = "3G-B dual stream";
        break;
    case SDI_TYPE_DL_CH1:
        szSdiType = "DL link CH1";
        break;
    case SDI_TYPE_DL_CH2:
        szSdiType = "DL link CH2";
        break;
    case SDI_TYPE_6G_MODE1:
        isCheckSDI = true;
        szSdiType = "6G Mode 1";
        break;
    //case SDI_TYPE_6G_MODE2:
    default:
        isCheckSDI = true;
        szSdiType = "6G Mode 2";
        break;
    }

    const char *szScanningFormat = "N/A";
    switch (specificStatus.sdiStatus.sdiScanningFormat) {
    case SDI_SCANING_INTERLACED:
        szScanningFormat = "Interlaced";
        break;
    case SDI_SCANING_SEGMENTED_FRAME:
        szScanningFormat = "Segmented frame";
        break;
    case SDI_SCANING_PROGRESSIVE:
        szScanningFormat = "Progressive";
        break;
    }

    const char *szBitDepth = "N/A";
    switch (specificStatus.sdiStatus.sdiBitDepth) {
    case SDI_BIT_DEPTH_8BIT:
        szBitDepth = "8 Bits";
        break;
    case SDI_BIT_DEPTH_10BIT:
        szBitDepth = "10 Bits";
        break;
    case SDI_BIT_DEPTH_12BIT:
        szBitDepth = "12 Bits";
        break;
    }

    const char *szSamplingStruct = "N/A";
    switch (specificStatus.sdiStatus.sdiSamplingStruct) {
    case SDI_SAMPLING_422_YCbCr:
        szSamplingStruct = "Y/Cb/Cr, 4:2:2";
        break;
    case SDI_SAMPLING_444_YCbCr:
        szSamplingStruct = "Y/Cb/Cr, 4:4:4";
        break;
    case SDI_SAMPLING_444_RGB:
        szSamplingStruct = "R/G/B, 4:4:4";
        break;
    case SDI_SAMPLING_420_YCbCr:
        szSamplingStruct = "Y/Cb/Cr, 4:2:2";
        break;
    case SDI_SAMPLING_4224_YCbCrA:
        szSamplingStruct = "Y/Cb/Cr/A, 4:2:2:4";
        break;
    case SDI_SAMPLING_4444_YCbCrA:
        szSamplingStruct = "Y/Cb/Cr/A, 4:4:4:4";
        break;
    case SDI_SAMPLING_4444_RGBA:
        szSamplingStruct = "R/G/B/A, 4:4:4:4";
        break;
    case SDI_SAMPLING_4224_YCbCrD:
        szSamplingStruct = "Y/Cb/Cr/D, 4:2:2:4";
        break;
    case SDI_SAMPLING_4444_YCbCrD:
        szSamplingStruct = "Y/Cb/Cr/A, 4:4:4:4";
        break;
    case SDI_SAMPLING_4444_RGBD:
        szSamplingStruct = "R/G/B/D, 4:4:4:4";
        break;
    case SDI_SAMPLING_444_XYZ:
        szSamplingStruct = "X/Y/Z, 4:4:4";
        break;
    }

    viprintf("Input specific\n"
           "  Signal status .......................... Valid\n"
           "  Type ................................... %s\n"
           "  Scanning type .......................... %s\n"
           "  Color depth ............................ %s\n"
           "  Sampling struct ........................ %s\n",
           szSdiType, szScanningFormat, szBitDepth, szSamplingStruct);

    if (specificStatus.sdiStatus.bST352DataValid)
        viprintf("  ST352 payloadID ........................ %d\n",
               specificStatus.sdiStatus.dwST352Data);

    viprintf("\n");
}

static void PrintInputHDMI(MWCAP_INPUT_SPECIFIC_STATUS specificStatus)
{
    const char *szPixelEncoding = "N/A";
    switch (specificStatus.hdmiStatus.pixelEncoding) {
    case HDMI_ENCODING_RGB_444:
        szPixelEncoding = "R/G/B 4:4:4";
        break;
    case HDMI_ENCODING_YUV_422:
        szPixelEncoding = "Y/U/V 4:2:2";
        break;
    case HDMI_ENCODING_YUV_444:
        szPixelEncoding = "Y/U/V 4:4:4";
        break;
    case HDMI_ENCODING_YUV_420:
        szPixelEncoding = "Y/U/V 4:2:0";
        break;
    }

    viprintf("Input specific\n"
           "  Signal status .......................... Valid\n"
           "  Mode ................................... %s\n"
           "  HDCP ................................... %s\n"
           "  Color depth ............................ %d bits\n"
           "  Pixel encoding ......................... %s\n"
           "  VIC .................................... %d\n"
           "  IT content ............................. %s\n"
           "  Timing - Scanning format ............... %s\n"
           "  Timing - Frame rate .................... %.2f\n"
           "  Timing - H Total ....................... %d\n"
           "  Timing - H Active ...................... %d\n"
           "  Timing - H Front porch ................. %d\n"
           "  Timing - H Sync width .................. %d\n"
           "  Timing - H back porch .................. %d\n"
           "  Timing - H field 0 V total ............. %d\n"
           "  Timing - H field 0 V active ............ %d\n"
           "  Timing - H field 0 V front porch ....... %d\n"
           "  Timing - H field 0 V sync width ........ %d\n"
           "  Timing - H field 0 V back porch ........ %d\n"
           "  Timing - H field 1 V total ............. %d\n"
           "  Timing - H field 1 V active ............ %d\n"
           "  Timing - H field 1 V front porch ....... %d\n"
           "  Timing - H field 1 V sync width ........ %d\n"
           "  Timing - H field 1 V back porch ........ %d\n\n"
           ,
           specificStatus.hdmiStatus.bHDMIMode ? "HDMI" : "DVI",
           specificStatus.hdmiStatus.bHDCP ? "Yes" : "No",
           specificStatus.hdmiStatus.byBitDepth,
           szPixelEncoding,
           specificStatus.hdmiStatus.byVIC,
           specificStatus.hdmiStatus.bITContent ? "True" : "False",
           specificStatus.hdmiStatus.videoTiming.bInterlaced ? "Interlaced" : "Progressive",
           1.0 * 10000000 / specificStatus.hdmiStatus.videoTiming.dwFrameDuration,
           specificStatus.hdmiStatus.videoTiming.wHTotalWidth,
           specificStatus.hdmiStatus.videoTiming.wHActive,
           specificStatus.hdmiStatus.videoTiming.wHFrontPorch,
           specificStatus.hdmiStatus.videoTiming.wHSyncWidth,
           specificStatus.hdmiStatus.videoTiming.wHBackPorch,
           specificStatus.hdmiStatus.videoTiming.wField0VTotalHeight,
           specificStatus.hdmiStatus.videoTiming.wField0VActive,
           specificStatus.hdmiStatus.videoTiming.wField0VFrontPorch,
           specificStatus.hdmiStatus.videoTiming.wField0VSyncWidth,
           specificStatus.hdmiStatus.videoTiming.wField0VBackPorch,
           specificStatus.hdmiStatus.videoTiming.wField1VTotalHeight,
           specificStatus.hdmiStatus.videoTiming.wField1VActive,
           specificStatus.hdmiStatus.videoTiming.wField1VFrontPorch,
           specificStatus.hdmiStatus.videoTiming.wField1VSyncWidth,
           specificStatus.hdmiStatus.videoTiming.wField1VBackPorch
           );
}

static void PrintInputVgaComponent(MWCAP_INPUT_SPECIFIC_STATUS specificStatus)
{
    const char *szSynctype = "N/A";
    switch (specificStatus.vgaComponentStatus.syncInfo.bySyncType) {
    case VIDEO_SYNC_ALL:
        szSynctype = "All";
        break;
    case VIDEO_SYNC_HS_VS:
        szSynctype = "+HS +VS";
        break;
    case VIDEO_SYNC_CS:
        szSynctype = "CS";
        break;
    case VIDEO_SYNC_EMBEDDED:
        szSynctype = "Embedded Sync";
        break;
    }

    const char *szTimingType = "N/A";
    switch (specificStatus.vgaComponentStatus.videoTiming.dwType) {
    case MWCAP_VIDEO_TIMING_NONE:
        szTimingType = "None";
        break;
    case MWCAP_VIDEO_TIMING_LEGACY:
        szTimingType = "Legacy";
        break;
    case MWCAP_VIDEO_TIMING_DMT:
        szTimingType = "DMT";
        break;
    case MWCAP_VIDEO_TIMING_CEA:
        szTimingType = "CEA";
        break;
    case MWCAP_VIDEO_TIMING_GTF:
        szTimingType = "GTF";
        break;
    case MWCAP_VIDEO_TIMING_CVT:
        szTimingType = "CVT";
        break;
    case MWCAP_VIDEO_TIMING_CVT_RB:
        szTimingType = "CVT_RB";
        break;
    case MWCAP_VIDEO_TIMING_FAILSAFE:
        szTimingType = "Failsafe";
        break;
    }
    viprintf("Input specific\n"
           "  Signal status .......................... Valid\n"
           "  Sync type .............................. %s\n"
           "  Frame rate ............................. %.2f Hz\n"
           "  Scanning format ........................ %s\n"
           "  VS width ............................... %d\n"
           "  Total scan lines ....................... %d\n"
           ,
           szSynctype,
           1.0 * 10000000 / specificStatus.vgaComponentStatus.syncInfo.dwFrameDuration,
           specificStatus.vgaComponentStatus.syncInfo.bInterlaced ? "Interlaced" : "Progressive",
           specificStatus.vgaComponentStatus.syncInfo.wVSyncLineCount,
           specificStatus.vgaComponentStatus.syncInfo.wFrameLineCount
           );

    if (specificStatus.vgaComponentStatus.syncInfo.bySyncType == VIDEO_SYNC_EMBEDDED)
        viprintf("  Emb sync ............................... %s\n",
               specificStatus.vgaComponentStatus.bTriLevelSync ? "Tri-Levle" : "Bi-Level");


    viprintf("  Timing - Type .......................... %s\n"
           "  Timing - Pixel clock ................... %d MHz\n"
           "  Timing - H Active ...................... %d\n"
           "  Timing - H Front porch ................. %d\n"
           "  Timing - H Sync width .................. %d\n"
           "  Timing - H Back proch .................. %d\n"
           "  Timing - V Active ...................... %d\n"
           "  Timing - V Front porch ................. %d\n"
           "  Timing - V Sync width  ................. %d\n"
           "  Timing - V back proch .................. %d\n\n",
           szTimingType,
           specificStatus.vgaComponentStatus.videoTiming.dwPixelClock/1000/1000,
           specificStatus.vgaComponentStatus.videoTiming.wHActive,
           specificStatus.vgaComponentStatus.videoTiming.wHFrontPorch,
           specificStatus.vgaComponentStatus.videoTiming.wHSyncWidth,
           specificStatus.vgaComponentStatus.videoTiming.wHBackPorch,
           specificStatus.vgaComponentStatus.videoTiming.wVActive,
           specificStatus.vgaComponentStatus.videoTiming.wVFrontPorch,
           specificStatus.vgaComponentStatus.videoTiming.wVSyncWidth,
           specificStatus.vgaComponentStatus.videoTiming.wVBackPorch
           );
}

static void PrintInputCvbsYc(MWCAP_INPUT_SPECIFIC_STATUS specificStatus)
{
    const char *szStandard = "N/A";
    switch (specificStatus.cvbsYcStatus.standard) {
    case MWCAP_SD_VIDEO_NONE:
        szStandard = "None";
        break;
    case MWCAP_SD_VIDEO_NTSC_M:
        szStandard = "NTSC-M";
        capture_cs_id = NTSC_1987;
        break;
    case MWCAP_SD_VIDEO_NTSC_433:
        szStandard = "NTSC-433";
        capture_cs_id = NTSC_1987;
        break;
    case MWCAP_SD_VIDEO_PAL_M:
        szStandard = "PAL-m";
        capture_cs_id = PAL_1970;
        break;
    case MWCAP_SD_VIDEO_PAL_60:
        szStandard = "PAL-60";
        capture_cs_id = PAL_1970;
        break;
    case MWCAP_SD_VIDEO_PAL_COMBN:
        szStandard = "PAL-COMBN";
        capture_cs_id = PAL_1970;
        break;
    case MWCAP_SD_VIDEO_PAL_BGHID:
        szStandard = "PAL-BGHID";
        capture_cs_id = PAL_1970;
        break;
    case MWCAP_SD_VIDEO_SECAM:
        szStandard = "SECAM";
        capture_cs_id = PAL_1970;
        break;
    case MWCAP_SD_VIDEO_SECAM_60:
        szStandard = "SECAM-60";
        capture_cs_id = PAL_1970;
        break;
    }

    viprintf("Input specific\n"
           "  Signal status .......................... Valid\n"
           "  TV standard ............................ %s\n"
           ,
           szStandard
           );

    if (specificStatus.cvbsYcStatus.b50Hz)
        viprintf("  Field .................................. 50 Hz\n\n");
    else
        viprintf("\n");
}


static MW_RESULT PrintInputSpecific(HCHANNEL hChannel)
{
    MW_RESULT mr;

    do {
        MWCAP_INPUT_SPECIFIC_STATUS specificStatus;
        mr = MWGetInputSpecificStatus(hChannel, &specificStatus);
        if (mr != MW_SUCCEEDED)
            break;

        if (specificStatus.bValid == false)
            viprintf("Input specific\n"
                   "  Signal status .......................... None\n\n");
        else {
            switch (specificStatus.dwVideoInputType) {
            case MWCAP_VIDEO_INPUT_TYPE_SDI:
                b_st352 = (specificStatus.sdiStatus.bST352DataValid != 0);
                u_st352.dwData = specificStatus.sdiStatus.dwST352Data;
                PrintInputSDI(specificStatus);
                break;
            case MWCAP_VIDEO_INPUT_TYPE_HDMI:
                isCheckHDMI = true;
                PrintInputHDMI(specificStatus);
                break;
            case MWCAP_VIDEO_INPUT_TYPE_VGA:
                PrintInputVgaComponent(specificStatus);
                break;
            case MWCAP_VIDEO_INPUT_TYPE_COMPONENT:
                PrintInputVgaComponent(specificStatus);
                break;
            case MWCAP_VIDEO_INPUT_TYPE_CVBS:
                PrintInputCvbsYc(specificStatus);
                break;
            case MWCAP_VIDEO_INPUT_TYPE_YC:
                PrintInputCvbsYc(specificStatus);
                break;
            default:
                break;
            }
        }
    } while (0);

    return mr;
}

static void check_hdmi(HCHANNEL hChannel)
{
    MW_RESULT xr;
    DWORD dwValidFlag = 0;
    xr = MWGetHDMIInfoFrameValidFlag(hChannel, &dwValidFlag);
    if (xr != MW_SUCCEEDED) {
        printf("ERROR: Get HDMI InfoFrame Flag!\n");
        return;
    }
    if (dwValidFlag == 0) {
        printf("No HDMI InfoFrame!\n");
        return;
    }

    HDMI_INFOFRAME_PACKET packet;
    if (dwValidFlag & MWCAP_HDMI_INFOFRAME_MASK_AVI) {
        xr = MWGetHDMIInfoFramePacket(hChannel, MWCAP_HDMI_INFOFRAME_ID_AVI, &packet);
        if (xr == MW_SUCCEEDED) {
            HDMI_AVI_INFOFRAME_PAYLOAD *aviinfo = &packet.aviInfoFramePayload;
            printf("Get HDMI InfoFrame AVI OK!%d::%d\n", aviinfo->byColorimetry, aviinfo->byExtendedColorimetry);
            switch (aviinfo->byColorimetry) {
            case 0:
                capture_cs_id = isRGB ? sRGB : ITU709_5;
                break;
            case 1:
                capture_cs_id = (capture_width > 720) ? ITU601_5 : NTSC_1987;
                break;
            case 2:
                capture_cs_id = ITU709_7;
                break;
            case 3:
                switch (aviinfo->byExtendedColorimetry) {
                case 0:
                    capture_cs_id = xvYCC601;
                    break;
                case 1:
                    capture_cs_id = xvYCC709;
                    break;
                case 2:
                    capture_cs_id = sYCC601;
                    break;
                case 3:
                    capture_cs_id = opYCC601;
                    break;
                case 4:
                    capture_cs_id = opRGB;
                    break;
                case 5:
                    capture_cs_id = ITU2020C;
                    break;
                case 6:
                    capture_cs_id = ITU2020;
                    break;
                }
                break;
            }
        }
    }
}

static void check_sdi(HCHANNEL hChannel)
{
    if (b_st352) {
        switch (u_st352.V1.byColorimetry) {
        case 0:
            capture_cs_id = ITU709;
            break;
        case 1:
            printf("Need VANC Packet impl!\n");
            break;
        case 2:
            capture_cs_id = ITU2020;
            break;
        }
    }
}

static void get_and_guess_misc_capture_param(HCHANNEL hChannel)
{
    b_st352 = false;
    u_st352 = {0};
    isRGB = false;
    isCheckSDI = false;/*6G or above SDI need.*/
    isCheckHDMI = false;
    capture_colorfmt = MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN;
    capture_range = MWCAP_VIDEO_QUANTIZATION_UNKNOWN;
    capture_width = 0;
    capture_height = 0;
    capture_fourcc = MWFOURCC_UNK;
    capture_cs_id = CS_UNKNOWN;
    video_signal_info[0] = 0;
    _s_cur = &video_signal_info[0];

    PrintInputCommon(hChannel);
    PrintInputVideo(hChannel);
    PrintInputSpecific(hChannel);
    printf("\n\n%s\n\n", video_signal_info);
    if (isCheckHDMI)
        check_hdmi(hChannel);
    if (isCheckSDI)
        check_sdi(hChannel);
}

static HCHANNEL open_channel(int nDevIndex)
{
    HCHANNEL hChannel = NULL;
    int nChannelCount = MWGetChannelCount();

    if (0 == nChannelCount) {
        printf("ERROR: Can't find channels!\n");
        return NULL;
    }
    printf("Find %d channels!\n",nChannelCount);
    int nProDevCount = 0;
    int nProDevChannel[32] = {-1};
    for (int i = 0; i < nChannelCount; i++) {
        MWCAP_CHANNEL_INFO info;
        MW_RESULT mr = MWGetChannelInfoByIndex(i, &info);
        if (strcmp(info.szFamilyName, "Pro Capture") == 0) {
            printf("find %s\n",info.szFamilyName);
            nProDevChannel[nProDevCount] = i;
            nProDevCount++;
        }
    }
    if (nProDevCount <= 0) {
        printf("\nERROR: Can't find pro or eco channels!\n");
        return NULL;
    }

    printf("Find %d pro channels.\n", nProDevCount);

    if (nDevIndex >= nProDevCount) {
        printf("ERROR: just have %d channel!\n",nProDevCount);
        return NULL;
    }
    char path[128] = {0};
    MWGetDevicePath(nProDevChannel[nDevIndex], path);
    hChannel = MWOpenChannelByPath(path);
    if (hChannel == NULL) {
        printf("ERROR: Open channel %d error!\n", nDevIndex);
        return NULL;
    }
    return hChannel;
}

static void save_raw_file(void *data, int length, const char *name)
{
    FILE* raw = fopen(name,"wb");
    assert(NULL != raw);
    fwrite(data, length, 1, raw);
    fclose(raw);
}

static void capture_frames(HCHANNEL hChannel, int cx, int cy, DWORD dwFourcc,
        MWCAP_VIDEO_COLOR_FORMAT colorfmt, MWCAP_VIDEO_QUANTIZATION_RANGE range,
        unsigned cnt, const char *base, bool isBMP)
{
    MW_RESULT xr;
    HNOTIFY hNotify;
    MWCAP_PTR hCaptureEvent;
    MWCAP_PTR hNotifyEvent;

    DWORD dwMinStride = FOURCC_CalcMinStride(dwFourcc, cx, 4);
    DWORD dwImageSize = FOURCC_CalcImageSize(dwFourcc, cx, cy, dwMinStride);
 
    int done = 0;
    HANDLE64 pbImage[cnt];
    for (int i = 0; i != cnt; ++i)
        pbImage[i] = 0;

    hCaptureEvent = MWCreateEvent();
    if (hCaptureEvent == 0)
        return;

    hNotifyEvent = MWCreateEvent();
    if (hNotifyEvent == 0)
        goto ERR_CREATE_HNOTIFYEVENT;

    xr = MWStartVideoCapture(hChannel, hCaptureEvent);
    if (xr != MW_SUCCEEDED)
        goto ERR_START_CAPTURE;

    hNotify = MWRegisterNotify(hChannel, hNotifyEvent, MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED);
    if (hNotify == 0)
        goto ERR_REGISTER_NOTIFY;

    for (; done != cnt; ++done) {
        MWCAP_VIDEO_BUFFER_INFO videoBufferInfo;
        MWCAP_VIDEO_FRAME_INFO videoFrameInfo;

        pbImage[done] = (HANDLE64)(unsigned long)malloc(dwImageSize);
        if (!pbImage[done])
            break;
        memset((void *)(unsigned long)pbImage[done], 0, dwImageSize);

        if (MWWaitEvent(hNotifyEvent, 1000) <= 0)
            break;

        ULONGLONG ullStatusBits = 0;
        if (MWGetNotifyStatus(hChannel, hNotify, &ullStatusBits) != MW_SUCCEEDED)
            break;

        if (MWGetVideoBufferInfo(hChannel, &videoBufferInfo) != MW_SUCCEEDED)
            break;

        if (MWGetVideoFrameInfo(hChannel, videoBufferInfo.iNewestBufferedFullFrame, &videoFrameInfo)
                != MW_SUCCEEDED)
            break;

        if ((ullStatusBits & MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED) == 0)
            break;

        if (MWCaptureVideoFrameToVirtualAddressEx(hChannel,
                videoBufferInfo.iNewestBufferedFullFrame, (unsigned char *)pbImage[done], dwImageSize, dwMinStride,
                1, 0, dwFourcc, cx, cy, 0, 0, 0, 0, 0, 100, 0, 100, 0, MWCAP_VIDEO_DEINTERLACE_BLEND,
                MWCAP_VIDEO_ASPECT_RATIO_CROPPING, 0, 0, 0, 0, colorfmt,
                range, MWCAP_VIDEO_SATURATION_UNKNOWN)
                != MW_SUCCEEDED)
            break;

        if (MWWaitEvent(hCaptureEvent, 1000) <= 0)
            break;
    }
    MWUnregisterNotify(hChannel, hNotify);
    for (int i = 0; i != done; ++i) {
        char name[1024];
        char fourcc[5];
        char *tmp = (char *)&dwFourcc;
        sprintf(fourcc, "%c%c%c%c", tmp[0], tmp[1], tmp[2], tmp[3]);
        fourcc[4] = 0;
        if (fourcc[3] == ' ')
            fourcc[3] = 0;
        if (isBMP) {
            sprintf(name,"%s%d.bmp", base, i);
            create_bitmap(name, pbImage[i], dwImageSize, cx, cy);
        } else {
            sprintf(name,"%s%d.%s", base, i, fourcc);
            save_raw_file((void *)(unsigned long)pbImage[i], dwImageSize, name);
        }
        free((void *)(unsigned long)pbImage[i]);
    }

ERR_REGISTER_NOTIFY:
    MWStopVideoCapture(hChannel);

ERR_START_CAPTURE:
    MWCloseEvent(hNotifyEvent);

ERR_CREATE_HNOTIFYEVENT:
    MWCloseEvent(hCaptureEvent);
}

void fresh_capture(uint8_t card, const char *base, unsigned cnt, bool need_bmp)
{
    if (!MWCaptureInitInstance())
        printf("have InitilizeFailed");

    HCHANNEL hChannel = NULL;

    do {
        MWRefreshDevice();
        hChannel = open_channel(card);
        if (NULL == hChannel)
            break;

        MWCAP_CHANNEL_INFO videoInfo = {0};
        if (MW_SUCCEEDED != MWGetChannelInfo(hChannel, &videoInfo)) {
            printf("ERROR: Can't get channel info!\n");
            break;
        }

        printf("Open channel - BoardIndex = %X, ChannelIndex = %d.\n", videoInfo.byBoardIndex, videoInfo.byChannelIndex);
        printf("Product Name: %s\n", videoInfo.szProductName);
        printf("Board SerialNo: %s\n\n", videoInfo.szBoardSerialNo);

        //check print and setting
        get_and_guess_misc_capture_param(hChannel);

        if (need_bmp && cnt) {//test for (sRGB, BGR). save as bmp
            DWORD dwFourcc = MWFOURCC_BGR24;
            MWCAP_VIDEO_COLOR_FORMAT colorfmt = MWCAP_VIDEO_COLOR_FORMAT_RGB;
            MWCAP_VIDEO_QUANTIZATION_RANGE range = MWCAP_VIDEO_QUANTIZATION_FULL;
            capture_frames(hChannel, capture_width, capture_height, dwFourcc, colorfmt, range, cnt, base, true);
        }

        if (cnt)
            capture_frames(hChannel, capture_width, capture_height, capture_fourcc,
                    capture_colorfmt, capture_range, cnt, base, false);

    } while (0);

    if (hChannel != NULL)
        MWCloseChannel(hChannel);

    MWCaptureExitInstance();
}

#ifdef MW_MAIN_
static void print_version_and_useage()
{
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.%d\n",byMaj,byMin,wBuild);
    printf("Usage:\n");
    printf("capture-demo <card-index> <base-name> <cnt> <need-bmp>\n\n");
}

int main(int argc, char* argv[])
{
    // Version
    print_version_and_useage();

    if (argc != 5) {
        printf("ERROR: Invalid params!\n");
        printf("\nPress 'Enter' to exit!\n");
        getchar();
        return 0;
    }
    int card = atoi(argv[1]);
    const char *base = argv[2];
    int cnt = atoi(argv[3]);
    bool need_bmp = (atoi(argv[4]) != 0);

    fresh_capture(card, base, cnt, need_bmp);

    printf("\nPress 'Enter' to exit!\n");
    getchar();
    return 0;
}
#endif
