#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define BLOCK_SIZE 0x400000

int cp(const char *const path1, const char *const path2)
{
    FILE *const fp1 = fopen(path1, "rb");
    if (fp1 == NULL)
        return -1;

    int ret;
    FILE *const fp2 = fopen(path2, "wb");
    if (fp2 == NULL) {
        ret = -1;
        goto label_error1;
    }

    void *const buf = malloc(BLOCK_SIZE);
    if (buf == NULL) {
        ret = -1;
        goto label_error2;
    }

    while (true) {
        const size_t read_size = fread(buf, 1, BLOCK_SIZE, fp1);
        if (fwrite(buf, 1, read_size, fp2) != read_size) {
            ret = -1;
            goto label_error3;
        }
        if (read_size < BLOCK_SIZE) {
            ret = ferror(fp1);
            if (ret != 0)
                goto label_error3;
            if (feof(fp1) == 0) {
                ret = -1;
                goto label_error3;
            }
            break;
        }
    }

    free(buf);

    ret = fclose(fp2);
    if (ret != 0)
        goto label_error1;

    return fclose(fp1);

label_error3:
    free(buf);
label_error2:
    fclose(fp2);
label_error1:
    fclose(fp1);
    return ret;
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        fputs("need two args\n", stderr);
        return -1;
    }
    printf("%d\n", cp(argv[1], argv[2]));
    return 0;
}
