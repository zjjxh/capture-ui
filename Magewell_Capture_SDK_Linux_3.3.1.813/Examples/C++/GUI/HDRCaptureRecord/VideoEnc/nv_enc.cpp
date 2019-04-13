#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>

#include "dynlink_cuda_cuda.h"
#include "nvEncodeAPI.h"
#include "nv_enc.h"
#define MWNV_MAX_DEVICE_NUM 4
#define MWNV_MAX_ENCODER_NUM 32

#define MWNV_IN_BUFFER_NUM 2
#define MWNV_OUT_BUFFER_NUM 2

#define MWNV_OUT_BITSTREAM_BUFFER_SIZE (2 * 1024 * 1024)
typedef NVENCSTATUS(NVENCAPI *MYPROC)(NV_ENCODE_API_FUNCTION_LIST*);
#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}

#ifndef _WIN32
typedef void* HANDLE;
typedef void* HINSTANCE;
#endif
typedef struct st_MWNV_INPUT_CACHE{
    unsigned char ucBUsing;
    unsigned char aucR[3];
    NV_ENC_INPUT_PTR hInputSurface;
}ST_MWNV_INPUT_CACHE;

typedef struct st_MWNV_OUTPUT_CACHE{
    unsigned char ucBUsing;
    unsigned char aucR[3];
    NV_ENC_OUTPUT_PTR hOutputStream;
    HANDLE  hOutPutEvent;
}ST_MWNV_OUTPUT_CACHE;

typedef struct st_MWNV_ENC_CTXT { 
    unsigned int uiChId; 
    unsigned char ucIsUsing;
    unsigned char ucNeedUnlockOutBuffer;
    unsigned char aucR[2];

    NV_ENC_BUFFER_FORMAT enEncBufferFormat;

    void *hEncoder;
    
    NV_ENC_INITIALIZE_PARAMS stEncodeInitParams;
    NV_ENC_CONFIG stEncodeConfig;
    ST_MWNV_INPUT_CACHE  astInCache[MWNV_IN_BUFFER_NUM];
    ST_MWNV_OUTPUT_CACHE astOutCache[MWNV_OUT_BUFFER_NUM];

    long long uiInFramNum;
    long long uiOutFramNum;
    int iInCacheNum;
    int iOutCacheNum;
}ST_MWNV_ENC_CTXT;

NV_ENCODE_API_FUNCTION_LIST *g_pEncodeAPI = NULL;

void *g_apDevice[MWNV_MAX_DEVICE_NUM] = { NULL, NULL, NULL, NULL };

ST_MWNV_ENC_CTXT *g_apstMwnvEncoder[MWNV_MAX_ENCODER_NUM] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

