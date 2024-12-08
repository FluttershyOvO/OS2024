# lab4 实验报告

## 练习一

```c++
proc->state = PROC_UNINIT;
proc->pid = -1;
proc->runs = 0;
proc->kstack = 0;
proc->need_resched = 0;
proc->parent = NULL;
proc->mm = NULL;
memset(&(proc->context), 0, sizeof(struct context));
proc->tf = NULL;
proc->cr3 = boot_cr3;
proc->flags = 0;
memset(proc->name, 0, PROC_NAME_LEN + 1);
```

分配并返回一个新的struct proc_struct结构, 用于存储新建立的内核线程的管理信息

- state  进程的当前状态
- pid  进程ID
- runs  进程运行次数
- kstack  分配进程的内核栈, 用于进程切换时存储进程的信息
- need_resched  标识进程是否需要重新调度
- parent  指向父进程的指针
- mm  进程的内存管理结构体
- context  用于保存进程的上下文
- tf  指向进程的 Trap Frame 的指针, 用于中断处理保存进程的寄存器状态
- cr3  指定进程的页目录, 用于虚拟地址到物理地址的映射
- flages  进程的标志位
- name  进程名称



**成员变量的含义和作用**

- context(struct context)
  - 主要用于进程间的上下文切换
  - 存储了进程切换时需要保存的关键几个寄存器的值
  - 保存上下文切换时当前进程的执行状态
  - 例如 栈指针, 程序计数器等
  - 作用主要是支持上下文切换, 通过调用`switch_to`去实现进程的寄存器保存, 从而实现后续切换回此进程时恢复状态
- tf(struct trapframe*)
  - 用于处理系统调用和中断时的寄存器状态保存与恢复
  - 保存进程的中断帧, 32个通用寄存器和异常相关的寄存器
  - 作用在于总是指向内核栈的某个位置，用于记录进程在被中断前的状态
  - 当内核需要跳回用户空间时，需要调整`trapframe`以恢复进程继续执行所需的各寄存器值



## 练习二: 为新创建的内核线程分配资源

- alloc_proc 是找到一小块内存用以记录进程的必要信息，并没有实际分配这些资源
- do_fork 则是实际创建新的内核线程, 为新创建的内核线程分配资源, 并完成父进程和子进程的相关初始化工作

```c++
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
```

- `proc = alloc_proc();`  调用alloc_proc()函数来分配一个新的进程控制块(proc_struct)
- `proc->parent = current`  将新创建进程的父进程设置为当前正在执行的进程
- `setup_kstack(proc)`  分配内核栈, 为新进程分配内核栈, 进程在内核态执行时使用的栈
- `copy_mm(clone_flags, proc)`  复制内存管理信息, 传入了克隆标志, 内核线程的创建主要是确保与父进程共享或独立管理各自的虚拟空间
- `copy_thread(proc, stack, tf)`  复制线程的信息, 设置新进程的trapframe和context
- `int pid = get_pid()`  分配一个唯一的进程的ID
- `proc->pid = pid`  将分配的进程ID添加到新进程的控制块中
- `hash_proc(proc)`  将新进程添加到PID的哈希表中, 以便查找
- `list_add(&proc_list, &(proc->list_link))`  将新进程添加到进程列表中, 用于进程的管理和调度
- `nr_process++`  添加线程数量
- `proc->state = PROC_RUNNABLE`  设置进程状态, 表示已经准备好运行, 能够被调度器调度
- `ret = proc->pid`  返回创建的新进程PID



**ucore是否做到给每个新fork的线程一个唯一的id**

在代码中`int pid = get_pid();`实现了ucore为新创建的线程分配一个唯一的PID

`get_pid()`的实现逻辑是

- 用last_pid记录上一次分配的PID
- 分配新的PID是将last_pid加一
- 将PID去遍历进程链表查看是否重复
- 若重复且到达最大PID值, 就从1开始重新递增来获取唯一的PID值

每次分配的PID都是进程链表中没有被占用的

从而能够保证每次分配PID都是唯一的



## 练习三

proc_run用于将指定的进程切换到CPU上运行

```c++
bool intr_flag;
struct proc_struct *prev = current, *next = proc;
local_intr_save(intr_flag);
{
    current = proc;
    lcr3(next->cr3);
    switch_to(&(prev->context), &(next->context));
}
local_intr_restore(intr_flag);
```

- `proc != current`  检查要切换的进程是否与当前进程相同
- `local_intr_save(intr_flag);`  禁用中断, 并保存中断状态
- `current = proc;` 切换进程
- ` lcr3(next->cr3);`  切换页表, 将当前的CR3寄存器更改为目标进程的页表基地址
- `switch_to(&(prev->context), &(next->context));`  使用switch_to函数去进行上下文切换
- `local_intr_restore(intr_flag);`  恢复允许中断



**创建且运行了几个内核线程**

2个

- idleproc 空闲进程 闲逛进程
  - 系统中的第一个内核进程
  - 在proc_init函数中被初始化, PID被设置为0
  - 系统启动时自动创建
  - 当没有其他进程时才会占用CPU运行
- initproc 初始化进程
  - 系统中的第二个内核进程
  - 在proc_init函数中通过调用kernel_thread函数创建
  - PID被设置为1
  - 用于执行用户空间的初始化操作



## 扩展练习

**`local_intr_save(intr_flag);....local_intr_restore(intr_flag);`是如何实现开关中断的？**

```c++
// kern/sync/sync.h
#define local_intr_save(x)      do { x = __intr_save(); } while (0)
#define local_intr_restore(x)   __intr_restore(x);

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


// kern/driver/intr.c
/* intr_enable - enable irq interrupt */
void intr_enable(void) { set_csr(sstatus, SSTATUS_SIE); }

/* intr_disable - disable irq interrupt */
void intr_disable(void) { clear_csr(sstatus, SSTATUS_SIE); }


// libs/riscv.h
#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })
#define set_csr(reg, bit) ({ unsigned long __tmp; \
  asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })
#define clear_csr(reg, bit) ({ unsigned long __tmp; \
  asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

```

**`local_intr_save(intr_flag);`**

- `local_intr_save(x)`会调用`__intr_save`
- 读取`sstatus`寄存器，判断`SIE`位
- 如果该位为1, 表示当前可以中断
- 则调用 `intr_disable`, 将该位置为0, 禁用中断
- 返回1, 将参数x赋值为1

**`local_intr_restore(intr_flag);`**

- `local_intr_restore(x)`会调用`__intr_restore`
- 读取`sstatus`寄存器，判断`SIE`位
- 如果该值为1, 说明是禁用中断
- 调用`intr_enable`, 将sstatus的SIE位置为1, 启用中断
- 如果该值为0, 不需要操作