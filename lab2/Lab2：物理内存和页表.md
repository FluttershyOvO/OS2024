# 实验目的

## 1.理解页表的建立和使用方法
### 页表的建立
页表的建立通常由操作系统完成，它涉及到以下几个步骤：

**初始化页表：**
操作系统在物理内存中分配一块连续的空间用于存放页表项。
每个页表项通常包含物理页框号（即页面所在的物理地址）、有效位（表示该页是否存在于物理内存中）、读写权限等信息。

**填充页表项：**
当进程请求加载页面到内存时，操作系统会在页表中为这个页面创建一个条目。
页表项中记录页面的物理地址，如果页面尚未加载到内存，则可能设置为无效标记或指向交换区。

**设置页表基址寄存器：**
在处理器中有一个特殊的寄存器，用于保存当前进程的页表起始地址（也称为页目录地址），这是查找页表所需的关键信息。
当切换到另一个进程时，操作系统需要更新这个寄存器以指向新的页表。
### 页表的使用
当程序试图访问某个内存地址时，处理器会自动执行以下操作来查找对应的物理地址：

**地址转换：**
处理器从程序提供的逻辑地址中提取出页号（即页面索引）和页内偏移量。
使用页号作为索引去访问页表，找到对应的页表项。

**检查页表项：**
如果页表项的有效位为无效，则意味着该页面不在物理内存中，此时会产生一个缺页异常（page fault），操作系统需要处理该异常，可能会从磁盘交换区加载页面到内存，并更新页表。
如果有效位为有效，则直接从页表项获取物理页框号。

**计算物理地址：**
将物理页框号与页内偏移量相加得到最终要访问的物理地址。
页表机制使得操作系统能够灵活地管理内存，允许多个进程共享有限的物理内存资源。此外，通过多级页表和反向映射等技术，还可以进一步提高页表管理的效率和灵活性。



## 2.理解物理内存的管理方法
物理内存管理是操作系统内核的重要职责之一，目的是为了确保所有运行的进程都能获得足够的内存资源，同时避免内存浪费和冲突。以下是几种常见的物理内存管理方法：

### 分区管理 (Partitioning)
分区管理是最简单的内存管理形式之一，它将内存划分为多个连续的区域（分区），每个区域可以分配给一个进程。这种管理方式有以下几种类型：

固定分区：内存被预先分成固定大小的区域。
可变分区：分区大小可以根据需要动态调整。
虽然这种方法简单易行，但它容易导致内存碎片问题，即小的未分配空间无法被利用，从而降低内存利用率。

### 分页管理 (Paging)
分页管理将内存分割成固定大小的页框（通常为4KB、8KB等），并将进程的虚拟地址空间也分割成相同大小的页面。操作系统使用页表来记录每个页面映射到哪个页框上。这种方法的主要优点包括：

减少内存碎片。
支持非连续分配，提高内存利用率。
提供硬件支持的内存保护。
### 分段管理 (Segmentation)
分段管理允许进程按逻辑单元（如代码段、数据段等）来组织其地址空间。每个段可以有不同的大小，并且可以独立于其他段进行分配。这种方法的优点包括：

逻辑上的清晰性，易于理解和调试。
支持动态增长的内存需求。
提供内存保护。
### 段页式管理 (Segmentation with Paging)
段页式管理结合了分页和分段的优势，先按逻辑分割成段，然后每个段内部再分割成页。这种方法可以提供更好的内存管理和保护机制。

### 虚拟内存管理 (Virtual Memory)
虚拟内存是一种抽象概念，它使操作系统能够在物理内存不足的情况下也能运行更多进程。虚拟内存系统通常结合分页技术，通过将不常用的数据交换到磁盘上的交换文件或交换分区来释放物理内存空间。主要技术包括：

缺页调度：当尝试访问不在物理内存中的页面时，操作系统会将所需的页面从磁盘加载到内存。
页面置换算法：决定哪些页面可以从物理内存中移除并换入磁盘，常用的算法有LRU（最近最少使用）、FIFO（先进先出）等。
工作集模型：用来确定进程当前实际使用的页面集合。

内存分配算法
为了有效地管理内存，操作系统还需要实现一些内存分配算法，以便决定如何分配和回收内存空间。常见的内存分配算法包括：

