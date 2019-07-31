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

#define __SYSNUM_flexsc_base 400

asmlinkage long (*orig_flexsc_register)(struct flexsc_init_info __user *info);
asmlinkage long (*orig_flexsc_exit)(void);

asmlinkage long syshook_flexsc_register(struct flexsc_init_info __user *info);
asmlinkage long syshook_flexsc_exit(void);

void print_sysentry(struct flexsc_sysentry *entry);
void print_multiple_sysentry(struct flexsc_sysentry *entry, size_t n);

inline void pretty_print_emph(char *s)
{
	pr_info("******************** %s ********************\n", s);
}
