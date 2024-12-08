#include <proc.h>
#include <kmalloc.h>
#include <string.h>
#include <sync.h>
#include <pmm.h>
#include <error.h>
#include <sched.h>
#include <elf.h>
#include <vmm.h>
#include <trap.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* ------------- process/thread mechanism design&implementation -------------
(an simplified Linux process/thread mechanism )
introduction:
  ucore implements a simple process/thread mechanism. process contains the independent memory sapce, at least one threads
for execution, the kernel data(for management), processor state (for context switch), files(in lab6), etc. ucore needs to
manage all these details efficiently. In ucore, a thread is just a special kind of process(share process's memory).
------------------------------
process state       :     meaning               -- reason
    PROC_UNINIT     :   uninitialized           -- alloc_proc
    PROC_SLEEPING   :   sleeping                -- try_free_pages, do_wait, do_sleep
    PROC_RUNNABLE   :   runnable(maybe running) -- proc_init, wakeup_proc, 
    PROC_ZOMBIE     :   almost dead             -- do_exit

-----------------------------
process state changing:
                                            
  alloc_proc                                 RUNNING
      +                                   +--<----<--+
      +                                   + proc_run +
      V                                   +-->---->--+ 
PROC_UNINIT -- proc_init/wakeup_proc --> PROC_RUNNABLE -- try_free_pages/do_wait/do_sleep --> PROC_SLEEPING --
                                           A      +                                                           +
                                           |      +--- do_exit --> PROC_ZOMBIE                                +
                                           +                                                                  + 
                                           -----------------------wakeup_proc----------------------------------
-----------------------------
process relations
parent:           proc->parent  (proc is children)
children:         proc->cptr    (proc is parent)
older sibling:    proc->optr    (proc is younger sibling)
younger sibling:  proc->yptr    (proc is older sibling)
-----------------------------
related syscall for process:
SYS_exit        : process exit,                           -->do_exit
SYS_fork        : create child process, dup mm            -->do_fork-->wakeup_proc
SYS_wait        : wait process                            -->do_wait
SYS_exec        : after fork, process execute a program   -->load a program and refresh the mm
SYS_clone       : create child thread                     -->do_fork-->wakeup_proc
SYS_yield       : process flag itself need resecheduling, -- proc->need_sched=1, then scheduler will rescheule this process
SYS_sleep       : process sleep                           -->do_sleep 
SYS_kill        : kill process                            -->do_kill-->proc->flags |= PF_EXITING
                                                                 -->wakeup_proc-->do_wait-->do_exit   
SYS_getpid      : get the process's pid

*/


/*ucore操作系统中进程/线程机制的设计与实现的注释。
主要介绍了进程的状态、状态转换、进程间的关系以及相关的系统调用。

1. **进程状态**：
   - `PROC_UNINIT`：未初始化状态，由`alloc_proc`创建。
   - `PROC_SLEEPING`：睡眠状态，由`try_free_pages`、`do_wait`、`do_sleep`等函数进入。
   - `PROC_RUNNABLE`：可运行状态，由`proc_init`、`wakeup_proc`等函数进入。
   - `PROC_ZOMBIE`：几乎死亡状态，由`do_exit`进入。

2. **状态转换**：
   - 从`PROC_UNINIT`通过`proc_init`或`wakeup_proc`变为`PROC_RUNNABLE`。
   - 从`PROC_RUNNABLE`通过`try_free_pages`、`do_wait`、`do_sleep`变为`PROC_SLEEPING`。
   - 从`PROC_RUNNABLE`通过`do_exit`变为`PROC_ZOMBIE`。
   - 从`PROC_SLEEPING`通过`wakeup_proc`变为`PROC_RUNNABLE`。

3. **进程关系**：
   - 父进程：`proc->parent`。
   - 子进程：`proc->cptr`。
   - 较年长的兄弟进程：`proc->optr`。
   - 较年轻的兄弟进程：`proc->yptr`。

4. **相关系统调用**：
   - `SYS_exit`：进程退出，调用`do_exit`。
   - `SYS_fork`：创建子进程，复制内存管理结构，调用`do_fork`和`wakeup_proc`。
   - `SYS_wait`：等待进程，调用`do_wait`。
   - `SYS_exec`：执行程序，加载程序并刷新内存管理结构。
   - `SYS_clone`：创建子线程，调用`do_fork`和`wakeup_proc`。
   - `SYS_yield`：进程标记自己需要重新调度，设置`proc->need_sched=1`。
   - `SYS_sleep`：进程睡眠，调用`do_sleep`。
   - `SYS_kill`：杀死进程，调用`do_kill`，设置`proc->flags |= PF_EXITING`，然后唤醒进程，等待其退出。
   - `SYS_getpid`：获取进程的PID。
*/


// the process set's list
list_entry_t proc_list;//用于存储进程的链表

#define HASH_SHIFT          10                  //2的十次方
#define HASH_LIST_SIZE      (1 << HASH_SHIFT)
//HASH_SHIFT 和 HASH_LIST_SIZE：定义了哈希表的大小，HASH_LIST_SIZE 是 1 << HASH_SHIFT，即 1024。

//pid_hashfn(x)：这是一个宏，用于计算给定PID的哈希值。
//它调用了hash32函数，并将PID作为输入，同时传递了HASH_SHIFT参数以限制输出的范围到哈希表的大小之内。
//这保证了哈希值会落在0到1023之间。

#define pid_hashfn(x)       (hash32(x, HASH_SHIFT))
//计算给定PID的哈希值，使用 hash32 函数。

//这里哈希表有1024个桶（bucket），每个桶可以链接一个或多个链表节点，这些节点包含了具有相同哈希值的进程。


// has list for process set based on pid
static list_entry_t hash_list[HASH_LIST_SIZE];
//这是一个静态数组，每个元素都是一个list_entry_t类型的链表头。
//它用于根据PID哈希值快速查找对应的进程控制块（proc_struct）。
//哈希表的大小由HASH_LIST_SIZE定义，即1024个桶。


// idle proc
struct proc_struct *idleproc = NULL;//idle:空闲的
//指向空闲进程（idle process）的指针。
//空闲进程是系统启动后创建的第一个进程，当没有其他可运行的进程时，CPU会执行这个进程。
//它的主要任务是保持CPU忙碌，等待下一个可调度事件的发生。

// init proc
struct proc_struct *initproc = NULL;
//指向初始化进程（init process）的指针。这是用户空间中的第一个进程，通常负责启动其他用户级服务或应用程序。

// current proc
struct proc_struct *current = NULL;
//指向当前正在执行的进程的指针。这是一个全局变量，使得内核中的任何地方都可以访问到当前运行的进程信息。

static int nr_process = 0;//记录当前系统中活动进程的数量。

void kernel_thread_entry(void);
//这个函数是所有内核线程的入口点。当一个新的内核线程被创建并开始执行时，它从这里启动。

void forkrets(struct trapframe *tf);
void switch_to(struct context *from, struct context *to);

// alloc_proc - alloc a proc_struct and init all fields of proc_struct
//用于分配并初始化一个新的进程结构体proc_struct
static struct proc_struct *
alloc_proc(void) {
    struct proc_struct *proc = kmalloc(sizeof(struct proc_struct));
    if (proc != NULL) {
    //LAB4:EXERCISE1 2211545，2210203
    /*
     * below fields in proc_struct need to be initialized
     *       enum proc_state state;                      // Process state
     *       int pid;                                    // Process ID
     *       int runs;                                   // the running times of Proces
     *       uintptr_t kstack;                           // Process kernel stack
     *       volatile bool need_resched;                 // bool value: need to be rescheduled to release CPU?
     *       struct proc_struct *parent;                 // the parent process
     *       struct mm_struct *mm;                       // Process's memory management field
     *       struct context context;                     // Switch here to run process
     *       struct trapframe *tf;                       // Trap frame for current interrupt
     *       uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT)
     *       uint32_t flags;                             // Process flag
     *       char name[PROC_NAME_LEN + 1];               // Process name
     */

    proc->state = PROC_UNINIT;//进程的状态设置为PROC_UNINIT，表明进程刚开始分配，尚未完全初始化
    proc->pid = -1;//进程ID初始值设为-1，表示进程ID尚未分配
    proc->runs = 0;//设为0，记录该进程运行的次数
    proc->kstack = 0;//指向内核栈地址，这里初始值设为0，直到实际分配
    proc->need_resched = 0;//设为0，表示不需要立即重新调度
    proc->parent = NULL;//指向父进程，对于idle进程而言，其没有父进程，因此设为NULL
    
    //mm_struct结构体（简称mm）是进程地址空间的管理器

    proc->mm = NULL;//指向内存描述符，对于idle进程，它不会有自己的地址空间，因此也设为NULL
    memset(&(proc->context), 0, sizeof(struct context));
    proc->tf = NULL;
    proc->cr3 = boot_cr3;
    proc->flags = 0;
    memset(proc->name, 0, PROC_NAME_LEN + 1);

    }
    return proc;
}

// set_proc_name - set the name of proc
//用于设置进程结构体 proc 中的名称字段 name
char *
set_proc_name(struct proc_struct *proc, const char *name) {
    memset(proc->name, 0, sizeof(proc->name));//使用 memset 函数将 proc->name 字符数组清零。
    return memcpy(proc->name, name, PROC_NAME_LEN);//使用 memcpy 函数将传入的 name 复制到 proc->name 中。
}//返回 memcpy 的结果，即 proc->name。

// get_proc_name - get the name of proc
//用于获取进程结构体 proc 中的进程名称
char *
get_proc_name(struct proc_struct *proc) {
    static char name[PROC_NAME_LEN + 1];
    //额外的一个字节用于存储字符串的终止符 \0，这使得 name 成为一个合法的、可以被标准库函数（如 printf, strlen 等）正确处理的字符串。
    
    memset(name, 0, sizeof(name));
    return memcpy(name, proc->name, PROC_NAME_LEN);
}

// get_pid - alloc a unique pid for process
//为进程分配一个唯一的PID（进程标识符）
static int
get_pid(void) {
    static_assert(MAX_PID > MAX_PROCESS);
    //确保最大PID (MAX_PID) 大于系统中可能的最大进程数 (MAX_PROCESS)
    //为了防止所有可用的PID都被占用，从而无法为新的进程分配PID。

    struct proc_struct *proc;
    list_entry_t *list = &proc_list, *le;
    static int next_safe = MAX_PID, last_pid = MAX_PID;
    if (++ last_pid >= MAX_PID) {
        last_pid = 1;
        goto inside;
    }//从上次分配的PID开始，尝试分配一个新的PID。
    if (last_pid >= next_safe) {
    inside:
        next_safe = MAX_PID;
    repeat:
        le = list;
        while ((le = list_next(le)) != list) {
            proc = le2proc(le, list_link);
            if (proc->pid == last_pid) {
                if (++ last_pid >= next_safe) {
                    if (last_pid >= MAX_PID) {
                        last_pid = 1;//边界检查：如果新的PID超出范围，则重置为最小值1。
                    }
                    next_safe = MAX_PID;
                    goto repeat;//冲突检测：遍历所有活动进程，确保新分配的PID不与任何现有进程的PID冲突。
                }
            }
            else if (proc->pid > last_pid && next_safe > proc->pid) {
                next_safe = proc->pid;
            }//更新安全边界：根据遍历结果更新next_safe，以便后续分配更加高效。
        }
    }
    return last_pid;
}

// proc_run - make process "proc" running on cpu
// NOTE: before call switch_to, should load  base addr of "proc"'s new PDT
void
proc_run(struct proc_struct *proc) {
    //检查要切换的进程是否与当前正在运行的进程相同，如果相同则不需要切换。
    if (proc != current) {
        // LAB4:EXERCISE3 2211545，2210203
        /*
        * Some Useful MACROs, Functions and DEFINEs, you can use them in below implementation.
        * MACROs or Functions:
        *   local_intr_save():        Disable interrupts
        *   local_intr_restore():     Enable Interrupts
        *   lcr3():                   Modify the value of CR3 register
        *   switch_to():              Context switching between two processes
        */
	
       bool intr_flag;
       struct proc_struct *prev = current;
       struct proc_struct *next = proc;
       local_intr_save(intr_flag); // 禁用中断
       {
            current=proc; // 更新当前线程为proc
            lcr3(next->cr3); // 更换页表
            switch_to(&(prev->context),&(next->context)); // 上下文切换
       }
       local_intr_restore(intr_flag); // 开启中断
       
    }
}

// forkret -- the first kernel entry point of a new thread/process
//forkret 是新线程或进程的第一个内核入口点。
//注释说明：forkret 的地址在 copy_thread 函数中设置。
//执行流程：在 switch_to 之后，当前进程会从这里开始执行。
// NOTE: the addr of forkret is setted in copy_thread function
//       after switch_to, the current proc will execute here.
static void
forkret(void) {
    forkrets(current->tf);
}

// hash_proc - add proc into proc hash_list
//将一个进程结构体 proc 插入到哈希表中
static void
hash_proc(struct proc_struct *proc) {
    list_add(hash_list + pid_hashfn(proc->pid), &(proc->hash_link));
}

// find_proc - find proc frome proc hash_list according to pid
//用于在进程表中查找指定PID的进程结构体指针
struct proc_struct *
find_proc(int pid) {
    if (0 < pid && pid < MAX_PID) {//确保pid在有效范围内
        list_entry_t *list = hash_list + pid_hashfn(pid), *le = list;//初始化le为hash_list[pid_hashfn(pid)]，即第一个链表头节点
        while ((le = list_next(le)) != list) {//遍历链表，直到遍历结束
            struct proc_struct *proc = le2proc(le, hash_link);
            if (proc->pid == pid) {//找到匹配的进程结构体，返回该指针
                return proc;
            }
        }
    }
    return NULL;//未找到匹配的进程，返回NULL
}

// kernel_thread - create a kernel thread using "fn" function
// NOTE: the contents of temp trapframe tf will be copied to 
//       proc->tf in do_fork-->copy_thread function
//用于创建一个新的内核线程
int
kernel_thread(int (*fn)(void *), void *arg, uint32_t clone_flags) {
    struct trapframe tf;
    memset(&tf, 0, sizeof(struct trapframe));//初始化 trapframe 结构体
    tf.gpr.s0 = (uintptr_t)fn;
    tf.gpr.s1 = (uintptr_t)arg;
    tf.status = (read_csr(sstatus) | SSTATUS_SPP | SSTATUS_SPIE) & ~SSTATUS_SIE;
    tf.epc = (uintptr_t)kernel_thread_entry;//设置入口地址：将 kernel_thread_entry 函数的地址赋值给 epc 寄存器。
    return do_fork(clone_flags | CLONE_VM, 0, &tf);
}

// setup_kstack - alloc pages with size KSTACKPAGE as process kernel stack
//为进程结构体 proc 分配内核栈
static int
setup_kstack(struct proc_struct *proc) {
    struct Page *page = alloc_pages(KSTACKPAGE);
    if (page != NULL) {
        proc->kstack = (uintptr_t)page2kva(page);
        return 0;//如果分配成功，将分配的内存地址转换为虚拟地址并赋值给 proc->kstack。
    }
    return -E_NO_MEM;//如果分配失败，返回 -E_NO_MEM 表示内存不足。
}

// put_kstack - free the memory space of process kernel stack
//释放进程结构体 proc 中的内核栈内存
static void
put_kstack(struct proc_struct *proc) {
    free_pages(kva2page((void *)(proc->kstack)), KSTACKPAGE);
    //kva2page 将虚拟地址转换为物理页面。
    //调用 free_pages 函数,释放由 kva2page 函数转换后的物理页面。
}

// copy_mm - process "proc" duplicate OR share process "current"'s mm according clone_flags
//         - if clone_flags & CLONE_VM, then "share" ; else "duplicate"
//根据传入的 clone_flags 和 proc 参数复制内存管理结构。
static int
copy_mm(uint32_t clone_flags, struct proc_struct *proc) {
    assert(current->mm == NULL);
    /* do nothing in this project */
    return 0;
}
/*mm的作用

mm_struct结构体（简称mm）是进程地址空间的管理器，它用于管理和维护一个进程的虚拟内存区域（VMA, Virtual Memory Area）。
每个proc_struct（进程控制块）中的mm成员指向该进程对应的mm_struct实例。

mm成员变量存在于每个proc_struct实例中，代表了该进程的内存管理信息。
如果mm为空（即NULL），则表示该进程没有自己的地址空间，这通常是内核线程的情况，例如idle进程。对于用户进程，mm将指向一个有效的mm_struct实例，其中包含了该进程所有必要的内存映射信息。
*/


// copy_thread - setup the trapframe on the  process's kernel stack top and
//             - setup the kernel entry point and stack of process
//用于在进程创建时复制父进程的线程上下文到子进程中。
static void
copy_thread(struct proc_struct *proc, uintptr_t esp, struct trapframe *tf) {
    proc->tf = (struct trapframe *)(proc->kstack + KSTACKSIZE - sizeof(struct trapframe));
    *(proc->tf) = *tf;//初始化子进程的陷阱帧

    // Set a0 to 0 so a child process knows it's just forked
    //设置子进程的寄存器值
    proc->tf->gpr.a0 = 0;
    proc->tf->gpr.sp = (esp == 0) ? (uintptr_t)proc->tf : esp;
    //根据传入的 esp 值决定子进程的栈指针 sp，如果 esp 为 0，则使用陷阱帧的地址作为栈指针。

    //设置子进程的上下文
    proc->context.ra = (uintptr_t)forkret;//将 ra 寄存器(返回地址寄存器)设为 forkret 函数的地址，以便子进程从 fork 返回时执行 forkret。
    proc->context.sp = (uintptr_t)(proc->tf);//将 sp 寄存器设为子进程的陷阱帧地址。
}

/* do_fork -     parent process for a new child process
 * @clone_flags: used to guide how to clone the child process
 * @stack:       the parent's user stack pointer. if stack==0, It means to fork a kernel thread.
 * @tf:          the trapframe info, which will be copied to child process's proc->tf
 */
int
do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf) {
    int ret = -E_NO_FREE_PROC;
    struct proc_struct *proc;
    if (nr_process >= MAX_PROCESS) {
        goto fork_out;
    }
    ret = -E_NO_MEM;
    //LAB4:EXERCISE2 2211545，2210203
    /*
     * Some Useful MACROs, Functions and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   alloc_proc:   create a proc struct and init fields (lab4:exercise1)
     * 创建并初始化进程结构体
     * 
     *   setup_kstack: alloc pages with size KSTACKPAGE as process kernel stack
     * 为进程分配大小为KSTACKPAGE的内核栈
     * 
     *   copy_mm:      process "proc" duplicate OR share process "current"'s mm according clone_flags
     *                 if clone_flags & CLONE_VM, then "share" ; else "duplicate"
     * 根据clone_flags决定是复制还是共享当前进程的内存管理结构（mm）。
     * 如果clone_flags包含CLONE_VM，则共享；否则，复制。
     * 
     *   copy_thread:  setup the trapframe on the  process's kernel stack top and
     *                 setup the kernel entry point and stack of process
     *在进程的内核栈顶部设置陷阱帧，并设置进程的内核入口点和栈。
     * 
     *   hash_proc:    add proc into proc hash_list
     * 将进程添加到进程哈希表中
     * 
     *   get_pid:      alloc a unique pid for process
     * 为进程分配唯一的 PID
     * 
     *   wakeup_proc:  set proc->state = PROC_RUNNABLE
     * 将进程状态设置为可运行
     * 
     * VARIABLES:
     *   proc_list:    the process set's list
     *   nr_process:   the number of process set
     */

    //    1. call alloc_proc to allocate a proc_struct
    //    2. call setup_kstack to allocate a kernel stack for child process
    //    3. call copy_mm to dup OR share mm according clone_flag
    //    4. call copy_thread to setup tf & context in proc_struct
    //    5. insert proc_struct into hash_list && proc_list
    //    6. call wakeup_proc to make the new child process RUNNABLE
    //    7. set ret vaule using child proc's pid

    proc = alloc_proc();
    proc->parent = current;
    setup_kstack(proc);
    copy_mm(clone_flags, proc);
    copy_thread(proc, stack, tf);
    int pid = get_pid();
    proc->pid = pid;
    hash_proc(proc);
    list_add(&proc_list, &(proc->list_link));
    nr_process++;
    proc->state = PROC_RUNNABLE;
    ret = proc->pid;
    
    /*
     if((proc = alloc_proc())==NULL)
    {
        goto fork_out;
    }
    proc->parent=current;
    if(setup_kstack(proc))
    {
        goto bad_fork_cleanup_kstack;
    }
    if(copy_mm(clone_flags,proc))
    {
        goto bad_fork_cleanup_proc;
    }
    copy_thread(proc,stack,tf);
   bool intr_flag;
   local_intr_save(intr_flag);
    {
        proc->pid=get_pid();
        hash_proc(proc);
        list_add(&proc_list,&(proc->list_link));
        nr_process++;
    }
   local_intr_restore(intr_flag);
   
   wakeup_proc(proc);
   ret = proc->pid;
   */

fork_out:
    return ret;

bad_fork_cleanup_kstack:
    put_kstack(proc);
bad_fork_cleanup_proc:
    kfree(proc);
    goto fork_out;
}

