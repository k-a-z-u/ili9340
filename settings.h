#ifndef ILI9340_SETTINGS
#define ILI9340_SETTINGS

// buffer width (in pixel)
#define ILI9340_BUFFER_WIDTH		320

// buffer height (in pixel)
#define ILI9340_BUFFER_HEIGHT		240

// number of bits (per pixel)
#define ILI9340_BUFFER_BPP			32

// number of bytes per line
#define ILI9340_BUFFER_LINE_BYTES	(ILI9340_BUFFER_WIDTH * ILI9340_BUFFER_BPP / 8)

// total buffer size
#define ILI9340_BUFFER_SIZE_TOTAL	(ILI9340_BUFFER_WIDTH * ILI9340_BUFFER_HEIGHT * ILI9340_BUFFER_BPP / 8)



// max pending bytes before SPI flush
#define ILI9340_SPI_MAX_SENDBUFFER_SIZE			16384

// SPI speed (in Hz)
#define ILI9340_SPI_SPEED						32UL * 1000UL * 1000UL

// pin-number the Data/Cmd Pin is attached to
#define ILI9340_DATA_CMD_PIN					17

// pin-number the CS-Pin is attached to
#define ILI9340_CHIPSELECT_PIN					18

#endif