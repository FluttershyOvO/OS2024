#include <defs.h>
#include <stdio.h>
#include <string.h>
#include <console.h>
#include <kdebug.h>
#include <trap.h>
#include <clock.h>
#include <intr.h>
#include <pmm.h>
#include <vmm.h>
#include <ide.h>
#include <swap.h>
#include <kmonitor.h>

int kern_init(void) __attribute__((noreturn));
void grade_backtrace(void);


int
kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);//初始化内存：使用memset将从edata到end之间的内存区域清零。

    const char *message = "(THU.CST) os is loading ...";
    cprintf("%s\n\n", message);//输出加载信息：通过cprintf输出内核加载的信息。

    print_kerninfo();//打印内核信息：调用print_kerninfo打印内核相关信息。

    // grade_backtrace();

    pmm_init();                 // init physical memory management
    //初始化物理内存管理：调用pmm_init初始化物理内存管理。

    idt_init();                 // init interrupt descriptor table
    //初始化中断描述符表：调用idt_init初始化中断描述符表。

    vmm_init();                 // init virtual memory management
    //初始化虚拟内存管理：调用vmm_init初始化虚拟内存管理。
    ide_init();                 // init ide devices
    //初始化IDE设备：调用ide_init初始化IDE设备。
    swap_init();                // init swap
    //初始化交换空间：调用swap_init初始化交换空间。
    clock_init();               // init clock interrupt
    //初始化时钟中断：调用clock_init初始化时钟中断。
    // intr_enable();              // enable irq interrupt



    /* do nothing */
    while (1);//在所有初始化完成后，进入一个无限循环，使内核保持运行状态。
}

void __attribute__((noinline))
grade_backtrace2(int arg0, int arg1, int arg2, int arg3) {
    mon_backtrace(0, NULL, NULL);
}

void __attribute__((noinline))
grade_backtrace1(int arg0, int arg1) {
    grade_backtrace2(arg0, (sint_t)&arg0, arg1, (sint_t)&arg1);
}

void __attribute__((noinline))
grade_backtrace0(int arg0, sint_t arg1, int arg2) {
    grade_backtrace1(arg0, arg2);
}

void
grade_backtrace(void) {
    grade_backtrace0(0, (sint_t)kern_init, 0xffff0000);
}

static void
lab1_print_cur_status(void) {
    static int round = 0;
    round ++;
}


