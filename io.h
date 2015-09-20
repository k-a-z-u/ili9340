#ifndef LCD_IO
#define LCD_IO

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/ioctl.h>

// contains the general IO part of the ILI9340 driver

struct file* open(char* path, int flags) {
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;
    int rights = 0;

    printk(KERN_DEBUG "lcd: opening file: %s", path);
    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if(IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void close(struct file* file) {
    filp_close(file, NULL);
}

int write(struct file* file, const unsigned char* data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;
    unsigned long long offset = 0;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int sync(struct file* file) {
    vfs_fsync(file, 0);
    return 0;
}

#endif