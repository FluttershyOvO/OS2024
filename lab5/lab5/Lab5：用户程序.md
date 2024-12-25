# lab5 实验报告

## 练习0


```c++
// alloc_proc - alloc a proc_struct and init all fields of proc_struct
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


     //LAB5 YOUR CODE : 2211545，2210203 (update LAB4 steps)
     /*
     * below fields(add in LAB5) in proc_struct need to be initialized  
     *       uint32_t wait_state;                        // waiting state
     *       struct proc_struct *cptr, *yptr, *optr;     // relations between processes
     */
    proc->wait_state = 0;
    proc->cptr = NULL; // Child Pointer 表示当前进程的子进程
    proc->optr = NULL; // Older Sibling Pointer 表示当前进程的上一个兄弟进程
    proc->yptr = NULL; // Younger Sibling Pointer 表示当前进程的下一个兄弟进程

    }
    return proc;
}
```

需要在/kern/process/proc.c中alloc_proc(void)的LAB4基础上添加代码

```c++
    proc->wait_state = 0;
    proc->cptr = NULL; 
    proc->optr = NULL; 
    proc->yptr = NULL; 
```

- `proc->wait_state = 0;`  表示进程的等待状态, 进程可能需要等待某个事件之后才能执行
- `proc->cptr = NULL;`  Child Pointer 表示指向当前进程的子进程, 进程创建的时候, 父进程会更新此指针指向新创建的子进程
- `proc->optr = NULL;`  Older Sibling Pointer 表示当前进程的上一个兄弟进程,  多进程系统中, 同一个父进程有多个子进程
- `proc->yptr = NULL;`  Younger Sibling Pointer 表示当前进程的下一个兄弟进程

在原来进程创建的基础上为了实现多进程系统, 添加了等待状态 为了处理进程之间的状态, 还添加了父进程与兄弟进程的指针, 用来处理父子进程和兄弟进程之间的关系



```c++
    if((proc = alloc_proc()) == NULL)
    {
        goto fork_out;
    }
    proc->parent = current; 
    assert(current->wait_state == 0);  // 添加
    if(setup_kstack(proc) != 0)
    {
        goto bad_fork_cleanup_proc;
    }
    ;
    if(copy_mm(clone_flags, proc) != 0)
    {
        goto bad_fork_cleanup_kstack;
    }
    copy_thread(proc, stack, tf);
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        int pid = get_pid();
        proc->pid = pid;
        hash_proc(proc);
        set_links(proc);
    }
    local_intr_restore(intr_flag);
    wakeup_proc(proc);
    ret = proc->pid;
```

/kern/process/proc.c中do_fork需要修改代码

`assert(current->wait_state == 0);`  增加判断创建的进程是否处于等待状态



## 练习1



### 补充`load_icode`

```c++
    tf->gpr.sp = USTACKTOP;
    tf->epc = elf->e_entry;
    tf->status = sstatus & ~(SSTATUS_SPP | SSTATUS_SPIE);
```

- `tf->gpr.sp = USTACKTOP;`  设置了用户进程的栈指针sp, USTACKTOP是用户栈的顶部地址, 语句确保了用户程序的栈操作是从栈顶开始向下进行的
- `tf->epc = elf->e_entry;`  设置了用户进程的程序计数器epc, 即下一条指令的地址, elf->e_entry是二进制文件头部中指定的入口点, 即程序的起始执行地址, 确保了用户进程切换为用户模式后能够从正确的地址开始执行
- `tf->status = sstatus & ~(SSTATUS_SPP | SSTATUS_SPIE);`  设置了用户进程的状态寄存器status, sstatus是当前进程的状态寄存器的值
  -  `SPP`位控制是否处于用户模式(0表示用户模式，1表示内核模式), `SPIE`位控制是否允许中断
  - 清零这两个位确保了用户进程在切换到用户模式后，将处于非中断允许的状态，并且不在内核模式



### 执行经过

用户态进程被ucore选择占用CPU执行（RUNNING态）到具体执行应用程序第一条指令

