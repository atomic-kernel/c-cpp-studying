#include <stdio.h>
#include <stdint.h>

void get_kong(FILE *fp)
{
    char a;
    while((a=getc(fp))=='\n'||a==' '||a=='\t');
    ungetc(a,fp);
}

