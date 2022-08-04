#include "screen.h"
#include "screen_75_v2.h"
#include <bcm2835.h>
#include <stdlib.h>
#include <stdio.h>

// Eink driver functions
//
// Based heavily on screen_75_v1.c and
// https://github.com/waveshare/e-Paper/blob/master/Arduino/epd7in5_V2/epd7in5_V2.cpp
//
//=====================

int sinit(void) {
  if(init_if() != 0){
    exit(-1);
  }
  sreset();

  /* 
   * Internal VDS_EN and VDG_EN.
   * VCOMH=VDH+VCOMDC, VGH=16V, VGL= -16V.
   * */

  ssend_command(POWER_SETTING); 
  ssend_data(0x17); // 1-0=11: internal power
  ssend_data(*(voltage_frame+6));  // VGH&VGL
  ssend_data(*(voltage_frame+1));  // VSH
  ssend_data(*(voltage_frame+2));  //  VSL
  ssend_data(*(voltage_frame+3));  //  VSHR
	
  ssend_command(VCOM_DC_SETTING);
  ssend_data(*(voltage_frame+4));  // VCOM

  ssend_command(BOOSTER_SOFT_START); // Booster setting
  ssend_data(0x27);
  ssend_data(0x27);
  ssend_data(0x2F);
  ssend_data(0x17);

  ssend_command(PLL_CONTROL); // OSC Setting
  ssend_data(*(voltage_frame+0));  // 2-0=100: N=4  ; 5-3=111: M=7  ;  3C=50Hz     3A=100HZ

  ssend_command(POWER_ON);
  //delay_ms(100);
  swait_until_idle();

  ssend_command(PANEL_SETTING);
  ssend_data(0x3F);   //KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

  ssend_command(RESOLUTION_SETTING); //tres
  ssend_data(0x03); //source 800
  ssend_data(0x20);
  ssend_data(0x01); //gate 480
  ssend_data(0xE0);
  
  ssend_command(DUAL_SPI);
  ssend_data(0x00);
  
  ssend_command(VCOM_AND_DATA_INTERVAL_SETTING);
  ssend_data(0x10);
  ssend_data(0x00);
  
  ssend_command(TCON_SETTING);
  ssend_data(0x22);
  
  ssend_command(GATE_SOURCE_START_SETTING); // resolution setting
  ssend_data(0x00);
  ssend_data(0x00); //800*480
  ssend_data(0x00);
  ssend_data(0x00);
  
  sset_lut_fast();

  return 0;
}

void sreset(void){
  digital_write(RST_PIN, HIGH);
  delay_ms(20);
  digital_write(RST_PIN, LOW);
  delay_ms(4);
  digital_write(RST_PIN, HIGH);
  delay_ms(20);
}

void ssend_command(unsigned char command){
  digital_write(DC_PIN, LOW);
  spi_transfer(command);
}

void ssend_data(unsigned char data){
  digital_write(DC_PIN, HIGH);
  spi_transfer(data);
}

void sdisplay_frame(const unsigned char* frame_buffer){
  
  unsigned char temp1, temp2;
  ssend_command(DISPLAY_START_TRANSMISSION_2);
  for (int j = 0; j < EPD_HEIGHT; j++) {
    for (int i = 0; i < EPD_WIDTH/8; i++) {
      ssend_data(~frame_buffer[i + j * EPD_WIDTH/8]);
    }
  }
  //TODO: try this without refreshing
  ssend_command(DISPLAY_REFRESH);
  delay_ms(100);
  swait_until_idle();
}

void swait_until_idle(void) {
  while(digital_read(BUSY_PIN) == 0){
    // ssend_command(GET_STATUS);
    delay_ms(100);
  }
}

void ssleep(void){
  ssend_command(POWER_OFF);
  swait_until_idle();
  ssend_command(DEEP_SLEEP);
  ssend_data(0xa5);
}

/*
 * LUTs-related code
 */
void sset_lut(void) {
  unsigned int count;     
  ssend_command(LUT_FOR_VCOM);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_vcom0[count]);
  }
  
  ssend_command(LUT_WHITE_TO_WHITE);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_ww[count]);
  }   
  
  ssend_command(LUT_BLACK_TO_WHITE);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_bw[count]);
  } 

  ssend_command(LUT_WHITE_TO_BLACK);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_wb[count]);
  } 

  ssend_command(LUT_BLACK_TO_BLACK);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_bb[count]);
  } 
}
void sset_lut_fast(void) {
  unsigned int count;     
  ssend_command(LUT_FOR_VCOM);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_vcom0_fast[count]);
  }
  
  ssend_command(LUT_WHITE_TO_WHITE);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_ww_fast[count]);
  }   
  
  ssend_command(LUT_BLACK_TO_WHITE);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_bw_fast[count]);
  } 

  ssend_command(LUT_WHITE_TO_BLACK);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_wb_fast[count]);
  } 

  ssend_command(LUT_BLACK_TO_BLACK);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_bb_fast[count]);
  } 
}

const int sorientation = ROTATE_0;

const unsigned char lut_vcom0[42]={
  0x00, 0x0F, 0x0F, 0x00, 0x00, 0x01,
  0x00, 0x0F, 0x01, 0x0F, 0x01, 0x02,
  0x00, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};      

const unsigned char lut_ww[42]={ 
  0x10, 0x0F, 0x0F, 0x00, 0x00, 0x01,
  0x84, 0x0F, 0x01, 0x0F, 0x01, 0x02,
  0x20, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};

const unsigned char lut_bw[42]={ 
  0x10, 0x0F, 0x0F, 0x00, 0x00, 0x01,
  0x84, 0x0F, 0x01, 0x0F, 0x01, 0x02,
  0x20, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};

const unsigned char lut_wb[42]={ 
  0x80, 0x0F, 0x0F, 0x00, 0x00, 0x01,
  0x84, 0x0F, 0x01, 0x0F, 0x01, 0x02,
  0x40, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};

const unsigned char lut_bb[42]={
  0x80, 0x0F, 0x0F, 0x00, 0x00, 0x01,
  0x84, 0x0F, 0x01, 0x0F, 0x01, 0x02,
  0x40, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};

// "Fast" tables are a hack developed by Ben Krasnow
// Watch https://www.youtube.com/watch?v=MsbiO8EAsGw&ab_channel=AppliedScience
// to learn more
const unsigned char lut_vcom0_fast[42]={
  0x00, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};      

const unsigned char lut_ww_fast[42]={
  0x20, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};

const unsigned char lut_bw_fast[42]={ 
  0x20, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};

const unsigned char lut_wb_fast[42]={ 
  0x40, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};

const unsigned char lut_bb_fast[42]={ 
  0x40, 0x0F, 0x0F, 0x00, 0x00, 0x01,
};

const unsigned char voltage_frame[]={
  0x06, 0x3F, 0x3F, 0x11, 0x24, 0x07, 0x17,
};