2211545：
- 在`init_main`中通过`kernel_thread`调用`do_fork`创建并唤醒线程, 使其执行函数`user_main`, 这时该线程状态已经为`PROC_RUNNABLE`, 表明该线程开始运行
- 在`user_main`中通过宏`KERNEL_EXECVE`, 调用`kernel_execve`
- 在`kernel_execve`中执行`ebreak`, 发生断点异常, 转到`__alltraps`, 转到`trap`, 再到`trap_dispatch`, 然后到`exception_handler`, 最后到`CAUSE_BREAKPOINT`处
- 在`CAUSE_BREAKPOINT`处调用`syscall`
- 在`syscall`中根据参数, 确定执行`sys_exec`, 调用`do_execve`
- 在`do_execve`中首先会检查并释放旧的内存空间进行初始化, 调用`load_icode`, 加载文件
- 在`load_icode`中将提供的二进制程序加载到当前进程的内存空间中
- 设置中断帧, 使得中断返回后能够进入用户态执行程序
- 加载完毕后返回, 直到`__alltraps`的末尾, 接着执行`__trapret`后的内容, 到`sret`, 表示退出S态, 回到用户态执行, 这时开始执行用户的应用程序

2210203：
进程调度：
调度器（scheduler）选择一个可运行的进程（状态为PROC_RUNNABLE）并将其状态设置为PROC_RUNNING。
调度器调用proc_run函数，将选定的进程设置为当前进程（current）。

上下文切换：
在proc_run函数中，首先禁用中断以确保上下文切换过程中的原子性。
更新当前进程指针current为新选择的进程。
调用lcr3函数更新CR3寄存器，指向新进程的页目录表（PDT）基地址。
调用switch_to函数进行上下文切换，从旧进程的上下文切换到新进程的上下文。
最后恢复中断。

进入内核线程入口：
新进程的上下文切换完成后，控制流会跳转到forkret函数，这是新进程的第一个内核入口点。
forkret函数调用forkrets函数，传递当前进程的陷阱帧（trapframe）。
设置用户态环境：

forkrets函数根据陷阱帧（trapframe）设置用户态环境，包括设置栈指针（sp）、程序计数器（epc）和状态寄存器（sstatus）。
特别地，tf->gpr.sp被设置为用户栈顶地址，tf->epc被设置为用户程序的入口地址，tf->status被设置为适当的用户模式状态。

返回用户态：
设置完用户态环境后，控制流会从内核态返回到用户态，开始执行用户程序的第一条指令。


## 练习二

### 补充`copy_range`

/kern/mm/pmm.c/copy_range

copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end,bool share)

- `pde_t *to`：目标进程的页目录表指针
- `pde_t *from`：原进程的页目录表指针
- `uintptr_t start`：待复制内存的起始位置(用户空间)
- `uintptr_t end`：待复制内存的结束位置
- `bool share`：true表示共享页面, false表示创建页面的新副本, 这里只使用了false即创建新副本的情况



```c++
uintptr_t* src = page2kva(page); // 获取原页面的虚拟地址（内核空间）
uintptr_t* dst = page2kva(npage); // 获取目标页面的虚拟地址（内核空间）
memcpy(dst,src,PGSIZE); // 复制
ret = page_insert(to,npage,start,perm); // 目标进程的页目录表中插入新页
```

获取源地址和目的地址对应的内核虚拟地址, 然后拷贝内存, 最后将拷贝完成的页插入到页表中



### COW机制
2211545：
- 进程调用fork时, 将父线程的所有页表项设置为只读, 子进程继承父进程的内存映射, 在新线程的结构中只复制栈和虚拟内存的页表, 不为其分配新的页
- 子线程执行时, 当尝试写入页面操作时, 所有页面的权限都是只读的, 页不允许被修改, 便会触发异常
- 遇到异常时, 要重新分配物理页, 将原来页面中的内容复制到新的物理页中(copy_range), 并更新页表项使新页可写
- 在可写的新物理页中去实现写的操作, 并且这个物理页只有此进程可用

