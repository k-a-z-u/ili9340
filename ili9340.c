#include <linux/init.h>
#include <linux/module.h>
#include <linux/fb.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include <linux/errno.h>
#include <linux/types.h>

#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <net/sock.h>

#include "display.h"


u8* mem = 0;
u8* mem2 = 0;
struct fb_info* info = 0;
struct task_struct* thread;

int threadActive = 0;
int threadTerminate = 0;

static int __init ili9340_probe(struct platform_device* dev);

struct platform_device* ili9340_device = 0;

/** describes the driver to register and its probe function */
static struct platform_driver ili9340_driver = {
	.probe  = ili9340_probe,
	.driver = {
		.name   = "ili9340_lcd",
	},
};


/*
void ili9340_fillrect(struct fb_info *p, const struct fb_fillrect *rect) {
	printk(KERN_DEBUG "lcd: ili9340_fillrect\n");
}

void ili9340_copyarea(struct fb_info *p, const struct fb_copyarea *area) {
	printk(KERN_DEBUG "lcd: ili9340_copyarea\n");
}

void ili9340_imageblit(struct fb_info *p, const struct fb_image *image) {
	printk(KERN_DEBUG "lcd: ili9340_imageblit\n");
}

int ili9340_sync(struct fb_info *info) {
	printk(KERN_DEBUG "lcd: ili9340_sync\n");
	return 0;
}
*/

/*
static int ili9340_mmap(struct fb_info *info, struct vm_area_struct *vma) {
	printk(KERN_DEBUG "myFB: ili9340_mmap\n");
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	return vm_iomap_memory(vma, virt_to_phys(mem), size_total);
	//return 0;
}
*/

/*
ssize_t ili9340_read(struct fb_info *info, char __user *buf, size_t count, loff_t *ppos) {
	printk(KERN_DEBUG "lcd: ili9340_read\n");
	return 0;
}

ssize_t ili9340_write(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos) {
	printk(KERN_DEBUG "lcd: ili9340_write\n");
	return 0;
}
*/

/*
int ili9340_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg) {
  printk(KERN_DEBUG "lcd: got IOCTL\n");
  int ret = 0;
  struct fb_var_screeninfo var;
  void __user *argp = (void __user *)arg;
  
  switch (cmd) {
    case FBIOGET_VSCREENINFO:
      if (!lock_fb_info(info)) {return -ENODEV;}
      var = info->var;
      unlock_fb_info(info);
      ret = copy_to_user(argp, &var, sizeof(var)) ? -EFAULT : 0;
      break;
  }
  return ret;
}
*/

/** pointers to all implemented module methods */
static struct fb_ops ili9340_ops = {
	
	.owner          = THIS_MODULE,
	
	.fb_destroy     = NULL,
	.fb_pan_display = NULL,
	.fb_fillrect	= NULL,		// ili9340_fillrect
	.fb_copyarea	= NULL,		// ili9340_copyarea
	.fb_imageblit	= NULL,		// ili9340_imageblit
	.fb_setcolreg	= NULL,
	.fb_blank		= NULL,
	.fb_ioctl		= NULL,		// ili9340_ioctl
	.fb_mmap		= NULL,		// ili9340_mmap,
	.fb_sync		= NULL,		// ili9340_sync,
	.fb_read		= NULL,		// ili9340_read,
	.fb_write		= NULL		// ili9340_write,
	
};


/**
 * background thread
 * refreshes the content of the LCD
 * using the internal memory buffer
 */
int ili9340_thread(void* data) {
	
	int cnt = 0;
	threadActive = 1;
	debug("thread starting");
	   
	displayInit();
	displayStartStream();
	
	while(!threadTerminate) {
     
		int i;
		
		// use a second buffer
		memcpy(mem2, mem, ILI9340_BUFFER_SIZE_TOTAL);
		
		// convert from RGB32 to RGB16 and enque for sending
		for (i = 0; i < ILI9340_BUFFER_SIZE_TOTAL; i += 4) {
			//const int b = mem2[i+0] >> 3;
			//const int g = mem2[i+1] >> 2;
			//const int r = mem2[i+2] >> 3;
			//const int c = (b << 0) | (g << 5) | (r << 11);
			const int c = ((mem2[i+0] & 0b11111000) >> 3) | ((mem2[i+1] & 0b11111100) << 3) | ((mem2[i+2] & 0b11111000) << 8);
			displayStreamPixel(c);
		}
		
		// restart drawing in the upper-left every 16 frames (sync if something goes wrong)
		if (++cnt % 16 == 0) {
			displayStopStream();
			displayStartStream();
		}
			
		// wait for next
		msleep(10);
			
	}
 
	// cleanup
	displayUninit();
		
	// return
	debug("thread done");
	threadActive = 0;
	do_exit(0);

}

