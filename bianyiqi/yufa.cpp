#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "cifa.h"
#include "yufa.h"
//语法分析-selet( <复合句> -> begin ...)
static void fenxi_61(Tree &,uint64_t &i);
//语法分析-对句子选择selete语句
static inline bool fenxi_62(Tree &,uint64_t &i);
//语法分析-selet(<句子>-><赋值语句>)
static void fenxi_63(Tree &,uint64_t &i);
//语法分析-selet(<句子>-><if语句>)
static void fenxi_64(Tree &,uint64_t &i);
//语法分析-selet(<句子>-><repeat语句>)
static void fenxi_65(Tree &,uint64_t &i);
//语法分析-setet(<句子>-><while语句>)
static void fenxi_66(Tree &,uint64_t &i);
//语法分析-setet(<句子>-><print语句>)
static void fenxi_71(Tree &,uint64_t &i);

//获取一个算术表达式
static inline bool get_suanshu(Tree &,uint64_t &i);
//获取一个布尔表达式
static inline bool get_bool(Tree &,uint64_t &i);

//除了fenxi_62()，其他都需要给空树

Tree root(61);
static inline uint64_t var_define();
void yufa()
{
    //第一部分，保证代码符合 "progame <标识符>;<变量说明> begin ... end ." ,并且确定好变量类型
    //这一部分采用自底向上的分析方法
    uint64_t i;
    if(list_Danci.size()<6)
    {
        goto label1;
    }
    if(list_Danci[list_Danci.size()-1].zhongbie!=46)
    {
        printf("语法分析错误！！\n");
        printf("程序末尾不是'.'，请检查\n");
        exit(1);
    }
    if(list_Danci[list_Danci.size()-2].zhongbie!=12)
    {
        goto label1;
    }
    if(list_Danci[0].zhongbie==23&&list_Danci[1].zhongbie==36&&list_Danci[2].zhongbie==52&&list_Danci[3].zhongbie==33)
    {
        list_BiaoShiFu[list_Danci[1].num].type=3;
        i=var_define();
        if(list_Danci[i].zhongbie!=3)
        {
            goto label1;
        }
    }
    else if(list_Danci[0].zhongbie==23&&list_Danci[1].zhongbie==36&&list_Danci[2].zhongbie==52&&list_Danci[3].zhongbie==3)
    {
        list_BiaoShiFu[list_Danci[1].num].type=3;
        i=3;
    }
    else
    {
label1:
        printf("语法分析错误！！\n");
        printf("<程序> → program <标识符> ；<变量说明> <复合语句>.\n");
        printf("<变量说明> → var <变量定义>│ε\n");
        printf("<复合句> → begin <语句表> end\n");
        printf("请检查");
        exit(1);
    }

    //第二部分，对中间的复合句进行分析
    //采用自顶向下的语法分析，开始符S为<复合句>
    fenxi_61(root,i);
    
    if(list_Danci[i].zhongbie!=46||i!=list_Danci.size()-1)
    {
        goto label1;
    }
}
void fenxi_61(Tree &root,uint64_t&i)
{
    i++;
label1:
    if(fenxi_62(root,i))
    {
        goto label1;
    }
    else if(list_Danci[i].zhongbie==52)
    {
        i++;
        goto label1;
    }
    else if(list_Danci[i].zhongbie==12)
    {
        i++;
    }
    else
    {
        printf("语法分析错误!!\n");
        printf("复合句错误\n");
        exit(1);
    }
}
inline bool fenxi_62(Tree &root,uint64_t &i)
{
    if(list_Danci[i].zhongbie==3)
    {
        root.list.emplace_back(61);
        fenxi_61(root.list[root.list.size()-1],i);
    }
    else if(list_Danci[i].zhongbie==36)
    {
        root.list.emplace_back(63);
        fenxi_63(root.list[root.list.size()-1],i);
    }
    else if(list_Danci[i].zhongbie==15)
    {
        root.list.emplace_back(64);
        fenxi_64(root.list[root.list.size()-1],i);
    }
    else if(list_Danci[i].zhongbie==26)
    {
        root.list.emplace_back(65);
        fenxi_65(root.list[root.list.size()-1],i);
    }
    else if(list_Danci[i].zhongbie==34)
    {
        root.list.emplace_back(66);
        fenxi_66(root.list[root.list.size()-1],i);
    }
    else if(list_Danci[i].zhongbie==70)
    {
        root.list.emplace_back(71);
        fenxi_71(root.list[root.list.size()-1],i);
    }
    else
    {
        return false;
    }
    return true;
}
void fenxi_71(Tree &root,uint64_t &i)
{
    i++;
    //变量
    if(list_Danci[i].zhongbie==36)
    {
        if(list_BiaoShiFu[list_Danci[i].num].type==0||list_BiaoShiFu[list_Danci[i].num].type==1)
        {
            root.list.emplace_back(0,list_Danci[i].num);
        }
        else
        {
            goto label_err;
        }
    }
    //常量
    else if(list_Danci[i].zhongbie==37||list_Danci[i].zhongbie==38)
    {
        root.list.emplace_back(list_Danci[i].zhongbie-36,list_Danci[i].num);
    }
    else
    {
label_err:
        printf("语法分析错误！\n");
        printf("print语句错误\n");
        exit(1);
    }
    i++;
}
void fenxi_63(Tree &root,uint64_t &i)
{
    i++;
    if(list_Danci[i].zhongbie!=51)
    {
label_err:
        printf("语法分析错误!!\n");
        printf("赋值语句错误\n");
        exit(1);
    }
    i++;
    //整型
    if(list_BiaoShiFu[list_Danci[i-2].num].type==0)
    {
        root.list.emplace_back(0,list_Danci[i-2].num);
        root.list.emplace_back(70);
        if(!get_suanshu(root.list[1],i))
        {
            goto label_err;
        }
    }
    //字符型
    else if(list_BiaoShiFu[list_Danci[i-2].num].type==1)
    {
        root.list.emplace_back(1,list_Danci[i-2].num);
        if(list_Danci[i].zhongbie==38)
        {
            root.list.emplace_back(2,list_Danci[i].num);
        }
        else if(list_Danci[i].zhongbie==36&&list_BiaoShiFu[list_Danci[i].num].type==1)
        {
            root.list.emplace_back(0,list_Danci[i].num);
        }
        else
        {
            goto label_err;
        }
        i++;
    }
    //布尔型
    else if(list_BiaoShiFu[list_Danci[i-2].num].type==2)
    {
        root.list.emplace_back(2,list_Danci[i-2].num);
        root.list.emplace_back(71);
        if(!get_bool(root.list[1],i))
        {
            goto label_err;
        }
    }
    else if(list_BiaoShiFu[list_Danci[i-2].num].type==3)
    {
        printf("语法分析错误!!\n");
        printf("赋值语句：变量名是程序名\n");
        exit(1);
    }
    else
    {
        printf("语法分析错误!!\n");
        printf("赋值语句：变量未定义\n");
        exit(1);
    }
}
void fenxi_64(Tree &root,uint64_t &i)
{
    i++;
    root.list.emplace_back(71);
    if(!get_bool(root.list[0],i)||list_Danci[i].zhongbie!=29)
    {
label_err:
        printf("语法分析错误!\n");
        printf("if语句错误\n");
        exit(1);
    }
    i++;
    if(fenxi_62(root,i))
    {
    }
    else if(list_Danci[i].zhongbie==11||list_Danci[i].zhongbie==12||list_Danci[i].zhongbie==32||list_Danci[i].zhongbie==52)
    {
        root.list.emplace_back(69);
    }
    else
    {
        goto label_err;
    }
    if(list_Danci[i].zhongbie==11)
    {
        i++;
        if(fenxi_62(root,i))
        {
        }
        else if(list_Danci[i].zhongbie==11||list_Danci[i].zhongbie==12||list_Danci[i].zhongbie==32||list_Danci[i].zhongbie==52)
        {
            root.list.emplace_back(69);
        }
        else
        {
            goto label_err;
        }
    }
    else if(list_Danci[i].zhongbie==12||list_Danci[i].zhongbie==32||list_Danci[i].zhongbie==52)
    {
        root.list.emplace_back(69);
    }
    else
    {
        goto label_err;
    }
}
void fenxi_65(Tree &root,uint64_t &i)
{
    i++;
    if(fenxi_62(root,i))
    {
    }
    else if(list_Danci[i].zhongbie==32)
    {
        root.list.emplace_back(69);
    }
    if(list_Danci[i].zhongbie!=32)
    {
        printf("语法分析错误\n");
        printf("repeat语句缺少until\n");
        exit(1);
    }
    i++;
    root.list.emplace_back(71);
    if(!get_bool(root.list[1],i))
    {
        printf("语法分析错误\n");
        printf("repeat语句缺少布尔表达式\n");
        exit(1);
    }
}
void fenxi_66(Tree &root,uint64_t &i)
{
    i++;
    root.list.emplace_back(71);
    if(!get_bool(root.list[0],i)||list_Danci[i].zhongbie!=10)
    {
        printf("语法分析错误!\n");
        printf("while语句缺少do\n");
        exit(1);
    }
    i++;
    if(fenxi_62(root,i))
    {
    }
    else if(list_Danci[i].zhongbie==11||list_Danci[i].zhongbie==12||list_Danci[i].zhongbie==32||list_Danci[i].zhongbie==52)
    {
        root.list.emplace_back(69);
    }
    else
    {
        printf("语法分析错误!\n");
        printf("while语句错误\n");
        exit(1);
    }
}
//18:布尔变量
//19：布尔常量true
//20：布尔常量false
//21-29：临时布尔变量
//30：整型变量
//31：整型常量
//>31：临时整型变量
//+ - * / 负号 < <= != == > >= and or not ( ) # O
uint8_t bool_b[18][18]={
    {3,3,1,1,1, 3,3,3,3,3,3, 0,0,0, 1,3, 3,0},//+
    {3,3,1,1,1, 3,3,3,3,3,3, 0,0,0, 1,3, 3,0},//-
    {3,3,3,3,1, 3,3,3,3,3,3, 0,0,0, 1,3, 3,0},//*
    {3,3,3,3,1, 3,3,3,3,3,3, 0,0,0, 1,3, 3,0},// /
    {3,3,3,3,1, 3,3,3,3,3,3, 0,0,0, 1,3, 3,0},// 负号
    {1,1,1,1,1, 3,3,3,3,3,3, 3,3,3, 1,3, 3,0},// <
    {1,1,1,1,1, 3,3,3,3,3,3, 3,3,3, 1,3, 3,0},// <
    {1,1,1,1,1, 3,3,3,3,3,3, 3,3,3, 1,3, 3,0},// <
    {1,1,1,1,1, 3,3,3,3,3,3, 3,3,3, 1,3, 3,0},// <
    {1,1,1,1,1, 3,3,3,3,3,3, 3,3,3, 1,3, 3,0},// <
    {1,1,1,1,1, 3,3,3,3,3,3, 3,3,3, 1,3, 3,0},// <
    {1,1,1,1,1, 1,1,1,1,1,1, 3,3,1, 1,3, 3,0},// and
    {1,1,1,1,1, 1,1,1,1,1,1, 1,3,1, 1,3, 3,0},// or
    {1,1,1,1,1, 1,1,1,1,1,1, 3,3,1, 1,3, 3,0},// not
    {1,1,1,1,1, 1,1,1,1,1,1, 1,1,1, 1,2, 0,0},// (
    {3,3,3,3,0, 3,3,3,3,3,3, 3,3,0, 0,3, 3,0},// )
    {1,1,1,1,1, 1,1,1,1,1,1, 1,1,1, 1,0, 2,3},//#
    {0,0,0,0,0, 0,0,0,0,0,0, 0,0,0, 0,0, 1,0} //O
};
inline bool get_bool(Tree &root,uint64_t &i)
{
    std::vector<Danci> temp;
    temp.reserve(30);
    temp.emplace_back(17);
    temp.emplace_back(16);
    while(1)
    {
        if(list_Danci[i].zhongbie==43)
        {
            temp.emplace_back(0);
        }
        else if(list_Danci[i].zhongbie==45)
        {
            if(temp[temp.size()-1].zhongbie==15||temp[temp.size()-1].zhongbie>=29)
            {
                temp.emplace_back(1);
            }
            else
            {
                temp.emplace_back(4);
            }
        }
        else if(list_Danci[i].zhongbie==41)
        {
            temp.emplace_back(2);
        }
        else if(list_Danci[i].zhongbie==48)
        {
            temp.emplace_back(3);
        }
        else if(53<=list_Danci[i].zhongbie&&list_Danci[i].zhongbie<=58)
        {
            temp.emplace_back(list_Danci[i].zhongbie-48);
        }
        else if(list_Danci[i].zhongbie==1)
        {
            temp.emplace_back(11);
        }
        else if(list_Danci[i].zhongbie==20)
        {
            temp.emplace_back(12);
        }
        else if(list_Danci[i].zhongbie==18)
        {
            temp.emplace_back(13);
        }
        else if(list_Danci[i].zhongbie==39)
        {
            temp.emplace_back(14);
        }
        else if(list_Danci[i].zhongbie==40)
        {
            temp.emplace_back(15);
        }
        else if(list_Danci[i].zhongbie==36)
        {
            if(list_BiaoShiFu[list_Danci[i].num].type==0)
            {
                temp.emplace_back(30,list_Danci[i].num);
            }
            else if(list_BiaoShiFu[list_Danci[i].num].type==2)
            {
                temp.emplace_back(18,list_Danci[i].num);
            }
            else
            {
                printf("语法分析错误！\n");
                printf("布尔表达式中含有非法变量\n");
                exit(1);
            }
        }
        else if(list_Danci[i].zhongbie==37)
        {
            temp.emplace_back(31,list_Danci[i].num);
        }
        else if(list_Danci[i].zhongbie==31)
        {
            temp.emplace_back(19);
        }
        else if(list_Danci[i].zhongbie==13)
        {
            temp.emplace_back(20);
        }
        else
        {
            break;
        }
        i++;
    }
    if(temp.size()==2)
    {
        return false;
    }
    temp.emplace_back(16);
    temp.emplace_back(17);


    //目前temp2中<左边的运算符
    std::vector<uint64_t> xiaoyul;
    xiaoyul.reserve(30);
    std::vector<Tree> temp2;
    temp2.reserve(30);
    temp2.emplace_back(temp[0].zhongbie,temp[0].num);
    //指向temp目前的位置
    uint64_t i1=1;
    //目前temp2最前面的运算符
    uint64_t i2=0;
labelx:
    //temp寻找下一个符号
    while(temp[i1].zhongbie>=18)
    {
        temp2.emplace_back(temp[i1].zhongbie,temp[i1].num);
        i1++;
    }
    if(bool_b[temp2[i2].zhongbie][temp[i1].zhongbie]==1)
    {
        xiaoyul.emplace_back(i2);
        i2=temp2.size();
        temp2.emplace_back(temp[i1].zhongbie);
        i1++;
        goto labelx;
    }
    else if(bool_b[temp2[i2].zhongbie][temp[i1].zhongbie]==2)
    {
        i2=temp2.size();
        temp2.emplace_back(temp[i1].zhongbie);
        i1++;
        goto labelx;
    }
    else if(bool_b[temp2[i2].zhongbie][temp[i1].zhongbie]==0)
    {
        goto label_err;
    }
    i2=xiaoyul[xiaoyul.size()-1];
    xiaoyul.pop_back();
    if(temp2.size()-i2==4)
    {
        // ( ? )
        if(temp2[i2+1].zhongbie==14&&temp2[i2+2].zhongbie>=18&&temp2[i2+3].zhongbie==15)
        {
            temp2[i2+1]=temp2[i2+2];
        }
        // ? (+|-|*|/) ?
        else if(temp2[i2+1].zhongbie>=30&&temp2[i2+2].zhongbie<=3&&temp2[i2+3].zhongbie>=30)
        {
            if(temp2[i2+1].zhongbie<=31)
            {
                //temp2[i2+1].list.emplace_back(temp2[i2+1]);
                {
                    Tree temp=temp2[i2+1];
                    temp2[i2+1].list.emplace_back(temp);
                }
                temp2[i2+1].list.emplace_back(temp2[i2+3]);
            }
            else
            {
                //temp2[i2+1].list[0]=temp2[i2+1];
                {
                    Tree temp=temp2[i2+1];
                    temp2[i2+1].list[0]=temp;
                }
                if(temp2[i2+1].zhongbie==36)
                {
                    temp2[i2+1].list.emplace_back(temp2[i2+3]);
                }
                else
                {
                    temp2[i2+1].list[1]=temp2[i2+3];
                }
            }
            temp2[i2+1].zhongbie=temp2[i2+2].zhongbie+32;
        }
        // # ? #
        else if(temp2[i2+1].zhongbie==16&&18<=temp2[i2+2].zhongbie&&temp2[i2+2].zhongbie<=29&&temp2[i2+3].zhongbie==16)
        {
            root=temp2[i2+2];
            return true;
        }
        // ? 比较符 ?
        else if(temp2[i2+1].zhongbie>=30&&5<=temp2[i2+2].zhongbie&&temp2[i2+2].zhongbie<=10&&temp2[i2+3].zhongbie>=30)
        {
            if(temp2[i2+1].zhongbie<=31)
            {
                //temp2[i2+1].list.emplace_back(temp2[i2+1]);
                {
                    Tree temp=temp2[i2+1];
                    temp2[i2+1].list.emplace_back(temp);
                }
                temp2[i2+1].list.emplace_back(temp2[i2+3]);
            }
            else
            {
                //temp2[i2+1].list[0]=temp2[i2+1];
                {
                    Tree temp=temp2[i2+1];
                    temp2[i2+1].list[0]=temp;
                }
                temp2[i2+1].list[1]=temp2[i2+3];
                if(temp2[i2+1].zhongbie==36)
                {
                    temp2[i2+1].list.emplace_back(temp2[i2+3]);
                }
                else
                {
                    temp2[i2+1].list[1]=temp2[i2+3];
                }
            }
            temp2[i2+1].zhongbie=temp2[i2+2].zhongbie+16;
        }
        // ? and/or ?
        else if(18<=temp2[i2+1].zhongbie&&temp2[i2+1].zhongbie<=29&&(temp2[i2+2].zhongbie==11||temp2[i2+2].zhongbie==12)&&18<=temp2[i2+3].zhongbie&&temp2[i2+3].zhongbie<=29)
        {
            if(temp2[i2+1].zhongbie<=20)
            {
                //temp2[i2+1].list.emplace_back(temp2[i2+1]);
                {
                    Tree temp=temp2[i2+1];
                    temp2[i2+1].list.emplace_back(temp);
                }
                temp2[i2+1].list.emplace_back(temp2[i2+3]);
            }
            else
            {
                //temp2[i2+1].list[0]=temp2[i2+1];
                {
                    Tree temp=temp2[i2+1];
                    temp2[i2+1].list[0]=temp;
                }
                if(temp2[i2+1].zhongbie==29)
                {
                    temp2[i2+1].list.emplace_back(temp2[i2+3]);
                }
                else
                {
                    temp2[i2+1].list[1]=temp2[i2+3];
                }
            }
            temp2[i2+1].zhongbie=temp2[i2+2].zhongbie+16;
        }
        // err
        else
        {
            goto label_err;
        }
        temp2.pop_back();
        temp2.pop_back();
    }
    else if(temp2.size()-i2==3)
    {
        if(temp2[i2+1].zhongbie==4&&temp2[i2+2].zhongbie>=30)
        {
            temp2[i2+1].list.emplace_back(temp2[i2+2]);
            temp2[i2+1].zhongbie=36;
        }
        else if(temp2[i2+1].zhongbie==13&&18<=temp2[i2+2].zhongbie&&temp2[i2+2].zhongbie<=29)
        {
            temp2[i2+1].list.emplace_back(temp2[i2+2]);
            temp2[i2+1].zhongbie=29;
        }
        else
        {
            goto label_err;
        }
        temp2.pop_back();
    }
    else
    {
        goto label_err;
    }
    goto labelx;
label_err:
    printf("语法分析错误!\n");
    printf("布尔表达式错误!\n");
    exit(1);
}
//9:整型变量
//10：整型常量
//>10：临时整型变量(右值
//+ - * / 负号 ( ) # O
uint8_t suanshu[9][9]={
    {3,3,1,1,1, 1,3, 3,0},//+
    {3,3,1,1,1, 1,3, 3,0},//-
    {3,3,3,3,1, 1,3, 3,0},//*
    {3,3,3,3,1, 1,3, 3,0},// /
    {3,3,3,3,1, 1,3, 3,0},// 负号
    {1,1,1,1,1, 1,2, 0,0},// (
    {3,3,3,3,0, 0,3, 3,0},// )
    {1,1,1,1,1, 1,0, 2,3},//#
    {0,0,0,0,0, 0,0, 1,0} //O
};
inline bool get_suanshu(Tree &root,uint64_t &i)
{
    std::vector<Danci> temp;
    temp.reserve(30);
    temp.emplace_back(8);
    temp.emplace_back(7);
    while(1)
    {
        if(list_Danci[i].zhongbie==43)
        {
            temp.emplace_back(0);
        }
        else if(list_Danci[i].zhongbie==45)
        {
            if(temp[temp.size()-1].zhongbie==6||temp[temp.size()-1].zhongbie>=9)
            {
                temp.emplace_back(1);
            }
            else
            {
                temp.emplace_back(4);
            }
        }
        else if(list_Danci[i].zhongbie==41)
        {
            temp.emplace_back(2);
        }
        else if(list_Danci[i].zhongbie==48)
        {
            temp.emplace_back(3);
        }
        else if(list_Danci[i].zhongbie==39)
        {
            temp.emplace_back(5);
        }
        else if(list_Danci[i].zhongbie==40)
        {
            temp.emplace_back(6);
        }
        else if(list_Danci[i].zhongbie==36)
        {
            if(list_BiaoShiFu[list_Danci[i].num].type!=0)
            {
                printf("语法分析错误！\n");
                printf("算术表达式中含有非整型变量\n");
                exit(1);
            }
            temp.emplace_back(9,list_Danci[i].num);
        }
        else if(list_Danci[i].zhongbie==37)
        {
            temp.emplace_back(10,list_Danci[i].num);
        }
        else
        {
            break;
        }
        i++;
    }
    if(temp.size()==2)
    {
        return false;
    }
    temp.emplace_back(7);
    temp.emplace_back(8);

    //目前temp2中<左边的运算符
    std::vector<uint64_t> xiaoyul;
    xiaoyul.reserve(30);
    std::vector<Tree> temp2;
    temp2.reserve(30);
    temp2.emplace_back(temp[0].zhongbie,temp[0].num);
    //指向temp目前的位置
    uint64_t i1=1;
    //目前temp2最前面的运算符
    uint64_t i2=0;
labelx:
    //temp寻找下一个符号
    while(temp[i1].zhongbie>=9)
    {
        temp2.emplace_back(temp[i1].zhongbie,temp[i1].num);
        i1++;
    }
    if(suanshu[temp2[i2].zhongbie][temp[i1].zhongbie]==1)
    {
        xiaoyul.emplace_back(i2);
        i2=temp2.size();
        temp2.emplace_back(temp[i1].zhongbie);
        i1++;
        goto labelx;
    }
    else if(suanshu[temp2[i2].zhongbie][temp[i1].zhongbie]==2)
    {
        i2=temp2.size();
        temp2.emplace_back(temp[i1].zhongbie);
        i1++;
        goto labelx;
    }
    else if(suanshu[temp2[i2].zhongbie][temp[i1].zhongbie]==0)
    {
        goto label_err;
    }
    i2=xiaoyul[xiaoyul.size()-1];
    xiaoyul.pop_back();
    if(temp2.size()-i2==4)
    {
        // ( ? )
        if(temp2[i2+1].zhongbie==5&&temp2[i2+2].zhongbie>=9&&temp2[i2+3].zhongbie==6)
        {
            temp2[i2+1]=temp2[i2+2];
        }
        // ? (+|-|*|/) ?
        else if(temp2[i2+1].zhongbie>=9&&temp2[i2+2].zhongbie<=3&&temp2[i2+3].zhongbie>=9)
        {
            if(temp2[i2+1].zhongbie<=10)
            {
                {
                    Tree temp=temp2[i2+1];
                    temp2[i2+1].list.emplace_back(temp);
                }
                temp2[i2+1].list.emplace_back(temp2[i2+3]);
            }
            else
            {
                {
                    Tree temp=temp2[i2+1];
                    temp2[i2+1].list[0]=temp;
                }
                if(temp2[i2+1].zhongbie==15)
                {
                    temp2[i2+1].list.emplace_back(temp2[i2+3]);
                }
                else
                {
                    temp2[i2+1].list[1]=temp2[i2+3];
                }
            }
            temp2[i2+1].zhongbie=temp2[i2+2].zhongbie+11;
        }
        // # ? #
        else if(temp2[i2+1].zhongbie==7&&temp2[i2+2].zhongbie>=9&&temp2[i2+3].zhongbie==7)
        {
            root=temp2[i2+2];
            return true;
        }
        // err
        else
        {
            goto label_err;
        }
        temp2.pop_back();
        temp2.pop_back();
    }
    else if(temp2.size()-i2==3)
    {
        if(temp2[i2+1].zhongbie!=4||temp2[i2+2].zhongbie<9)
        {
            goto label_err;
        }
        temp2[i2+1].list.emplace_back(temp2[i2+2]);
        temp2[i2+1].zhongbie=15;
        temp2.pop_back();
    }
    else
    {
        goto label_err;
    }
    goto labelx;
label_err:
    printf("语法分析错误!\n");
    printf("算术表达式错误!\n");
    exit(1);
}
inline uint64_t var_define()
{
    uint64_t start=4;
    if(list_Danci[start].zhongbie!=36)
    {
        goto label_err;
    }
    do
    {
        uint64_t i=start+1;
        goto label1;
        while(list_Danci[i].zhongbie==44&&list_Danci[i+1].zhongbie==36)
        {
            i+=2;
label1:
            if(list_Danci.size()<=i+1)
            {
                goto label_err;
            }
        }
        const uint64_t end=i-1;
        if(list_Danci.size()<=end+3)
        {
            goto label_err;
        }
        if(!(
             list_Danci[end+1].zhongbie==50 && 
             (list_Danci[end+2].zhongbie==4||list_Danci[end+2].zhongbie==7||list_Danci[end+2].zhongbie==17) &&
             list_Danci[end+3].zhongbie==52
             ))
        {
            goto label_err;
        }
        const uint8_t type=list_Danci[end+2].zhongbie==4?2:(list_Danci[end+2].zhongbie==7?1:0);
        while(1)
        {
            list_BiaoShiFu[list_Danci[start].num].type=type;
            if(start==end)
            {
                break;
            }
            start+=2;
        }
        start+=4;
    }
    while(list_Danci.size()>start&&list_Danci[start].zhongbie==36);
    return start;
label_err:
    printf("语法分析错误！！\n");
    printf("<变量定义> → <标识符>(,<标识符>)*:<类型>;\n");
    printf("请检查");
    exit(1);
}
