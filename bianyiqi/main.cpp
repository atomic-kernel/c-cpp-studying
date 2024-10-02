#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include "yuchuli.h"
#include "cifa.h"
#include "yufa.h"
#include "siyuan.h"
#include "mubiao.h"

static inline void print_cifa();
static inline void print_siyuan();
static inline void print_siyuan_part(uint8_t,uint64_t);
int main(int argc,char *argv[])
{
    printf("计科一班谢元彬\n");
    if(argc!=2)
    {
        printf("使用方法:编译器可执行文件名 源代码文件名\n");
        return 1;
    }

    //预处理
    printf("\n预处理。。。\n");
    std::string filename_s=argv[1];
    FILE *fp_1=fopen(filename_s.c_str(),"r");
    if(fp_1==NULL)
    {
        printf("打开源码文件失败！\n");
        return 1;
    }
    FILE *fp_2=fopen((filename_s+".i").c_str(),"w");
    if(fp_2==NULL)
    {
        printf("创建临时文件失败！\n");
        exit(1);
    }
    yuchuli(fp_1,fp_2);
    fclose(fp_1);
    fclose(fp_2);
    printf("预处理输出=>'%s'\n",(filename_s+".i").c_str());

    //词法分析
    printf("\n词法分析。。。\n");
    fp_1=fopen((filename_s+".i").c_str(),"r");
    if(fp_1==NULL)
    {
        printf("打开临时文件失败！\n");
        return 1;
    }
    cifa(fp_1);
    fclose(fp_1);

    //词法分析输出
    printf("词法分析输出：\n");
    print_cifa();
    putchar('\n');

    //语法分析
    printf("\n语法分析。。。\n");
    yufa();

    //生成四元式
    printf("\n四元式生成。。。\n");
    siyuan();

    //打印四元式
    printf("打印四元式：\n");
    print_siyuan();
    
    //生成目标代码
    printf("\n目标代码生成。。。\n");
    fp_1=fopen((filename_s+".c").c_str(),"w");
    if(fp_1==NULL)
    {
        printf("打开临时文件失败！\n");
        exit(1);
    }
    mubiao(fp_1);
    fclose(fp_1);
    printf("生成目标代码==>'%s.c'\n",filename_s.c_str());

    //生成可执行文件
    printf("\n可执行文件生成。。。\n");
    if(system(("gcc -Wall -Wextra -pedantic -O3 "+filename_s+".c -o "+filename_s+".out").c_str())!=0)
    {
        printf("生成可执行文件失败！\n");
        printf("可能系统组件中缺少gcc，无法生成可执行文件\n");
        printf("建议在Linux系统中运行\n");
        exit(1);
    }
    else
    {
        printf("生成可执行文件==>'%s'\n",(filename_s+".out").c_str());
    }
    return 0;
}
inline void print_cifa()
{
    for(uint64_t i=0;i<list_Danci.size();i++)
    {
        printf("%lu.(%hhu,",i,list_Danci[i].zhongbie);
        if(list_Danci[i].zhongbie==36)
        {
            printf("标识符%lu,%s) ",list_Danci[i].num+1,list_BiaoShiFu[list_Danci[i].num].zhi.c_str());
        }
        else if(list_Danci[i].zhongbie==38)
        {
            printf("字符常量%lu,%s) ",list_Danci[i].num+1,list_ConstStr[list_Danci[i].num].c_str());
        }
        else if(list_Danci[i].zhongbie==37)
        {
            printf("整数常量%lu,%ld) ",list_Danci[i].num+1,list_ConstInt[list_Danci[i].num]);
        }
        else
        {
            printf("-) ");
        }
    }
}
inline void print_siyuan()
{
    for(uint64_t i=0;i<list_Siyuan.size();i++)
    {
        if(list_Siyuan[i].zhongbie==0)
        {
            printf(".L%lu:\n",list_Siyuan[i].num1);
            continue;
        }
        printf("    ");
        if(list_Siyuan[i].zhongbie==1)
        {
            printf("jmp    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
        }
        else if(list_Siyuan[i].zhongbie==2)
        {
            printf("jl     ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==3)
        {
            printf("jle    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==4)
        {
            printf("jne    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==5)
        {
            printf("je     ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==6)
        {
            printf("jg     ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==7)
        {
            printf("jge    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==8)
        {
            printf("mov    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
        }
        else if(list_Siyuan[i].zhongbie==9)
        {
            printf("add    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==10)
        {
            printf("sub    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==11)
        {
            printf("mul    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==12)
        {
            printf("div    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==14)
        {
            printf("and    ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==15)
        {
            printf("or     ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
            print_siyuan_part(list_Siyuan[i].zb3,list_Siyuan[i].num3);
        }
        else if(list_Siyuan[i].zhongbie==16)
        {
            printf("xor    1     ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
            print_siyuan_part(list_Siyuan[i].zb2,list_Siyuan[i].num2);
        }
        else if(list_Siyuan[i].zhongbie==17)
        {
            printf("print  ");
            print_siyuan_part(list_Siyuan[i].zb1,list_Siyuan[i].num1);
        }
        putchar('\n');
    }
}
inline void print_siyuan_part(uint8_t zb,uint64_t num)
{
    if(zb==0)
    {
        printf("%s    ",list_BiaoShiFu[num].zhi.c_str());
    }
    else if(zb==1)
    {
        printf("%ld    ",list_ConstInt[num]);
    }
    else if(zb==2)
    {
        printf("'%s'   ",list_ConstStr[num].c_str());
    }
    else if(zb==3)
    {
        printf("@Tint%lu   ",num);
    }
    else if(zb==4)
    {
        printf("@Tbool%lu   ",num);
    }
    else if(zb==5)
    {
        printf("1      ");
    }
    else if(zb==6)
    {
        printf("0      ");
    }
    else if(zb==7)
    {
        printf(".L%lu",num);
    }
}
