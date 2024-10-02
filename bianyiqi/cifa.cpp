#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include "function.h"
#include "cifa.h"
std::vector<Danci> list_Danci;
std::vector<BiaoShiFu> list_BiaoShiFu;
std::vector<int64_t> list_ConstInt;
std::vector<std::string> list_ConstStr;
static inline Danci get_ci(FILE *);
void cifa(FILE *fp_i)
{
    list_Danci.reserve(1024);
    list_BiaoShiFu.reserve(1024);
    list_ConstInt.reserve(1024);
    list_ConstStr.reserve(1024);
    char temp;
    goto label1;
    while((temp=getc(fp_i))!=EOF)
    {
        ungetc(temp,fp_i);
        list_Danci.emplace_back(get_ci(fp_i));
label1:
        get_kong(fp_i);
    }
}

inline Danci get_ci(FILE *fp_i)
{
    Danci x;
    std::string temp_s;
    char temp_c;
    temp_s+=(temp_c=getc(fp_i));
    if( temp_c=='_' || ('a'<=temp_c&&temp_c<='z') || ('A'<=temp_c&&temp_c<='Z'))
    {
        while( ('a'<=(temp_c=getc(fp_i))&&temp_c<='z') || ('A'<=temp_c&&temp_c<='Z') || ('0'<=temp_c&&temp_c<='9') || temp_c=='_' )
        {
            temp_s+=temp_c;
        }
        ungetc(temp_c,fp_i);
        if(temp_s=="and")
        {
            x.zhongbie=1;
        }
        else if(temp_s=="array")
        {
            x.zhongbie=2;
        }
        else if(temp_s=="begin")
        {
            x.zhongbie=3;
        }
        else if(temp_s=="bool")
        {
            x.zhongbie=4;
        }
        else if(temp_s=="call")
        {
            x.zhongbie=5;
        }
        else if(temp_s=="case")
        {
            x.zhongbie=6;
        }
        else if(temp_s=="char")
        {
            x.zhongbie=7;
        }
        else if(temp_s=="constant")
        {
            x.zhongbie=8;
        }
        else if(temp_s=="dim")
        {
            x.zhongbie=9;
        }
        else if(temp_s=="do")
        {
            x.zhongbie=10;
        }
        else if(temp_s=="else")
        {
            x.zhongbie=11;
        }
        else if(temp_s=="end")
        {
            x.zhongbie=12;
        }
        else if(temp_s=="false")
        {
            x.zhongbie=13;
        }
        else if(temp_s=="for")
        {
            x.zhongbie=14;
        }
        else if(temp_s=="if")
        {
            x.zhongbie=15;
        }
        else if(temp_s=="input")
        {
            x.zhongbie=16;
        }
        else if(temp_s=="integer")
        {
            x.zhongbie=17;
        }
        else if(temp_s=="not")
        {
            x.zhongbie=18;
        }
        else if(temp_s=="of")
        {
            x.zhongbie=19;
        }
        else if(temp_s=="or")
        {
            x.zhongbie=20;
        }
        else if(temp_s=="output")
        {
            x.zhongbie=21;
        }
        else if(temp_s=="procedure")
        {
            x.zhongbie=22;
        }
        else if(temp_s=="program")
        {
            x.zhongbie=23;
        }
        else if(temp_s=="read")
        {
            x.zhongbie=24;
        }
        else if(temp_s=="real")
        {
            x.zhongbie=25;
        }
        else if(temp_s=="repeat")
        {
            x.zhongbie=26;
        }
        else if(temp_s=="set")
        {
            x.zhongbie=27;
        }
        else if(temp_s=="stop")
        {
            x.zhongbie=28;
        }
        else if(temp_s=="then")
        {
            x.zhongbie=29;
        }
        else if(temp_s=="to")
        {
            x.zhongbie=30;
        }
        else if(temp_s=="true")
        {
            x.zhongbie=31;
        }
        else if(temp_s=="until")
        {
            x.zhongbie=32;
        }
        else if(temp_s=="var")
        {
            x.zhongbie=33;
        }
        else if(temp_s=="while")
        {
            x.zhongbie=34;
        }
        else if(temp_s=="write")
        {
            x.zhongbie=35;
        }
        else if(temp_s=="print")
        {
            x.zhongbie=70;
        }
        else
        {
            x.zhongbie=36;
            for(uint64_t i=list_BiaoShiFu.size();i!=0;)
            {
                i--;
                if(list_BiaoShiFu[i].zhi==temp_s)
                {
                    x.num=i;
                    return x;
                }
            }
            x.num=list_BiaoShiFu.size();
            list_BiaoShiFu.emplace_back( 4 , std::move(temp_s) );
        }
    }
    else if('0'<=temp_c&&temp_c<='9')
    {
        while('0'<=(temp_c=getc(fp_i))&&temp_c<='9')
        {
            temp_s+=temp_c;
        }
        if( temp_c=='_'|| ('A'<=temp_c&&temp_c<='Z') || ('a'<=temp_c&&temp_c<='z'))
        {
            printf("错误，变量以数字开头！\n");
            goto label_error;
        }
        ungetc(temp_c,fp_i);
        x.zhongbie=37;
        int64_t temp;
        sscanf(temp_s.c_str(),"%ld",&temp);
        for(uint64_t i=list_ConstInt.size();i!=0;)
        {
            i--;
            if(list_ConstInt[i]==temp)
            {
                x.num=i;
                return x;
            }
        }
        x.num=list_ConstInt.size();
        list_ConstInt.emplace_back(temp);
    }
    else if(temp_c=='\'')
    {
        temp_s.clear();
        while( (temp_c=getc(fp_i))!='\'' && temp_c!=EOF )
        {
            temp_s+=temp_c;
        }
        if(temp_c==EOF)
        {
            printf("错误，字符串只有一半!\n");
            goto label_error;
        }
        x.zhongbie=38;
        for(uint64_t i=list_ConstStr.size();i!=0;)
        {
            i--;
            if(list_ConstStr[i]==temp_s)
            {
                x.num=i;
                return x;
            }
        }
        x.num=list_ConstStr.size();
        list_ConstStr.emplace_back( std::move(temp_s) );
    }
    else if(temp_c=='(')
    {
        x.zhongbie=39;
    }
    else if(temp_c==')')
    {
        x.zhongbie=40;
    }
    else if(temp_c=='*')
    {
        x.zhongbie=41;
    }
    else if(temp_c=='+')
    {
        x.zhongbie=43;
    }
    else if(temp_c==',')
    {
        x.zhongbie=44;
    }
    else if(temp_c=='-')
    {
        x.zhongbie=45;
    }
    else if(temp_c=='.')
    {
        if( (temp_c=getc(fp_i))=='.' )
        {
            x.zhongbie=47;
        }
        else
        {
            ungetc(temp_c,fp_i);
            x.zhongbie=46;
        }
    }
    else if(temp_c=='/')
    {
        x.zhongbie=48;
    }
    else if(temp_c==':')
    {
        if( (temp_c=getc(fp_i))=='=' )
        {
            x.zhongbie=51;
        }
        else
        {
            ungetc(temp_c,fp_i);
            x.zhongbie=50;
        }
    }
    else if(temp_c==';')
    {
        x.zhongbie=52;
    }
    else if(temp_c=='<')
    {
        if( (temp_c=getc(fp_i))=='=' )
        {
            x.zhongbie=54;
        }
        else if( temp_c=='>' )
        {
            x.zhongbie=55;
        }
        else
        {
            ungetc(temp_c,fp_i);
            x.zhongbie=53;
        }
    }
    else if(temp_c=='=')
    {
        x.zhongbie=56;
    }
    else if(temp_c=='>')
    {
        if( (temp_c=getc(fp_i))=='=' )
        {
            x.zhongbie=58;
        }
        else
        {
            ungetc(temp_c,fp_i);
            x.zhongbie=57;
        }
    }
    else if(temp_c=='[')
    {
        x.zhongbie=59;
    }
    else if(temp_c==']')
    {
        x.zhongbie=60;
    }
    else
    {
        printf("非法字符 \'%c\'\n",temp_c);
label_error:
        printf("词法分析错误!\n");
        exit(1);
    }
    return x;
}