2210203：
概要设计
1.页表项标志：在页表项（PTE）中加入一个只读标志（如PTE_COW），用来表示该页面是否受到COW保护。如果设置了这个标志，任何对该页面的写操作都会触发一次页面错误。
2.页面错误处理程序：当进程尝试对一个带有COW标志的页面进行写入时，会产生一个页面错误。此时，内核的页面错误处理程序会被激活来处理这种情况。
3.页面拷贝：页面错误处理程序会做以下几件事情：
	分配一个新的物理页面。
	将原有页面的内容复制到新的页面。
	更新当前进程的页表，使其指向新分配的页面，并移除COW标志，允许写入。
	确保其他共享相同页面的进程仍然引用原来的只读页面。
4.同步和锁定：为了保证多线程环境下的正确性，必须实现适当的同步机制，以防止竞态条件。
5.释放机制：当最后一个进程不再需要某个页面时，确保能够正确地回收该页面所占用的资源。



## 练习3

### fork/exec/wait/exit的执行流程

**fork**

创建一个与当前进程（父进程）几乎完全相同的子进程

- 用户态: 用户程序调用 `fork()`, 触发系统调用中断
- 内核态
  - 系统调用处理函数 `syscall()` 被触发
  - 在 `syscall.c` 中, 调用 `sys_fork`, 传递当前进程的 `trapframe`
  - `sys_fork` 调用 `do_fork` 来实际创建子进程
    - 初始化新线程
    - 为新线程分配内核栈空间
    - 为新线程分配新的虚拟内存或与其他线程共享虚拟内存
    - 设置当前线程的上下文与中断帧
    - 将新线程插入哈希表和链表中
    - 唤醒新线程
    - 返回线程`id`
- 用户态: `fork()` 返回给父进程子进程的 PID, 子进程从 `fork()` 返回 `0`
- 用户态与内核态的交错
  - 用户态通过系统调用触发内核态执行 `do_fork`
  - 进程创建完成后, 使用 `sret` 指令返回用户态, 传递返回值



**exec**

用一个新的程序替换当前进程的地址空间和执行上下文

- 用户态: 调用 `exec()`, 触发系统调用中断

- 内核态

  - 在 `syscall.c` 中, 调用 `sys_exec`, 传递参数

  - `sys_exec` 调用 `do_execve` 进行程序替换

    - ```c++
      int
      do_execve(const char *name, size_t len, unsigned char *binary, size_t size) {
          struct mm_struct *mm = current->mm;
          if (!user_mem_check(mm, (uintptr_t)name, len, 0)) {
              return -E_INVAL;
          }
          if (len > PROC_NAME_LEN) {
              len = PROC_NAME_LEN;
          }
      
          char local_name[PROC_NAME_LEN + 1];
          memset(local_name, 0, sizeof(local_name));
          memcpy(local_name, name, len);
      
          if (mm != NULL) {
              cputs("mm != NULL");
              lcr3(boot_cr3);
              if (mm_count_dec(mm) == 0) {
                  exit_mmap(mm);
                  put_pgdir(mm);
                  mm_destroy(mm);
              }
              current->mm = NULL;
          }
          int ret;
          if ((ret = load_icode(binary, size)) != 0) {
              goto execve_exit;
          }
          set_proc_name(current, local_name);
          return 0;
      
      execve_exit:
          do_exit(ret);
          panic("already exit: %e.\n", ret);
      }
      ```

    - 检查参数是否合法

    - 释放当前的内存

    - 加载新的程序, 为新的线程分配新的虚拟内存空间

    - 设置名称

    - 返回用户态

- 用户态: 当前进程执行新程序的入口地址

- 用户态与内核态的交错与上一个函数雷同



**wait**

使父进程等待其子进程结束 并回收子进程的资源

- 用户态: 调用 `wait(pid, &status)`, 触发系统调用中断

