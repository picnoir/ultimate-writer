#include "screen.h"
#include <bcm2835.h>
#include <stdlib.h>
#include <stdio.h>

int sorientation = ROTATE_0;

//Interface-Level primitives
//==========================
int init_if(void){
  if(!bcm2835_init()) {
    printf("Cannot init BCM2835. Is the kernel module installed?\n");
    exit(-1);
  }
  bcm2835_gpio_fsel(CS_PIN, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(RST_PIN, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(DC_PIN, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(BUSY_PIN, BCM2835_GPIO_FSEL_INPT);

  if(!bcm2835_spi_begin()){                                         //Start spi interface, set spi pin for the reuse function
    printf("Cannot init SPI interface. Is it activated in raspi-config?\n");
    exit(-1);
  }
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);     //High first transmission
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                  //spi mode 0
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);  //Frequency
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                     //set CE0
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);     //enable cs0
  return 0;
}

void digital_write(int pin, int value) {
  bcm2835_gpio_write(pin, value);
}

int digital_read(int pin) {
  return bcm2835_gpio_lev(pin);
}

void delay_ms(unsigned int delaytime) {
  bcm2835_delay(delaytime);
}

void spi_transfer(unsigned char data) {
  bcm2835_spi_transfer(data);
}

// Eink driver functions
//
// Note: despite being greatly inspired by waveshare's stock firmwire, this
// initiation phase has been quite modified. See IL0373 datasheet for more infos
// <https://www.smart-prototyping.com/image/data/9_Modules/EinkDisplay/GDEW0154T8/IL0373.pdf>
//
// This init phase has been optimized for the 7.2 inches screen. You'll need to tweak some commands
// to properly handle another screen. (The init phase has been commented out for more clarity).
//=====================
int sinit(void) {
  switch(screen_type) {
    case GOODDISPLAY75 :
      return init_waveshare_75();
    case GOODDISPLAY42 :
      return init_waveshare_42();
  }
}

int init_waveshare_75(void) {
  printf("init 75\n");
  if(init_if() != 0){
    exit(-1);
  }
  sreset();

  ssend_command(POWER_SETTING); 
  /* 
   * Internal VDS_EN and VDG_EN.
   * VCOMH=VDH+VCOMDC, VGH=16V, VGL= -16V.
   * */
  ssend_data(0x37); 
  ssend_data(0x00);

  ssend_data(PANEL_SETTING);
  /* 160x296, LUTs from OTP, black/white/red, 
   * scan up, shift right, booster on */
  ssend_data(0xCF); 
  
  ssend_command(BOOSTER_SOFT_START);
  /*
   * The datasheet seems incorrect for this section.
   * Probably better to leave it as it was in wavshare's 
   * firmware.
   *
   * Phase A: Soft start period: 40ms, strength 1.
   * Minimum OFF time setting of GDR in phase A: 6.58uS.
   * Soft start period of phase B: 40ms, strength 2.
   * Minimum OFF time setting of GDR in phase B: 0.80uS.
   */
  ssend_data(0xc7);     
  ssend_data(0xcc);
  ssend_data(0x28);

  ssend_command(POWER_ON);
  swait_until_idle();

  ssend_command(PLL_CONTROL);
  /*
   * PLL clock: 50Hz.
   */
  ssend_data(0x3c);        

  ssend_command(TEMPERATURE_CALIBRATION);
  /*
   * Internal temperature sensor value.
   */
  ssend_data(0x00);

  ssend_command(VCOM_AND_DATA_INTERVAL_SETTING);
  /*
   * Vcom and data interval: 10 hsync (VBDW).
   */
  ssend_data(0x77);

  ssend_command(RESOLUTION_SETTING);
  /*
   * 12 periods.
   */
  ssend_data(0x22);

  ssend_command(RESOLUTION_SETTING);
  /*
   * Horizontal res: 640
   * Vertical res: 384
   */
  ssend_data(0x02);
  ssend_data(0x80);
  ssend_data(0x01);
  ssend_data(0x80);

  ssend_command(VCM_DC_SETTING);
  /*
   * VCOM DC value.
   */
  ssend_data(0x1E);

  ssend_command(TSSET);
  /*
   * Force set temperature for cascade.
   */
  ssend_data(0x03);  

  return 0;
}

