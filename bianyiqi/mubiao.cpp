#include <stdio.h>
#include <stdint.h>
#include "siyuan.h"
#include "cifa.h"
static inline void mubiaoju(FILE*fp,Siyuan&x);
static inline void mubiaoci1(FILE*fp,Siyuan&x);
static inline void mubiaoci2(FILE*fp,Siyuan&x);
static inline void mubiaoci3(FILE*fp,Siyuan&x);
void mubiao(FILE *fp)
{
    fprintf(fp,"#include <stdio.h>\n");
    fprintf(fp,"#include <stdint.h>\n");
    fprintf(fp,"#include <stdbool.h>\n");
    fprintf(fp,"int main()\n");
    fprintf(fp,"{\n");
    for(uint64_t i=0;i<num_tmp_i;i++)
    {
        fprintf(fp,"    int64_t tmp_i%lu;\n",i);
    }
    for(uint64_t i=0;i<num_tmp_b;i++)
    {
        fprintf(fp,"    bool tmp_b%lu;\n",i);
    }
    for(uint64_t i=1;i<list_BiaoShiFu.size();i++)
    {
        if(list_BiaoShiFu[i].type==0)
        {
            fprintf(fp,"    int64_t var%lu;\n",i);
        }
        else if(list_BiaoShiFu[i].type==1)
        {
            fprintf(fp,"    const char *var%lu=NULL;\n",i);
        }
        else if(list_BiaoShiFu[i].type==2)
        {
            fprintf(fp,"    bool var%lu;\n",i);
        }
        else
        {
            printf("生成目标代码错误\n");
            exit(1);
        }
    }
    for(uint64_t i=0;i<list_Siyuan.size();i++)
    {
        mubiaoju(fp,list_Siyuan[i]);
    }
    fprintf(fp,"    return 0;\n");
    fprintf(fp,"}\n");
}
inline void mubiaoju(FILE*fp,Siyuan &x)
{
    if(x.zhongbie==0)
    {
        fprintf(fp,"label%lu:\n",x.num1);
        return;
    }
    fprintf(fp,"    ");
    if(x.zhongbie==1)
    {
        fprintf(fp,"goto label%lu",x.num1);
    }
    else if(2<=x.zhongbie&&x.zhongbie<=7)
    {
        fprintf(fp,"if(");
        mubiaoci1(fp,x);
        if(x.zhongbie==2)
        {
            fprintf(fp,"<");
        }
        else if(x.zhongbie==3)
        {
            fprintf(fp,"<=");
        }
        else if(x.zhongbie==4)
        {
            fprintf(fp,"!=");
        }
        else if(x.zhongbie==5)
        {
            fprintf(fp,"==");
        }
        else if(x.zhongbie==6)
        {
            fprintf(fp,">");
        }
        else if(x.zhongbie==7)
        {
            fprintf(fp,">=");
        }
        mubiaoci2(fp,x);
        fprintf(fp,")goto label%lu",x.num3);
    }
    else if(x.zhongbie==8)
    {
        mubiaoci2(fp,x);
        putc('=',fp);
        mubiaoci1(fp,x);
    }
    else if(9<=x.zhongbie&&x.zhongbie<=15)
    {
        mubiaoci3(fp,x);
        putc('=',fp);
        mubiaoci1(fp,x);
        if(x.zhongbie==9)
        {
            putc('+',fp);
        }
        else if(x.zhongbie==10)
        {
            putc('-',fp);
        }
        else if(x.zhongbie==11)
        {
            putc('*',fp);
        }
        else if(x.zhongbie==12)
        {
            putc('/',fp);
        }
        else if(x.zhongbie==14)
        {
            putc('&',fp);
        }
        else if(x.zhongbie==15)
        {
            putc('|',fp);
        }
        else
        {
            printf("error\n");
            exit(1);
        }
        mubiaoci2(fp,x);
    }
    else if(x.zhongbie==16)
    {
        mubiaoci1(fp,x);
        fprintf(fp,"^=1");
    }
    else if(x.zhongbie==17)
    {
        if((x.zb1==0&&list_BiaoShiFu[x.num1].type==0)||x.zb1==1)
        {
            fprintf(fp,"printf(\"%%ld\",");
        }
        else if((x.zb1==0&&list_BiaoShiFu[x.num1].type==1)||x.zb1==2)
        {
            fprintf(fp,"printf(\"%%s\",");
        }
        else
        {
            printf("error\n");
            exit(1);
        }
        mubiaoci1(fp,x);
        putc(')',fp);
    }
    putc(';',fp);
    putc('\n',fp);
}
inline void mubiaoci1(FILE*fp,Siyuan&x)
{
    if(x.zb1==0)
    {
        fprintf(fp,"var%lu",x.num1);
    }
    else if(x.zb1==1)
    {
        fprintf(fp,"%ld",list_ConstInt[x.num1]);
    }
    else if(x.zb1==2)
    {
        fprintf(fp,"\"%s\"",list_ConstStr[x.num1].c_str());
    }
    else if(x.zb1==3)
    {
        fprintf(fp,"tmp_i%lu",x.num1);
    }
    else if(x.zb1==4)
    {
        fprintf(fp,"tmp_b%lu",x.num1);
    }
    else
    {
        fprintf(fp,"%hhu",6-x.zb1);
    }
}
inline void mubiaoci2(FILE*fp,Siyuan&x)
{
    if(x.zb2==0)
    {
        fprintf(fp,"var%lu",x.num2);
    }
    else if(x.zb2==1)
    {
        fprintf(fp,"%ld",list_ConstInt[x.num2]);
    }
    else if(x.zb2==2)
    {
        fprintf(fp,"\"%s\"",list_ConstStr[x.num2].c_str());
    }
    else if(x.zb2==3)
    {
        fprintf(fp,"tmp_i%lu",x.num2);
    }
    else if(x.zb2==4)
    {
        fprintf(fp,"tmp_b%lu",x.num2);
    }
    else
    {
        fprintf(fp,"%hhu",6-x.zb2);
    }
}
inline void mubiaoci3(FILE*fp,Siyuan&x)
{
    if(x.zb3==0)
    {
        fprintf(fp,"var%lu",x.num3);
    }
    else if(x.zb3==1)
    {
        fprintf(fp,"%ld",list_ConstInt[x.num3]);
    }
    else if(x.zb3==2)
    {
        fprintf(fp,"\"%s\"",list_ConstStr[x.num3].c_str());
    }
    else if(x.zb3==3)
    {
        fprintf(fp,"tmp_i%lu",x.num3);
    }
    else if(x.zb3==4)
    {
        fprintf(fp,"tmp_b%lu",x.num3);
    }
    else
    {
        fprintf(fp,"%hhu",6-x.zb3);
    }
}
