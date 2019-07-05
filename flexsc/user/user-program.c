#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
/* #include "syscall_info.h" */
#include "../libflexsc/flexsc_syscalls.h"
#include <pthread.h>

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
// This thread is responsible for checking sysentry for its status and return value
pthread_t systhread;

void *thread_main(void *arg)
{
	while (1)
	{

    }

	pthread_exit((void *) 0);
}

void create_systhread(void) 
{
    pthread_create(&systhread, NULL, &thread_main, (void *)NULL);
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
    struct flexsc_sysentry *entry;
    struct flexsc_sysentry *receiver;
    struct flexsc_sysentry *receiver2;
    struct flexsc_init_info info;
    int i, num_entry, cnt = 0;
    pid_t mypid, mypid2;

    signal(SIGINT, (void *)sig_handler);
    
    /*
     * You may ask "where the info struct is initialized?"
     * For ease of testing, it has default setting 
     */
    entry = flexsc_register(&info);
    printf("After registering flexsc\n");

    /* Call getpid() - flexsc version*/
    receiver = flexsc_getpid();
    receiver2 = flexsc_getpid();

    /* Do something until issued system call is done */
    while (receiver->rstatus != FLEXSC_STATUS_DONE || receiver2->rstatus != FLEXSC_STATUS_DONE) {
        //printf("wait count: %d\n", cnt++);
    }

    /*Consumes return value*/
    receiver2->rstatus = FLEXSC_STATUS_FREE;
    mypid = receiver->sysret;
    mypid2 = receiver2->sysret;

    printf("PID2: %d\n", mypid);
    /*Change a entry to FREE*/
    receiver->rstatus = FLEXSC_STATUS_FREE;

    /*Usage of sysret*/
    printf("PID: %d\n", mypid);

    /* Wait until it gets SIGTERM */

    return 0;
}

void test_register() 
{

}