int init_waveshare_42(void) {
  printf("Init 42");
  if (init_if() != 0) {
    exit(-1);
  }
  sreset();
  ssend_command(POWER_SETTING);
  ssend_data(0x03);                  // VDS_EN, VDG_EN
  ssend_data(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
  ssend_data(0x2b);                  // VDH
  ssend_data(0x2b);                  // VDL
  ssend_data(0xff);                  // VDHR
  ssend_command(BOOSTER_SOFT_START);
  ssend_data(0x17);
  ssend_data(0x17);
  ssend_data(0x17);                  //07 0f 17 1f 27 2F 37 2f
  ssend_command(POWER_ON);
  swait_until_idle();
  ssend_command(PANEL_SETTING);
//  ssend_data(0xbf);    // KW-BF   KWR-AF  BWROTP 0f
//  ssend_data(0x0b);
//	ssend_data(0x0F);  //300x400 Red mode, LUT from OTP
//	ssend_data(0x1F);  //300x400 B/W mode, LUT from OTP
  ssend_data(0x3F); //300x400 B/W mode, LUT set by register
//	ssend_data(0x2F); //300x400 Red mode, LUT set by register

  ssend_command(PLL_CONTROL);
  ssend_data(0x3C);        // 3A 100Hz   29 150Hz   39 200Hz    31 171Hz       3C 50Hz (default)    0B 10Hz
//  ssend_data(0x0B);   //0B is 10Hz
  return 0;
}

void sreset(void){
  digital_write(RST_PIN, LOW);
  delay_ms(200);
  digital_write(RST_PIN, HIGH);
  delay_ms(200);
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
  switch(screen_type){
    GOODDISPLAY75:
      sdisplay_frame_75(frame_buffer);
      break;
    GOODDISPLAY42:
      sdisplay_frame_42(frame_buffer);
      break;
  }
}

void sdisplay_frame_42(const unsigned char* frame_buffer){
  printf("Starting display frame");
  unsigned int width = 400, height = 300;
  ssend_command(RESOLUTION_SETTING);
  ssend_data(width >> 8);        
  ssend_data(width & 0xff);
  ssend_data(height >> 8);
  ssend_data(height & 0xff);

  ssend_command(VCM_DC_SETTING);
  ssend_data(0x12);                   

  ssend_command(VCOM_AND_DATA_INTERVAL_SETTING);
  ssend_command(0x97);    //VBDF 17|D7 VBDW 97  VBDB 57  VBDF F7  VBDW 77  VBDB 37  VBDR B7

  if (frame_buffer != NULL) {
      ssend_command(DATA_START_TRANSMISSION_1);
      for(int i = 0; i < width / 8 * height; i++) {
          ssend_data(0xFF);      // bit set: white, bit reset: black
      }
      delay_ms(2);
      ssend_command(DATA_START_TRANSMISSION_2); 
      for(int i = 0; i < width / 8 * height; i++) {
          ssend_data(frame_buffer[i]);
      }  
      delay_ms(2);                  
  }

  set_lut();
  ssend_command(DISPLAY_REFRESH); 
  delay_ms(100);
  swait_until_idle();
  printf("Displayed frame");
}

void sdisplay_frame_75(const unsigned char* frame_buffer){
  unsigned char temp1, temp2;
  ssend_command(DATA_START_TRANSMISSION_1);
  for(int i = 0; i < 30720; i++) {   
    temp1 = frame_buffer[i];
    for(unsigned char j = 0; j < 8; j++) {
      if(temp1 & 0x80)
          temp2 = 0x03;
      else
          temp2 = 0x00;
      temp2 <<= 4;
      temp1 <<= 1;
      j++;
      if(temp1 & 0x80)
          temp2 |= 0x03;
      else
          temp2 |= 0x00;
      temp1 <<= 1;
      ssend_data(temp2); 
    }
  }
  //TODO: try this without refreshing
  ssend_command(DISPLAY_REFRESH);
  delay_ms(100);
  swait_until_idle();
}

void sdisplay_frame_fast(const unsigned char* frame_buffer) {
  switch(screen_type){
    GOODDISPLAY75:
      // No fast refresh for you :(
      sdisplay_frame_75(frame_buffer);
      break;
    GOODDISPLAY42:
      sdisplay_frame_fast_42(frame_buffer);
      break;
  }
}

void sdisplay_frame_fast_42(const unsigned char* frame_buffer){
  unsigned int width = 400, height = 300;
  ssend_command(RESOLUTION_SETTING);
  ssend_data(width >> 8);        
  ssend_data(width & 0xff);
  ssend_data(height >> 8);
  ssend_data(height & 0xff);

  ssend_command(VCM_DC_SETTING);
  ssend_data(0x12);                   

  ssend_command(VCOM_AND_DATA_INTERVAL_SETTING);
  ssend_command(0x97);    //VBDF 17|D7 VBDW 97  VBDB 57  VBDF F7  VBDW 77  VBDB 37  VBDR B7

  if (frame_buffer != NULL) {
      ssend_command(DATA_START_TRANSMISSION_1);
      for(int i = 0; i < width / 8 * height; i++) {
          ssend_data(0xFF);      // bit set: white, bit reset: black
      }
      delay_ms(2);
      ssend_command(DATA_START_TRANSMISSION_2); 
      for(int i = 0; i < width / 8 * height; i++) {
          ssend_data(frame_buffer[i]);
      }  
      delay_ms(2);                  
  }

  set_fast_lut();
  ssend_command(DISPLAY_REFRESH); 
  swait_until_idle();
  return;
}

void swait_until_idle(void) {
  while(digital_read(BUSY_PIN) == 0){
    delay_ms(100);
  }
}

void ssleep(void){
  ssend_command(POWER_OFF);
  swait_until_idle();
  ssend_command(DEEP_SLEEP);
  ssend_data(0xa5);
}

// Framebuffer painting functions.
//
// Note: these function have been greatly inspired by waveshare's
// epdpaint.cpp.
//=====================

void pclear (int colored, unsigned char* frame_buffer) {
  for (int x = 0; x < EPD_WIDTH; x++) {
    for (int y = 0; y < EPD_HEIGHT; y++) {
        pdraw_absolute_pixel(x, y, colored, frame_buffer);
    }
  }
}

void pdraw_absolute_pixel(int x, int y, int colored, unsigned char* frame_buffer) {
  if (x < 0 || x >= EPD_WIDTH || y < 0 || y >= EPD_HEIGHT) {
    return;
  }
  if (IF_INVERT_COLOR) {
    if (colored) {
      frame_buffer[(x + y * EPD_WIDTH) / 8] |= 0x80 >> (x % 8);
    } else {
      frame_buffer[(x + y * EPD_WIDTH) / 8] &= ~(0x80 >> (x % 8));
    }
  } else {
    if (colored) {
      frame_buffer[(x + y * EPD_WIDTH) / 8] &= ~(0x80 >> (x % 8));
    } else {
      frame_buffer[(x + y * EPD_WIDTH) / 8] |= 0x80 >> (x % 8);
    }
  }
}

void pdraw_pixel(int x, int y, int colored, unsigned char* frame_buffer) {
  int point_temp;
  if (sorientation == ROTATE_0) {
    if(x < 0 || x >= EPD_WIDTH || y < 0 || y >= EPD_HEIGHT) {
      return;
    }
    pdraw_absolute_pixel(x, y, colored, frame_buffer);
  } else if (sorientation == ROTATE_90) {
    if(x < 0 || x >= EPD_HEIGHT || y < 0 || y >= EPD_WIDTH) {
      return;
    }
    point_temp = x;
    x = EPD_WIDTH - y;
    y = point_temp;
    pdraw_absolute_pixel(x, y, colored, frame_buffer);
  } else if (sorientation == ROTATE_180) {
    if(x < 0 || x >= EPD_WIDTH || y < 0 || y >= EPD_HEIGHT) {
      return;
    }
    x = EPD_WIDTH - x;
    y = EPD_HEIGHT - y;
    pdraw_absolute_pixel(x, y, colored, frame_buffer);
  } else if (sorientation == ROTATE_270) {
    if(x < 0 || x >= EPD_HEIGHT || y < 0 || y >= EPD_WIDTH) {
      return;
    }
    point_temp = x;
    x = y;
    y = EPD_HEIGHT - point_temp;
    pdraw_absolute_pixel(x, y, colored, frame_buffer);
  }
}

void pdraw_char_at(int x, int y, char ascii_char, sFONT* font, int colored, unsigned char* frame_buffer){
  int i, j;
  unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
  const unsigned char* ptr = &font->table[char_offset];

  for (j = 0; j < font->Height; j++){
    for (i = 0; i < font->Width; i++){
      if (*ptr & (0x80 >> (i % 8))){
        pdraw_pixel(x + i, y + j, colored, frame_buffer);
      }
      if (i % 8 == 7){
        ptr++;
      }
    }
    if (font->Width % 8 != 0){
      ptr++;
    }
  }
}

void pdraw_string_at(int x, int y, const char* text, sFONT* font, int colored, unsigned char* frame_buffer){
  const char* p_text = text;
  unsigned int counter = 0;
  int refcolumn = x;
  
  /* Send the string character by character on EPD */
  while (*p_text != 0) {
    /* Display one character on EPD */
    pdraw_char_at(refcolumn, y, *p_text, font, colored, frame_buffer);
    /* Decrement the column position by 16 */
    refcolumn += font->Width;
    /* Point on the next character */
    p_text++;
    counter++;
  }
}

void pdraw_line(int x0, int y0, int x1, int y1, int colored, unsigned char* frame_buffer){
  /* Bresenham algorithm */
  int dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
  int sx = x0 < x1 ? 1 : -1;
  int dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  while((x0 != x1) && (y0 != y1)) {
    pdraw_pixel(x0, y0 , colored, frame_buffer);
    if (2 * err >= dy) {     
      err += dy;
      x0 += sx;
    }
    if (2 * err <= dx) {
      err += dx; 
      y0 += sy;
    }
  }
}

void pdraw_vertical_line(int x, int y, int line_height, int colored, unsigned char* frame_buffer){
  int i;
  for (i = y; i < y + line_height; i++) {
    pdraw_pixel(x, i, colored, frame_buffer);
  }
}

void pdraw_horizontal_line(int x, int y, int line_width, int colored, unsigned char* frame_buffer){
  int i;
  for (i = x; i < x + line_width; i++) {
    pdraw_pixel(x, i, colored, frame_buffer);
  }

}

void pdraw_filled_rectangle(int x0, int y0, int x1, int y1, int colored, unsigned char* frame_buffer){
  int min_x, min_y, max_x, max_y;
  int i;
  min_x = x1 > x0 ? x0 : x1;
  max_x = x1 > x0 ? x1 : x0;
  min_y = y1 > y0 ? y0 : y1;
  max_y = y1 > y0 ? y1 : y0;
  
  for (i = min_x; i <= max_x; i++) {
    pdraw_vertical_line(i, min_y, max_y - min_y + 1, colored, frame_buffer);
  }
}

void pdraw_rectangle(int x0, int y0, int x1, int y1, int colored, unsigned char* frame_buffer) {
  int min_x, min_y, max_x, max_y;
  min_x = x1 > x0 ? x0 : x1;
  max_x = x1 > x0 ? x1 : x0;
  min_y = y1 > y0 ? y0 : y1;
  max_y = y1 > y0 ? y1 : y0;
  
  pdraw_horizontal_line(min_x, min_y, max_x - min_x + 1, colored, frame_buffer);
  pdraw_horizontal_line(min_x, max_y, max_x - min_x + 1, colored, frame_buffer);
  pdraw_vertical_line(min_x, min_y, max_y - min_y + 1, colored, frame_buffer);
  pdraw_vertical_line(max_x, min_y, max_y - min_y + 1, colored, frame_buffer);
}

void pdraw_term(Line* lines, unsigned char* frame_buffer) {
  int i, j;
  char* str = (char*)malloc((cols + 10) * sizeof(char));
  for(i=0; i < rows; i++) {
    for(j=0; j < cols; j++) {
      str[j] = lines[i][j].u;
    }
    pdraw_string_at(0, i * Font16.Height, str, &Font16, COLORED, frame_buffer);
  }
  free(str);
}

/*
 * LUTs-related code
 */
void set_lut(void) {
  switch(screen_type) {
    case GOODDISPLAY75:
      // No LUTs for you :(
      break;
    case GOODDISPLAY42:
      set_lut_42();
      break;
  }
}

void set_lut_42(void) {
  unsigned int count;     
  ssend_command(LUT_FOR_VCOM);
  for(count = 0; count < 44; count++) {
    ssend_data(lut_vcom0_42[count]);
  }
  
  ssend_command(LUT_WHITE_TO_WHITE);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_ww_42[count]);
  }   
  
  ssend_command(LUT_BLACK_TO_WHITE);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_bw_42[count]);
  } 

  ssend_command(LUT_WHITE_TO_BLACK);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_wb_42[count]);
  } 

  ssend_command(LUT_BLACK_TO_BLACK);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_bb_42[count]);
  } 
}