- 内核态

  - 在 `syscall.c` 中, 调用 `sys_wait`, 传递子进程的 PID 和状态存储地址

  - `sys_wait` 调用 `do_wait` 执行等待操作

    - ```c++
      int
      do_wait(int pid, int *code_store) {
          struct mm_struct *mm = current->mm;
          if (code_store != NULL) {
              if (!user_mem_check(mm, (uintptr_t)code_store, sizeof(int), 1)) {
                  return -E_INVAL;
              }
          }
      
          struct proc_struct *proc;
          bool intr_flag, haskid;
      repeat:
          haskid = 0;
          if (pid != 0) {
              proc = find_proc(pid);
              if (proc != NULL && proc->parent == current) {
                  haskid = 1;
                  if (proc->state == PROC_ZOMBIE) {
                      goto found;
                  }
              }
          }
          else {
              proc = current->cptr;
              for (; proc != NULL; proc = proc->optr) {
                  haskid = 1;
                  if (proc->state == PROC_ZOMBIE) {
                      goto found;
                  }
              }
          }
          if (haskid) {
              current->state = PROC_SLEEPING;
              current->wait_state = WT_CHILD;
              schedule();
              if (current->flags & PF_EXITING) {
                  do_exit(-E_KILLED);
              }
              goto repeat;
          }
          return -E_BAD_PROC;
      
      found:
          if (proc == idleproc || proc == initproc) {
              panic("wait idleproc or initproc.\n");
          }
          if (code_store != NULL) {
              *code_store = proc->exit_code;
          }
          local_intr_save(intr_flag);
          {
              unhash_proc(proc);
              remove_links(proc);
          }
          local_intr_restore(intr_flag);
          put_kstack(proc);
          kfree(proc);
          return 0;
      }
      ```

    - 检查参数指针的合法性

    - 根据PID查找子进程

    - 如果`pid`不为0, 调用`find_proc`函数根据`pid`查找子进程, 如果找到的进程是当前进程的子进程, 并且状态为`PROC_ZOMBIE`(僵尸状态), 则跳转到`found`标签处理

      - 不是`idleproc`或`initproc`, 则继续处理
      - 如果`code_store`不为`NULL`, 则将子进程的退出代码写入`code_store`指向的地址
      - 使用关中断和启用中断来保证移除进程过程中不会被打断
      - 调用`unhash_proc`和`remove_links`函数从进程表和兄弟链表中移除子进程
      - 释放子进程的内核栈并 `kfree`子进程结构体

    - 如果`pid`为0, 遍历当前进程的所有子进程, 查找状态为`PROC_ZOMBIE`的子进程

    - 如果有子进程但没有找到僵尸子进程, 当前进程将进入睡眠状态, 并调用`schedule`函数进行调度

    - 如果当前进程被标记为退出, 则调用`do_exit`函数退出进程, 然后回到`repeat`标签重新检查

    - 成功处理了僵尸子进程, 函数返回0

- 用户态: 返回子进程的退出状态, 等待父进程处理



**exit**

终止当前进程 并释放其占用的资源

