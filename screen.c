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

  bcm2835_spi_begin();                                         //Start spi interface, set spi pin for the reuse function
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
  //TODO: try without refreshing
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