void set_fast_lut(void) {
  unsigned int count;     
  ssend_command(LUT_FOR_VCOM);
  for(count = 0; count < 44; count++) {
    ssend_data(lut_vcom0_fast_42[count]);
  }
  
  ssend_command(LUT_WHITE_TO_WHITE);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_ww_fast_42[count]);
  }   
  
  ssend_command(LUT_BLACK_TO_WHITE);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_bw_fast_42[count]);
  } 

  ssend_command(LUT_WHITE_TO_BLACK);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_wb_fast_42[count]);
  } 

  ssend_command(LUT_BLACK_TO_BLACK);
  for(count = 0; count < 42; count++) {
    ssend_data(lut_bb_fast_42[count]);
  } 
}



const unsigned char lut_vcom0_42[] =
{
0x00, 0x17, 0x00, 0x00, 0x00, 0x02,        
0x00, 0x17, 0x17, 0x00, 0x00, 0x02,        
0x00, 0x0A, 0x01, 0x00, 0x00, 0x01,        
0x00, 0x0E, 0x0E, 0x00, 0x00, 0x02,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_vcom0_fast_42[] =
{
0x00, 0x0E, 0x00, 0x00, 0x00, 0x01,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};



const unsigned char lut_ww_42[] =
{
0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char lut_ww_fast_42[] ={
0xA0, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


const unsigned char lut_bw_42[] ={
0x40, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x40, 0x0A, 0x01, 0x00, 0x00, 0x01,
0xA0, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};


const unsigned char lut_bw_fast_42[] ={
0xA0, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};

const unsigned char lut_bb_42[] ={
0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,        
};

const unsigned char lut_bb_fast_42[] ={
0x50, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     
};


const unsigned char lut_wb_42[] ={
0x80, 0x17, 0x00, 0x00, 0x00, 0x02,
0x90, 0x17, 0x17, 0x00, 0x00, 0x02,
0x80, 0x0A, 0x01, 0x00, 0x00, 0x01,
0x50, 0x0E, 0x0E, 0x00, 0x00, 0x02,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         
};

const unsigned char lut_wb_fast_42[] ={
0x50, 0x0E, 0x00, 0x00, 0x00, 0x01,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         
};