int MWNVInitEncodeAPI(){
    MYPROC nvEncodeAPICreateInstance;
    HINSTANCE hInstLib;
#ifndef _WIN32
    hInstLib = dlopen("libnvidia-encode.so.1", RTLD_LAZY);
#else
#if defined (_WIN64)
    hInstLib = LoadLibrary(TEXT("nvEncodeAPI64.dll"));
#else
    hInstLib = LoadLibrary(TEXT("nvEncodeAPI.dll"));
#endif

#endif
    if (NULL == hInstLib) {
        printf("open lib fail\n");
        return -1;
    }
#ifdef _WIN32    
     nvEncodeAPICreateInstance = (MYPROC)GetProcAddress(hInstLib, "NvEncodeAPICreateInstance");
#else
     nvEncodeAPICreateInstance = (MYPROC)dlsym(hInstLib, "NvEncodeAPICreateInstance");
#endif
    if (NULL == nvEncodeAPICreateInstance) {
        printf("get addr fail\n");
        return -2;
    }
    g_pEncodeAPI = (NV_ENCODE_API_FUNCTION_LIST *)malloc(sizeof(NV_ENCODE_API_FUNCTION_LIST));
    if (NULL == g_pEncodeAPI){
        printf("alloc g_pEncodeAPI fail\n");
        return -3;
    }
    memset(g_pEncodeAPI, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
    g_pEncodeAPI->version = NV_ENCODE_API_FUNCTION_LIST_VER;
    if (NV_ENC_SUCCESS != nvEncodeAPICreateInstance(g_pEncodeAPI)) {
        printf("get g_pEncodeAPI fail\n");
        free(g_pEncodeAPI);
        return -4;
    }

    return 0;
}

int MWNVInitCuda(int iDeviceId)
{
    CUresult cuResult;
    CUdevice device;
    CUcontext cuContextCurr;
    int  deviceCount = 0;
    int  SMminor = 0, SMmajor = 0;

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    typedef HMODULE CUDADRIVER;
#else
    typedef void *CUDADRIVER;
#endif
    CUDADRIVER hHandleDriver = 0;
    cuResult = CUDA_SUCCESS;
    if (NULL == hHandleDriver) {
        cuResult = cuInit(0, __CUDA_API_VERSION, hHandleDriver);
    }
    if (cuResult != CUDA_SUCCESS) {
        return -1;
    }
    cuResult = cuDeviceGetCount(&deviceCount);
    if (cuResult != CUDA_SUCCESS) {
        return -2;
    }
    // If dev is negative value, we clamp to 0
    if (iDeviceId < 0){
        iDeviceId = 0;
    }
    if (iDeviceId > deviceCount - 1) {
        return -3;
    }
    cuResult = cuDeviceGet(&device, iDeviceId);
    if (cuResult != CUDA_SUCCESS){
        return -4;
    }
    cuResult = cuDeviceComputeCapability(&SMmajor, &SMminor, iDeviceId);
    if (cuResult != CUDA_SUCCESS) {
        return -5;
    }
    if (((SMmajor << 4) + SMminor) < 0x30){
        return -6;
    }
    cuResult = cuCtxCreate((CUcontext*)(&g_apDevice[iDeviceId]), 0, device);
    if (cuResult != CUDA_SUCCESS){
        return -7;
    }
    cuResult = cuCtxPopCurrent(&cuContextCurr);
    if (cuResult != CUDA_SUCCESS){
        return -8;
    }
    return 0;
}
int MWNVCreateInAndOutBuffer(ST_MWNV_ENC_CTXT *pstMwnvEncoder, EN_PIXEL_FORMAT enPixelFormat)
{
    NV_ENC_CREATE_INPUT_BUFFER stInputBufferParams;
    int i;
    memset(&stInputBufferParams, 0, sizeof(stInputBufferParams));
    SET_VER(stInputBufferParams, NV_ENC_CREATE_INPUT_BUFFER);
    stInputBufferParams.width = pstMwnvEncoder->stEncodeInitParams.encodeWidth;
    stInputBufferParams.height = pstMwnvEncoder->stEncodeInitParams.encodeHeight;
    stInputBufferParams.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;//NV_ENC_MEMORY_HEAP_VID;
    if (EN_PIXEL_FORMAT_NV12 == enPixelFormat){
        stInputBufferParams.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12;
    }
    else if (EN_PIXEL_FORMAT_ABGR == enPixelFormat){
        stInputBufferParams.bufferFmt = NV_ENC_BUFFER_FORMAT_ABGR;
    }
    else if (EN_PIXEL_FORMAT_ARGB == enPixelFormat){
        stInputBufferParams.bufferFmt = NV_ENC_BUFFER_FORMAT_ARGB;
    }
    else if (EN_PIXEL_FORMAT_YUV420_10BIT == enPixelFormat){
        stInputBufferParams.bufferFmt = NV_ENC_BUFFER_FORMAT_YUV420_10BIT;
    }
    pstMwnvEncoder->enEncBufferFormat = stInputBufferParams.bufferFmt;
    for (i = 0; i < pstMwnvEncoder->iInCacheNum; i++)
    {
        if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncCreateInputBuffer(pstMwnvEncoder->hEncoder, &stInputBufferParams)){
            printf("create input buffer fail\n");
            return -1;
        }
        pstMwnvEncoder->astInCache[i].hInputSurface = stInputBufferParams.inputBuffer;
    }
    for (i = 0; i < pstMwnvEncoder->iOutCacheNum; i++)
    {
        NV_ENC_CREATE_BITSTREAM_BUFFER createBitstreamBufferParams;
        memset(&createBitstreamBufferParams, 0, sizeof(createBitstreamBufferParams));
        SET_VER(createBitstreamBufferParams, NV_ENC_CREATE_BITSTREAM_BUFFER);

        createBitstreamBufferParams.size = MWNV_OUT_BITSTREAM_BUFFER_SIZE;
        createBitstreamBufferParams.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;// NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;

        if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncCreateBitstreamBuffer(pstMwnvEncoder->hEncoder, &createBitstreamBufferParams)){
            printf("create output buffer fail\n");
            return -1;
        }
        pstMwnvEncoder->astOutCache[i].hOutputStream = createBitstreamBufferParams.bitstreamBuffer;
        if (pstMwnvEncoder->stEncodeInitParams.enableEncodeAsync){
            NV_ENC_EVENT_PARAMS stEventParams;
            memset(&stEventParams, 0, sizeof(stEventParams));
            SET_VER(stEventParams, NV_ENC_EVENT_PARAMS);
#if defined(_WIN32)
            stEventParams.completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
            stEventParams.completionEvent = NULL;
            return 0;
#endif
            if (g_pEncodeAPI->nvEncRegisterAsyncEvent(pstMwnvEncoder->hEncoder, &stEventParams)){
                return -1;
            }

            pstMwnvEncoder->astOutCache[i].hOutPutEvent = stEventParams.completionEvent;
        }
        else{
            pstMwnvEncoder->astOutCache[i].hOutPutEvent = NULL;
        }
    }
    return 0;
}
HMWNVENC MWNVOpenEncoder(int iDeviceId, int iWidth, int iHeight, int iFpsNum, int iFpsDen, EN_PIXEL_FORMAT enPixelFormat,
    int iBitRate, int iKeyInterval, EN_CODEC_TYPE enCodeType, EN_CODEC_PROFILE enCodecProfile, unsigned int uiIfAsyncEncodec)
{
    int i;
    ST_MWNV_ENC_CTXT *pstMwnvEncoder = NULL;

    if ((iDeviceId > MWNV_MAX_DEVICE_NUM) || (iDeviceId < 0)){
        printf("iDeviceId[%d] > MAX_DEVICE_NUM[%d]\n", iDeviceId, MWNV_MAX_DEVICE_NUM);
        return NULL;
    }
    if (NULL == g_apDevice[iDeviceId]){
        if (MWNVInitCuda(iDeviceId)){
            printf("iDeviceId[%d] init cuda fail\n", iDeviceId);
            return NULL;
        }
    }
    if (NULL == g_pEncodeAPI){
        if (MWNVInitEncodeAPI()){
            return NULL;
        }
    }
    for (i = 0; i < MWNV_MAX_ENCODER_NUM; i++){
        if (g_apstMwnvEncoder[i] == NULL) {
            g_apstMwnvEncoder[i] = (ST_MWNV_ENC_CTXT *)malloc(sizeof(ST_MWNV_ENC_CTXT));
            g_apstMwnvEncoder[i]->uiChId = i;
            pstMwnvEncoder = g_apstMwnvEncoder[i];
            break;
        }
        else if (g_apstMwnvEncoder[i]->ucIsUsing == 0) {
            pstMwnvEncoder = g_apstMwnvEncoder[i];
            break;
        }
    }
    if (NULL == pstMwnvEncoder){
        printf("alloc pstMwnvEncoder fail\n", iDeviceId);
        return NULL;
    }

    pstMwnvEncoder->ucIsUsing = 1;
    pstMwnvEncoder->uiChId += MWNV_MAX_ENCODER_NUM;
    pstMwnvEncoder->ucNeedUnlockOutBuffer = 0;
    pstMwnvEncoder->hEncoder = NULL;

    pstMwnvEncoder->uiInFramNum = 0;
    pstMwnvEncoder->uiOutFramNum = 0;
    memset(pstMwnvEncoder->astInCache, 0, sizeof(pstMwnvEncoder->astInCache));
    memset(pstMwnvEncoder->astOutCache, 0, sizeof(pstMwnvEncoder->astOutCache));
    pstMwnvEncoder->iInCacheNum = MWNV_IN_BUFFER_NUM;
    pstMwnvEncoder->iOutCacheNum = MWNV_OUT_BUFFER_NUM;
    if (!uiIfAsyncEncodec){
        pstMwnvEncoder->iInCacheNum = 1;
        pstMwnvEncoder->iOutCacheNum = 1;
    }

    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS stOpenSessionExParams;
    memset(&stOpenSessionExParams, 0, sizeof(stOpenSessionExParams));
    SET_VER(stOpenSessionExParams, NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS);

    stOpenSessionExParams.device = g_apDevice[iDeviceId];
    stOpenSessionExParams.deviceType = NV_ENC_DEVICE_TYPE_CUDA;
    stOpenSessionExParams.apiVersion = NVENCAPI_VERSION;

    if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncOpenEncodeSessionEx(&stOpenSessionExParams, &(pstMwnvEncoder->hEncoder))){
        printf("nvEncOpenEncodeSessionEx fail\n");
        return NULL;
    }
    memset(&(pstMwnvEncoder->stEncodeInitParams), 0, sizeof(NV_ENC_INITIALIZE_PARAMS));
    SET_VER(pstMwnvEncoder->stEncodeInitParams, NV_ENC_INITIALIZE_PARAMS);

    pstMwnvEncoder->stEncodeInitParams.encodeWidth = iWidth;
    pstMwnvEncoder->stEncodeInitParams.encodeHeight = iHeight;
    pstMwnvEncoder->stEncodeInitParams.darWidth = iWidth;
    pstMwnvEncoder->stEncodeInitParams.darHeight = iHeight;
    pstMwnvEncoder->stEncodeInitParams.maxEncodeWidth = iWidth;
    pstMwnvEncoder->stEncodeInitParams.maxEncodeHeight = iHeight;
    if (EN_CODEC_TYPE_H265 == enCodeType) {
        pstMwnvEncoder->stEncodeInitParams.encodeGUID = NV_ENC_CODEC_HEVC_GUID;
    }
    else{
        pstMwnvEncoder->stEncodeInitParams.encodeGUID = NV_ENC_CODEC_H264_GUID;
    }

    pstMwnvEncoder->stEncodeInitParams.presetGUID = NV_ENC_PRESET_HP_GUID;
    pstMwnvEncoder->stEncodeInitParams.frameRateNum = iFpsNum;
    pstMwnvEncoder->stEncodeInitParams.frameRateDen = iFpsDen;
    pstMwnvEncoder->stEncodeInitParams.enableEncodeAsync = uiIfAsyncEncodec;
    pstMwnvEncoder->stEncodeInitParams.enablePTD = 1;
    pstMwnvEncoder->stEncodeInitParams.reportSliceOffsets = 0;
    pstMwnvEncoder->stEncodeInitParams.enableSubFrameWrite = 0;
    pstMwnvEncoder->stEncodeInitParams.enableExternalMEHints = 0;
    pstMwnvEncoder->stEncodeInitParams.encodeConfig = &(pstMwnvEncoder->stEncodeConfig);

    // apply preset
    NV_ENC_PRESET_CONFIG stPresetCfg;
    memset(&stPresetCfg, 0, sizeof(NV_ENC_PRESET_CONFIG));
    SET_VER(stPresetCfg, NV_ENC_PRESET_CONFIG);
    SET_VER(stPresetCfg.presetCfg, NV_ENC_CONFIG);

    if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncGetEncodePresetConfig(pstMwnvEncoder->hEncoder, pstMwnvEncoder->stEncodeInitParams.encodeGUID, 
        pstMwnvEncoder->stEncodeInitParams.presetGUID, &stPresetCfg)) {
        printf("nvEncGetEncodePresetConfig fail\n");
        return NULL;
    }

    memcpy(&(pstMwnvEncoder->stEncodeConfig), &stPresetCfg.presetCfg, sizeof(NV_ENC_CONFIG));

    pstMwnvEncoder->stEncodeConfig.rcParams.averageBitRate = iBitRate;

    pstMwnvEncoder->stEncodeConfig.gopLength = iKeyInterval;
    if (EN_CODEC_TYPE_H265 == enCodeType) {
        pstMwnvEncoder->stEncodeConfig.encodeCodecConfig.hevcConfig.repeatSPSPPS = 1;
        pstMwnvEncoder->stEncodeConfig.encodeCodecConfig.hevcConfig.idrPeriod = iKeyInterval;
        if ((EN_PIXEL_FORMAT_YUV420_10BIT == enPixelFormat) || (EN_PIXEL_FORMAT_YUV444_10BIT == enPixelFormat)) {
            pstMwnvEncoder->stEncodeConfig.profileGUID = NV_ENC_HEVC_PROFILE_MAIN10_GUID;
            pstMwnvEncoder->stEncodeConfig.encodeCodecConfig.hevcConfig.pixelBitDepthMinus8 = 2;
        }
        else {
            pstMwnvEncoder->stEncodeConfig.profileGUID = NV_ENC_HEVC_PROFILE_MAIN_GUID;
        }
    }
    else {
        pstMwnvEncoder->stEncodeConfig.encodeCodecConfig.h264Config.repeatSPSPPS = 1;
        pstMwnvEncoder->stEncodeConfig.encodeCodecConfig.h264Config.idrPeriod = iKeyInterval;
        if (enCodecProfile == EN_CODEC_PROFILE_BASELINE) {
            pstMwnvEncoder->stEncodeConfig.profileGUID = NV_ENC_H264_PROFILE_BASELINE_GUID;
        }
        else if (enCodecProfile == EN_CODEC_PROFILE_MAIN) {
            pstMwnvEncoder->stEncodeConfig.profileGUID = NV_ENC_H264_PROFILE_MAIN_GUID;
        }
        else {
            pstMwnvEncoder->stEncodeConfig.profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
        }
    }

    if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncInitializeEncoder(pstMwnvEncoder->hEncoder, &(pstMwnvEncoder->stEncodeInitParams))){
        printf("nvEncInitializeEncoder fail\n");
        return NULL;
    }

    MWNVCreateInAndOutBuffer(pstMwnvEncoder, enPixelFormat);
    return (HMWNVENC)(pstMwnvEncoder->uiChId);
}
ST_MWNV_ENC_CTXT *MWNVGetEncoderByHandle(HMWNVENC hMWNVEnc)
{
    unsigned int uiChId = (unsigned int)hMWNVEnc;
    ST_MWNV_ENC_CTXT *pstMwnvEncoder = NULL;
    if (uiChId == 0){
        printf("invalid handle\n");
        return NULL;
    }
    pstMwnvEncoder = g_apstMwnvEncoder[uiChId % MWNV_MAX_ENCODER_NUM];
    if ((0 == pstMwnvEncoder->ucIsUsing) || (pstMwnvEncoder->uiChId != uiChId)){
        printf("error handle\n");
        return NULL;
    }
    return pstMwnvEncoder;
}
int MWNVCloseEncoder(HMWNVENC hMWNVEnc)
{
    int i;
    ST_MWNV_ENC_CTXT *pstMwnvEncoder = MWNVGetEncoderByHandle(hMWNVEnc);
    if (pstMwnvEncoder == NULL){
        return -1;
    }
    for (i = 0; i < pstMwnvEncoder->iInCacheNum; i++){
        g_pEncodeAPI->nvEncDestroyInputBuffer(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astInCache[i].hInputSurface);
        pstMwnvEncoder->astInCache[i].hInputSurface = NULL;
    }

    for (i = 0; i < pstMwnvEncoder->iOutCacheNum; i++){
        g_pEncodeAPI->nvEncDestroyBitstreamBuffer(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astOutCache[i].hOutputStream);
        pstMwnvEncoder->astOutCache[i].hOutputStream = NULL;
        
        if (pstMwnvEncoder->astOutCache[i].hOutPutEvent) {
            NV_ENC_EVENT_PARAMS eventParams;
            memset(&eventParams, 0, sizeof(eventParams));
            SET_VER(eventParams, NV_ENC_EVENT_PARAMS);
            eventParams.completionEvent = pstMwnvEncoder->astOutCache[i].hOutPutEvent;
            g_pEncodeAPI->nvEncUnregisterAsyncEvent(pstMwnvEncoder->hEncoder, &eventParams);
        }
        pstMwnvEncoder->astOutCache[i].hOutPutEvent = NULL;
    }

    g_pEncodeAPI->nvEncDestroyEncoder(pstMwnvEncoder->hEncoder);
    //todo
    //FreeLibrary(hInstLib);
    //cuCtxDestroy((CUcontext)g_apDevice);
    pstMwnvEncoder->ucIsUsing = 0;
    return 0;
}

