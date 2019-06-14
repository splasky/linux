#include "syshook.h"

/* syscall thread main function */
int scanner_thread(void *arg)
{
	struct flexsc_sysentry *entry = (struct flexsc_sysentry *)arg;
	int i, cpu;
	bool ret;
	cpu = smp_processor_id();

	BUG_ON(DEFAULT_CPU != cpu);

	// printk("kthread[%d %d %d %d], user[%d, %d] starts\n", current->pid,
	//        current->parent->pid, DEFAULT_CPU, cpu, utask->pid,
	//        utask->parent->pid);

	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);

		if (kthread_should_stop()) {
			printk("kernel thread dying...\n");
			do_exit(0);
		}

		for (i = 0; i < NUM_SYSENTRY; i++) {
			if (entry[i].rstatus == FLEXSC_STATUS_SUBMITTED) {
				printk("entry[%d].rstatus == SUBMITTED\n", i);

				entry[i].rstatus = FLEXSC_STATUS_BUSY;
				printk("qq nice %d\n",entry[i].sysnum);
				ret = queue_work_on(DEFAULT_CPU, sys_workqueue,
						    &sys_works[i]);

				if (ret == false) {
					printk("sys_work already queued\n");
				}
			}
		}
		schedule_timeout(HZ);
	}
	return 0;
}

static __always_inline long do_syscall(unsigned int sysnum,
				       struct pt_regs *regs)
{
	printk("do syscall was calling!\n");
	if (unlikely(sysnum >= __SYSNUM_flexsc_base)) {
		return -1;
	}

	if (likely(sysnum < 500)) {
		return ((sys_call_ptr_t *)sys_call_table)[sysnum](regs);
	}

	return -ENOSYS;
}

static void syscall_handler(struct work_struct *work)
{
	struct flexsc_sysentry *entry = work->work_entry;
	long sysret;

	print_sysentry(entry);

	sysret = do_syscall(entry->sysnum, entry->regs);

	if (sysret == -ENOSYS) {
		printk("%d %s: do_syscall failed!\n", __LINE__, __func__);
	}

	entry->sysret = sysret;
	entry->rstatus = FLEXSC_STATUS_DONE;
	return;
}

asmlinkage long syshook_flexsc_register(struct flexsc_init_info __user *info)
{
	void *sysentry_start_addr;

	int i, err, npinned_pages;
	struct flexsc_sysentry *entry;

	/* Print first 8 sysentries */
	pretty_print_emph("User address space");
	print_multiple_sysentry(info->sysentry, 8);

	/* Get syspage from user space 
     * and map it to kernel virtual address space */
	npinned_pages = get_user_pages(
		(unsigned long)(&(info->sysentry[0])), /* Start address to map */
		NUM_PINNED_PAGES, /* Number of pinned pages */
		FOLL_WRITE | FOLL_FORCE, /* Writable flag, Force flag */
		pinned_pages, /* struct page ** pointer to pinned pages */
		NULL);

	if (npinned_pages < 0) {
		printk("Error on getting pinned pages\n");
	}

	sysentry_start_addr = kmap(pinned_pages[0]);

	entry = (struct flexsc_sysentry *)sysentry_start_addr;

	sys_workqueue = create_workqueue("flexsc_workqueue");
	workqueue_set_max_active(sys_workqueue, NUM_SYSENTRY);

	sys_works = (struct work_struct *)kmalloc(
		sizeof(struct work_struct) * NUM_SYSENTRY, GFP_KERNEL);
	if (sys_works == NULL) {
		printk("Error on allocating sys_works\n");
		return -1;
	}

	for (i = 0; i < NUM_SYSENTRY; i++) {
		FLEXSC_INIT_WORK(&sys_works[i], syscall_handler, &(entry[i]));
	}

	kstruct = kthread_create(scanner_thread, (void *)entry,
				 "flexsc scanner thread");
	kthread_bind(kstruct, DEFAULT_CPU);

	if (IS_ERR(kstruct)) {
		printk("queueing thread creation fails\n");
		err = PTR_ERR(kstruct);
		kstruct = NULL;
		return err;
	}

	wake_up_process(kstruct);
	printk("**************syshook_flexsc_register success!!!!************\n");
	return 0;
}

long syshook_flexsc_exit(void)
{
	int i, ret;

	ret = 1;
	printk("flexsc_exit hooked start\n");
	for (i = 0; i < NUM_PINNED_PAGES; i++) {
		kunmap(pinned_pages[i]);
	}

	if (kstruct) {
		ret = kthread_stop(kstruct);
		kstruct = NULL;
	}

	if (!ret) {
		printk("kthread stopped\n");
	}

	if (!sys_workqueue) {
		destroy_workqueue(sys_workqueue);
	}

	if (!sys_works) {
		kfree(sys_works);
	}

	printk("flexsc_exit hooked end\n");
	return 0;
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

	orig_flexsc_register = sys_call_table[__NR_flexsc_register];
	orig_flexsc_exit = sys_call_table[__NR_flexsc_exit];
	
	pr_info("flexsc exit orig: %d %p\n", __NR_flexsc_exit, orig_flexsc_register);
	pr_info("flexsc register orig:%d %p\n", __NR_flexsc_register, orig_flexsc_exit);

	disable_write_protection();
	sys_call_table[__NR_flexsc_register] = (void*)&syshook_flexsc_register;
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
	       entry->regs->di, entry->regs->si, entry->regs->dx,
	       entry->regs->r10, entry->regs->r8, entry->regs->r9);
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
