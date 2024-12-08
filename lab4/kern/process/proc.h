#ifndef __KERN_PROCESS_PROC_H__
#define __KERN_PROCESS_PROC_H__

#include <defs.h>
#include <list.h>
#include <trap.h>
#include <memlayout.h>


// process's state in his life cycle
enum proc_state {
    PROC_UNINIT = 0,  // uninitialized
    PROC_SLEEPING,    // sleeping
    PROC_RUNNABLE,    // runnable(maybe running)
    PROC_ZOMBIE,      // almost dead, and wait parent proc to reclaim his resource
};

//用于存储RISC-V架构下的寄存器状态
struct context {
    uintptr_t ra;//返回地址寄存器
    uintptr_t sp;//堆栈指针
    uintptr_t s0;
    uintptr_t s1;
    uintptr_t s2;
    uintptr_t s3;
    uintptr_t s4;
    uintptr_t s5;
    uintptr_t s6;
    uintptr_t s7;
    uintptr_t s8;
    uintptr_t s9;
    uintptr_t s10;
    uintptr_t s11;//s0 至 s11：保存寄存器（ callee-saved registers）
};

#define PROC_NAME_LEN               15
#define MAX_PROCESS                 4096
#define MAX_PID                     (MAX_PROCESS * 2)

extern list_entry_t proc_list;

//表示操作系统中的进程
struct proc_struct {
    enum proc_state state;                      // 进程的状态
    //四种：PROC_UNINIT、PROC_SLEEPING、PROC_RUNNABLE、PROC_ZOMBIE
    int pid;                                    // Process ID
    int runs;                                   // 运行次数
    uintptr_t kstack;                           // 内核栈
    volatile bool need_resched;                 // 是否需要重新调度
    //当 need_resched 被设置为真（通常是1），
    //表示当前进程不再希望继续执行，或者有更高优先级的进程准备就绪，此时调度器应该考虑选择一个新的进程来执行。
  
    struct proc_struct *parent;                 // 父进程指针
    struct mm_struct *mm;                       // 内存管理信息
    struct context context;                     // 上下文切换信息
    struct trapframe *tf;                       // 中断陷阱帧
    uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT)
    uint32_t flags;                             // Process flag 标志位
    char name[PROC_NAME_LEN + 1];               // Process name 进程名称
    list_entry_t list_link;                     // Process link list 
    list_entry_t hash_link;                     // Process hash list
};
/*  state: 进程状态，例如运行、就绪、阻塞等。
    pid: 进程ID，唯一标识一个进程。
    runs: 进程的运行次数，记录进程被调度执行的次数。
    kstack: 进程的内核栈地址，用于保存内核模式下的上下文信息。
    need_resched: 布尔值，表示是否需要重新调度以释放CPU。
    parent: 指向父进程的指针。
    mm: 指向进程的内存管理结构体，管理进程的虚拟内存。
    context: 上下文切换时使用的上下文信息。
    tf: 当前中断的陷阱帧，保存中断发生时的寄存器状态。
    cr3: CR3寄存器的值，指向页目录表的基地址。
    flags: 进程的标志位，用于存储各种标志。
    name: 进程的名称。
    list_link: 进程在链表中的链接信息。
    hash_link: 进程在哈希表中的链接信息。
*/

#define le2proc(le, member)         \
    to_struct((le), struct proc_struct, member)

extern struct proc_struct *idleproc, *initproc, *current;

void proc_init(void);
void proc_run(struct proc_struct *proc);
int kernel_thread(int (*fn)(void *), void *arg, uint32_t clone_flags);

char *set_proc_name(struct proc_struct *proc, const char *name);
char *get_proc_name(struct proc_struct *proc);
void cpu_idle(void) __attribute__((noreturn));

struct proc_struct *find_proc(int pid);
int do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf);
int do_exit(int error_code);

#endif /* !__KERN_PROCESS_PROC_H__ */

