#ifndef __KERN_MM_VMM_H__
#define __KERN_MM_VMM_H__

#include <defs.h>
#include <list.h>
#include <memlayout.h>
#include <sync.h>

//pre define
struct mm_struct;

// the virtual continuous memory area(vma), [vm_start, vm_end), 
// addr belong to a vma means  vma.vm_start<= addr <vma.vm_end 
struct vma_struct {
    struct mm_struct *vm_mm; // the set of vma using the same PDT 
    uintptr_t vm_start;      // start addr of vma      
    uintptr_t vm_end;        // end addr of vma, not include the vm_end itself
    uint_t vm_flags;       // flags of vma
    list_entry_t list_link;  // linear list link which sorted by start addr of vma
};

// 该结构体 `vma_struct` 用于表示虚拟内存区域（Virtual Memory Area），主要字段功能如下：
// - `vm_mm`：指向共享同一页目录表（PDT）的内存管理结构。
// - `vm_start`：虚拟内存区域的起始地址。
// - `vm_end`：虚拟内存区域的结束地址（不包含该地址本身）。
// - `vm_flags`：虚拟内存区域的标志位。
// - `list_link`：用于将虚拟内存区域按起始地址排序的链表链接。


#define le2vma(le, member)                  \
    to_struct((le), struct vma_struct, member)

#define VM_READ                 0x00000001
#define VM_WRITE                0x00000002
#define VM_EXEC                 0x00000004

// the control struct for a set of vma using the same PDT
struct mm_struct {
    list_entry_t mmap_list;        // linear list link which sorted by start addr of vma
    struct vma_struct *mmap_cache; // current accessed vma, used for speed purpose
    pde_t *pgdir;                  // the PDT of these vma
    int map_count;                 // the count of these vma
    void *sm_priv;                   // the private data for swap manager
};
/*
*该结构体 `mm_struct` 用于管理一组使用相同页目录表（PDT）的虚拟内存区域（VMA）：
*
*- `mmap_list`：按 VMA 起始地址排序的链表节点。
*- `mmap_cache`：当前访问的 VMA，用于加速访问。
*- `pgdir`：这些 VMA 共享的页目录表。
*- `map_count`：VMA 的数量。
*- `sm_priv`：交换管理器的私有数据。
*/


struct vma_struct *find_vma(struct mm_struct *mm, uintptr_t addr);
struct vma_struct *vma_create(uintptr_t vm_start, uintptr_t vm_end, uint_t vm_flags);
void insert_vma_struct(struct mm_struct *mm, struct vma_struct *vma);

struct mm_struct *mm_create(void);
void mm_destroy(struct mm_struct *mm);

void vmm_init(void);

int do_pgfault(struct mm_struct *mm, uint_t error_code, uintptr_t addr);

extern volatile unsigned int pgfault_num;
extern struct mm_struct *check_mm_struct;

#endif /* !__KERN_MM_VMM_H__ */

