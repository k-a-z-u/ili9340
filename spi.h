#ifndef ILI9340_SPI
#define ILI9340_SPI

// contains the SPI part of the ILI9340 driver

#include "io.h"
#include "settings.h"
#include "debug.h"

#include <asm/bug.h>
#include <linux/spi/spidev.h>
#include <linux/init.h>
#include <linux/spi/spi.h>


struct {

	struct {
		uint8_t* tx;		// send buffer
		uint32_t txPos;		// current position within the send-buffer
		uint8_t* rx;		// receive buffer (currently unused)
	} buffer;
	
	// the spi device for the display
	struct spi_device* dev;
	
	// the raspberry's spi controller
	struct spi_master* master;
  
} spi;


/** check whether the given device matches the given chip-select pin number */
int spi_config_match_cs(struct device* dev, void* data) {
	const struct spi_device* sdev = (struct spi_device*) dev;
	const u8 cs = (int)data;
	return (sdev->chip_select == cs);
}

/** initialize the SPI subsystem */
int spiInit(void) {
	
	int ret;
	struct device* found;
	
	// we work on the raspberries SPI-bus 0
	// TODO the chip-select pin is currently used via GPIO...
	struct spi_board_info spi_device_info = {
		.modalias = "ili9340_via_spi",
		.max_speed_hz = ILI9340_SPI_SPEED,
		.bus_num = 0,
		.chip_select = 0,
		.mode = SPI_MODE_0,
	};
	
	// allocate send and receive buffers
	debug("allocating buffers");
	spi.buffer.tx = (uint8_t*) vmalloc(1024*1024);
	spi.buffer.rx = (uint8_t*) vmalloc(1024*1024);
	spi.buffer.txPos = 0; 
     
	// get the master device, given SPI the bus number
	debug("fetching spi-master device");
	spi.master = spi_busnum_to_master( spi_device_info.bus_num );
	debugPtr("spi-master is: ", spi.master);
   
	// removing raspberry's default spi devices on chip-select 0
	debug("searching for existing spi-slaves on chip-select 0");
	found = device_find_child(&spi.master->dev, (void*)(int)spi_device_info.chip_select, spi_config_match_cs);
	if (found) {
		debugPtr("removing existing slave", found);
		spi_unregister_device((struct spi_device*)found);
		put_device(found);
	}
	
	// create a new spi-slave device for the display
	debug("adding new spi-slave");
	spi.dev = spi_new_device( spi.master, &spi_device_info );
	if( !spi.dev ) {return -ENODEV;}
	debugPtr("spi-slave is",  spi.dev);
    
	// configure the created slave
	debug("configuring spi-slave");
	spi.dev->bits_per_word = 8;
	ret = spi_setup( spi.dev );
	if (ret) {spi_unregister_device(spi.dev); spi.dev = NULL;}
    debug("spi-slave initialized");

	return 0;
   
 }

/** cleanup all spi resources */
 void spiUninit(void) {

	// remove the display's spi device
	debug("spi_unregister_device");
	if (spi.dev)		{spi_unregister_device(spi.dev); spi.dev = NULL;}
   
	// free allocated buffers
	debug("free-ing buffers");
	if (spi.buffer.tx)	{vfree(spi.buffer.tx); spi.buffer.tx = 0;}
	if (spi.buffer.rx)	{vfree(spi.buffer.rx); spi.buffer.rx = 0;}
	
}

/** flush the currently buffered bytes onto the SPI-bus */
void spiFlush(void) {
	
	struct spi_message m;
	struct spi_transfer t = {
		.tx_buf	= spi.buffer.tx,
		.len	= spi.buffer.txPos,
	};
	
	// nothing to send? -> abort
	if (spi.buffer.txPos == 0) {return;}
	
	// send (blocking!)
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	spi_sync(spi.dev, &m);
	
	// buffer is empty again
	spi.buffer.txPos = 0;
	
}

/** enque one byte for writing to the spi */
inline void spiWrite(const u_int8_t byte){
	if (spi.buffer.txPos >= ILI9340_SPI_MAX_SENDBUFFER_SIZE) {spiFlush();}
	spi.buffer.tx[spi.buffer.txPos++] = byte;
}
    
#endif
    