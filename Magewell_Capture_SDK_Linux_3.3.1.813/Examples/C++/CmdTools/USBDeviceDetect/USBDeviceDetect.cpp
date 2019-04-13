/************************************************************************************************/
// USBDeviceDetect.cpp : Defines the entry point for the console application.

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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "LibMWCapture/MWCapture.h"
#include "LibMWCapture/MWUSBCapture.h"


typedef void(*LPFN_HOT_PLUG_CALLBACK)(MWUSBHOT_PLUG_EVETN event, const char *pszDevicePath, void* pParam);

static void HotplugCheckCallback(MWUSBHOT_PLUG_EVETN event, const char *pszDevicePath, void* pParam)
{
    switch(event)
    {
    case USBHOT_PLUG_EVENT_DEVICE_ARRIVED:
        printf("\nDevice reconnect\n");
        break;
    case USBHOT_PLUG_EVENT_DEVICE_LEFT:
        printf("\nDevice disconnect\n");
        break;
    default:
        break;
    }
}

int main(int argc, char* argv[])
{
    // Version
    BYTE byMaj, byMin;
    WORD wBuild;
    MWGetVersion(&byMaj, &byMin, &wBuild);
    printf("Magewell MWCapture SDK V%d.%d.%d - USBDeviceDetect\n",byMaj,byMin,wBuild);
    printf("Only USB devices are supported\n");
    printf("Usage:\n");
    printf("USBDeviceDetect <channel index>\n");

    if (argc > 2) {
        printf("ERROR: Invalid params!\n");
        printf("\nPress 'Enter' to exit!\n");
        getchar();
        return 0;
    }

    MWCaptureInitInstance();

    HCHANNEL hChannel = NULL;
    MW_RESULT mr = MW_SUCCEEDED;

    do
    {
        MWRefreshDevice();
        int nCount = MWGetChannelCount();
		if (nCount <= 0){
            printf("ERROR: Can't find channels!\n");
            break;
        }
        printf("Log: Find %d channels!\n",nCount);
        int nUsbCount = 0;
        int nUsbDevice[16] = {-1};
        for (int i = 0; i < nCount; i++){
            MWCAP_CHANNEL_INFO info;
            mr = MWGetChannelInfoByIndex(i, &info);
            if (strcmp(info.szFamilyName, "USB Capture") == 0) {
                nUsbDevice[nUsbCount] = i;
                nUsbCount ++;
            }
        }

        if (nUsbCount <= 0){
            printf("\nERROR: Can't find usb device !\n");
            break;
        }

        int nDevIndex = -1;

        MWCAP_CHANNEL_INFO videoInfo = { 0 };
        if (argc == 1) {
            nDevIndex = 0;
        }
        else {
            if (strlen(argv[1]) > 2)
                nDevIndex = -1;
            else if (strlen(argv[1]) == 2) {
                if ((argv[1][0] >= '0' && argv[1][0] <= '9') && (argv[1][1] >= '0' && argv[1][1] <= '9'))
                    nDevIndex = atoi(argv[1]);
                else
                    nDevIndex = -1;
            }
            else if (strlen(argv[1]) == 1)
                nDevIndex = (argv[1][0] >= '0' && argv[1][0] <= '9') ? atoi(argv[1]) : -1;

            if (nDevIndex < 0) {
                printf("\nERROR: Invalid params!\n");
                break;
            }
            if(nDevIndex >= nUsbCount){
                printf("ERROR: just have %d channel!\n",nUsbCount);
                break;
            }
        }

        MW_RESULT mr = MW_SUCCEEDED;

        MWCAP_CHANNEL_INFO infoDevice;
        mr = MWGetChannelInfoByIndex(nUsbDevice[nDevIndex], &infoDevice);
        printf("Family name: %s\n", infoDevice.szFamilyName);
        printf("Product name: %s\n", infoDevice.szProductName);
        printf("Serial number: %s\n", infoDevice.szBoardSerialNo);
        printf("Firmware version: %d.%d.%d\n", (infoDevice.dwDriverVersion >> 24) , (infoDevice.dwDriverVersion >> 16) & 0x00ff, infoDevice.dwDriverVersion & 0xffff);

        char wPath[256] = {0};
        mr = MWGetDevicePath(nUsbDevice[nDevIndex], wPath);

        hChannel = MWOpenChannelByPath(wPath);

        mr = MWUSBRegisterHotPlug(HotplugCheckCallback, NULL);
        if (mr != MW_SUCCEEDED) {
            printf("ERROR: Set usb device detect event failed\n");
            break;
        }

        printf("\nPlease disconnect and reconnect the specific usb device\n");

        usleep(10000000);
        MWUSBUnRegisterHotPlug();
    } while (FALSE);

    if (hChannel != NULL) {
        MWCloseChannel(hChannel);
        hChannel = NULL;
    }

    MWCaptureExitInstance();

    printf("\nPress 'Enter' to exit!\n");
    getchar();

    return 0;
}

