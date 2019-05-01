#include "../flexsc.h"

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/hugetlb.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/kallsyms.h>
#include <linux/fdtable.h>
#include <linux/set_memory.h>

#define NUM_PINNED_PAGES 1
#define NUM_SYSENTRY 64
#define MAX_KSTRUCT 64

#ifdef CONFIG_X86_64
typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
#else
typedef asmlinkage long (*sys_call_ptr_t)(unsigned long, unsigned long,
					  unsigned long, unsigned long,
					  unsigned long, unsigned long);
#endif /* CONFIG_X86_64 */
#define __SYSNUM_flexsc_base 400

//int kstruct_cnt;
static struct workqueue_struct *sys_workqueue;
static struct work_struct *sys_works;

//struct task_struct *utask;
//struct page *mypage;
//struct page *pinned_pages[NUM_PINNED_PAGES];
//void *sysentry_start_addr;
static struct task_struct *kstruct;
static struct page *pinned_pages[NUM_PINNED_PAGES];
static unsigned long *sys_call_table;

// TODO: try typeof
// static typeof(flexsc_register) *orig_flexsc_register;
// static typeof(flexsc_exit) *orig_flexsc_exit;

static asmlinkage long (*orig_flexsc_register)(
	struct flexsc_init_info __user *info);
static asmlinkage long (*orig_flexsc_exit)(void);

asmlinkage long syshook_flexsc_register(struct flexsc_init_info __user *info);
asmlinkage long syshook_flexsc_exit(void);

void print_sysentry(struct flexsc_sysentry *entry);
void print_multiple_sysentry(struct flexsc_sysentry *entry, size_t n);

inline void pretty_print_emph(char *s)
{
	printk("******************** %s ********************\n", s);
}
