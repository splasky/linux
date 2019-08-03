#include "syshook.h"

/* workqueue declaration */
static struct workqueue_struct *flexsc_workqueue;
static struct work_struct *flexsc_works = NULL;
struct page *kernel_pages;
static struct task_struct *systhread;
struct task_struct *utask;

typedef asmlinkage unsigned long (*sys_call_ptr_t)(unsigned long, unsigned long,
						   unsigned long, unsigned long,
						   unsigned long,
						   unsigned long);
static sys_call_ptr_t *sys_call_table;

size_t nentry;

void flexsc_work_handler(struct work_struct *work);

/* syscall thread main function */
int systhread_main(void *arg)
{
	struct flexsc_init_info *info = (struct flexsc_init_info *)arg;

	int cpu = cpu = smp_processor_id();
	printk("kthread[%d %d %d %d], user[%d, %d] starts\n", current->pid,
	       current->parent->pid, DEFAULT_CPU, cpu, utask->pid,
	       utask->parent->pid);

	while (!kthread_should_stop()) {
		int idx;
		for (idx = 0; idx < nentry; ++idx) {
			if (info->sysentry[idx].rstatus ==
			    FLEXSC_STATUS_SUBMITTED) {
				pr_info("got work index(%d)\n", idx);
				info->sysentry[idx].rstatus =
					FLEXSC_STATUS_BUSY;
				queue_work(flexsc_workqueue,
					   &flexsc_works[idx]);
			}
		}
	}
	printk(KERN_INFO "Thread Stopping\n");
	do_exit(0);
	return 0;
}

static __always_inline long do_syscall(unsigned int sysnum,
				       unsigned long args[6])
{
	if (unlikely(sysnum >= __SYSNUM_flexsc_base)) {
		return -1;
	}

	if (likely(sysnum < 500)) {
		return sys_call_table[sysnum](args[0], args[1], args[2],
					      args[3], args[4], args[5]);
	}

	return -ENOSYS;
}

asmlinkage long syshook_flexsc_register(struct flexsc_init_info __user *info)
{
	struct flexsc_sysentry *k_sysentry;
	int err, npinned_pages;

	utask = current;

	/* Get syspage from user space
     * and map it to kernel virtual address space */
	npinned_pages = get_user_pages(
		(unsigned long)(&(info->sysentry[0])), /* Start address to map */
		NUM_PINNED_PAGES, /* Number of pinned pages */
		1, /* Writable flag, Force flag */
		&kernel_pages, /* struct page ** pointer to pinned pages */
		NULL);

	if (npinned_pages < 0) {
		pr_err("Error on getting pinned pages\n");
		return -1;
	}

	k_sysentry = (struct flexsc_sysentry *)kmap(kernel_pages);

	flexsc_create_workqueue("flexsc_workqueue");

	alloc_workstruct(info);
	if (flexsc_works == NULL) {
		printk("Error on allocating sys_works\n");
		return -1;
	}

	systhread = kthread_run(systhread_main, info, "systhread_main");
	if (IS_ERR(systhread)) {
		printk("queueing thread creation fails\n");
		err = PTR_ERR(systhread);
		systhread = NULL;
		return err;
	}

	pretty_print_emph("flexsc register hook success");
	return 0;
}

void flexsc_create_workqueue(char *name)
{
	printk("Creating flexsc workqueue...\n");
	/* Create workqueue so that systhread can put a work */
	flexsc_workqueue =
		alloc_workqueue(name, WQ_MEM_RECLAIM | WQ_UNBOUND, 0);
	printk("Address of flexsc_workqueue: %p\n", flexsc_workqueue);
}

void alloc_workstruct(struct flexsc_init_info *info)
{
	int nentry = info->nentry; /* Number of sysentry */
	int i;
	printk("INFO Allocating work_struct(#%d)\n", nentry);
	flexsc_works = (struct work_struct *)kmalloc(
		sizeof(struct work_struct) * nentry, GFP_KERNEL);

	printk("Initializing: Binding work_struct and work_handler\n");
	for (i = 0; i < nentry; i++) {
		memset(&(info->sysentry[i]), 0, sizeof(struct flexsc_sysentry));
		FLEXSC_INIT_WORK(&flexsc_works[i], flexsc_work_handler,
				 &(info->sysentry[i]));
	}
}

