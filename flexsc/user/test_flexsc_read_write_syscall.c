/*
 * sch-test
 * test program fro syscallh
 * Feb 23, 2019
 * root@davejingtian.org
 * https://davejingtian.org
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static const char* readme = "README.md";
static const char buf[64];

int main(void)
{
    printf("pid:%d\n", getpid());
    int fd = open(readme, O_RDONLY);
    printf("open:%d,buf:%d\n", fd, (int)buf);
    int bytes = read(fd, buf, sizeof(buf) / 2);
    printf("Bytes: %zu\n%s\n", bytes, buf);

    close(fd);
    return 0;
}
