#include <unistd.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fputs("argc != 3\n", stderr);
        return -1;
    }
    printf("%d\n", symlink(argv[1], argv[2]));
    return 0;
}
