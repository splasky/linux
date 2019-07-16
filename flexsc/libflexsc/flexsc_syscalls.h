#include "flexsc.h"

struct call_info {
    struct flexsc_sysentry *entry;
    struct flexsc_cb *cb;
};

/* long flexsc_getpid(struct call_info *info);
long flexsc_read(struct call_info *info);
long flexsc_write(struct call_info *info);
long flexsc_mmap(struct call_info *info);
long flexsc_stat(struct call_info *info);
 */

struct flexsc_sysentry *flexsc_getpid(void);
struct flexsc_sysentry *flexsc_getppid(void);
struct flexsc_sysentry *flexsc_read(unsigned int fd, char *buf, size_t count);
struct flexsc_sysentry *flexsc_write(unsigned int fd, char *buf, size_t count);
struct flexsc_sysentry* flexsc_stat(const char *pathname, struct stat *statbuf);
struct flexsc_sysentry* flexsc_pthread_create(pthread_t *newthread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);

void request_syscall_read(struct flexsc_sysentry *entry, unsigned int fd, char  *buf, size_t count);
void request_syscall_write(struct flexsc_sysentry *entry, unsigned int fd, char  *buf, size_t count);
void request_syscall_open(struct flexsc_sysentry *entry, const char  *filename, int flags, mode_t mode);
void request_syscall_close(struct flexsc_sysentry *entry, unsigned int fd);
void request_syscall_getpid(struct flexsc_sysentry *entry);
void request_syscall_getppid(struct flexsc_sysentry *entry);
void request_syscall_stat(struct flexsc_sysentry *entry, const char *pathname, struct stat *statbuf);
void request_syscall_pthread_create(struct flexsc_sysentry *entry, pthread_t *newthread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);


/* long flexsc_getpid(struct flexsc_sysentry *entry);
long flexsc_read(struct flexsc_sysentry *entry, unsigned int fd, char *buf, size_t count);
long flexsc_write(struct flexsc_sysentry *entry, unsigned int fd, char *buf, size_t count); */
// long flexsc_mmap(struct flexsc_sysentry *entry, unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long pgoff);

