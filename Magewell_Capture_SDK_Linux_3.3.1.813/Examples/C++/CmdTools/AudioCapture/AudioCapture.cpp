/************************************************************************************************/
// AudioCapture.cpp : Defines the entry point for the console application.

// MAGEWELL PROPRIETARY INFORMATION

// The following license only applies to head files and library within Magewell’s SDK 
// and not to Magewell’s SDK as a whole. 

// Copyrights © Nanjing Magewell Electronics Co., Ltd. (“Magewell”) All rights reserved.

// Magewell grands to any person who obtains the copy of Magewell’s head files and library 
// the rights,including without limitation, to use, modify, publish, sublicense, distribute
// the Software on the conditions that all the following terms are met:
// - The above copyright notice shall be retained in any circumstances.
// -The following disclaimer shall be included in the software and documentation and/or 
// other materials provided for the purpose of publish, distribution or sublicense.

// THE SOFTWARE IS PROVIDED BY MAGEWELL “AS IS” AND ANY EXPRESS, INCLUDING BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL MAGEWELL BE LIABLE 

// FOR ANY CLAIM, DIRECT OR INDIRECT DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT,
// TORT OR OTHERWISE, ARISING IN ANY WAY OF USING THE SOFTWARE.

// CONTACT INFORMATION:
// SDK@magewell.net
// http://www.magewell.com/
//
/************************************************************************************************/
#include <string.h>

#include <pthread.h>
#include "WaveFile.h"
#include "wchar.h"
#include "stdio.h"
#include "unistd.h"

#include "LibMWCapture/MWCapture.h"

#define AUDIO_CAPTURE_SECONDS 10

static bool g_bAudioCapture = false;
static bool g_bThreadExit = true;
static pthread_t g_pThreadID;

HCHANNEL g_hChannel=NULL;
MWCAP_AUDIO_SIGNAL_STATUS g_audioSignalStatus;

static void *aduio_capture_thread_proc(void *data)
{
    MWCAP_PTR hNotifyEvent = 0;
    HANDLE64 hNotify = 0;
    int nRet;

    do{
        if (g_hChannel == NULL)
            break;

        hNotifyEvent = MWCreateEvent();
        if (hNotifyEvent == 0) {
            printf("Error: Create Notify event error\n");
            break;
        }

        hNotify = MWRegisterNotify(g_hChannel, hNotifyEvent, MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED);
        if (hNotify == 0) {
            printf("Error: Register notify error\n");
            break;
        }

        ULONGLONG ullStatusBit = 0LL;
        MWCAP_AUDIO_SIGNAL_STATUS mass;
        if (MW_SUCCEEDED != MWGetAudioSignalStatus(g_hChannel, &mass)){
            break;
        }
        if (!mass.bLPCM){
            break;
        }
        int nBitDepthInByte = mass.cBitsPerSample / 8;
        int nPresentChannel = 0;
        for (int i = 0; i < 4; i++){
            bool bPresent  = (mass.wChannelValid & (0x01 << i)) ? true:false;
            nPresentChannel += bPresent ? 2 : 0;
        }

        CWaveFile file;
        file.Init("AudioCapture.wav", mass.dwSampleRate, nPresentChannel, nBitDepthInByte * 8);
        BYTE pbFrame[MWCAP_AUDIO_SAMPLES_PER_FRAME * nPresentChannel * nBitDepthInByte];

        while(!g_bThreadExit){
            nRet = MWWaitEvent(hNotifyEvent, 1000);
            if (nRet <= 0){
                continue;
            }
            ullStatusBit=0;
            if (MW_SUCCEEDED != MWGetNotifyStatus(g_hChannel, hNotify, &ullStatusBit)){
                continue;
            }
            if ((ullStatusBit & MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED) == 0){
                continue;
            }
            MWCAP_AUDIO_CAPTURE_FRAME macf;
            nRet = MWCaptureAudioFrame(g_hChannel, &macf);
            if (MW_ENODATA == nRet){
                continue;
            }
            for (int j = 0; j < (nPresentChannel/2); j++){
                for (int i = 0 ; i < MWCAP_AUDIO_SAMPLES_PER_FRAME; i++){
                    int nWritePos = (i * nPresentChannel + j * 2) * nBitDepthInByte;
                    int nReadPos = (i * MWCAP_AUDIO_MAX_NUM_CHANNELS + j);
                    int nReadPos2 = (i * MWCAP_AUDIO_MAX_NUM_CHANNELS + j + MWCAP_AUDIO_MAX_NUM_CHANNELS/2);
                    DWORD dwLeft = macf.adwSamples[nReadPos] >> (32 - nBitDepthInByte * 8);
                    DWORD dwRight = macf.adwSamples[nReadPos2] >> (32 - nBitDepthInByte * 8);
                    memcpy(&pbFrame[nWritePos], &dwLeft, nBitDepthInByte);
                    memcpy(&pbFrame[nWritePos + nBitDepthInByte], &dwRight, nBitDepthInByte);
                }
            }
            if (file.IsOpen()){
                file.Write((const unsigned char *)pbFrame, MWCAP_AUDIO_SAMPLES_PER_FRAME * nPresentChannel * nBitDepthInByte);
            }
        }
        file.Exit();
    }while (false);

    if (hNotify!= 0){
        MWUnregisterNotify(g_hChannel, hNotify);
        hNotify = 0;
    }

    if (hNotifyEvent != 0){
        MWCloseEvent(hNotifyEvent);
        hNotifyEvent = 0;
    }

    return (void *)0;
}

