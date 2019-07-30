/*
 * sch-test
 * test program fro syscallh
 * Feb 23, 2019
 * root@davejingtian.org
 * https://davejingtian.org
 */
#include "../libflexsc/flexsc_syscalls.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static const char* readme = "README.md";
static const char buf[64];

int main(void)
{
    struct flexsc_sysentry* entry;
    struct flexsc_sysentry* receiver;
    struct flexsc_init_info info;

    entry = flexsc_register(&info);
    printf("After registering flexsc\n");

    int fd = open(readme, O_RDONLY);
    printf("open:%d\n", fd);
    receiver = flexsc_read(fd, buf, sizeof(buf) / 2);
    while (receiver->rstatus != FLEXSC_STATUS_DONE)
        ;
    printf("Bytes: %zu\n%s\n", (ssize_t)receiver->sysret, buf);

    close(fd);
    return 0;
}