void flexsc_work_handler(struct work_struct *work)
{
	/* Here is the place where system calls are actually executed */
	struct flexsc_sysentry *entry = work->work_entry;

	pr_info("In flexsc_work_handler\n");

	entry->sysret = do_syscall(entry->sysnum, entry->args);
	entry->rstatus = FLEXSC_STATUS_DONE;
	pr_info("flexsc_work_handler done\n");
	return;
}

long syshook_flexsc_exit(void)
{
	int ret;
	printk("flexsc_exit hooked start\n");
	kunmap(kernel_pages);
	flexsc_destroy_workqueue(flexsc_workqueue);
	flexsc_free_works(flexsc_works);
	if (systhread) {
		ret = kthread_stop(systhread);
		systhread = NULL;
	}

	printk("flexsc_exit hooked end\n");
	return 0;
}

void flexsc_destroy_workqueue(struct workqueue_struct *flexsc_workqueue)
{
	if (flexsc_workqueue == NULL) {
		printk("flexsc workqueue is empty!\n");
		return;
	}

	pr_info("flushing flexsc workqueue");
	flush_workqueue(flexsc_workqueue);
	printk("Destroying flexsc workqueue...\n");
	destroy_workqueue(flexsc_workqueue);
}

void flexsc_free_works(struct work_struct *flexsc_works)
{
	int i;
	if (flexsc_works == NULL) {
		printk("flexsc works is empty!\n");
		return;
	}

	printk("Deallocating flexsc work structs...\n");
	for (i = 0; i < nentry; ++i)
		kfree(&flexsc_works[i]);
}

/**
 * disable_write_protection - disable syscall table write protect
 *
 * disable_write_protection() use read_cr0 and write_cr0 to disable
 * syscall table write protect
 */
void disable_write_protection(void)
{
	unsigned long cr0 = read_cr0();
	clear_bit(16, &cr0);
	write_cr0(cr0);
}

/**
 * enable_write_protection - enable syscall table protect
 *
 * enable_write_protection() use read_cr0 and write_cr0 to enable
 * syscall table protect
 */
void enable_write_protection(void)
{
	unsigned long cr0 = read_cr0();
	set_bit(16, &cr0);
	write_cr0(cr0);
}

static __init int syscall_hooking_init(void)
{
	pr_info("Entering: %s\n", __func__);

	sys_call_table = (void *)kallsyms_lookup_name("sys_call_table");
	if (!sys_call_table) {
		pr_err("sch: Couldn't look up sys_call_table\n");
		return -1;
	}

	pr_info("-----------------------syscall hooking module-----------------------\n");
	pr_info("[%p] sys_call_table\n", sys_call_table);

	/* add flexsc register and exit */

	orig_flexsc_register = (void *)sys_call_table[__NR_flexsc_register];
	orig_flexsc_exit = (void *)sys_call_table[__NR_flexsc_exit];

	disable_write_protection();
	sys_call_table[__NR_flexsc_register] = (void *)&syshook_flexsc_register;
	sys_call_table[__NR_flexsc_exit] = (void *)&syshook_flexsc_exit;
	enable_write_protection();
	pr_info("%d %s syscall hooking module init\n", __LINE__, __func__);
	return 0;
}

void syscall_hooking_cleanup(void)
{
	disable_write_protection();
	sys_call_table[__NR_flexsc_register] = (void *)orig_flexsc_register;
	sys_call_table[__NR_flexsc_exit] = (void *)orig_flexsc_exit;
	enable_write_protection();

	pr_info("Hooking moudle cleanup\n");
	return;
}

void print_sysentry(struct flexsc_sysentry *entry)
{
	printk("print sysentry\n");

	if (entry == NULL) {
		printk("Cannot print sysentry! Entry is NULL");
		return;
	}

	printk("[%p] %d-%d-%d-%d with %lu,%lu,%lu,%lu,%lu,%lu\n", entry,
	       entry->sysnum, entry->nargs, entry->rstatus, entry->sysret,
	       entry->args[0], entry->args[1], entry->args[2], entry->args[3],
	       entry->args[4], entry->args[5]);
}

void print_multiple_sysentry(struct flexsc_sysentry *entry, size_t n)
{
	int i;
	for (i = 0; i < n; i++) {
		print_sysentry(&entry[i]);
	}
}

module_init(syscall_hooking_init);
module_exit(syscall_hooking_cleanup);
MODULE_LICENSE("GPL");
