#include <unistd.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>

#define fork_execlp_wait(file, ...) \
({ \
    int _ret; \
    pid_t _pid = fork(); \
    if (_pid == -1) { \
        fputs("error: fork failed!\n", stderr); \
        _ret = -1; \
        goto __label_return; \
    } \
    if (_pid == 0) { \
        _ret = execlp(file, __VA_ARGS__); \
        if (_ret == 0) { \
            fputs("error: execlp return 0!\n", stderr); \
            _ret = -1; \
        } \
        exit(_ret); \
    } \
    wait(&_ret); \
__label_return: \
    _ret; \
})

int fork_execve_wait(const char *const pathname, char *const argv[], char *const envp[])
{
    pid_t pid = fork();
    if (pid == -1) {
        fputs("error: fork failed!\n", stderr);
        return -1;
    }
    if (pid == 0) {
        int ret = execve(pathname, argv, envp);
        if (ret == 0) {
            fputs("error: execve return 0!\n", stderr);
            ret = -1;
        }
        exit(ret);
    }
    int statval;
    wait(&statval);
    return statval;
}


int main(int argc, char *argv[])
{
    char *env = NULL;
    int statval = fork_execve_wait(argv[1], &argv[1], &env);
    if (WIFEXITED(statval) && WEXITSTATUS(statval) == 0)
        printf("子进程执行成功！%d\n", statval);
    else
        printf("子进程执行失败！%d\n", statval);

    statval = fork_execlp_wait("ls", "ls", "-al", "/", NULL);
    if (WIFEXITED(statval) && WEXITSTATUS(statval) == 0)
        printf("子进程执行成功！%d\n", statval);
    else
        printf("子进程执行失败！%d\n", statval);
    return 0;
}
