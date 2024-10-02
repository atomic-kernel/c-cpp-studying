#include <signal.h>

#include <time.h>
#include <stdio.h>

void sighandler(int sig)
{
    printf("get sig:%d\n", sig);
}

int main()
{
    // ctrl + '\'
    signal(SIGQUIT, sighandler);
    // kill
    signal(SIGTERM, sighandler);
    // ctrl + z
    signal(SIGTSTP, sighandler);
    // ctrl + c
    signal(SIGINT, sighandler);
    printf("1111\n");
    struct timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 99999999;
    printf("nanosleep return :%d\n", nanosleep(&ts, NULL));
    printf("2222\n");
    return 0;
}
