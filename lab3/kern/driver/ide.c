#include <assert.h>
#include <defs.h>
#include <fs.h>
#include <ide.h>
#include <stdio.h>
#include <string.h>
#include <trap.h>
#include <riscv.h>

void ide_init(void) {}//初始化

#define MAX_IDE 2//设备号
#define MAX_DISK_NSECS 56//设备大小 总扇区数
static char ide[MAX_DISK_NSECS * SECTSIZE];

bool ide_device_valid(unsigned short ideno) { return ideno < MAX_IDE; }
//设备有效性检查：
//ide_device_valid 函数检查给定的设备编号 ideno 是否小于 MAX_IDE，
//如果是则返回 true，否则返回 false。
size_t ide_device_size(unsigned short ideno) { return MAX_DISK_NSECS; }
//设备大小查询：
//ide_device_size 函数返回指定设备的总扇区数 MAX_DISK_NSECS。

int ide_read_secs(unsigned short ideno, uint32_t secno, void *dst,
                  size_t nsecs) {
    int iobase = secno * SECTSIZE;
    memcpy(dst, &ide[iobase], nsecs * SECTSIZE);
    return 0;
}
//读取扇区：
//ide_read_secs 函数计算起始扇区的偏移量 iobase，
//然后使用 memcpy 将数据从 ide 数组复制到目标缓冲区 dst 中。

int ide_write_secs(unsigned short ideno, uint32_t secno, const void *src,
                   size_t nsecs) {
    int iobase = secno * SECTSIZE;
    memcpy(&ide[iobase], src, nsecs * SECTSIZE);
    return 0;
}
//写入扇区：
//ide_write_secs 函数计算起始扇区的偏移量 iobase，
//然后使用 memcpy 将数据从源缓冲区 src 复制到 ide 数组中。