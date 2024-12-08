#include <list.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <assert.h>

//用于唤醒一个进程
void
wakeup_proc(struct proc_struct *proc) {
    assert(proc->state != PROC_ZOMBIE && proc->state != PROC_RUNNABLE);
    proc->state = PROC_RUNNABLE;
}


/*
这段C代码实现了一个简单的进程调度器 schedule 函数，其主要功能如下：

1.保存中断状态：通过 local_intr_save 保存当前的中断状态。
2.标记当前进程不需要重新调度：将 current->need_resched 置为0。
3.查找下一个可运行的进程：
    从当前进程或空闲进程开始遍历进程链表。
    查找状态为 PROC_RUNNABLE 的进程。
4.选择下一个进程：
    如果找到合适的进程，则将其设置为下一个运行的进程。
    如果没有找到合适的进程，则选择空闲进程 idleproc。
5.更新进程运行次数：增加 next->runs。
6.切换进程：如果 next 不是当前进程，则调用 proc_run 切换到 next 进程。
7.恢复中断状态：通过 local_intr_restore 恢复中断状态。
*/
void
schedule(void) {
    bool intr_flag;
    list_entry_t *le, *last;
    struct proc_struct *next = NULL;
    local_intr_save(intr_flag);
    {
        current->need_resched = 0;
        last = (current == idleproc) ? &proc_list : &(current->list_link);
        le = last;
        do {
            if ((le = list_next(le)) != &proc_list) {
                next = le2proc(le, list_link);
                if (next->state == PROC_RUNNABLE) {
                    break;
                }
            }
        } while (le != last);
        if (next == NULL || next->state != PROC_RUNNABLE) {
            next = idleproc;
        }
        next->runs ++;
        if (next != current) {
            proc_run(next);
        }
    }
    local_intr_restore(intr_flag);
}

