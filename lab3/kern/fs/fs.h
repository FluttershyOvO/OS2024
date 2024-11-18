#ifndef __KERN_FS_FS_H__
#define __KERN_FS_FS_H__

#include <mmu.h>

#define SECTSIZE            512
//定义了扇区的大小为512字节
#define PAGE_NSECT          (PGSIZE / SECTSIZE)
//就本实验来说，页面大小4KB，也就是4096字节
//扇区大小是512字节
//那么每个页面包含的扇区数量是 8 个

#define SWAP_DEV_NO         1
//定义了交换设备的编号为1

#endif /* !__KERN_FS_FS_H__ */