int MWNVCopyRawToInBuf(void *hEncoder, NV_ENC_INPUT_PTR hInputSurface, NV_ENC_BUFFER_FORMAT enEncBufferFormat, unsigned char *pucInFrame, int iWidth, int iHeight)
{   
    int i;
    NV_ENC_LOCK_INPUT_BUFFER stLockInputBufferParams;
    unsigned char *pucDataPtr;
    memset(&stLockInputBufferParams, 0, sizeof(stLockInputBufferParams));
    SET_VER(stLockInputBufferParams, NV_ENC_LOCK_INPUT_BUFFER);

    stLockInputBufferParams.inputBuffer = hInputSurface;
    if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncLockInputBuffer(hEncoder, &stLockInputBufferParams)){
        printf("lock inbuf fail");
        return -1;
    }
    pucDataPtr = (unsigned char *)stLockInputBufferParams.bufferDataPtr;
    if (NV_ENC_BUFFER_FORMAT_NV12 == enEncBufferFormat) {
        int iCopyTimes = 3 * iHeight / 2;
        for (i = 0; i < iCopyTimes; i++){
            memcpy(pucDataPtr, pucInFrame, iWidth);
            pucInFrame += iWidth;
            pucDataPtr += stLockInputBufferParams.pitch;
        }
    }
    else if ((NV_ENC_BUFFER_FORMAT_ARGB == enEncBufferFormat) || (NV_ENC_BUFFER_FORMAT_ABGR == enEncBufferFormat)){
        int iCopyTimes = iHeight;
        int iCopyStride = 4 * iWidth;
        for (i = 0; i < iCopyTimes; i++){
            memcpy(pucDataPtr, pucInFrame, iCopyStride);
            pucInFrame += iCopyStride;
            pucDataPtr += stLockInputBufferParams.pitch;
        }
    }
    else if (NV_ENC_BUFFER_FORMAT_YUV420_10BIT == enEncBufferFormat){
        int iCopyTimes = 3 * iHeight / 2;
        int iCopyStride = 2 * iWidth;
        for (i = 0; i < iCopyTimes; i++){
            memcpy(pucDataPtr, pucInFrame, iCopyStride);
            pucInFrame += iCopyStride;
            pucDataPtr += stLockInputBufferParams.pitch;
        }
    }
    if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncUnlockInputBuffer(hEncoder, hInputSurface)){
        printf("unlock inbuf fail");
        return -1;
    }
    return 0;
}

