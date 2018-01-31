#include "screen.h"
#include <bcm2835.h>


//Interface-Level primitives
//==========================
int init_if(void){
  if(!bcm2835_init()) {
    return -1;
  }
  bcm2835_gpio_fsel(RST_PIN, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(DC_PIN, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(BUSY_PIN, BCM2835_GPIO_FSEL_INPT);

  if(!bcm2835_spi_begin()){                                         //Start spi interface, set spi pin for the reuse function
    return -1;
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
// Note: this custom driver has been greatly inspired by waveshare's
// epd7in5 driver. 
// This may not properly on smaller screens... I have actually no clue.
//=====================

// This function has been extracted from the epd7in5.cpp file. No clue what the
// data payload means.
int sinit(void) {
  if(init_if() != 0){
    return -1;
  }
  sreset();

  ssend_command(POWER_SETTING); 
  ssend_data(0x37);
  ssend_data(0x00);

  ssend_data(PANEL_SETTING);
  ssend_data(0xCF);
  ssend_data(0x08);
  
  ssend_command(BOOSTER_SOFT_START);
  ssend_data(0xc7);     
  ssend_data(0xcc);
  ssend_data(0x28);

  ssend_command(POWER_ON);
  swait_until_idle();

  ssend_command(PLL_CONTROL);
  ssend_data(0x3c);        

  ssend_command(TEMPERATURE_CALIBRATION);
  ssend_data(0x00);

  ssend_command(VCOM_AND_DATA_INTERVAL_SETTING);
  ssend_data(0x77);

  ssend_command(TCON_SETTING);
  ssend_data(0x22);

  ssend_command(TCON_RESOLUTION);
  ssend_data(0x02);     //source 640
  ssend_data(0x80);
  ssend_data(0x01);     //gate 384
  ssend_data(0x80);

  ssend_command(VCM_DC_SETTING);
  ssend_data(0x1E);      //decide by LUT file

  ssend_command(0xe5);   //FLASH MODE            
  ssend_data(0x03);  

  return 0;
}

void sreset(void){
  digital_write(RST_PIN, LOW);
  delay_ms(200);
  digital_write(RST_PIN, HIGH);
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
        pdraw_pixel(x, y, colored, frame_buffer);
    }
  }
}

void pdraw_pixel(int x, int y, int colored, unsigned char* frame_buffer) {
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

void pdraw_vertical_line(int x, int y, int line_height, int colored, unsigned char* frame_buffer){
  int i;
  for (i = y; i < y + line_height; i++) {
    pdraw_pixel(x, i, colored, frame_buffer);
  }
}
