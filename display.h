#ifndef ILI9340_DISPLAY_H_
#define ILI9340_DISPLAY_H_

// contains the display part of the ILI9340 driver
// see: https://github.com/adafruit/Adafruit_ILI9340

/***************************************************
  This is an Arduino Library for the Adafruit 2.2" SPI display.
  This library works with the Adafruit 2.2" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/1480
 
  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#define ILI9340_TFTWIDTH 240
#define ILI9340_TFTHEIGHT 320
#define ILI9340_NOP 0x00
#define ILI9340_SWRESET 0x01
#define ILI9340_RDDID 0x04
#define ILI9340_RDDST 0x09
#define ILI9340_SLPIN 0x10
#define ILI9340_SLPOUT 0x11
#define ILI9340_PTLON 0x12
#define ILI9340_NORON 0x13
#define ILI9340_RDMODE 0x0A
#define ILI9340_RDMADCTL 0x0B
#define ILI9340_RDPIXFMT 0x0C
#define ILI9340_RDIMGFMT 0x0A
#define ILI9340_RDSELFDIAG 0x0F
#define ILI9340_INVOFF 0x20
#define ILI9340_INVON 0x21
#define ILI9340_GAMMASET 0x26
#define ILI9340_DISPOFF 0x28
#define ILI9340_DISPON 0x29
#define ILI9340_CASET 0x2A
#define ILI9340_PASET 0x2B
#define ILI9340_RAMWR 0x2C
#define ILI9340_RAMRD 0x2E
#define ILI9340_PTLAR 0x30
#define ILI9340_MADCTL 0x36
#define ILI9340_MADCTL_MY 0x80
#define ILI9340_MADCTL_MX 0x40
#define ILI9340_MADCTL_MV 0x20
#define ILI9340_MADCTL_ML 0x10
#define ILI9340_MADCTL_RGB 0x00
#define ILI9340_MADCTL_BGR 0x08
#define ILI9340_MADCTL_MH 0x04
#define ILI9340_PIXFMT 0x3A
#define ILI9340_FRMCTR1 0xB1
#define ILI9340_FRMCTR2 0xB2
#define ILI9340_FRMCTR3 0xB3
#define ILI9340_INVCTR 0xB4
#define ILI9340_DFUNCTR 0xB6
#define ILI9340_PWCTR1 0xC0
#define ILI9340_PWCTR2 0xC1
#define ILI9340_PWCTR3 0xC2
#define ILI9340_PWCTR4 0xC3
#define ILI9340_PWCTR5 0xC4
#define ILI9340_VMCTR1 0xC5
#define ILI9340_VMCTR2 0xC7
#define ILI9340_RDID1 0xDA
#define ILI9340_RDID2 0xDB
#define ILI9340_RDID3 0xDC
#define ILI9340_RDID4 0xDD
#define ILI9340_GMCTRP1 0xE0
#define ILI9340_GMCTRN1 0xE1

#include "settings.h"
#include "gpio.h"
#include "spi.h"


void displaySetup(void);

struct {
	u_int16_t width;
	u_int16_t height;
} display;

/** initialize all needed components and perform basic display setup */
void displayInit(void) {
    
	// initialize the GPIO pins
	gpioInit();
	gpioIn(ILI9340_DATA_CMD_PIN);
	gpioOut(ILI9340_DATA_CMD_PIN);
	gpioIn(ILI9340_CHIPSELECT_PIN);
	gpioOut(ILI9340_CHIPSELECT_PIN);

	// initialize SPI
	spiInit();

	// basic display setup
	display.width = 320;
	display.height = 240;
	displaySetup();

}

/** uninitialize all needed display components */
void displayUninit(void) {
	gpioUninit();
	spiUninit();
}


/** disable the chip-select (= Hi)*/
inline void displayCSHI(void) {gpioSet(ILI9340_CHIPSELECT_PIN);}

/** enable the chip-select (= Lo) */
inline void displayCSLO(void) {gpioClear(ILI9340_CHIPSELECT_PIN);}


/** switch to data-mode (data/cmd-pin = Hi)  */
inline void displaySetData(void) {
	gpioSet(ILI9340_DATA_CMD_PIN);
	displayCSLO();
}

/** switch to command-mode (data/cmd-pin = Lo) */
inline void displaySetCommand(void){
	gpioClear(ILI9340_DATA_CMD_PIN);
}



/** send the given command to the display */
void displayWritecommand(const u_int8_t c) {
	displaySetCommand();
	displayCSLO();
	spiWrite(c);
    spiFlush();
	displayCSHI();
}

/** send the given command to the display */
void displayWritedata(const u_int8_t c) {
	displaySetData();
	displayCSLO();
	spiWrite(c);
	spiFlush();
	displayCSHI();
}



