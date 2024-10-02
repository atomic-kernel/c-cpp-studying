#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void yuchuli(FILE *fp_s,FILE *fp_i)
{
    char temp;
    while((temp=getc(fp_s))!=EOF)
    {
        if(temp=='/')
        {
            if((temp=getc(fp_s))=='*')
            {
label1:
                while( (temp=fgetc(fp_s))!=EOF && temp!='*' );
                if(temp==EOF)
                {
                    return;
                }
                else if(getc(fp_s)!='/')
                {
                    goto label1;
                }
                putc(' ',fp_i);
            }
            else if(temp=='/')
            {
                while( (temp=fgetc(fp_s))!=EOF && temp!='\n' );
                putc('\n',fp_i);
            }
            else if(temp==EOF)
            {
                putc('/',fp_i);
                return;
            }
            else
            {
                putc('/',fp_i);
                putc(temp,fp_i);
            }
        }
        else
        {
            putc(temp,fp_i);
        }
    }
}