int get_id(char c)
{
    if(c >= '0' && c <= '9')
        return (int)(c - '0');
    if(c >= 'a' && c <= 'f')
        return (int)(c - 'a' + 10);
    if(c >= 'A' && c <= 'F')
        return (int)(c - 'F' + 10);
    return 0;
}

HCHANNEL open_channel(int argc, char* argv[]){
    HCHANNEL hChannel = NULL;
    int nChannelCount = MWGetChannelCount();

    if (0 == nChannelCount) {
        printf("ERROR: Can't find channels!\n");
        return NULL;
    }
    printf("Find %d channels!\n",nChannelCount);
    int nProDevCount = 0;
    int nProDevChannel[32] = {-1};
    for (int i = 0; i < nChannelCount; i++){
        MWCAP_CHANNEL_INFO info;
        MW_RESULT mr = MWGetChannelInfoByIndex(i, &info);
        if (0 == strcmp(info.szFamilyName, "Pro Capture")){
            nProDevChannel[nProDevCount] = i;
            nProDevCount++;
        }
    }
    if (nProDevCount <= 0){
        printf("\nERROR: Can't find pro channels!\n");
        return NULL;
    }

    printf("Find %d pro channels.\n", nProDevCount);

    // Get <board id > <channel id> or <channel index>
    int byBoardId = -1;
    int byChannelId = -1;
    int nDevIndex = -1;
    BOOL bIndex = FALSE;

    MWCAP_CHANNEL_INFO videoInfo = { 0 };
    if (argc == 1) {
        if (MW_SUCCEEDED != MWGetChannelInfoByIndex(nProDevChannel[0], &videoInfo)) {
            printf("ERROR: Can't get channel info!\n");
            return NULL;
        }

        bIndex = TRUE;
        nDevIndex = 0;
    }
    else if (NULL == strstr(argv[1], ":")){
        bIndex = TRUE;
        if ((strlen(argv[1]) > 2) || (argv[1][0] > '9') || argv[1][0] < '0') {
            printf("\nERROR: Invalid params!\n");
            return NULL;
        }
        nDevIndex = atoi(argv[1]);
        if(nDevIndex >= nProDevCount){
            printf("ERROR: just have %d channel!\n",nProDevCount);
            return NULL;
        }
    }
    else{
        bIndex = FALSE;
        if (strlen(argv[1]) == 3){
            if ((argv[1][0] >= '0' && argv[1][0] <= '9') || (argv[1][0] >= 'a' && argv[1][0] <= 'f') || (argv[1][0] >= 'A' && argv[1][0] <= 'F')){
                byBoardId = get_id(argv[1][0]);//atoi(argv[1]);
            }

            if ((argv[1][2] >= '0' && argv[1][2] <= '3')){
                byChannelId = get_id(argv[1][2]);//atoi(&argv[1][2]);
            }
        }

        if (-1 == byBoardId || -1 == byChannelId) {
            printf("\nERROR: Invalid params!\n");
            return NULL;
        }
    }

    // Open channel
    if (bIndex == TRUE){
        char path[128] = {0};
        MWGetDevicePath(nProDevChannel[nDevIndex], path);
        hChannel = MWOpenChannelByPath(path);
        if (hChannel == NULL) {
            printf("ERROR: Open channel %d error!\n", nDevIndex);
            return NULL;
        }
    }
    else{
        hChannel = MWOpenChannel(byBoardId, byChannelId);
        if (hChannel == NULL) {
            printf("ERROR: Open channel %X:%d error!\n", byBoardId, byChannelId);
            return NULL;
        }
    }

    if (MW_SUCCEEDED != MWGetChannelInfo(hChannel, &videoInfo)) {
        printf("ERROR: Can't get channel info!\n");
        return NULL;
    }

    printf("Open channel - BoardIndex = %X, ChannelIndex = %d.\n", videoInfo.byBoardIndex, videoInfo.byChannelIndex);
    printf("Product Name: %s\n", videoInfo.szProductName);
    printf("Board SerialNo: %s\n\n", videoInfo.szBoardSerialNo);
    return hChannel;
}
void print_version_and_useage()
{
    
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.%d - AudioCapture\n",byMaj,byMin,wBuild);
    printf("Only Pro Capture Series Devices are supported\n");
    printf("Usage:\n");
    printf("AudioCapture <channel index>\n");
    printf("AudioCapture <board id>:<channel id>\t(Pro Capture Series Devices)\t\n\n");

}
bool check_file()
{
    FILE* wavFile = NULL;
    wavFile=fopen("temp.wav","wb");
    if (NULL == wavFile){
        printf("ERROR: can't create file on now dir!\n");
        printf("\nPress ENTER to exit...\n");
        getchar();
        return false;
    }
    else{
        fclose(wavFile);
        wavFile = NULL;
        remove("temp.wav");
    }
    return true;
}
int main(int argc, char* argv[])
{
// Version
    print_version_and_useage();
//permission
    if(!check_file()){
        return 0;
    }

    if (argc > 2){
        printf("ERROR: Invalid params!\n");
        printf("\nPress 'Enter' to exit!\n");
        getchar();
        return 0;
    }

    if(!MWCaptureInitInstance()){
        printf("have InitilizeFailed");
    }

    HCHANNEL hChannel = NULL;
    MW_RESULT mr;
    do {
        MWRefreshDevice();
        hChannel = open_channel(argc, argv);
        if(NULL == hChannel){
            break;
        }

        MW_RESULT xr;
        DWORD dwInputCount = 0;
        xr = MWGetAudioInputSourceArray(hChannel, NULL, &dwInputCount);
        if (dwInputCount == 1) {
            printf("ERROR: Can't find audio input!\n");
            break;
        }
        MWCAP_AUDIO_SIGNAL_STATUS audioSignalStatus;
        MWGetAudioSignalStatus(hChannel, &audioSignalStatus);
        if (audioSignalStatus.bChannelStatusValid == FALSE){
            printf("ERROR: Audio signal is invalid\n");
            break;
        }
        char chSupChannels[128] = {0};
        for (int i = 0; i < 4; i++){
            if (audioSignalStatus.wChannelValid & (0x01 << i))
                sprintf(chSupChannels, "%s %d&%d;", chSupChannels, (i * 2 + 1), (i * 2 + 2));
        }

        printf("Audio Signal: bValid = %d, SampleRate = %d, ChannelValid = %s\n",
               audioSignalStatus.bChannelStatusValid, audioSignalStatus.dwSampleRate, chSupChannels);

        // Open Audio Capture
        xr = MWStartAudioCapture(hChannel);
        if (xr != MW_SUCCEEDED) {
            printf("ERROR: Start Audio Capture error!\n");
            break;
        }

        g_bThreadExit=false;
        g_hChannel=hChannel;
        g_audioSignalStatus=audioSignalStatus;
        if(pthread_create(&g_pThreadID, NULL, aduio_capture_thread_proc, NULL)!=0){
            fprintf(stderr, "thread create fail: %m\n");
            MWStopAudioCapture(hChannel);
            g_bThreadExit = true;
            break;
        }
        printf("Start capture audio data -- 0 s");
        fflush(stdout);
        for (int i = 0; i < AUDIO_CAPTURE_SECONDS; i++){
            sleep(1);
            printf("\rStart capture audio data -- %d s", i+1);
            fflush(stdout);
        }
        printf("\n");
        printf("Stop capture audio data\n\n"
               "Audio data is saved in AudioCapture.wav!\n");
        g_bThreadExit = true;
        pthread_join(g_pThreadID, NULL);

        MWStopAudioCapture(hChannel);
        g_bAudioCapture = false;

    } while (FALSE);

    if (hChannel)
        MWCloseChannel(hChannel);

    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}

