#include <stdio.h>
#include <unistd.h>

extern char **environ;

int main(int argc, const char *argv[])
{
    printf("args:\n");
    for (size_t i = 0; i < argc; ++i)
        printf("%s\n", argv[i]);
    printf("environment:\n");
    for (size_t i = 0;; ++i) {
        printf("%s\n", environ[i]);
        if (environ[i] == NULL)
            break;
    }

    return 0;
}
