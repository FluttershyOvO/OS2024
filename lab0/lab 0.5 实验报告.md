# lab 0.5 实验报告

## 练习1: 使用 GDB 验证启动流程

### 要求

- 熟悉使用 qemu 和 gdb 进行调试工作

- 使用 gdb 调试 qemu 模拟的 riscv 计算机

- 加电开始运行 到 执行应用程序的第一条指令(即跳转到0x80200000) 的执行过程

- riscv 硬件加电后的几条指令

  - 所处位置
  - 所完成的功能

- 可以使用示例代码 Makefile 中的 make debug和make gdb 指令。

- ```bash
  # gdb 指令
  x/10i 0x80000000  # 显示 0x80000000 处的10条汇编指令。
  x/10i $pc  # 显示即将执行的10条汇编指令。
  
  x/10xw 0x80000000  # 显示 0x80000000 处的10条数据，格式为16进制32bit。
  
  info register  # 显示当前所有寄存器信息。
  
  info r t0  # 显示 t0 寄存器的值。
  
  break funcname  # 在目标函数第一条指令处设置断点。
  
  break *0x80200000  # 在 0x80200000 处设置断点。
  
  continue  # 执行直到碰到断点。
  
  si  #单步执行一条汇编指令。
  ```

### 验证

使用 make debug 和 make gdb 指令 启动 gdb 调试 qemu 模拟的 riscv 计算机

- 远程调试

先让 qemu 进入等待 gdb 调试器的接入 且 qemu 中的 CPU 还不能够执行

```makefile
# -s 可以使得 qemu 监听本地TCP端口1234等待gdb客户端连接
# -S 可以使得qemu在收到gdb的请求后再开始运行
.PHONY: debug
debug: $(UCOREIMG) $(SWAPIMG) $(SFSIMG)
	$(V)$(QEMU) \
       -machine virt \
       -nographic \
       -bios default \
       -device loader,file=$(UCOREIMG),addr=0x80200000\
       -s -S
       
# 此时 qemu 中的CPU不会马上开始执行
# 启动 gdb
# 在 gdb 命令行中运行以下语句连接到 qemu
.PHONY: gdb
gdb:
    riscv64-unknown-elf-gdb \  # 启动 gdb 调试器
    -ex 'file bin/kernel' \  
    # gdb 加载目标文件,目标文件通常是编译后的可执行文件或库文件
    -ex 'set arch riscv:rv64' \  # gdb 的目标体系结构是riscv64
    -ex 'target remote localhost:1234'  
	# gdb 连接到主机的端口1234, 与端口建立连接来远程调试目标文件
```

```bash
# 终端1中启动 qemu
make debug
# 输入后会等待 gdb 连接

# 终端2中启动 gdb
make gdb
# 输入后启动 gdb 并连接到 qemu
```

运行成功后会输出

```assembly
Reading symbols from bin/kernel...
The target architecture is set to "riscv:rv64".
Remote debugging using localhost:1234
0x0000000000001000 in ?? ()
(gdb) 
```

可以观察到程序停留在 0x0000000000001000地址处

- 这与 ==复位地址== 有关
  - 是CPU在上电的时候, 或者按下复位键的时候, PC被赋的初始值
  - QEMU 模拟的这款 riscv 处理器的复位地址是 ==0x1000==, 而不是0x80000000
- 所以 gdb 连接到 qemu 后首先会在 0x1000 地址处

说明 GDB 从程序的最开始的地方 开始调试

使用`x/10i $pc`来观察程序 最开始要执行的10条汇编指令

```assembly
(gdb) x/10i $pc
=> 0x1000:	auipc	t0,0x0
   0x1004:	addi	a2,t0,40
   0x1008:	csrr	a0,mhartid
   0x100c:	ld	a1,32(t0)
   0x1010:	ld	t0,24(t0)
   0x1014:	jr	t0
   0x1018:	unimp
   0x101a:	0x8000
   0x101c:	unimp
   0x101e:	unimp
```

得到的汇编指令 即 riscv 硬件加电后要运行的指令

