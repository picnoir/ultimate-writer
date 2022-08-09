/* See LICENSE for license details. */
#ifndef SCREEN_75_V2_H
#define SCREEN_75_V2_H

#include "fonts.h"
#include "st.h"

// Display resolution
#define EPD_WIDTH                                   800
#define EPD_HEIGHT                                  480

// Command definition
// From https://www.waveshare.com/w/upload/6/60/7.5inch_e-Paper_V2_Specification.pdf
#define PANEL_SETTING                               0x00
#define POWER_SETTING                               0x01
#define POWER_OFF                                   0x02
#define POWER_OFF_SEQUENCE_SETTING                  0x03
#define POWER_ON                                    0x04
#define POWER_ON_MEASURE                            0x05
#define BOOSTER_SOFT_START                          0x06
#define DEEP_SLEEP                                  0x07
#define DISPLAY_START_TRANSMISSION_1                0x10
#define DATA_STOP                                   0x11
#define DISPLAY_REFRESH                             0x12
#define DISPLAY_START_TRANSMISSION_2                0x13
#define DUAL_SPI                                    0x15
#define AUTO_SEQUENCE                               0x17
#define KW_LUT_OPTION                               0x2B
#define PLL_CONTROL                                 0x30
#define TEMPERATURE_SENSOR_CALIBRATION              0x40
#define TEMPERATURE_SENSOR_SELECTION                0x41
#define TEMPERATURE_SENSOR_WRITE                    0x42
#define TEMPERATURE_SENSOR_READ                     0x43
#define PANEL_BREAK_CHECK                           0x44
#define VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define LOWER_POWER_DETECTION                       0x51
#define END_VOLTAGE_SETTING                         0x52
#define TCON_SETTING                                0x60
#define RESOLUTION_SETTING                          0x61
#define GATE_SOURCE_START_SETTING                   0x65
#define REVISION                                    0x70
#define GET_STATUS                                  0x71
#define AUTO_MEASUREMENT_VCOM                       0x80
#define READ_VCOM_VALUE                             0x81
#define VCOM_DC_SETTING                             0x82
#define PARTIAL_WINDOW                              0x90
#define PARTIAL_IN                                  0x91
#define PARTIAL_OUT                                 0x92
#define PROGRAM_MODE                                0xA0
#define ACTIVE_PROGRAMMING                          0xA1
#define READ_OTP                                    0xA2
#define CASCADE_SETTING                             0xE0
#define POWER_SAVING                                0xE3
#define LVD_VOLTAGE_SELECT                          0xE4
#define FORCE_TEMPERATURE                           0xE5
#define TEMPERATURE_BOUNDRY_PHASE_C2                0xE7

// Undocumented in spec sheet. From https://github.com/waveshare/e-Paper/blob/master/Arduino/epd7in5_V2/epd7in5_V2.cpp
#define LUT_FOR_VCOM                                0x20
#define LUT_WHITE_TO_WHITE                          0x21
#define LUT_BLACK_TO_WHITE                          0x22
#define LUT_WHITE_TO_BLACK                          0x23
#define LUT_BLACK_TO_BLACK                          0x24

extern const int sorientation; 

// E-Ink functions
// Interface-Level primitives.
int init_if(void);
void digital_write(int pin, int value);
int digital_read(int pin);
void delay_ms(unsigned int delayTime);
void spi_transfer(unsigned char data);
// LUTs
void sset_lut(void); // TODO: Unused?
//void sset_fast_lut(void);

extern const unsigned char lut_vcom0[];
extern const unsigned char lut_ww[];
extern const unsigned char lut_bw[];
extern const unsigned char lut_bb[];
extern const unsigned char lut_wb[];

extern const unsigned char lut_vcom0_fast[];
extern const unsigned char lut_ww_fast[];
extern const unsigned char lut_bw_fast[];
extern const unsigned char lut_bb_fast[];
extern const unsigned char lut_wb_fast[];
extern const unsigned char voltage_frame[];
#endif
