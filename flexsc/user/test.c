#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
/* #include "syscall_info.h" */
#include "../libflexsc/flexsc_syscalls.h"
#include <pthread.h>
#define SYSCALL_AMOUNT 100
#define RECEIVER_AMOUNT 20

#include <sys/stat.h>
/* int stat(const char *file_name, struct stat *buf); */
#include <signal.h>

void sig_handler(int signo)
{
    printf("flexsc user program is going die SIGINT(%d)\n", SIGINT);
    /**
     * flexsc_exit terminates systhreads,
     * deallocates sysentry array */
    flexsc_exit();
    exit(EXIT_SUCCESS);
}

struct flexsc_sysentry *receiver[RECEIVER_AMOUNT];
int find_free_receiver() {
	for (int i=0; i<RECEIVER_AMOUNT; ++i) {
		if (receiver == NULL) return 0;
		if (receiver[i] == NULL) return i; // First time
		if (receiver[i]->rstatus == FLEXSC_STATUS_DONE) {
			receiver[i]->rstatus = FLEXSC_STATUS_FREE;
			printf("receiver[%d] sysret: %u\n", i, receiver[i]->sysret);
			return i;
		}
		if (receiver[i]->rstatus != FLEXSC_STATUS_BUSY && \
				receiver[i]->rstatus != FLEXSC_STATUS_SUBMITTED)
			return i;
	}
	return -1;
}

void wait() {
	int cnt = 0;
	int total = 7;
	while (cnt != total) {
		for (int i=0; i<RECEIVER_AMOUNT; ++i) {
			if (receiver[i] == NULL) --total;
			else if (receiver[i]->rstatus == FLEXSC_STATUS_DONE) {
			printf("receiver[%d] sysret: %u\n", i, receiver[i]->sysret);
				++cnt;
				receiver[i]->rstatus = 9999;
			}
			else if (receiver[i]->rstatus == FLEXSC_STATUS_FREE) {
				++cnt;
				receiver[i]->rstatus == 9999;
			}
		}
	}
	exit(0);
}

/* This program contains system calls listed in table1, flexsc paper(OSDI 10),
 * Below is the system calls that will be tested
 *****************************************************************
 * stat
 * pread
 * pwrite
 * open
 * close
 * write
 * mmap
 * munmap
 *
 ******************************************************************
*/
int main(int argc, const char *argv[])
{
    pid_t pid = getpid();
    pid_t pid2 = syscall(39);
    printf("Pid: %lu %lu\n", pid, pid2);
    sleep(2);
    struct flexsc_sysentry *entry;
    struct flexsc_init_info info;
    entry = flexsc_register(&info);
    
    signal(SIGINT, (void *)sig_handler);
    for (int i=0; i < SYSCALL_AMOUNT; ++i) {
	int ind = find_free_receiver();
	if (ind == -1) {
		--i;
		continue;
	}
	printf("************ current %d *******************\n", i);
	receiver[ind] = flexsc_getpid();
    }
    wait();
    
    /*
     * You may ask "where the info struct is initialized?"
     * For ease of testing, it has default setting 
     */
    return 0;
}