```assembly
0x1000:	auipc	t0,0x0   # 将PC的值保存到t0中
0x1004:	addi	a2,t0,40   # 将a2寄存器赋值为0x1040 
0x1008:	csrr	a0,mhartid   # 将a0寄存器设置为当前hart的ID
							 # mhartid 是机器硬件线程的ID
0x100c:	ld	a1,32(t0)   # 将0x1020中的数据加载到al
0x1010:	ld	t0,24(t0)   # 将0x1018中的数据加载到t0
0x1014:	jr	t0   # 跳转到t0中的地址
```

可以观察到程序会在地址0x1014时, 跳转到t0所存储的地址, 可以使用`x/1xw 0x1018`来查看加载到t0中的0x1018处数据

```assembly
(gdb) x/1xw 0x1018
0x1018:	0x80000000
```

可以得到t0的值为地址0x80000000, 这是程序会跳转到相应的地址0x80000000处

bootloader 需要首先去把操作系统加载到内存中, qemu自带的 bootloader 即为 OpenSBI固件

在 qemu 开始执行任何指令之前，首先两个文件将被加载到 qemu 的物理内存中

- 作为 bootloader 的 OpenSBI.bin 被加载到物理内存以物理地址 0x80000000 开头的区域上
- 同时内核镜像 os.bin 被加载到以物理地址 0x80200000 开头的区域上

程序跳转运行0x80000000处时, 即开始运行OpenSBI

通过查看链接脚本(kernel.ld), 可以观察到如下代码

```assembly
ENTRY(kern_entry)
# 用于指定程序的 入口点
BASE_ADDRESS = 0x80200000;
# 内核镜像的 基地址
```

可以得知内核镜像的起始地址就是 0x80200000, 所以当 OpenSBI 运行后会引导跳转到 0x80200000 来加载内核 

为了正确地和上一阶段的 OpenSBI 对接，内核的第一条指令是位于物理地址 0x80200000 处，因为这里的代码是地址相关的，这个地址是由处理器，即Qemu指定的。内核镜像会预先加载到 Qemu 物理内存以地址 0x80200000 开头的区域上。一旦 CPU 开始执行内核的第一条指令，证明计算机的控制权已经被移交给内核。

已经确定了程序会运行到0x80000000处, 运行 OpenSBI 后在跳转到 0X80200000 地址处去运行内核, 在0x80200000处设置断点, 执行程序直到断点

```assembly
(gdb) break *0x80200000
Breakpoint 1 at 0x80200000: file kern/init/entry.S, line 7.
(gdb) continue
Continuing.

Breakpoint 1, kern_entry () at kern/init/entry.S:7
7	    la sp, bootstacktop
(gdb) x/10i $pc
=> 0x80200000 <kern_entry>:	auipc	sp,0x3
   0x80200004 <kern_entry+4>:	mv	sp,sp
   0x80200008 <kern_entry+8>:	j	0x8020000a <kern_init>
   0x8020000a <kern_init>:	auipc	a0,0x3
   0x8020000e <kern_init+4>:	addi	a0,a0,-2
   0x80200012 <kern_init+8>:	auipc	a2,0x3
   0x80200016 <kern_init+12>:	addi	a2,a2,-10
   0x8020001a <kern_init+16>:	addi	sp,sp,-16
   0x8020001c <kern_init+18>:	li	a1,0
   0x8020001e <kern_init+20>:	sub	a2,a2,a0
(gdb) 
```

通过查看`/lab0/obj/kern/init/kernel.asm`文件, 可以观察到其中的kern_entry 部分代码

```assembly
Disassembly of section .text:

0000000080200000 <kern_entry>:
#include <memlayout.h>

    .section .text,"ax",%progbits
    .globl kern_entry
kern_entry:
    la sp, bootstacktop
    80200000:	00003117          	auipc	sp,0x3
    80200004:	00010113          	mv	sp,sp

    tail kern_init
    80200008:	a009                	j	8020000a <kern_init>
```

这与 gdb 中在 0x80200000 中查看的将要执行的汇编指令相吻合, 证明程序已经开始执行内核



