int MWNVEncodeInBuf(void *hEncoder, NV_ENC_INPUT_PTR hInputSurface, NV_ENC_BUFFER_FORMAT enEncBufferFormat, NV_ENC_OUTPUT_PTR hOutStream, void *pEvent, int iWidth, int iHeight)
{
    NV_ENC_PIC_PARAMS stPicParams;
    NVENCSTATUS nvStatus;
    memset(&stPicParams, 0, sizeof(stPicParams));
    SET_VER(stPicParams, NV_ENC_PIC_PARAMS);


    stPicParams.inputBuffer = hInputSurface;
    stPicParams.bufferFmt = enEncBufferFormat;
    stPicParams.inputWidth = iWidth;
    stPicParams.inputHeight = iHeight;
    stPicParams.outputBitstream = hOutStream;
    stPicParams.completionEvent = pEvent;
    stPicParams.inputTimeStamp = 0;
    stPicParams.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
    stPicParams.qpDeltaMap = NULL;
    stPicParams.qpDeltaMapSize = 0;

    nvStatus = g_pEncodeAPI->nvEncEncodePicture(hEncoder, &stPicParams);
    if (nvStatus == NV_ENC_ERR_NEED_MORE_INPUT){
        printf("need input frame\n");
        return 1;
    }
    else if (nvStatus == NV_ENC_SUCCESS){
        return 0;
    }
    printf("enc fail nvStatus[%d]", nvStatus);
    return -1;
}
int MWNVGetOutStream(void *hEncoder, NV_ENC_OUTPUT_PTR hOutStream, void *pEvent, unsigned char **ppucOutFrame, unsigned int *puiOutFrameLen)
{
    NV_ENC_LOCK_BITSTREAM stLockBitstreamData;
    if (pEvent)
    {
        //to do
#if defined(_WIN32)
        WaitForSingleObject(pEvent, INFINITE);
#endif
    }


    memset(&stLockBitstreamData, 0, sizeof(stLockBitstreamData));
    SET_VER(stLockBitstreamData, NV_ENC_LOCK_BITSTREAM);
    stLockBitstreamData.outputBitstream = hOutStream;
    stLockBitstreamData.doNotWait = false;

    if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncLockBitstream(hEncoder, &stLockBitstreamData)){
        printf("lock outbuf fail");
        return -1;
    }
    *ppucOutFrame = (uint8_t *)stLockBitstreamData.bitstreamBufferPtr;
    *puiOutFrameLen = stLockBitstreamData.bitstreamSizeInBytes;
    return 0;
}
int MWNVEncodeFrameSync(HMWNVENC hMWNVEnc, unsigned char *pucInFrame, unsigned char **ppucOutFrame, unsigned int *puiOutFrameLen)
{
    int iRet;
    ST_MWNV_ENC_CTXT *pstMwnvEncoder = MWNVGetEncoderByHandle(hMWNVEnc);
    if (pstMwnvEncoder == NULL){
        return -1;
    }
    if ((pucInFrame == NULL) || (ppucOutFrame == NULL) || (puiOutFrameLen == NULL)){
        printf("err inparm");
        return -2;
    }
    if (pstMwnvEncoder->ucNeedUnlockOutBuffer){
        pstMwnvEncoder->ucNeedUnlockOutBuffer = 0;
        if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncUnlockBitstream(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astOutCache[0].hOutputStream)){
            printf("unlock outbuf fail");
            return -3;
        }
    }
    *puiOutFrameLen = 0;
    if (MWNVCopyRawToInBuf(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astInCache[0].hInputSurface, pstMwnvEncoder->enEncBufferFormat, pucInFrame,
        pstMwnvEncoder->stEncodeInitParams.encodeWidth, pstMwnvEncoder->stEncodeInitParams.maxEncodeHeight) < 0){
        return -4;
    }
    iRet = MWNVEncodeInBuf(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astInCache[0].hInputSurface, pstMwnvEncoder->enEncBufferFormat, 
        pstMwnvEncoder->astOutCache[0].hOutputStream, pstMwnvEncoder->astOutCache[0].hOutPutEvent, pstMwnvEncoder->stEncodeInitParams.encodeWidth, 
        pstMwnvEncoder->stEncodeInitParams.maxEncodeHeight);
    if (iRet < 0){
        return -5;
    }
    pstMwnvEncoder->uiInFramNum++;
    if (iRet > 0)
        return 0;
    if (MWNVGetOutStream(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astOutCache[0].hOutputStream, pstMwnvEncoder->astOutCache[0].hOutPutEvent,
        ppucOutFrame, puiOutFrameLen) < 0){
        return -6;
    }
    pstMwnvEncoder->uiOutFramNum++;
    pstMwnvEncoder->ucNeedUnlockOutBuffer = 1;
    return 0;
}