- 用户态: 调用 `exit(status) 触发系统调用中断

- 内核态

  - 调用 `sys_exit`

  - `sys_exit` 调用 `do_exit` 执行退出操作

    - ```c++
      int
      do_exit(int error_code) {
          if (current == idleproc) {
              panic("idleproc exit.\n");
          }
          if (current == initproc) {
              panic("initproc exit.\n");
          }
          struct mm_struct *mm = current->mm;
          if (mm != NULL) {
              lcr3(boot_cr3);
              if (mm_count_dec(mm) == 0) {
                  exit_mmap(mm);
                  put_pgdir(mm);
                  mm_destroy(mm);
              }
              current->mm = NULL;
          }
          current->state = PROC_ZOMBIE;
          current->exit_code = error_code;
          bool intr_flag;
          struct proc_struct *proc;
          local_intr_save(intr_flag);
          {
              proc = current->parent;
              if (proc->wait_state == WT_CHILD) {
                  wakeup_proc(proc);
              }
              while (current->cptr != NULL) {
                  proc = current->cptr;
                  current->cptr = proc->optr;
          
                  proc->yptr = NULL;
                  if ((proc->optr = initproc->cptr) != NULL) {
                      initproc->cptr->yptr = proc;
                  }
                  proc->parent = initproc;
                  initproc->cptr = proc;
                  if (proc->state == PROC_ZOMBIE) {
                      if (initproc->wait_state == WT_CHILD) {
                          wakeup_proc(initproc);
                      }
                  }
              }
          }
          local_intr_restore(intr_flag);
          schedule();
          panic("do_exit will not return!! %d.\n", current->pid);
      }
      ```

    - `idleproc`(空闲进程)或`initproc`(初始化进程)直接调用`panic`函数

    - 清理虚拟内存空间

    - 将当前线程状态设为`PROC_ZOMBIE` 并唤醒该线程的父线程

    - 关中断和启动中断防止被打断

    - 处理子进程, 必要时唤醒`initproc`, 若父进程是等待状态便唤醒

    - 调用`schedule`调度到其他进程

- 用户态: 进程退出, 资源挥手



### 生命周期图

```shell
+-------------+ 
|	 none 	  |  <------------------------------- + ------------ +
+-------------+                                   |              |
     |                                            |              | 
     | alloc_proc                                 |              |
     V	                                          |              |
+-------------+                                   |              |
| PROC_UNINIT |                                   |              |
+-------------+                                   |              |
     |                                            |              |
     | wakeup_proc                                |              |
     V	                                          |              |
+--------------+                                  |  do_fork     |
|   RUNNABLE   |<------------------------+        |              | 
+--------------+                         |        |              |
        |                                |        |              |
        | 调度器选择进程执行                |        |              |
        V                                |        |              |
+--------------+                         |        |              |
|   RUNNING    |                         |        |              |
+--------------+                         |        |              |
        |                                |        |              |
        | 系统调用（fork/exec/wait/exit）  |        |              |
        V                                |        |              |
+--------------+                         |        |              |
|     内核态    |                         |        |              |
+--------------+                         |        |              |
        |                                         |              |
        + --------------------------------------- +              |
        |                                                        |
        |       do_wait             +---------------+            |           
        + <--------------------- >  | PROC_SLEEPING |            |           
        |       wake_up             +---------------+            |            
        |                                                        | 
        |  do_exit                                               |
        |                                                        |
        V                                                        |
+--------------+                   do_exit                       |
|  PROC_ZOMBIE | ----------------------------------------------- +
+--------------+
```



## Challenge
cow.h
```c
#ifndef __KERN_MM_COW_H__
#define __KERN_MM_COW_H__

#include <proc.h>

// 函数声明：复制进程的内存管理结构（mm_struct），并设置COW。
int cow_copy_mm(struct proc_struct *proc);

// 函数声明：复制一个进程的虚拟内存映射到另一个进程中。
int cow_copy_mmap(struct mm_struct *to, struct mm_struct *from);

// 函数声明：复制指定地址范围内的页表项（PTE）到新的页目录中，并设置为只读。
int cow_copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end);

// 函数声明：处理页面错误，用于COW机制下的写时复制。
int cow_pgfault(struct mm_struct *mm, uint_t error_code, uintptr_t addr);

#endif /* !__KERN_MM_COW_H__ */

```



cow.c
```c
#include <cow.h>
#include <kmalloc.h>
#include <string.h>
#include <sync.h>
#include <pmm.h>
#include <error.h>
#include <sched.h>
#include <elf.h>
#include <vmm.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

// 设置一个新的页目录，并初始化为启动时的页目录内容。
static int setup_pgdir(struct mm_struct *mm) {
    struct Page *page;
    if ((page = alloc_page()) == NULL) { // 分配一个物理页作为新的页目录。
        return -E_NO_MEM;
    }
    pde_t *pgdir = page2kva(page); // 获取该页的虚拟地址。
    memcpy(pgdir, boot_pgdir, PGSIZE); // 复制启动页目录的内容到新页目录。

    mm->pgdir = pgdir; // 更新mm_struct中的页目录指针。
    return 0;
}

