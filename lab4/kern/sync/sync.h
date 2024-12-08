#ifndef __KERN_SYNC_SYNC_H__
#define __KERN_SYNC_SYNC_H__

#include <defs.h>
#include <intr.h>
#include <riscv.h>

static inline bool __intr_save(void) {
    if (read_csr(sstatus) & SSTATUS_SIE) {
        intr_disable();
        return 1;
    }
    return 0;
}

static inline void __intr_restore(bool flag) {
    if (flag) {
        intr_enable();
    }
}

#define local_intr_save(x) \
    do {                   \
        x = __intr_save(); \
    } while (0)
#define local_intr_restore(x) __intr_restore(x);

#endif /* !__KERN_SYNC_SYNC_H__ */


/*
禁用中断:在需要保护临界区(例如操作共享资源或执行重要的内存管理操作)时，
调用local_intr_save(x)宏，它会调用_intr_save()，保存当前中断状态，并禁用中断。
这样做可以确保这段代码在执行时不会被中断打断。

恢复中断:在关键操作执行完毕后，
调用local intr_restore(x)宏，它会调用 intr _restore(x)，恢复之前保存的中断状态。
通过这种方式，操作系统保证在禁用中断期间，不会影响系统的正常中断响应。
*/