int MWNVEncodeFrameAsync(HMWNVENC hMWNVEnc, unsigned char *pucInFrame)
{
    int iRet;
    int iInCacheIndex = 0;
    int iOutCacheIndex = 0;
    ST_MWNV_ENC_CTXT *pstMwnvEncoder = MWNVGetEncoderByHandle(hMWNVEnc);
    if (pstMwnvEncoder == NULL){
        return -1;
    }
    if (pucInFrame == NULL){
        printf("err inparm");
        return -2;
    }

    iInCacheIndex = pstMwnvEncoder->uiInFramNum % pstMwnvEncoder->iInCacheNum;
    iOutCacheIndex = pstMwnvEncoder->uiInFramNum % pstMwnvEncoder->iOutCacheNum;
    while (1)
    {
        if ((pstMwnvEncoder->astInCache[iInCacheIndex].ucBUsing == 0) && (pstMwnvEncoder->astOutCache[iOutCacheIndex].ucBUsing == 0)){
            break;
        }
    }
    if (MWNVCopyRawToInBuf(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astInCache[iInCacheIndex].hInputSurface, pstMwnvEncoder->enEncBufferFormat, pucInFrame,
        pstMwnvEncoder->stEncodeInitParams.encodeWidth, pstMwnvEncoder->stEncodeInitParams.maxEncodeHeight) < 0){
        return -4;
    }
    iRet = MWNVEncodeInBuf(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astInCache[iInCacheIndex].hInputSurface, pstMwnvEncoder->enEncBufferFormat,
        pstMwnvEncoder->astOutCache[iOutCacheIndex].hOutputStream, pstMwnvEncoder->astOutCache[iOutCacheIndex].hOutPutEvent, pstMwnvEncoder->stEncodeInitParams.encodeWidth,
        pstMwnvEncoder->stEncodeInitParams.maxEncodeHeight);
    if (iRet < 0){
        return -5;
    }
    pstMwnvEncoder->astInCache[iInCacheIndex].ucBUsing = 1;
    pstMwnvEncoder->astOutCache[iOutCacheIndex].ucBUsing = 1;
    pstMwnvEncoder->uiInFramNum++;
    if (iRet > 0)
        return 0;
    return 1;
}