首次适应算法（First Fit）：寻找第一个足够大的空闲分区。
最佳适应算法（Best Fit）：寻找最接近所需大小的空闲分区。
最差适应算法（Worst Fit）：寻找最大的空闲分区。
这些方法各有优缺点，在不同的场景下可能有不同的表现。

通过上述各种技术和算法，操作系统能够有效地管理物理内存资源，保证系统的稳定性和性能。

## 3.理解页面分配算法
页面分配算法是指在虚拟内存系统中，操作系统用来决定如何将物理内存分配给进程页面的方法。这些算法对于优化内存使用、减少内存碎片、提高系统性能至关重要。以下是一些常见的**页面分配算法**：

**1. 首次适应算法（First-Fit）**
首次适应算法从内存的开始处搜索第一个足够大的空闲页框，并将页面分配给请求的进程。这种算法简单易实现，但是可能导致内存中形成许多小的、无法利用的空闲空间（即外部碎片）。

**2. 最佳适应算法（Best-Fit）**
最佳适应算法从整个内存中寻找最接近所需大小的空闲页框。这种方法可以减少外部碎片，但是搜索整个内存来找到最佳匹配会增加分配时间。

**3. 最差适应算法（Worst-Fit）**
最差适应算法选择当前已知的最大空闲页框来分配。这种方法假设这样做可以保留较大的空闲块，以便未来更大的请求使用。然而，它也可能导致较大的空闲块被分割成更小的部分，从而增加碎片。

**4. 循环首次适应算法（Circular First-Fit）**
循环首次适应算法类似于首次适应算法，但在遍历完一次内存后，会回到内存的开头继续搜索，从而均匀分布内存的使用。

**页面置换算法**
除了上述的页面分配算法外，还有一类专门针对页面置换（Page Replacement）的算法，它们是在物理内存已满时，决定哪些页面应该从物理内存中移除，以便为新页面腾出空间。常见的页面置换算法包括：

**4.1 最佳置换算法（Optimal Replacement）**
这是一种理论上的算法，它总是选择未来最长时间不会被访问的页面进行替换。虽然理想化，但在实际中不可行，因为它需要预知未来。

**4.2 最近最少使用算法（Least Recently Used, LRU）**
LRU算法选择最近最少使用的页面进行替换。它假设最近没有被访问过的页面在未来一段时间内也不会被访问。实际中，可以通过维护一个最近使用列表来近似实现LRU。

**4.3 先进先出算法（First In First Out, FIFO）**
FIFO算法按照页面进入内存的时间顺序进行替换，即最早进入的页面最先被替换出去。这种方法容易产生**Belady's Anomaly**现象，即增加分配给进程的物理页框数量反而会导致更多的页面故障。

**4.4 第二次机会算法（Second Chance）**
第二次机会算法是对FIFO的一种改进，当第一次扫描到一个页面时，并不立即替换它，而是给它一个“第二次机会”标志，如果再次扫描时发现这个标志还在，那么就替换它。

**4.5 随机置换算法（Random Replacement）**
随机置换算法随机选择一个页面进行替换，这种方法简单，但可能不是最优的选择。

以上算法各有优缺点，实际操作系统中通常会根据系统的特点和需求选择合适的算法，有时还会结合多种算法来优化内存管理。例如，Linux 内核使用了一种结合了LRU和其他策略的复杂算法来进行页面置换。