// do_exit - called by sys_exit
//   1. call exit_mmap & put_pgdir & mm_destroy to free the almost all memory space of process
//   2. set process' state as PROC_ZOMBIE, then call wakeup_proc(parent) to ask parent reclaim itself.
//   3. call scheduler to switch to other process
int
do_exit(int error_code) {
    panic("process exit!!.\n");
}

// init_main - the second kernel thread used to create user_main kernel threads
static int
init_main(void *arg) {
    cprintf("this initproc, pid = %d, name = \"%s\"\n", current->pid, get_proc_name(current));
    cprintf("To U: \"%s\".\n", (const char *)arg);
    cprintf("To U: \"en.., Bye, Bye. :)\"\n");
    return 0;
}

// proc_init - set up the first kernel thread idleproc "idle" by itself and 
//           - create the second kernel thread init_main
void
proc_init(void) {
    int i;

    list_init(&proc_list);
    for (i = 0; i < HASH_LIST_SIZE; i ++) {
        list_init(hash_list + i);
    }

    if ((idleproc = alloc_proc()) == NULL) {
        panic("cannot alloc idleproc.\n");
    }

    // check the proc structure
    int *context_mem = (int*) kmalloc(sizeof(struct context));
    memset(context_mem, 0, sizeof(struct context));
    int context_init_flag = memcmp(&(idleproc->context), context_mem, sizeof(struct context));
    //在本实验中，context用于存储线程的执行上下文，特别是用于线程切换时保存和恢复线程的状态。
    //例如，在创建idle进程时，通过检查idleproc->context是否已经被正确初始化（memcmp对比），可以验证alloc_proc函数是否已经正确地初始化了这个结构。

    int *proc_name_mem = (int*) kmalloc(PROC_NAME_LEN);
    memset(proc_name_mem, 0, PROC_NAME_LEN);
    int proc_name_flag = memcmp(&(idleproc->name), proc_name_mem, PROC_NAME_LEN);

    if(idleproc->cr3 == boot_cr3 && idleproc->tf == NULL && !context_init_flag
        && idleproc->state == PROC_UNINIT && idleproc->pid == -1 && idleproc->runs == 0
        && idleproc->kstack == 0 && idleproc->need_resched == 0 && idleproc->parent == NULL
        && idleproc->mm == NULL && idleproc->flags == 0 && !proc_name_flag
    ){
        cprintf("alloc_proc() correct!\n");

    }
    
    idleproc->pid = 0;
    idleproc->state = PROC_RUNNABLE;
    idleproc->kstack = (uintptr_t)bootstack;
    idleproc->need_resched = 1;
    set_proc_name(idleproc, "idle");
    nr_process ++;

    current = idleproc;

    int pid = kernel_thread(init_main, "Hello world!!", 0);
    if (pid <= 0) {
        panic("create init_main failed.\n");
    }


    initproc = find_proc(pid);
    set_proc_name(initproc, "init");

    assert(idleproc != NULL && idleproc->pid == 0);
    assert(initproc != NULL && initproc->pid == 1);
}

// cpu_idle - at the end of kern_init, the first kernel thread idleproc will do below works
void
cpu_idle(void) {
    while (1) {
        if (current->need_resched) {
            schedule();
        }//idleproc内核线程的工作就是不停地查询，看是否有其他内核线程可以执行了，如果有，马上让调度器选择那个内核线程执行
    }
}