int MWNVGetBitStream(HMWNVENC hMWNVEnc, unsigned char **ppucOutFrame, unsigned int *puiOutFrameLen)
{
    int iInCacheIndex = 0;
    int iOutCacheIndex = 0;
    ST_MWNV_ENC_CTXT *pstMwnvEncoder = MWNVGetEncoderByHandle(hMWNVEnc);
    if (pstMwnvEncoder == NULL){
        return -1;
    }
    if ((ppucOutFrame == NULL) || (puiOutFrameLen == NULL)){
        printf("err inparm");
        return -2;
    }
    *puiOutFrameLen = 0;

    if (pstMwnvEncoder->ucNeedUnlockOutBuffer){
        iOutCacheIndex = (pstMwnvEncoder->uiOutFramNum - 1) % pstMwnvEncoder->iOutCacheNum;
        pstMwnvEncoder->ucNeedUnlockOutBuffer = 0;
        if (NV_ENC_SUCCESS != g_pEncodeAPI->nvEncUnlockBitstream(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astOutCache[iOutCacheIndex].hOutputStream)){
            printf("unlock outbuf fail");
            return -3;
        }
        pstMwnvEncoder->astOutCache[iOutCacheIndex].ucBUsing = 0;
    }
    if (pstMwnvEncoder->uiInFramNum <= pstMwnvEncoder->uiOutFramNum){
        return 1;
    }
    iOutCacheIndex = pstMwnvEncoder->uiOutFramNum % pstMwnvEncoder->iOutCacheNum;
    if (MWNVGetOutStream(pstMwnvEncoder->hEncoder, pstMwnvEncoder->astOutCache[iOutCacheIndex].hOutputStream, pstMwnvEncoder->astOutCache[iOutCacheIndex].hOutPutEvent,
        ppucOutFrame, puiOutFrameLen) < 0){
        return -6;
    }
    iInCacheIndex = pstMwnvEncoder->uiOutFramNum % pstMwnvEncoder->iInCacheNum;
    pstMwnvEncoder->astInCache[iInCacheIndex].ucBUsing = 0;
    pstMwnvEncoder->uiOutFramNum++;
    pstMwnvEncoder->ucNeedUnlockOutBuffer = 1;
    return 0;
}