/** set the region where to draw new pixels to */
void displaySetAddrWindow(const u_int16_t x0, const u_int16_t y0, const u_int16_t x1, const u_int16_t y1) {
	displayWritecommand(ILI9340_CASET); // Column addr set
	displayWritedata(x0 >> 8);
	displayWritedata(x0 & 0xFF); // XSTART
	displayWritedata(x1 >> 8);
	displayWritedata(x1 & 0xFF); // XEND
	displayWritecommand(ILI9340_PASET); // Row addr set
	displayWritedata(y0>>8);
	displayWritedata(y0); // YSTART
	displayWritedata(y1>>8);
	displayWritedata(y1); // YEND
}

/** start writing something into the display's memory */
void displayStartMemoryWrite(void){
	displayWritecommand(ILI9340_RAMWR);
}

/** perform the usual display setup routine */
void displaySetup(void) {
		   
	//basic display configuration
	displayWritecommand(0xEF);
	displayWritedata(0x03);
	displayWritedata(0x80);
	displayWritedata(0x02);
	displayWritecommand(0xCF);
	displayWritedata(0x00);
	displayWritedata(0XC1);
	displayWritedata(0X30);
	displayWritecommand(0xED);
	displayWritedata(0x64);
	displayWritedata(0x03);
	displayWritedata(0X12);
	displayWritedata(0X81);
	displayWritecommand(0xE8);
	displayWritedata(0x85);
	displayWritedata(0x00);
	displayWritedata(0x78);
	displayWritecommand(0xCB);
	displayWritedata(0x39);
	displayWritedata(0x2C);
	displayWritedata(0x00);
	displayWritedata(0x34);
	displayWritedata(0x02);
	displayWritecommand(0xF7);
	displayWritedata(0x20);
	displayWritecommand(0xEA);
	displayWritedata(0x00);
	displayWritedata(0x00);


	displayWritecommand(ILI9340_PWCTR1);	//Power control
	displayWritedata(0x23);					//VRH[5:0]
	displayWritecommand(ILI9340_PWCTR2);	//Power control
	displayWritedata(0x10);					//SAP[2:0];BT[3:0]
	displayWritecommand(ILI9340_VMCTR1);	//VCM control
	displayWritedata(0x3e);
	displayWritedata(0x28);
	displayWritecommand(ILI9340_VMCTR2);	//VCM control2
	displayWritedata(0x86);
	displayWritecommand(ILI9340_MADCTL);	// Memory Access Control
	displayWritedata(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
	displayWritecommand(ILI9340_PIXFMT);
	displayWritedata(0x55);
	displayWritecommand(ILI9340_FRMCTR1);
	displayWritedata(0x00);
	displayWritedata(0x18);
	displayWritecommand(ILI9340_DFUNCTR);	// Display Function Control
	displayWritedata(0x08);
	displayWritedata(0x82);
	displayWritedata(0x27);
	displayWritecommand(0xF2);				// 3Gamma Function Disable
	displayWritedata(0x00);
	displayWritecommand(ILI9340_GAMMASET);	//Gamma curve selected
	displayWritedata(0x01);
	displayWritecommand(ILI9340_GMCTRP1);	//Set Gamma
	displayWritedata(0x0F);
	displayWritedata(0x31);
	displayWritedata(0x2B);
	displayWritedata(0x0C);
	displayWritedata(0x0E);
	displayWritedata(0x08);
	displayWritedata(0x4E);
	displayWritedata(0xF1);
	displayWritedata(0x37);
	displayWritedata(0x07);
	displayWritedata(0x10);
	displayWritedata(0x03);
	displayWritedata(0x0E);
	displayWritedata(0x09);
	displayWritedata(0x00);
	displayWritecommand(ILI9340_GMCTRN1);	//Set Gamma
	displayWritedata(0x00);
	displayWritedata(0x0E);
	displayWritedata(0x14);
	displayWritedata(0x03);
	displayWritedata(0x11);
	displayWritedata(0x07);
	displayWritedata(0x31);
	displayWritedata(0xC1);
	displayWritedata(0x48);
	displayWritedata(0x08);
	displayWritedata(0x0F);
	displayWritedata(0x0C);
	displayWritedata(0x31);
	displayWritedata(0x36);
	displayWritedata(0x0F);
	displayWritecommand(ILI9340_SLPOUT);	//Exit Sleep
	msleep(120);
	displayWritecommand(ILI9340_DISPON);	//Display on

	// rotate the display (320x240 instead of 240x320);
	displayWritecommand(ILI9340_MADCTL);
	displayWritedata(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
    
}

/** start writing pixels to the display */
void displayStartStream(void) {
	displaySetAddrWindow(0, 0, display.width-1, display.height-1);
	displayStartMemoryWrite();
	displaySetData();
}

/** send one 16-bit pixel to the display */
inline void displayStreamPixel(const u_int16_t color){
	spiWrite(color >> 8);
	spiWrite(color & 0xFF);
}

/** stop writing pixels to the display */
void displayStopStream(void) {
	spiFlush();
	displayCSHI();
}

#endif /* DISPLAY_H_ */
