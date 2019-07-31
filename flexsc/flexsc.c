#include "flexsc.h"
#include <linux/syscalls.h>

SYSCALL_DEFINE1(flexsc_register, struct flexsc_init_info *, info)
{
	pr_err("You must insert flexsc kernel module!");
	return -1;
}

SYSCALL_DEFINE0(flexsc_wait)
{
	pr_err("You must insert flexsc kernel module!");
	return -1;
}

SYSCALL_DEFINE0(flexsc_exit)
{
	pr_err("You must insert flexsc kernel module!");
	return -1;
}
