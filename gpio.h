#ifndef LCD_GPIO
#define LCD_GPIO

// contains the GPIO part of the ILI9340 driver

#include <asm/page.h>

#include "debug.h"
#include "io.h"
#define LCD_GPIO_BASE    0x3f200000

  /** memory address of the GPIO controller (see /proc/iomem) */
  
  struct {
    volatile unsigned* mem;
  } gpio;

  void gpioInit(void) {
    gpio.mem = (volatile unsigned*) ioremap(LCD_GPIO_BASE, 4096);
    debugPtr("ioremap:", gpio.mem);
  }
  
  void gpioUninit(void) {
    iounmap((void*) gpio.mem); gpio.mem = NULL;
  }
  
  void gpioIn(const u_int8_t pin) {
    volatile unsigned* addr = gpio.mem+((pin)/10);
    unsigned cur = readl(addr);
    cur &= ~(7<<(((pin)%10)*3));
    writel(cur, addr);
  }
  
  void gpioOut(const u_int8_t pin) {
    volatile unsigned* addr = gpio.mem+((pin)/10);
    unsigned cur = readl(addr);
    cur |=  (1<<(((pin)%10)*3));
    writel(cur, addr);
  }
  
  /** set output to HI */
  void gpioSet(const u_int8_t pin) {
    writel(1 << pin, gpio.mem+7);
  }
  
  /** set output to LO */
  void gpioClear(const u_int8_t pin) {
    writel(1 << pin, gpio.mem+10);
  }
  
  #define INP_GPIO(g) *(gpio.mem+((g)/10)) &= ~(7<<(((g)%10)*3))
  #define OUT_GPIO(g) *(gpio.mem+((g)/10)) |=  (1<<(((g)%10)*3))
  #define SET_GPIO_ALT(g,a) *(gpio.mem+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
 
  #define GPIO_SET *(gpio.mem+7)  // sets   bits which are 1 ignores bits which are 0
  #define GPIO_CLR *(gpio.mem+10) // clears bits which are 1 ignores bits which are 0
 
  #define GET_GPIO(g) (*(gpio.mem+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
 
  #define GPIO_PULL *(gpio.mem+37) // Pull up/pull down
  #define GPIO_PULLCLK0 *(gpio.mem+38) // Pull up/pull down clock
  
  

/*
  // export the given pin (make it visible)
  void gpioExportPin(const u_int8_t pin) {
      char buf[3];
      int len = snprintf(buf, 3, "%i", pin);
      struct file* fd = open("/sys/class/gpio/export", O_WRONLY);
      write(fd, buf, len);
      close(fd);
  }
  
  // switch the given pin to output mode
  void gpioPinOut(const u_int8_t pin) {
    
    char filename[128];
    struct file* fd;
    const char* OUT = "out";
    
    snprintf(filename, 128, "/sys/class/gpio/gpio%i/direction", pin);
    
    fd = open(filename, O_WRONLY);
    write(fd, OUT, 3);
    close(fd);
    
  }
  
  struct file* gpioOpen(const u_int8_t pin) {
    char filename[128];
    snprintf(filename, 128, "/sys/class/gpio/gpio%i/value", pin);
    return open(filename, O_WRONLY);
  }
  
  void gpioClose(struct file* fd) {
    close(fd);
  }
  */
  
  
#endif