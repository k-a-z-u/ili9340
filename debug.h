#ifndef ILI9340_DEBUG
#define ILI9340_DEBUG

// contains some helper methods to debug log the module's actions

#define debug(str)			printk(KERN_DEBUG "ILI9340: %s\n", str)
#define debugStr(str, val)	printk(KERN_DEBUG "ILI9340: %s %s", str, val)
#define debugPtr(str, ptr)	printk(KERN_DEBUG "ILI9340: %s: %pK\n", str, ptr)


#endif