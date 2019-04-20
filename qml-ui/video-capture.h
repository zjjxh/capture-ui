#ifndef VIDEO_CAPTURE_H_
#define VIDEO_CAPTURE_H_

#include <stdint.h>

typedef enum _COLORSPACE_ID {
    CS_UNKNOWN = 0,
    NTSC_1953,
    PAL_1970,
    NTSC_1987,
    ITU601,
    ITU601_5,
    sRGB,
    ITU709,
    ITU709_5,
    ITU709_7,
    xvYCC601,
    xvYCC709,
    sYCC601,
    opYCC601,
    opRGB,
    DCI_P3,
    ITU2020,
    ITU2020C,
} CS_ID;

static const char *const CS_NAME[] = {
    "Unknown",
    "NTSC-1953",
    "PAL/SECAM-1970",
    "NTSC-1987/SMPTE 170M-1",
    "ITU601",
    "ITU601-5",
    "sRGB",
    "ITU709",
    "ITU709-5",
    "ITU709-7",
    "xvYCC601",
    "xvYCC709",
    "sYCC601",
    "opYCC601",
    "opRGB",
    "DCI-P3",
    "ITU2020",
    "ITU2020C",
};

/*
 * card: card index.
 * base: saving name base. if cnt == 0 && !need_bmp, ignored.
 * cnt: capture cnt. if cnt == 0, just fresh meta-info.
 * need_bmp: if true, save a bmp file.
 */
void fresh_capture(uint8_t card, const char *base, unsigned cnt, bool need_bmp);

uint32_t get_capture_fourcc();
uint32_t get_capture_width();
uint32_t get_capture_height();
uint32_t get_capture_cs_id();
const char *get_capture_inputinfo();

#endif
