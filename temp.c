#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define SIZE 3L * 1024 * 1024 * 1024

void error()
{
    fprintf(stderr, "error!\n");
    perror(NULL);
    exit(-1);
}

size_t ms(struct timespec time1, struct timespec time2)
{
    if (time2.tv_nsec >= time1.tv_nsec)
        return (time2.tv_nsec - time1.tv_nsec) / 1000000 + (time2.tv_sec - time1.tv_sec) * 1000;
    else
        return (1000000000 + time2.tv_nsec - time1.tv_nsec) / 1000000 + (time2.tv_sec - time1.tv_sec - 1) * 1000;
}

int main(int argc, char *argv[])
{
    struct timespec time;
    struct timespec time2;
    clock_gettime(CLOCK_MONOTONIC, &time);
    printf("start:%lu %lu\n", time.tv_sec, time.tv_nsec);
    if (argc != 2)
        return -1;
    void *const buf = malloc(SIZE);
    if (!buf)
        error();
    for (size_t i = 0; i < SIZE / 4; ++i) {
        ((int *)buf)[i] = rand();
    }
    //memset(buf, 0xc8, SIZE);
    printf("init end!\n");
    clock_gettime(CLOCK_MONOTONIC, &time);
    printf("init:%lu %lu\n", time.tv_sec, time.tv_nsec);

    const int fd = open(argv[1], O_CREAT | O_WRONLY, S_IRWXU);
    if (fd < 0)
        error();
    ssize_t need = SIZE;
    while (true) {
        ssize_t tt = write(fd, buf, need);
        if (tt < 0 || tt > need) {
            perror(NULL);
            fprintf(stderr, "error, %ld!\n", tt);
            exit(-1);
        }
        printf("write: %ld\n", tt);
        clock_gettime(CLOCK_MONOTONIC, &time2);
        printf("write:%lu\n", ms(time, time2));
        need -= tt;
        if (need == 0)
            break;
    }

    syncfs(fd);
    clock_gettime(CLOCK_MONOTONIC, &time2);
    printf("syncfs:%lu\n", ms(time, time2));

    close(fd);
    clock_gettime(CLOCK_MONOTONIC, &time2);
    printf("close:%lu\n", ms(time, time2));

    sync();
    clock_gettime(CLOCK_MONOTONIC, &time2);
    printf("sync:%lu\n", ms(time, time2));

    free(buf);
    return 0;
}