// 释放页目录所使用的物理页。
static void put_pgdir(struct mm_struct *mm) {
    free_page(kva2page(mm->pgdir)); // 将页目录对应的物理页返回给空闲链表。
}

// 创建一个新进程的内存管理结构（mm_struct），并初始化它。
int cow_copy_mm(struct proc_struct *proc) {
    struct mm_struct *mm, *oldmm = current->mm;

    // 如果当前进程没有mm_struct，则认为是内核线程，无需复制。
    if (oldmm == NULL) {
        return 0;
    }

    int ret = 0;
    if ((mm = mm_create()) == NULL) { // 创建新的mm_struct。
        goto bad_mm;
    }

    if (setup_pgdir(mm) != 0) { // 初始化新mm_struct的页目录。
        goto bad_pgdir_cleanup_mm;
    }

    lock_mm(oldmm); // 锁定旧的mm_struct以确保同步。
    {
        ret = cow_copy_mmap(mm, oldmm); // 复制旧进程的VMA到新进程。
    }
    unlock_mm(oldmm); // 解锁旧的mm_struct。

    if (ret != 0) { // 如果复制失败，则清理资源。
        goto bad_dup_cleanup_mmap;
    }

good_mm:
    mm_count_inc(mm); // 增加新mm_struct的引用计数。
    proc->mm = mm; // 更新新进程的mm_struct指针。
    proc->cr3 = PADDR(mm->pgdir); // 更新CR3寄存器指向新的页目录。
    return 0;

bad_dup_cleanup_mmap:
    exit_mmap(mm); // 清理新进程的VMA。
    put_pgdir(mm); // 释放新进程的页目录。
bad_pgdir_cleanup_mm:
    mm_destroy(mm); // 销毁新创建的mm_struct。
bad_mm:
    return ret; // 返回错误代码。
}

// 复制一个进程的虚拟内存区域（VMA）到另一个进程中。
int cow_copy_mmap(struct mm_struct *to, struct mm_struct *from) {
    assert(to != NULL && from != NULL); // 确保目标和源mm_struct非空。

    list_entry_t *list = &(from->mmap_list), *le = list;
    while ((le = list_prev(le)) != list) { // 遍历源进程的所有VMA。
        struct vma_struct *vma, *nvma;
        vma = le2vma(le, list_link); // 获取当前VMA。
        nvma = vma_create(vma->vm_start, vma->vm_end, vma->vm_flags); // 在目标进程中创建新的VMA。
        if (nvma == NULL) { // 如果创建失败，则返回错误。
            return -E_NO_MEM;
        }
        insert_vma_struct(to, nvma); // 插入新VMA到目标进程的VMA列表。
        if (cow_copy_range(to->pgdir, from->pgdir, vma->vm_start, vma->vm_end) != 0) { // 复制页面范围。
            return -E_NO_MEM;
        }
    }
    return 0;
}

// 复制指定地址范围内的页表项（PTE）到新的页目录中，并设置为只读。
int cow_copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end) {
    assert(start % PGSIZE == 0 && end % PGSIZE == 0); // 确保起始和结束地址都是页对齐的。
    assert(USER_ACCESS(start, end)); // 确保访问的是用户空间地址。

    do {
        pte_t *ptep = get_pte(from, start, 0); // 获取旧页目录中的PTE。
        if (ptep == NULL || !(*ptep & PTE_V)) { // 如果PTE无效或不存在，则跳过。
            start = ROUNDDOWN(start + PTSIZE, PTSIZE);
            continue;
        }

        // 设置旧PTE为只读，并在新页目录中插入相同的映射。
        uint32_t perm = (*ptep & PTE_USER & ~PTE_W); // 只保留用户权限，移除写权限。
        struct Page *page = pte2page(*ptep); // 获取PTE对应的物理页。
        assert(page != NULL);
        int ret = page_insert(to, page, start, perm); // 在新页目录中插入相同的映射。
        assert(ret == 0); // 确保插入成功。

        start += PGSIZE; // 移动到下一个页。
    } while (start < end); // 继续直到遍历完所有页。
    return 0;
}

