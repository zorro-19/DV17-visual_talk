#include "generic/typedef.h"
#include "asm/cpu.h"
#include "asm/dsi.h"
#include "asm/lcd_config.h"
#include "device/lcd_driver.h"
#include "gpio.h"

#ifdef LCD_DSI_DCS_4LANE_1080P
//------------------------------------------------------//
// lcd command initial
//------------------------------------------------------//
//command list
const static u8 init_cmd_list[] = {
    _R, DELAY(0), PACKET_DCS, SIZE(2), 0x0A, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xFF, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xFB, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x00, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x01, 0x55,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x02, 0x59,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x04, 0x0C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x05, 0x3B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x06, 0x6E,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x07, 0xC6,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0D, 0xBB,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0E, 0xBB,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0F, 0xE0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x10, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x11, 0x64,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x12, 0x5A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x13, 0x64,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x14, 0x64,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x15, 0x60,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x16, 0x16,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x17, 0x16,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x75, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x76, 0x0A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x77, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x78, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x79, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7A, 0x4D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7B, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7C, 0x66,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7D, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7E, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7F, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x80, 0x93,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x81, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x82, 0xA7,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x83, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x84, 0xB8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x85, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x86, 0xC8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x87, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x88, 0xFC,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x89, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8A, 0x25,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8B, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8C, 0x60,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8D, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8E, 0x92,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8F, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x90, 0xDE,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x91, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x92, 0x1A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x93, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x94, 0x1C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x95, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x96, 0x52,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x97, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x98, 0x8B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x99, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9A, 0xB0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9B, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9C, 0xE1,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9D, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9E, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9F, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA0, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA2, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA3, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA4, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA5, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA6, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA7, 0x63,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA9, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAA, 0x74,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAB, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAC, 0x86,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAD, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAE, 0x9A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAF, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB0, 0xAF,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB1, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB2, 0xB9,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB3, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB4, 0x0A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB5, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB6, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB7, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB8, 0x4D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB9, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBA, 0x66,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBB, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBC, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBD, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBE, 0x93,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBF, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC0, 0xA7,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC1, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC2, 0xB8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC3, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC4, 0xC8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC5, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC6, 0xFC,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC7, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC8, 0x25,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC9, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCA, 0x60,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCB, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCC, 0x92,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCE, 0xDE,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCF, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD0, 0x1A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD1, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD2, 0x1C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD3, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD4, 0x52,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD5, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD6, 0x8B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD7, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD8, 0xB0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD9, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDA, 0xE1,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDB, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDC, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDD, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDE, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDF, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE0, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE1, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE2, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE3, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE4, 0x63,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE5, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE6, 0x74,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE7, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE8, 0x86,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE9, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEA, 0x9A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEB, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEC, 0xAF,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xED, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEE, 0xB9,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEF, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF0, 0x0A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF1, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF2, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF3, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF4, 0x4D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF5, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF6, 0x66,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF7, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF8, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xF9, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xFA, 0x93,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xFF, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xFB, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x00, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x01, 0xA7,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x02, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x03, 0xB8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x04, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x05, 0xC8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x06, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x07, 0xFC,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x08, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x09, 0x25,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0A, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0B, 0x60,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0C, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0D, 0x92,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0E, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0F, 0xDE,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x10, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x11, 0x1A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x12, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x13, 0x1C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x14, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x15, 0x52,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x16, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x17, 0x8B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x18, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x19, 0xB0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1A, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1B, 0xE1,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1C, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1D, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1E, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1F, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x20, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x21, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x22, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x23, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x24, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x25, 0x63,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x26, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x27, 0x74,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x28, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x29, 0x86,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2A, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2B, 0x9A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2D, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2F, 0xAF,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x30, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x31, 0xB9,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x32, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x33, 0x0A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x34, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x35, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x36, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x37, 0x4D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x38, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x39, 0x66,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3B, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3D, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3F, 0x93,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x40, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x41, 0xA7,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x42, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x43, 0xB8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x44, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x45, 0xC8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x46, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x47, 0xFC,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x48, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x49, 0x25,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4A, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4B, 0x60,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4C, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4D, 0x92,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4E, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4F, 0xDE,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x50, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x51, 0x1A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x52, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x53, 0x1C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x54, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x55, 0x52,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x56, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x58, 0x8B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x59, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5A, 0xB0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5B, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5C, 0xE1,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5D, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5E, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5F, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x60, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x61, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x62, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x63, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x64, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x65, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x66, 0x63,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x67, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x68, 0x74,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x69, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6A, 0x86,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6B, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6C, 0x9A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6D, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6E, 0xAF,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6F, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x70, 0xB9,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x71, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x72, 0x0A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x73, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x74, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x75, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x76, 0x4D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x77, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x78, 0x66,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x79, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7A, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7B, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7C, 0x93,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7D, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7E, 0xA7,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7F, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x80, 0xB8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x81, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x82, 0xC8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x83, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x84, 0xFC,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x85, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x86, 0x25,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x87, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x88, 0x60,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x89, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8A, 0x92,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8B, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8C, 0xDE,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8D, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8E, 0x1A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x8F, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x90, 0x1C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x91, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x92, 0x52,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x93, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x94, 0x8B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x95, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x96, 0xB0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x97, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x98, 0xE1,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x99, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9A, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9B, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9C, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9D, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9E, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9F, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA0, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA2, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA3, 0x63,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA4, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA5, 0x74,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA6, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA7, 0x86,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xA9, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAA, 0x9A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAB, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAC, 0xAF,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAD, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAE, 0xB9,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xAF, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB0, 0x0A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB1, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB2, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB3, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB4, 0x4D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB5, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB6, 0x66,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB7, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB8, 0x80,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xB9, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBA, 0x93,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBB, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBC, 0xA7,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBD, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBE, 0xB8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBF, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC0, 0xC8,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC1, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC2, 0xFC,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC3, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC4, 0x25,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC5, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC6, 0x60,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC7, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC8, 0x92,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC9, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCA, 0xDE,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCB, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCC, 0x1A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCD, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCE, 0x1C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCF, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD0, 0x52,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD1, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD2, 0x8B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD3, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD4, 0xB0,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD5, 0x02,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD6, 0xE1,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD7, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD8, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD9, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDA, 0x35,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDB, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDC, 0x44,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDD, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDE, 0x53,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xDF, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE0, 0x63,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE1, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE2, 0x74,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE3, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE4, 0x86,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE5, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE6, 0x9A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE7, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE8, 0xAF,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xE9, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xEA, 0xB9,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xFF, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xFB, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x00, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x01, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x02, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x03, 0x06,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x04, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x05, 0x16,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x06, 0x18,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x07, 0x1A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x08, 0x1C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x09, 0x1E,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0A, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0B, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0C, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0D, 0x26,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0E, 0x28,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x0F, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x10, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x11, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x12, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x13, 0x0E,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x14, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x15, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x16, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x17, 0x07,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x18, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x19, 0x17,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1A, 0x19,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1B, 0x1B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1C, 0x1D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1D, 0x1F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1E, 0x21,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x1F, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x20, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x21, 0x26,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x22, 0x28,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x23, 0x09,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x24, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x25, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x26, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x27, 0x0F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x28, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x29, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2A, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2B, 0x09,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2D, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x2F, 0x19,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x30, 0x17,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x31, 0x21,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x32, 0x1F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x33, 0x1D,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x34, 0x1B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x35, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x36, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x37, 0x26,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x38, 0x28,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x39, 0x07,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3A, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3B, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3D, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x3F, 0x0F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x40, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x41, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x42, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x43, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x44, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x45, 0x18,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x46, 0x16,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x47, 0x20,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x48, 0x1E,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x49, 0x1C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4A, 0x1A,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4B, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4C, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4D, 0x26,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4E, 0x28,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x4F, 0x06,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x50, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x51, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x52, 0x38,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x53, 0x0E,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x54, 0x07,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x55, 0x19,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x59, 0x24,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5B, 0x69,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5C, 0x12,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5D, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x5E, 0x22,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x62, 0x21,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x63, 0x69,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x64, 0x12,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x66, 0x57,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x67, 0x11,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x68, 0x2B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x69, 0x12,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6A, 0x05,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6B, 0x29,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6C, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6D, 0x18,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x6F, 0x3C,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x70, 0x03,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x72, 0x22,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x73, 0x22,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7D, 0x01,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7E, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x7F, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x80, 0xAA,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x81, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x85, 0x3F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x86, 0x3F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBD, 0xA6,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBE, 0x08,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xBF, 0x12,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC8, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xC9, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCA, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCB, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCC, 0x09,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCE, 0x18,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xCF, 0x88,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD0, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD1, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD2, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD3, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD4, 0x40,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x90, 0x7B,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x91, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x92, 0x10,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x98, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x99, 0x00,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0x9F, 0x0F,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD7, 0x31,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xD8, 0x7E,
    _W, DELAY(0), PACKET_DCS, SIZE(2), 0xFF, 0x00,
    /* _W,DELAY(0),PACKET_DCS, SIZE(2),0xD3,0x0A, */
    /* _W,DELAY(0),PACKET_DCS, SIZE(2),0xD4,0x10, */
    /* _W,DELAY(0),PACKET_DCS, SIZE(2),0xD5,0x24, */
    /* _W,DELAY(0),PACKET_DCS, SIZE(2),0xD6,0x40, */
    _W, DELAY(20), PACKET_DCS, SIZE(2), 0x11,
    _W, DELAY(4), PACKET_DCS, SIZE(2), 0x29,
    _R, DELAY(0), PACKET_DCS, SIZE(2), 0x0A, 0x01,
    _R, DELAY(0), PACKET_DCS, SIZE(2), 0x0C, 0x01,
};

