#include <stdio.h>
#include "siyuan.h"
#include <deque>
static void siyuan_61(Tree &root);
static inline void siyuan_62(Tree &root);
static void siyuan_63(Tree &root);
static void siyuan_64(Tree &root);
static void siyuan_65(Tree &root);
static void siyuan_66(Tree &root);
static uint64_t siyuan_bool(Tree&root);
static uint64_t siyuan_suanshu(Tree &root,uint8_t offset);
class Reg
{
public:
    uint64_t new_()
    {
        uint64_t i=0;
        for(;i<reg.size();i++)
        {
            if(reg[i]!=i)
            {
                reg.insert(reg.begin()+i,i);
                goto label1;
            }
        }
        reg.emplace_back(i);
label1:
        if(reg.size()>max)
        {
            max=reg.size();
        }
        return i;
    }
    void delete_(uint64_t x)
    {
        std::deque<uint64_t>::iterator i=reg.begin();
        while(i!=reg.end())
        {
            if(*i==x)
            {
                reg.erase(i);
                return;
            }
            i++;
        }
    }
    std::deque<uint64_t> reg;
    uint64_t max=0;
};
Reg reg_i,reg_b;
uint64_t num_tmp_i;
uint64_t num_tmp_b;
uint64_t label=0;
std::vector<Siyuan> list_Siyuan;
void siyuan()
{
    list_Siyuan.reserve(1000);
    siyuan_61(root);
    num_tmp_i=reg_i.max;
    num_tmp_b=reg_b.max;
}
uint64_t siyuan_bool(Tree&root)
{
    uint64_t a;
    // ? 关系运算符 ？
    if(21<=root.zhongbie&&root.zhongbie<=26)
    {
        //zhongbie:    0: 变量   1：整型常量    2：字符常量    3：临时整型变量    4：临时布尔变量    5:true    6:false
        uint8_t zb1;
        uint64_t num1;
        uint8_t zb2;
        uint64_t num2;
        a=reg_b.new_();
        if(root.list[0].zhongbie>=32)
        {
            zb1=3;
            num1=siyuan_suanshu(root.list[0],30);
            reg_i.delete_(num1);
            if(root.list[1].zhongbie>=32)
            {
                zb2=3;
                num2=siyuan_suanshu(root.list[1],30);
                reg_i.delete_(num2);
            }
            else
            {
                zb2=root.list[1].zhongbie-30;
                num2=root.list[1].num;
            }
        }
        else if(root.list[1].zhongbie>=32)
        {
            zb1=root.list[0].zhongbie-30;
            num1=root.list[0].num;
            zb2=3;
            num2=siyuan_suanshu(root.list[1],30);
            reg_i.delete_(num2);
        }
        else
        {
            zb1=root.list[0].zhongbie-30;
            num1=root.list[0].num;
            zb2=root.list[1].zhongbie-30;
            num2=root.list[1].num;
        }
        uint64_t lab=label;
        label+=2;
        list_Siyuan.emplace_back(root.zhongbie-19,zb1,num1,zb2,num2,7,lab);
        //设置为false
        list_Siyuan.emplace_back(8,6,0,4,a);
        list_Siyuan.emplace_back(1,7,lab+1);
        list_Siyuan.emplace_back(0,7,lab);
        //设置为true
        list_Siyuan.emplace_back(8,5,0,4,a);
        list_Siyuan.emplace_back(0,7,lab+1);
    }
    // not ?
    else if(root.zhongbie==29)
    {
        if(18<=root.list[0].zhongbie&&root.list[0].zhongbie<=20)
        {
            a=reg_b.new_();
            list_Siyuan.emplace_back(16,
                    root.list[0].zhongbie==18?0:root.list[0].zhongbie-14,root.list[0].num,
                    4,a);
        }
        else
        {
            a=siyuan_bool(root.list[0]);
            list_Siyuan.emplace_back(16, 4,a, 4,a);
        }
    }
    // ? and/or ?
    else if(root.list[0].zhongbie>=21)
    {
        a=siyuan_bool(root.list[0]);
        if(root.list[1].zhongbie>=21)
        {
            uint64_t b=siyuan_bool(root.list[1]);
            reg_i.delete_(b);
            list_Siyuan.emplace_back(root.zhongbie-13,
                    4,a,
                    4,b,
                    4,a);
        }
        else
        {
            list_Siyuan.emplace_back(root.zhongbie-13,
                    4,a,
                    root.list[1].zhongbie==18?0:root.list[1].zhongbie-14,root.list[1].num,
                    4,a);
        }
    }
    else if(root.list[0].zhongbie>=21)
    {
        a=siyuan_bool(root.list[1]);
        list_Siyuan.emplace_back(root.zhongbie-13,
                root.list[0].zhongbie==18?0:root.list[0].zhongbie-14,root.list[0].num,
                4,a,
                4,a);
    }
    else
    {
        a=reg_b.new_();
        list_Siyuan.emplace_back(root.zhongbie-13,
                root.list[0].zhongbie==18?0:root.list[0].zhongbie-14,root.list[0].num,
                root.list[1].zhongbie==18?0:root.list[1].zhongbie-14,root.list[1].num,
                4,a);
    }
    return a;
}
uint64_t siyuan_suanshu(Tree &root,uint8_t offset)
{
    uint64_t a;
    // -?
    if(root.zhongbie==offset+6)
    {
        if(root.list[0].zhongbie==offset||root.list[0].zhongbie==offset+1)
        {
            a=reg_i.new_();
            list_Siyuan.emplace_back(10,
                    1,0,
                    root.list[0].zhongbie-offset,root.list[0].num,
                    3,a);
        }
        else
        {
            a=siyuan_suanshu(root.list[0],offset);
            list_Siyuan.emplace_back(10, 1,0, 3,a, 3,a);
        }
        return a;
    }
    // ? +|-|*|/ ?
    if(root.list[0].zhongbie>=offset+2)
    {
        a=siyuan_suanshu(root.list[0],offset);
        if(root.list[1].zhongbie>=offset+2)
        {
            uint64_t b=siyuan_suanshu(root.list[1],offset);
            reg_i.delete_(b);
            list_Siyuan.emplace_back(root.zhongbie-offset+7, 
                    3,a,   
                    3,b,
                    3,a);
        }
        else
        {
            list_Siyuan.emplace_back(root.zhongbie-offset+7, 
                    3,a,
                    root.list[1].zhongbie-offset,root.list[1].num,
                3,a);
        }
    }
    else if(root.list[1].zhongbie>=offset+2)
    {
        a=siyuan_suanshu(root.list[1],offset);
        list_Siyuan.emplace_back(root.zhongbie-offset+7,
                root.list[0].zhongbie-offset,root.list[0].num,
                3,a,
                3,a);
    }
    else
    {
        a=reg_i.new_();
        list_Siyuan.emplace_back(root.zhongbie-offset+7, 
                root.list[0].zhongbie-offset,root.list[0].num,
                root.list[1].zhongbie-offset,root.list[1].num,
                3,a);
    }
    return a;
}
void siyuan_63(Tree &root)
{
    //整型 = 算术表达式
    if(root.list[0].zhongbie==0)
    {
        if(root.list[1].zhongbie==9||root.list[1].zhongbie==10)
        {
            list_Siyuan.emplace_back(8,
                    root.list[1].zhongbie-9,root.list[1].num, 
                    0,root.list[0].num);
        }
        else
        {
            uint64_t a=siyuan_suanshu(root.list[1],9);
            reg_i.delete_(a);
            list_Siyuan.emplace_back(8,
                    3,a,
                    0,root.list[0].num);
        }
    }
    else if(root.list[0].zhongbie==1)
    {
        list_Siyuan.emplace_back(8,
                root.list[1].zhongbie,root.list[1].num,
                0,root.list[0].num);
    }
    //布尔型 =布尔表达式
    else
    {
        if(root.list[1].zhongbie==18)
        {
            list_Siyuan.emplace_back(8,
                    0,root.list[1].num,
                    0,root.list[0].num);
        }
        else if(root.list[1].zhongbie==19||root.list[1].zhongbie==20)
        {
            list_Siyuan.emplace_back(8,
                    root.list[1].zhongbie-14,0,
                    0,root.list[0].num);
        }
        else
        {
            uint64_t a=siyuan_bool(root.list[1]);
            reg_b.delete_(a);
            list_Siyuan.emplace_back(8,
                    4,a,
                    0,root.list[0].num);
        }
    }
}
void siyuan_64(Tree &root)
{
    if(root.list[1].zhongbie==69&&root.list[2].zhongbie==69)
    {
        return;
    }
    // if ture
    if(root.list[0].zhongbie==19)
    {
        if(root.list[1].zhongbie==69)
        {
            return;
        }
        else
        {
            siyuan_62(root.list[1]);
        }
    }
    // if false
    else if(root.list[0].zhongbie==20)
    {
        if(root.list[2].zhongbie==69)
        {
            return;
        }
        else
        {
            siyuan_62(root.list[2]);
        }
    }
    else
    {
        uint8_t hz;
        uint64_t num;
        if(root.list[0].zhongbie==18)
        {
            hz=0;
            num=root.list[0].num;
        }
        else
        {
            hz=4;
            num=siyuan_bool(root.list[0]);
            reg_b.delete_(num);
        }
        if(root.list[1].zhongbie==69||root.list[2].zhongbie==69)
        {
            uint64_t lab=label;
            label++;
            list_Siyuan.emplace_back(5,hz,num,root.list[1].zhongbie==69?5:6,0,7,lab);
            siyuan_62(root.list[1].zhongbie==69?root.list[2]:root.list[1]);
            list_Siyuan.emplace_back(0,7,lab);
        }
        else
        {
            uint64_t lab=label;
            label+=2;
            list_Siyuan.emplace_back(5,hz,num,6,0,7,lab);
            siyuan_62(root.list[1]);
            list_Siyuan.emplace_back(1,7,lab+1);
            list_Siyuan.emplace_back(0,7,lab);
            siyuan_62(root.list[2]);
            list_Siyuan.emplace_back(0,7,lab+1);
        }
    }
}
void siyuan_65(Tree&root)
{
    if(root.list[1].zhongbie==19)
    {
        if(root.list[0].zhongbie!=69)
        {
            siyuan_62(root.list[0]);
        }
        return;
    }
    uint64_t lab;
    lab=label;
    label++;
    list_Siyuan.emplace_back(0,7,lab);
    if(root.list[0].zhongbie!=69)
    {
        siyuan_62(root.list[0]);
    }
    if(root.list[1].zhongbie==20)
    {
        list_Siyuan.emplace_back(1,7,lab);
    }
    else
    {
        uint8_t hz;
        uint64_t num;
        if(root.list[1].zhongbie==18)
        {
            hz=0;
            num=root.list[1].num;
        }
        else
        {
            hz=4;
            num=siyuan_bool(root.list[1]);
            reg_b.delete_(num);
        }
        list_Siyuan.emplace_back(5,hz,num,6,0,7,lab);
    }
}
void siyuan_66(Tree &root)
{
    if(root.list[0].zhongbie==20)
    {
        return;
    }
    //while true
    if(root.list[0].zhongbie==19)
    {
        uint64_t lab=label;
        label++;
        list_Siyuan.emplace_back(0,7,lab);
        if(root.list[1].zhongbie!=69)
        {
            siyuan_62(root.list[1]);
        }
        list_Siyuan.emplace_back(1,7,lab);
        return;
    }
    uint64_t lab=label;
    label+=2;
    uint8_t hz;
    uint64_t num;
    list_Siyuan.emplace_back(1,7,lab+1);
    list_Siyuan.emplace_back(0,7,lab);
    if(root.list[1].zhongbie!=69)
    {
        siyuan_62(root.list[1]);
    }
    list_Siyuan.emplace_back(0,7,lab+1);
    if(root.list[0].zhongbie==18)
    {
        hz=0;
        num=root.list[0].num;
    }
    else
    {
        hz=4;
        num=siyuan_bool(root.list[0]);
        reg_b.delete_(num);
    }
    list_Siyuan.emplace_back(5,hz,num,5,0,7,lab);
}
void siyuan_71(Tree& root)
{
    list_Siyuan.emplace_back(17,root.list[0].zhongbie,root.list[0].num);
}
inline void siyuan_62(Tree &root)
{
    if(root.zhongbie==61)
    {
        siyuan_61(root);
    }
    else if(root.zhongbie==63)
    {
        siyuan_63(root);
    }
    else if(root.zhongbie==64)
    {
        siyuan_64(root);
    }
    else if(root.zhongbie==65)
    {
        siyuan_65(root);
    }
    else if(root.zhongbie==66)
    {
        siyuan_66(root);
    }
    else if(root.zhongbie==71)
    {
        siyuan_71(root);
    }
    else
    {
        printf("四元式生成出错\n");
        exit(1);
    }
}
void siyuan_61(Tree &root)
{
    for(uint64_t i=0;i<root.list.size();i++)
    {
        siyuan_62(root.list[i]);
    }
}