static int __init ili9340_probe(struct platform_device* dev) {
	
	debug("probe");	
	
	// try to allocate framebuffer
	info = framebuffer_alloc(sizeof(u32) * 256, &dev->dev);
	debugPtr("framebuffer_alloc:", info);
	if (!info) {return -ENOMEM;}
	
	// configure
	info->pseudo_palette = info->par;
	info->par = NULL;
	
	// the struct containing the pointers of all implemented module methods
	info->fbops = &ili9340_ops;
	info->flags = FBINFO_FLAG_DEFAULT | FBINFO_VIRTFB;
 	
	// allocate memory to store the image
	mem = kmalloc(ILI9340_BUFFER_SIZE_TOTAL, GFP_KERNEL);
	debugPtr("primary memory:", mem);
	if (!mem) {return -ENOMEM;}
	
	// allocate the 2nd buffer (swap)
	mem2 = vmalloc(ILI9340_BUFFER_SIZE_TOTAL);
	debugPtr("secondary memory:", mem2);
	if (!mem2) {return -ENOMEM;}
	
	// framebuffer configuration
	info->screen_size			= ILI9340_BUFFER_SIZE_TOTAL;
	info->screen_base			= (u8 __iomem*) mem;
	
	info->fix.smem_len			= ILI9340_BUFFER_SIZE_TOTAL;
	info->fix.smem_start		= virt_to_phys(mem);

	info->fix.accel =			FB_ACCEL_NONE;
	info->fix.type =			FB_TYPE_PACKED_PIXELS;
	info->fix.visual =			FB_VISUAL_TRUECOLOR;
	info->fix.xpanstep =		0;
	info->fix.ypanstep =		0;
	info->fix.ywrapstep =		0;
	info->fix.line_length =		ILI9340_BUFFER_LINE_BYTES;
 
	
	// screen
	info->var.xres				= ILI9340_BUFFER_WIDTH;
	info->var.yres				= ILI9340_BUFFER_HEIGHT;
	info->var.xres_virtual		= ILI9340_BUFFER_WIDTH;
	info->var.yres_virtual		= ILI9340_BUFFER_HEIGHT;
	info->var.bits_per_pixel	= ILI9340_BUFFER_BPP;
	info->var.activate			= FB_ACTIVATE_NOW;
	info->var.vmode				= FB_VMODE_NONINTERLACED;
	
	info->var.nonstd			= 1;
  
	info->var.red.offset		= 16;
	info->var.green.offset		= 8;
	info->var.blue.offset		= 0;
	
	info->var.red.length		= 8;
	info->var.green.length		= 8;
	info->var.blue.length		= 8;
	
	info->apertures = alloc_apertures(1);
	debugPtr("alloc_apertures:", info->apertures);
	if (!info->apertures) { return -ENOMEM; }
	info->apertures->ranges[0].base = virt_to_phys(mem);
	info->apertures->ranges[0].size = ILI9340_BUFFER_SIZE_TOTAL;
	
	// color palette
	debug("fb_alloc_cmap");
	if (fb_alloc_cmap(&info->cmap, 256, 0) < 0) { return -ENOMEM; }

	// try to create a new framebuffer device
	debug("register_framebuffer");
	if (register_framebuffer(info) < 0) { return -EINVAL; }

	//debug("lcd: %d, %s frame buffer device\n", info->node, info->fix.id);

	// start the thread
	threadTerminate = 0;
	thread = kthread_run(&ili9340_thread, (void *)0, "ili9340_thread");
	debugPtr("kthread_run: ", thread);

	return 0;

}


static int __init ili9340_init(void) {
	
	int ret;
	
	debug("init");
			
	// register
	debug("platform_driver_register");
	ret = platform_driver_register(&ili9340_driver);		// should be 0?
	if (ret) {return ret;}
	
	// allocate space to add the new device
	ili9340_device = platform_device_alloc("ili9340_lcd", 0);
	debugPtr("platform_device_alloc:", ili9340_device);
	if (!ili9340_device) {return -ENOMEM;}
		
	// add the new device
	debug("platform_device_add");
	ret = platform_device_add(ili9340_device);
	
	// adding failed?
	if (ret) {
		debug("error during platform_device_add()");
		platform_device_put(ili9340_device);			// de-alloc
		platform_driver_unregister(&ili9340_driver);	// remove
	}
			
	debug("init complete");
	return ret;
	
}

/** called on module unload */
static void __exit ili9340_exit(void) {
		
	debug("exit");
	
	// stop the thread
	if (threadActive) {
		debug("stopping thread");
		threadTerminate = 1;
		while (threadActive) {msleep(1);}
		debug("thread stopped");
	}
	
	// remove framebuffer
	if (info) {
		debug("unregister_framebuffer");
		unregister_framebuffer(info);
		info = NULL;
	}
	
	// remove device
	if (ili9340_device) {
		debug("platform_device_del");
		platform_device_del(ili9340_device);
		ili9340_device = NULL;
	}
	
	// unregister driver
	debug("platform_driver_unregister");
	platform_driver_unregister(&ili9340_driver);
	
	// free memory
	if (mem)	{kfree(mem); mem = 0;}
	if (mem2)	{vfree(mem2); mem2 = 0;}
	
}

/** module settings */
MODULE_LICENSE("GPL");
module_init(ili9340_init);
module_exit(ili9340_exit);