// 页面错误处理程序，用于COW机制下的写时复制。
int cow_pgfault(struct mm_struct *mm, uint_t error_code, uintptr_t addr) {
    // 注意：调试信息应该仅在调试模式下输出，生产环境中应避免不必要的输出。
    cprintf("COW page fault at 0x%x\n", addr);

    int ret = 0;
    pte_t *ptep = get_pte(mm->pgdir, ROUNDDOWN(addr, PGSIZE), 0); // 获取发生错误的PTE。
    if (ptep == NULL || !(*ptep & PTE_V)) { // 如果PTE无效或不存在，则返回错误。
        return -E_PAGE_FAULT;
    }

    // 设置新PTE的权限为用户可写。
    uint32_t perm = (*ptep & PTE_USER) | PTE_W;
    struct Page *page = pte2page(*ptep); // 获取原始PTE对应的物理页。
    struct Page *npage = alloc_page(); // 分配新的物理页。
    if (npage == NULL) { // 如果分配失败，则返回错误。
        return -E_NO_MEM;
    }

    // 复制原始页面内容到新页面。
    memcpy(page2kva(npage), page2kva(page), PGSIZE);

    // 清除旧PTE，并在页目录中插入新的PTE。
    *ptep = 0;
    ret = page_insert(mm->pgdir, npage, ROUNDDOWN(addr, PGSIZE), perm);
    if (ret != 0) { // 如果插入失败，则释放新页面。
        free_page(npage);
        return ret;
    }

    return 0; // 成功完成页面错误处理。
}

```

进程创建时的内存共享：
	当一个进程调用fork()创建子进程时，子进程并不会立即获得父进程所有内存页面的独立副本。
	相反，父子进程会共享同一组物理页面，但这些页面被设置为只读（Read-Only），并且页表项中设置了特殊的标志（如PTE_COW），以标识它们是COW页面。

页面错误处理：
	如果任意进程试图写入一个COW页面，这将导致一次页面错误（Page Fault），因为页面是只读的。
	页面错误处理程序（如cow_pgfault函数）会被激活来处理这种情况。

写时复制：
	页面错误处理程序首先确认发生的是一次写操作，并检查是否是针对COW页面。
	然后，它会分配一个新的物理页面，并将原始页面的内容复制到新的页面中。
	接着更新当前进程的页表，使该进程指向新的可写页面，而其他共享相同页面的进程继续看到只读版本。
	这样就实现了写时复制，即只有在实际发生写入时才创建页面副本。

维护一致性：
	在进行上述操作时，必须确保系统的状态保持一致，避免竞态条件和其他并发问题。
	使用适当的同步机制（如锁或原子操作）来保护共享数据结构（例如页表、VMA列表等）。

资源回收：
	当最后一个引用某个物理页面的进程结束或释放该页面时，操作系统需要正确地回收该页面所占用的资源。
	可以通过参考计数或其他机制来跟踪有多少个进程正在共享同一个物理页面。

关键点与挑战
	效率：COW机制的关键在于延迟复制直到真正需要，这样可以减少不必要的内存分配和拷贝操作，提高系统性能。
	安全性：必须保证即使在多线程或多处理器环境中，COW操作也是安全的，不会引起数据竞争或不一致的状态。
	复杂性：处理页面错误并正确地管理页表是一项复杂的任务，需要仔细设计以确保系统的稳定性和可靠性。


cow_copy_mm 和 cow_copy_mmap 函数负责初始化新进程的内存映射，并确保所有可写的页面都标记为COW。而cow_copy_range 则具体执行了将父进程的页面映射为只读的操作，使得后续的任何写操作都会触发页面错误。最后，cow_pgfault 函数作为页面错误处理程序，完成了实际的写时复制过程。

这种设计不仅实现了COW的核心思想，还确保了新老进程之间的内存隔离，同时尽可能减少了内存消耗和提升了系统效率。