#define freq 1000

#define lane_num 4
#define bpp_num  24

#define vsa_line 1
#define vbp_line 15
#define vda_line 1920
#define vfp_line 16

#define hsa_pixel  8
#define hbp_pixel  64
#define hda_pixel  1080
#define hfp_pixel  64


REGISTER_MIPI_DEVICE_BEGIN(mipi_dev_t) = {
    .info = {
        .xres 			= LCD_DEV_WIDTH,
        .yres 			= LCD_DEV_HIGHT,
        .target_xres 	 = LCD_DEV_WIDTH,
        .target_yres 	 = LCD_DEV_HIGHT,
        .buf_addr 		= LCD_DEV_BUF,
        .buf_num 		= LCD_DEV_BNUM,
        .sample         = LCD_DEV_SAMPLE,
        .test_mode 		= false,
        .test_mode_color = 0x0000ff,
        .canvas_color = 0x000000,
        .format 		= FORMAT_RGB888,
        .len 			= LEN_256,

        .rotate_en 		= true,			// 旋转使能
        .hori_mirror_en = true,			// 水平镜像使能
        .vert_mirror_en = false,		// 垂直镜像使能

        .adjust = {
            .y_gain = 0x100,
            .u_gain = 0x100,
            .v_gain = 0x100,
            .r_gain = 0x80,
            .g_gain = 0x80,
            .b_gain = 0x80,
            .r_coe0 = 0x80,
            .g_coe1 = 0x80,
            .b_coe2 = 0x80,
            .r_gma  = 100,
            .g_gma  = 100,
            .b_gma  = 100,
        },
    },
    .lane_mapping = {
        .x0_lane = MIPI_LANE_EN | MIPI_LANE_EX | MIPI_LANE_CLK,
        .x1_lane = MIPI_LANE_EN | MIPI_LANE_EX | MIPI_LANE_D0,
        .x2_lane = MIPI_LANE_EN | MIPI_LANE_EX | MIPI_LANE_D1,
        .x3_lane = MIPI_LANE_EN | MIPI_LANE_EX | MIPI_LANE_D2,
        .x4_lane = MIPI_LANE_EN | MIPI_LANE_EX | MIPI_LANE_D3,
    },
    .video_timing = {
        .video_mode = VIDEO_STREAM_VIDEO,
        .sync_mode  = SYNC_PULSE_MODE,
        .color_mode = COLOR_FORMAT_RGB888,
        .pixel_type = PIXEL_RGB888,
        .virtual_ch   = 0,
        .hs_eotp_en = true,

        .dsi_vdo_hsa_v   = ((bpp_num * hsa_pixel) / 8) - 10,
        .dsi_vdo_hbp_v   = ((bpp_num * hbp_pixel) / 8) - 10,
        .dsi_vdo_hact_v  = ((bpp_num * hda_pixel) / 8),
        .dsi_vdo_hfp_v   = ((bpp_num * hfp_pixel) / 8) - 6,
        .dsi_vdo_bllp0_v = ((bpp_num * (hbp_pixel + hda_pixel + hfp_pixel) / 8) - 10),
        .dsi_vdo_bllp1_v = ((bpp_num * hda_pixel) / 8),
    },
    .timing = {
        .tval_lpx   = ((80     * freq / 1000) / 2 - 1),
        .tval_wkup  = ((100000 * freq / 1000) / 8 - 1),
        .tval_c_pre = ((40     * freq / 1000) / 2 - 1),
        .tval_c_sot = ((300    * freq / 1000) / 2 - 1),
        .tval_c_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_c_brk = ((150    * freq / 1000) / 2 - 1),
        .tval_d_pre = ((60     * freq / 1000) / 2 - 1),
        .tval_d_sot = ((160    * freq / 1000) / 2 - 1),
        .tval_d_eot = ((100    * freq / 1000) / 2 - 1),
        .tval_d_brk = ((150    * freq / 1000) / 2 - 1),
        .tval_c_rdy = 400/* 64 */,
    },
    .pll4 = {
        .pll_freq = 1000,
        .source = PLL4_SRC_x12M,
    },

    .cmd_list = init_cmd_list,
    .cmd_list_item = sizeof(init_cmd_list),
    .debug_mode = false,
},
REGISTER_MIPI_DEVICE_END()
//------------------------------------------------------//
// dsi run
//------------------------------------------------------//
static int dsi_init(void *_data)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;
    u8 lcd_reset = data->lcd_io.lcd_reset;

    /*
     * lcd reset
     */
    if ((u8) - 1 != lcd_reset) {
        gpio_direction_output(lcd_reset, 0);
        delay_2ms(5);
        gpio_direction_output(lcd_reset, 1);
        delay_2ms(5);
    }


    return 0;
}

static void mipi_backlight_ctrl(void *_data, u8 on)
{
    struct lcd_platform_data *data = (struct lcd_platform_data *)_data;

    if (on) {
        gpio_direction_output(data->lcd_io.backlight, data->lcd_io.backlight_value);
    } else {
        gpio_direction_output(data->lcd_io.backlight, !data->lcd_io.backlight_value);
    }
}

REGISTER_LCD_DEVICE_DRIVE(dev)  = {
    .type = LCD_MIPI,
    .init = dsi_init,
    .dev  = &mipi_dev_t,
    .bl_ctrl = mipi_backlight_ctrl,
};
#endif