# 知识
## 相关视频
[页式存储管理讲解](https://www.bilibili.com/list/ml1668517737?oid=267847914&bvid=BV1uY411k73T)
## 内存管理的不同方法
页式管理 段式管理 段页式管理

## 页式存储管理
![进程划分和内存划分](https://i-blog.csdnimg.cn/direct/ae1db9ad6daa4c88b99835024b7fbbd2.jpeg)

### 进程
进程划分
逻辑页面
逻辑页号
页内地址
地址 = 逻辑页号*n + 页内地址
逻辑地址：（逻辑页号，页内地址）

### 内存
内存划分
页框
页框号
页内地址
地址 = 页框号*n + 页内地址
物理地址：（页框号，页内地址）

### 页表，快表
进程和内存都是一整块连续的区域
进程与内存每段长度一样，为n
进程空间连续，但对应存放在内存中的位置不连续，因此需要一个表格来记录进程与内存的对应关系，这个表就是页表
进程和页表都是存放在内存中的，每次查询进程数据要访问两次内存，降低了速度，因此把页表常用和正在使用的数据放入寄存器中，提升速度，寄存器中存放这些数据的表格叫快表

### 多级页表

### 反置页表

# 实验内容
## 练习1：理解first-fit 连续物理内存分配算法（思考题）
first-fit 连续物理内存分配算法作为物理内存分配一个很基础的方法，需要同学们理解它的实现过程。请大家仔细阅读实验手册的教程并结合kern/mm/default_pmm.c中的相关代码，认真分析default_init，default_init_memmap，default_alloc_pages， default_free_pages等相关函数，并描述程序在进行物理内存分配的过程以及各个函数的作用。 请在实验报告中简要说明你的设计实现过程。请回答如下问题：

你的first fit算法是否有进一步的改进空间？

### default_init
```
static void
default_init(void) {
    list_init(&free_list);
    nr_free = 0;
}
```
该函数 default_init 用于**初始化一些全局变量**：
1.调用 list_init(&free_list) 初始化一个名为 free_list 的链表。
2.将全局变量 nr_free 设置为 0，表示当前没有可用的资源。

**函数list_init**
```
static inline void
list_init(list_entry_t *elm) {
    elm->prev = elm->next = elm;
}
```

**结构体list_entry**
```
struct list_entry {
    struct list_entry *prev, *next;
};

```

### default_init_memmap
```
static void
default_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(PageReserved(p));
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    nr_free += n;
    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }
}
```
该函数 default_init_memmap 用于**初始化内存页管理结构**。主要功能如下：
1.参数检查：确保传入的页数 n 大于 0。
2.初始化页：遍历从 base 开始的 n 个页，将每个页的 flags 和 property 置为 0，并设置引用计数为 0。
3.设置基页属性：将 base 页的 property 设置为 n，并标记其具有特定属性。
4.更新空闲页计数：增加全局变量 nr_free 的值，表示新增了 n 个空闲页。
5.插入空闲页链表：如果空闲页链表为空，直接将 base 插入链表。否则，遍历链表找到合适的位置插入 base，确保链表按页地址排序。


### default_alloc_pages
```
static struct Page *
default_alloc_pages(size_t n) {
    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {
            page = p;
            break;
        }
    }
    if (page != NULL) {
        list_entry_t* prev = list_prev(&(page->page_link));
        list_del(&(page->page_link));
        if (page->property > n) {
            struct Page *p = page + n;
            p->property = page->property - n;
            SetPageProperty(p);
            list_add(prev, &(p->page_link));
        }
        nr_free -= n;
        ClearPageProperty(page);
    }
    return page;
}
```
该函数 default_alloc_pages 用于**从空闲页面列表中分配指定数量的连续页面**。
1.参数检查：首先检查请求的页面数 n 是否大于0且不超过当前空闲页面数 nr_free，如果超过则返回 NULL。
2.遍历空闲列表：从空闲页面列表 free_list 中查找一个具有足够属性值（即连续页面数）的页面。
3.分配页面：找到合适的页面后，从列表中删除该页面。如果该页面的属性值大于请求的页面数 n，则将剩余部分重新插入空闲列表。
4.更新计数：减少空闲页面计数 nr_free 并清除分配页面的属性。
5.返回结果：返回分配的页面指针。


### default_free_pages
```
static void
default_free_pages(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    nr_free += n;

    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }

    list_entry_t* le = list_prev(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        if (p + p->property == base) {
            p->property += base->property;
            ClearPageProperty(base);
            list_del(&(base->page_link));
            base = p;
        }
    }

    le = list_next(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        if (base + base->property == p) {
            base->property += p->property;
            ClearPageProperty(p);
            list_del(&(p->page_link));
        }
    }
}
```
该函数 default_free_pages 用于**释放内存页**，并将其加入空闲页列表中。具体功能如下：
1.初始化和检查：
检查传入的页数 n 是否大于 0。
遍历从 base 开始的 n 个页，确保每个页未被预留且没有属性，并将这些页的标志和引用计数清零。
2.设置页属性：
将 base 页的属性设置为 n，并标记该页具有属性。
增加全局空闲页计数 nr_free。
3.插入空闲页列表：
如果空闲页列表为空，直接将 base 插入列表。
否则，遍历空闲页列表，找到合适的位置插入 base，保持列表按地址排序。
4.合并相邻空闲页：
检查 base 前一个页是否可以与 base 合并，如果可以，则更新属性并删除 base 的列表项。
检查 base 后一个页是否可以与 base 合并，如果可以，则更新属性并删除后一个页的列表项。



### 改进？（也许吧）
```
static void
default_free_pages_improved(struct Page *base, size_t n) {

//初始化和检查
//检查传入的页数 n 是否大于 0，并遍历从 base 开始的 n 个页，确保每个页未被预留且没有属性。然后将这些页的标志和引用计数清零。
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p++) {
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }

//设置页属性
//将 base 页的属性设置为 n，并标记该页具有属性。同时，增加全局空闲页计数 nr_free。
    base->property = n;
    SetPageProperty(base);
    nr_free += n;


//插入空闲页列表
//如果空闲页列表为空，直接将 base 插入列表。否则，遍历空闲页列表，找到合适的位置插入 base，保持列表按地址排序。
    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }



    // 合并相邻空闲页
    //检查 base 前一个页是否可以与 base 合并，如果可以，则更新属性并删除 base 的列表项。同样，检查 base 后一个页是否可以与 base 合并，如果可以，则更新属性并删除后一个页的列表项。
    list_entry_t* le = list_prev(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        if (p + p->property == base) {
            p->property += base->property;
            ClearPageProperty(base);
            list_del(&(base->page_link));
            base = p;
        }
    }

    le = list_next(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        if (base + base->property == p) {
            base->property += p->property;
            ClearPageProperty(p);
            list_del(&(p->page_link));
        }
    }



    // 重新排序以减少碎片
    //重新排序空闲页列表，以减少内存碎片。使用 list_sort 函数对列表进行排序，确保空闲页按地址顺序排列。
    list_sort(&free_list, compare_pages);
}

//比较两个页的地址，用于 list_sort 函数的排序。返回值为负数表示 pa 在 pb 之前，正数表示 pa 在 pb 之后，0 表示两者相等。
static int
compare_pages(const void *a, const void *b) {
    const struct Page *pa = container_of(a, struct Page, page_link);
    const struct Page *pb = container_of(b, struct Page, page_link);
    return (pa < pb) ? -1 : (pa > pb) ? 1 : 0;
}
```
改进的 First Fit 算法在原有基础上增加了重新排序的步骤，以减少内存碎片。通过合并相邻的空闲页和重新排序，可以更有效地管理内存，提高内存利用率。


## 练习2：实现 Best-Fit 连续物理内存分配算法（需要编程）
在完成练习一后，参考kern/mm/default_pmm.c对First Fit算法的实现，编程实现Best Fit页面分配算法，算法的时空复杂度不做要求，能通过测试即可。 请在实验报告中简要说明你的设计实现过程，阐述代码是如何对物理内存进行分配和释放，并回答如下问题：

你的 Best-Fit 算法是否有进一步的改进空间？

### 什么是Best-Fit

最佳适应算法（Best-Fit） 是一种用于分配连续物理内存的算法。它的目标是**在可用的连续内存块中找到最接近所需大小的那个，以尽量减少剩余的未使用空间（即外部碎片）**。下面是 Best-Fit 算法的工作原理和特点：

**工作原理**
1.请求处理：当一个进程请求一定大小的连续物理内存时，Best-Fit 算法会搜索当前所有空闲的内存块。
2.选择最小的合适块：算法会选择大小大于等于请求大小并且是最接近请求大小的空闲内存块。如果存在多个满足条件的块，则选择其中最小的那个。
3.分配和分割：选中的内存块将被分配给请求者。如果选中的内存块比请求的大小大，那么多余的内存会被分割成一个新的空闲块，并保持在空闲列表中。
**特点**
1.减少外部碎片：通过选择最接近请求大小的内存块，Best-Fit 算法可以减少外部碎片，即那些太小而不能被使用的空闲空间。
2.较慢的速度：由于每次请求都需要在整个空闲内存列表中寻找最合适的块，因此 Best-Fit 算法的分配速度相对较慢。
3.潜在的恶化问题：随着时间推移，可能会出现大量小的空闲块，使得找到合适大小的连续内存块变得越来越困难。
**实现细节**
为了有效地实施 Best-Fit 算法，操作系统需要维护一个空闲内存块的列表或链表。当进程请求内存时，算法会遍历这个列表来找到最合适的块。**在某些实现中，空闲块列表会被排序，以便快速定位最适合的块。**

**示例**
假设当前有以下空闲内存块（单位：字节）：
Block A: 1000 字节
Block B: 1500 字节
Block C: 2000 字节
Block D: 2500 字节
如果一个进程请求 1200 字节的内存，Best-Fit 算法则会选择 Block B（1500 字节），因为它是大于或等于 1200 字节的最小块。

**总结**
尽管 Best-Fit 算法可以减少外部碎片，但由于其遍历整个空闲列表来寻找最佳匹配块的过程，可能会导致较高的搜索成本。此外，随着时间的推移，可能会形成大量小的空闲块，这反过来又会影响内存分配的效率。因此，在实际操作系统中，可能需要结合其他算法或者采取一些额外措施来优化内存管理。

### best_fit_init,best_fit_init_memmap
best_fit_init,best_fit_init_memmap分别和default_init，default_init_memmap一样，这里不再做说明。

### best_fit_alloc_pages

```
static struct Page *
best_fit_alloc_pages(size_t n) {
    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    size_t min_size = nr_free + 1;
     /*LAB2 EXERCISE 2: 2210203*/ 
    // 下面的代码是first-fit的部分代码，请修改下面的代码改为best-fit
    // 遍历空闲链表，查找满足需求的空闲页框
    // 如果找到满足需求的页面，记录该页面以及当前找到的最小连续空闲页框数量
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        if (p->property >= n && p->property < min_size) {
            page = p;
            min_size = p->property;
        }
    }

    if (page != NULL) {
        list_entry_t* prev = list_prev(&(page->page_link));
        list_del(&(page->page_link));
        
        if (page->property > n) {
            // 分割多余的内存
            struct Page *new_page = page + n;
            new_page->property = page->property - n;
            SetPageProperty(new_page);
            list_add(prev, &(new_page->page_link));
        }
        
        nr_free -= n;
        ClearPageProperty(page);
    }
    
    return page;
}

```
**解释**
1.初始化变量：
min_size 用于记录当前找到的最小连续空闲页框数量，初始值设为 nr_free + 1。
2.遍历空闲页链表：
使用 while 循环遍历空闲页链表 free_list。
检查每个空闲页 p 是否满足 p->property >= n 并且 p->property < min_size。如果是，则更新 page 和 min_size。
3.分配页框：
如果找到了符合条件的页 page，则从空闲页链表中删除该页。
如果分配的页框大小大于所需大小 n，则分割多余的内存，并将多余的部分重新插入到空闲页链表中。
更新空闲页数 nr_free。
清除页框的属性标志。
4.返回结果：
返回分配的页框指针 page。

示例
假设当前 free_list 中有以下空闲页框：

Page A: 1000 字节
Page B: 1500 字节
Page C: 2000 字节
如果请求分配 1200 字节的内存，best_fit_alloc_pages 函数将返回 Page B，因为 Page B 的大小（1500 字节）最接近 1200 字节，并且大于等于 1200 字节。

如果请求分配 1600 字节的内存，函数将返回 Page C，因为 Page C 的大小（2000 字节）最接近 1600 字节，并且大于等于 1600 字节。

通过这种方式，best_fit_alloc_pages 函数实现了 best-fit 算法，有效地减少了外部碎片。


### best_fit_free_pages
```

static void
best_fit_free_pages(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }
    /*LAB2 EXERCISE 2: 2210203*/ 
    // 编写代码
    // 具体来说就是设置当前页块的属性为释放的页块数、并将当前页块标记为已分配状态、最后增加nr_free的值

    // 设置当前页块的属性为释放的页块数，并将当前页块标记为已分配状态
    base->property = n;
    SetPageProperty(base);

    // 增加 nr_free 的值
    nr_free += n;

    // 如果 free_list 为空，则将 base 添加到链表头部
    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }

    list_entry_t* le = list_prev(&(base->page_link));
    if (le != &free_list) {
        p = le2page(le, page_link);
        /*LAB2 EXERCISE 2: 2210203*/ 
         // 编写代码
        // 1、判断前面的空闲页块是否与当前页块是连续的，如果是连续的，则将当前页块合并到前面的空闲页块中
        // 2、首先更新前一个空闲页块的大小，加上当前页块的大小
        // 3、清除当前页块的属性标记，表示不再是空闲页块
        // 4、从链表中删除当前页块
        // 5、将指针指向前一个空闲页块，以便继续检查合并后的连续空闲页块

        if (p + p->property == base) {
            p->property += base->property;
            ClearPageProperty(base);
            list_del(&(base->page_link));
            base = p;
        }
    }

    // 合并后面的空闲页块（如果有的话）
    le = list_next(&(base->page_link));
    if (le != &free_list) {
        struct Page *next_page = le2page(le, page_link);
        if (base + base->property == next_page) {
            base->property += next_page->property;
            ClearPageProperty(next_page);
            list_del(&(next_page->page_link));
        }
    }
}

```

### 改进
排序空闲块列表，以便快速定位最适合的块。



## 扩展练习Challenge：buddy system（伙伴系统）分配算法（需要编程）
Buddy System算法把系统中的可用存储空间划分为存储块(Block)来进行管理, 每个存储块的大小必须是2的n次幂(Pow(2, n)), 即1, 2, 4, 8, 16, 32, 64, 128...

参考伙伴分配器的一个极简实现， 在ucore中实现buddy system分配算法，要求有比较充分的测试用例说明实现的正确性，需要有设计文档。


### 什么是buddy system（伙伴系统）分配算法？
"Buddy system"（伙伴系统）是一种内存分配算法，主要用于连续内存块的分配与回收。它在操作系统中用于管理虚拟内存或者磁盘空间。这个算法最初是由Douglas Comer在1981年提出的，并且在多种操作系统中得到了应用。

伙伴系统的核心思想是将内存分割成大小为2^n的块，其中n是一个非负整数。当一个进程请求一块特定大小的内存时，伙伴系统会尝试找到一个尽可能小的、能够满足请求要求的空闲块。如果找不到合适大小的空闲块，则会找到一个更大的块并将其分割成两个相等的部分。这个过程会一直持续到产生至少一个可以满足请求的空闲块为止。

当释放内存时，如果两个相邻的空闲块大小相同，则它们会被合并成一个更大的空闲块。这种合并有助于减少内存碎片。

伙伴系统的优点包括：
它有效地处理了内存碎片的问题，因为它允许合并相邻的空闲区域。
分配和回收操作通常较快，因为只需要简单地进行分割或合并操作。

缺点则有：
可能会有一定程度的空间浪费，因为分配的块总是2的幂次大小，这可能比实际需要的要大一些。
在某些情况下，可能会出现一些较大块无法被分配出去的情况，因为没有合适的伙伴块来与之合并。

总的来说，伙伴系统是一种有效管理内存的技术，在某些应用场景下非常有用，特别是在早期的操作系统设计中。不过，随着技术的发展，现代操作系统也采用了其他更先进的内存管理技术，如按需分页（demand paging）和内存映射（memory mapping）。


## 扩展练习Challenge：硬件的可用物理内存范围的获取方法（思考题）

硬件管理工具
许多服务器硬件厂商提供了专用的管理工具，这些工具可以远程监控和管理硬件状态，包括内存的详细信息。

硬件文档和手册
对于特定的主板或者服务器设备，厂商通常会提供详细的硬件规格文档，其中包括内存插槽的位置、支持的最大容量等信息。

内核模块或驱动：
如果你需要更加详细的内存布局信息，可能需要编写内核模块或驱动来直接访问或解析物理内存范围。这种方式比较复杂，需要对操作系统内核有深入的理解。


# 结
通过本次实验，我们不仅加深了对物理内存管理的理解，还掌握了以下几点：
1.内存页的初始化和管理：学会了如何初始化内存页，并使用链表结构来管理空闲页块。
2.最佳适应算法的应用：通过实现 best_fit_alloc_pages 和 best_fit_free_pages 函数，熟悉了最佳适应算法在内存分配和释放中的应用。
3.页表的建立和使用：掌握了页表的基本操作，包括页表项的初始化、更新和查询，以及如何通过页表实现虚拟地址到物理地址的映射。

本次实验成功地实现了物理内存的高效管理，并建立了页表来支持虚拟内存的映射。通过动手实践，我们不仅巩固了理论知识，还提升了编程能力和解决问题的能力。这些经验将为我们今后深入研究操作系统内存管理打下坚实的基础。
