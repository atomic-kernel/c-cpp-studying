#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>


#define WEI 6   //每位数字表示十进制多少位数字
#define NUM_WIDTH int64_t //每位数字用什么数据类型表示，这里为int64_t
#define FLOAT_WIDTH int64_t  //每个数字的浮点数用什么数据类型表示，这里为int64_t
#define LEN uint64_t    //每个数字的长度用什么数据类型表示，这里为uint64_t
#define NUM_WIDTH_MAX LONG_MAX
#define NUM_WIDTH_MIN LONG_MIN
#define FLOAT_WIDTH_MAX LONG_MAX
#define FLOAT_WIDTH_MIN LONG_MIN
#define LEN_MAX ULONG_MAX
#define MALLOC_MAX ULONG_MAX  //malloc可以接受的最大传参，即size_t的最大值


#define TO_STR(x) #x
#define TO_STR_V(x) TO_STR(x)
#define WEI_STR TO_STR_V(WEI)   //WEI_STR == "6"
#define LIANJIE(a,b) a##b
#define A_E_B(a,b) (LIANJIE(a##e,b))
typedef struct
{
    NUM_WIDTH *num;
    bool is_negative;
    FLOAT_WIDTH float_segment;
    uint8_t float_offset;
    LEN len;
} Num;
static inline void calc_baoliu(const int8_t fuhao,const Num *const a,const Num *const b,Num *const result,const LEN baoliu);
static inline void chu(Num const * const Num_a,Num const *const Num_b,const LEN baoliu,Num *const Num_result);
static inline void cheng(Num const *const Num_a,Num const *const Num_b,Num *const Num_result);
static inline void jian(Num const*const a,Num const*const b,Num *const result);
static inline void jia(Num const*const a,Num const*const b,Num *const result);
static inline NUM_WIDTH numcmp(const NUM_WIDTH *const a,const NUM_WIDTH *b,const LEN compare_number);
static inline void jinwei_zhengxiang(NUM_WIDTH *a,LEN len);
static inline void jinwei_fanxiang(NUM_WIDTH *a,LEN len);
static inline int8_t Num_compare_jueduizhi(Num const * const a,Num const * const b);
void scan(Num *a,Num *b,int8_t *fuhao,LEN *baoliu);
void print(Num const *const a);
void string_to_Num(char *string,Num *const a,int8_t const fuhao);
static inline void xi();
bool check(const char *);
int main()
{
    Num a={NULL},b={NULL},result={NULL};
    printf("-------------------------------高精度计算器2.0-------------------------------\n");
    printf("可进行大数/高精度的四则运算，支持输入负数\n");
    printf("计算两个600位的随机数相除，保留一亿位小数，耗时6.3秒(i5-8300H)\n");
    printf("计算两个一百万位的随机数相乘，精确输出，耗时14秒(i5-8300H)\n");
    printf("-----------------------------------------------------------------------------\n\n\n");
    int8_t fuhao;
    LEN baoliu;
    scan(&a,&b,&fuhao,&baoliu);
    printf("计算中。。。\n");
    clock_t start=clock();
    calc_baoliu(fuhao,&a,&b,&result,baoliu);
    clock_t stop=clock();
    putchar('\n');
    print(&a);
    putchar('\n');
    (fuhao==0&&(putchar('+'),1))||(fuhao==1&&(putchar('-'),1))||(fuhao==2&&(putchar('*'),1))||(fuhao==3&&(putchar('/'),1));
    putchar('\n');
    print(&b);
    printf("\n=\n");
    print(&result);
    putchar('\n');
    putchar('(');
    fuhao==3&&(printf("未四舍五入，"),1);
    printf("计算耗时：%ldms)\n",(stop-start)/(CLOCKS_PER_SEC/1000));
    free(a.num);
    free(b.num);
    free(result.num);
    return 0;
}
inline void calc_baoliu(const int8_t fuhao,const Num *const a,const Num *const b,Num *const result,const LEN baoliu)
{
    if(fuhao == 3)
    {
        chu(a,b,baoliu,result);
    }
    else if(fuhao==2)
    {
        cheng(a,b,result);
    }
    else if(fuhao==1||fuhao==0)
    {
        result->float_offset=0;
        bool temp_fuhao=fuhao^b->is_negative;
        if(a->is_negative==temp_fuhao)
        {
            result->is_negative=a->is_negative;
            jia(a,b,result);
        }
        else
        {
            int8_t temp;
            if((temp=Num_compare_jueduizhi(a,b))>0)
            {
                result->is_negative=a->is_negative;
                jian(a,b,result);
            }
            else if(temp<0)
            {
                result->is_negative=temp_fuhao;
                jian(b,a,result);
            }
            else
            {
                result->len=0;
                result->is_negative=0;
            }
        }
    }
}
inline int8_t Num_compare_jueduizhi(Num const * const a,Num const * const b)
{
    NUM_WIDTH temp;
    if(a->float_segment+a->len>b->float_segment+b->len)
    {
        return 1;
    }
    else if(a->float_segment+a->len<b->float_segment+b->len)
    {
        return -1;
    }
    else if(a->len>b->len)
    {
        return 1;
    }
    else if(a->len<b->len)
    {
        return -1;
    }
    else if((temp=numcmp(a->num,b->num,a->len))<0)
    {
        return -1;
    }
    else
    {
        return temp!=0;
    }
}
inline void jian(Num const*const Num_a,Num const*const Num_b,Num *const Num_result)
{
    if((__int128_t)Num_a->float_segment+Num_a->len>FLOAT_WIDTH_MAX||(__int128_t)Num_b->float_segment+Num_b->len>FLOAT_WIDTH_MAX)
    {
        printf("位数过多\n");
        exit(1);
    }
    FLOAT_WIDTH max=(Num_a->float_segment+(FLOAT_WIDTH)Num_a->len>Num_b->float_segment+(FLOAT_WIDTH)Num_b->len)?Num_a->float_segment+Num_a->len:Num_b->float_segment+Num_b->len;
    FLOAT_WIDTH min=Num_a->float_segment<Num_b->float_segment?Num_a->float_segment:Num_b->float_segment;
    if((__int128_t)max-min+1>MALLOC_MAX)
    {
        printf("位数过多，无法malloc\n");
        exit(1);
    }
    NUM_WIDTH *const result=Num_result->num=(NUM_WIDTH *)calloc(max-min+1,sizeof(NUM_WIDTH));
    if(result==NULL)
    {
        printf("内存不足\n");
        exit(1);
    }
    Num_result->float_segment=min;
    if((__int128_t)Num_a->len>MALLOC_MAX/sizeof(NUM_WIDTH))
    {
        printf("位数过多，无法malloc\n");
        exit(1);
    }
    memcpy(result+Num_a->float_segment-min,Num_a->num,Num_a->len*sizeof(NUM_WIDTH));
    NUM_WIDTH *const result_b_start=result+Num_b->float_segment-min;
    {
        NUM_WIDTH *temp_result_b=result_b_start;
        const NUM_WIDTH *temp_b_num=Num_b->num;
        const NUM_WIDTH *const temp_b_num_max=temp_b_num+Num_b->len;
        for(;temp_b_num!=temp_b_num_max;temp_result_b++,temp_b_num++)
        {
            *temp_result_b-=*temp_b_num;
        }
    }
    jinwei_fanxiang(result_b_start,(LEN)(max-Num_b->float_segment+1));
    LEN result_len=max-min+1;
    NUM_WIDTH * result_max=result+result_len-1;
    while(*result_max==0&&result_max!=result)
    {
        result_max--;
        result_len--;
    }
    if(result_max==result&&*result==0)
    {
        Num_result->is_negative=0;
        result_len--;
    }
    Num_result->len=result_len;
}
inline void jia(Num const*const Num_a,Num const*const Num_b,Num *const Num_result)
{
    if((__int128_t)Num_a->float_segment+Num_a->len>FLOAT_WIDTH_MAX||(__int128_t)Num_b->float_segment+Num_b->len>FLOAT_WIDTH_MAX)
    {
        printf("位数过多\n");
        exit(1);
    }
    FLOAT_WIDTH max=(Num_a->float_segment+(FLOAT_WIDTH)Num_a->len>Num_b->float_segment+(FLOAT_WIDTH)Num_b->len)?Num_a->float_segment+Num_a->len:Num_b->float_segment+Num_b->len;
    FLOAT_WIDTH min=Num_a->float_segment<Num_b->float_segment?Num_a->float_segment:Num_b->float_segment;
    if((__int128_t)max-min+1>MALLOC_MAX)
    {
        printf("位数过多，无法malloc\n");
        exit(1);
    }
    NUM_WIDTH *const result=Num_result->num=(NUM_WIDTH *)calloc(max-min+1,sizeof(NUM_WIDTH));
    if(result==NULL)
    {
        printf("内存不足\n");
        exit(1);
    }
    Num_result->float_segment=min;
    if(Num_a->len>=Num_b->len)
    {
        memcpy(result+Num_a->float_segment-min,Num_a->num,Num_a->len*sizeof(NUM_WIDTH));
        {
            NUM_WIDTH *temp_result_b=result+Num_b->float_segment-min;
            const NUM_WIDTH *temp_b_num=Num_b->num;
            const NUM_WIDTH *const temp_b_num_max=temp_b_num+Num_b->len;
            for(;temp_b_num!=temp_b_num_max;temp_result_b++,temp_b_num++)
            {
                *temp_result_b+=*temp_b_num;
            }
        }
    }
    else
    {
        memcpy(result+Num_b->float_segment-min,Num_b->num,Num_b->len*sizeof(NUM_WIDTH));
        {
            NUM_WIDTH *temp_result_a=result+Num_a->float_segment-min;
            const NUM_WIDTH *temp_a_num=Num_a->num;
            const NUM_WIDTH *const temp_a_num_max=temp_a_num+Num_a->len;
            for(;temp_a_num!=temp_a_num_max;temp_result_a++,temp_a_num++)
            {
                *temp_result_a+=*temp_a_num;
            }
        }
    }
    FLOAT_WIDTH min_max=Num_a->float_segment>min?Num_a->float_segment:Num_b->float_segment;
    jinwei_zhengxiang(result+min_max-min,(LEN)(max-min_max+1));
    if(*(result+max-min)==0)
    {
        Num_result->len=max-min;
    }
    else
    {
        Num_result->len=max-min+1;
    }
}
inline void chu(Num const * const Num_a,Num const *const Num_b,const LEN baoliu,Num *const Num_result)
{
    if(Num_a->len==0||Num_b->len==0)
    {
        Num_result->len=0;
        Num_result->is_negative=Num_b->len==0;
        return;
    }
    const NUM_WIDTH * const chushu=Num_b->num;
    const LEN len_chushu=Num_b->len;
    //求要算几次 准备好结果
    LEN const cishu=(baoliu-1)/WEI+2;
    if(cishu>MALLOC_MAX/sizeof(NUM_WIDTH))
    {
        printf("结果位数过多，无法malloc\n");
        exit(1);
    }
    NUM_WIDTH *const result=Num_result->num=(NUM_WIDTH *)malloc(sizeof(NUM_WIDTH)*cishu);
    if(result==NULL)
    {
        printf("内存不足！！\n");
        exit(1);
    }
    NUM_WIDTH *fake_result=result+cishu;
    
    LEN len_beichushu;
    if((__int128_t)len_chushu+cishu>(__int128_t)LEN_MAX)
    {
        printf("结果位数过多\n");
        exit(1);
    }
    //看第一个数字是否有效 同时准备好被除数
    int8_t flag=0;
    if( len_chushu > Num_a->len )
    {
        if(numcmp(chushu+len_chushu-Num_a->len ,Num_a->num ,Num_a->len)>=0)
        {
            len_beichushu=len_chushu+cishu;
        }
        else
        {
            len_beichushu=len_chushu+cishu-1;
            flag=1;
        }
    }
    else
    {
        NUM_WIDTH temp;
        if((temp=numcmp(chushu ,Num_a->num+Num_a->len-len_chushu ,len_chushu))>0)
        {
            len_beichushu=len_chushu+cishu;
        }
        else
        {
            len_beichushu=len_chushu+cishu-1;
            if(temp==0)
            {
                flag=2;
            }
            else
            {
                flag=1;
            }
        }
    }
    if(len_beichushu>MALLOC_MAX/sizeof(NUM_WIDTH))
    {
        printf("结果位数过多\n");
        exit(1);
    }
    NUM_WIDTH *const beichushu=(NUM_WIDTH *)malloc(sizeof(NUM_WIDTH)*len_beichushu);
    if(beichushu==NULL)
    {
        printf("内存不足！！\n");
        exit(1);
    }
    if(len_beichushu>Num_a->len)
    {
        memcpy(beichushu+len_beichushu-Num_a->len , Num_a->num , sizeof(NUM_WIDTH)*Num_a->len);
        memset(beichushu                                         , 0                  , sizeof(NUM_WIDTH)*(len_beichushu-Num_a->len));
    }
    else
    {
        memcpy(beichushu,Num_a->num,sizeof(NUM_WIDTH)*len_beichushu);
    }
    
    Num_result->is_negative=Num_a->is_negative^Num_b->is_negative;
    Num_result->float_segment=Num_a->len-Num_b->len+Num_a->float_segment-Num_b->float_segment-cishu+(uint8_t)(bool)flag;
    Num_result->len=cishu;
    if(Num_a->float_offset>=Num_b->float_offset)
    {
        Num_result->float_offset=Num_a->float_offset-Num_b->float_offset;
    }
    else
    {
        Num_result->float_offset=Num_a->float_offset+WEI-Num_b->float_offset;
        Num_result->float_segment--;
    }
    //计算
    NUM_WIDTH *beichushu_high=beichushu+len_beichushu-1;
    const NUM_WIDTH * const chushu_high=chushu+len_chushu-1;


    //计算除数位数<=2的情况
    if(len_chushu <=2)
    {
        if(len_chushu==1)
        {
            NUM_WIDTH chushu1=*chushu;
            if(flag)
            {
                //计算第一个数字
                fake_result--;
                if(flag==2)
                {
                    *fake_result=1;
                    *beichushu_high=0;
                }
                else
                {
                    *fake_result=*beichushu_high/chushu1;
                    *beichushu_high-=*fake_result*chushu1;
                }
            }
            while(fake_result!=result)
            {
                fake_result--;
                *fake_result=( *beichushu_high * (NUM_WIDTH)A_E_B(1,WEI) + *(beichushu_high-1) )
                    / chushu1;
                //对被除数进位
                *(--beichushu_high)-= *fake_result * chushu1;
                if(*beichushu_high<0)
                {
                    NUM_WIDTH temp=(-*beichushu_high-1)/(NUM_WIDTH)A_E_B(1,WEI)+1;
                    *beichushu_high+=temp*(NUM_WIDTH)A_E_B(1,WEI);
                    *(beichushu_high+1)-=temp;
                }
            }
        }
        else
        {
            NUM_WIDTH chushu1=*chushu_high * (NUM_WIDTH)A_E_B(1,WEI) + *chushu;
            NUM_WIDTH chushu1_high=*chushu_high;
            NUM_WIDTH chushu1_low=*chushu;
            if(flag)
            {
                //计算第一个数字
                fake_result--;
                if(flag==2)
                {
                    *fake_result=1;
                    *beichushu_high=0;
                    *(beichushu_high-1)=0;
                }
                else
                {
                    *fake_result=( *beichushu_high * (NUM_WIDTH)A_E_B(1,WEI) + *(beichushu_high-1) )
                    / chushu1;
                    *beichushu_high-= *fake_result * chushu1_high;
                    *(--beichushu_high)-= *fake_result * chushu1_low;
                    //对被除数进位
                    if(*beichushu_high<0)
                    {
                        NUM_WIDTH temp=(-*beichushu_high-1)/(NUM_WIDTH)A_E_B(1,WEI)+1;
                        *beichushu_high+=temp*(NUM_WIDTH)A_E_B(1,WEI);
                        *(beichushu_high+1)-=temp;
                    }
                    beichushu_high++;
                }
            }

            while(fake_result!=result)
            {
                fake_result--;
                *fake_result=( *beichushu_high * (NUM_WIDTH)A_E_B(1,WEI) * (NUM_WIDTH)A_E_B(1,WEI)
                        + *(beichushu_high-1) * (NUM_WIDTH)A_E_B(1,WEI)
                        + *(beichushu_high-2) )
                    / chushu1;
                *(--beichushu_high)-= *fake_result * chushu1_high;
                *(--beichushu_high)-= *fake_result * chushu1_low;
                //对被除数进位
                if(*beichushu_high<0)
                {
                    NUM_WIDTH temp=(-*beichushu_high-1)/(NUM_WIDTH)A_E_B(1,WEI)+1;
                    *beichushu_high+=temp*(NUM_WIDTH)A_E_B(1,WEI);
                    *(++beichushu_high)-=temp;
                }
                else
                {
                    beichushu_high++;
                }
                if(*beichushu_high<0)
                {
                    NUM_WIDTH temp=(-*beichushu_high-1)/(NUM_WIDTH)A_E_B(1,WEI)+1;
                    *beichushu_high+=temp*(NUM_WIDTH)A_E_B(1,WEI);
                    *(beichushu_high+1)-=temp;
                }
            }
        }
        free(beichushu);
        return;
    }


    //对于除数位数>=3的情况
    if(len_chushu>MALLOC_MAX/sizeof(NUM_WIDTH)-1)
    {
        printf("位数太多，无法malloc\n");
        exit(1);
    }
    NUM_WIDTH *temp_chushu=(NUM_WIDTH *)malloc((len_chushu+1)*sizeof(NUM_WIDTH));
    if(temp_chushu==NULL)
    {
        printf("内存不足\n");
        exit(1);
    }
    NUM_WIDTH chushu_zuigaoliangwei=*chushu_high * (NUM_WIDTH)A_E_B(1,WEI)+*(chushu_high-1);
    if(flag)
    {
        //计算第一个数字
        fake_result--;
        if(flag==2)
        {
            *fake_result=1;
            memset(beichushu_high-len_chushu+1,0,sizeof(NUM_WIDTH)*len_chushu);
        }
        else
        {
            NUM_WIDTH temp1=*beichushu_high * (NUM_WIDTH)A_E_B(1,WEI) * (NUM_WIDTH)A_E_B(1,WEI) 
                + *(beichushu_high-1) * (NUM_WIDTH)A_E_B(1,WEI)
                + *(beichushu_high-2)
                ,temp2=chushu_zuigaoliangwei * (NUM_WIDTH)A_E_B(1,WEI)
                + *(chushu_high-2)
                ;
            if(len_chushu==3)
            {
                *fake_result =temp1/temp2;
                goto label2;
            }
            if( temp1/(temp2+1) != (*fake_result=(temp1+1)/temp2)    )
            {
                memcpy(temp_chushu,chushu,sizeof(NUM_WIDTH)*len_chushu);
                for(LEN i=0;i<len_chushu;i++)
                {
                    temp_chushu[i]*=*fake_result;
                }
                jinwei_zhengxiang(temp_chushu,len_chushu);
                if(numcmp(temp_chushu,beichushu_high-len_chushu+1,len_chushu)>0)
                {
                    (*fake_result)--;
                }
            }
label2:;
            //对被除数进位
            NUM_WIDTH *temp_beichushu=beichushu_high+1;
            for(const NUM_WIDTH *temp_chushu=chushu_high+1 ; temp_chushu!=chushu ;)
            {
                temp_beichushu--;
                temp_chushu--;
                *temp_beichushu-=*temp_chushu * *fake_result;
            }
            jinwei_fanxiang(temp_beichushu,len_chushu);
        }
    }

    //计算后面的数字
    while(fake_result!=result)
    {
        fake_result--;
        NUM_WIDTH temp1=*beichushu_high * (NUM_WIDTH)A_E_B(1,WEI) * (NUM_WIDTH)A_E_B(1,WEI) 
            + *(beichushu_high-1) * (NUM_WIDTH)A_E_B(1,WEI)
            + *(beichushu_high-2)
            ;
        if( temp1/(chushu_zuigaoliangwei+1) != (*fake_result=(temp1+1)/chushu_zuigaoliangwei) )
        {
            temp_chushu[len_chushu]=0;
            memcpy(temp_chushu,chushu,sizeof(NUM_WIDTH)*len_chushu);
            for(LEN i=0;i<len_chushu;i++)
            {
                temp_chushu[i]*=*fake_result;
            }
            jinwei_zhengxiang(temp_chushu,len_chushu+1);
            if(numcmp(temp_chushu,beichushu_high-len_chushu,len_chushu+1)>0)
            {
                (*fake_result)--;
            }
        }
        //对被除数进位
        NUM_WIDTH *temp2=beichushu_high--;
        for(const NUM_WIDTH *temp3=chushu_high+1;temp3!=chushu;)
        {
            temp2--;
            temp3--;
            *temp2-=*fake_result * *temp3;
        }
        jinwei_fanxiang(temp2,len_chushu+1);
    }
    free(beichushu);
    free(temp_chushu);
}
inline void cheng(Num const *const Num_a,Num const *const Num_b,Num *const Num_result)
{
    if(Num_a->len==0||Num_b->len==0)
    {
        Num_result->len=0;
        Num_result->is_negative=0;
        return;
    }
    if((__int128_t)Num_a->len+Num_b->len>LEN_MAX)
    {
        printf("结果位数过多\n");
        exit(1);
    }
    LEN len_result=Num_a->len+Num_b->len;
    if(len_result>MALLOC_MAX/sizeof(NUM_WIDTH))
    {
        printf("结果位数过多，无法malloc\n");
        exit(1);
    }
    NUM_WIDTH* temp_result=Num_result->num=(NUM_WIDTH *)calloc(len_result,sizeof(NUM_WIDTH));
    if(temp_result==NULL)
    {
        printf("内存不足\n");
        exit(1);
    }

    Num_result->float_segment=Num_a->float_segment+Num_b->float_segment;
    Num_result->float_offset=Num_a->float_offset+Num_b->float_offset;
    if(Num_result->float_offset>=WEI)
    {
        Num_result->float_offset-=WEI;
        Num_result->float_segment++;
    }
    Num_result->is_negative=Num_a->is_negative^Num_b->is_negative;

    const NUM_WIDTH *const beichengshu=Num_a->num;
    const NUM_WIDTH *const beichengshu_max=beichengshu+Num_a->len;
    const NUM_WIDTH *chengshu=Num_b->num;
    const NUM_WIDTH *const chengshu_max=chengshu+Num_b->len;
    //计算
    NUM_WIDTH jishu=(NUM_WIDTH)A_E_B(1,WEI)-1; //循环次数：(NUM_WIDTH)A_E_B(1,WEI)
    if((__int128_t)Num_a->len+(NUM_WIDTH)A_E_B(1,WEI)>LEN_MAX)
    {
        printf("位数过多\n");
        exit(1);
    }
    const LEN len_beichengshu_jia_xunhuancishu=Num_a->len+(NUM_WIDTH)A_E_B(1,WEI);
    NUM_WIDTH *mark_result=temp_result;
    while(chengshu!=chengshu_max)
    {
        NUM_WIDTH *temp_temp_result=temp_result;
        const NUM_WIDTH *temp_beichengshu=beichengshu;
        while(temp_beichengshu!=beichengshu_max)
        {
            *temp_temp_result+=*temp_beichengshu**chengshu;
            temp_temp_result++;
            temp_beichengshu++;
        }
        //jinwei_zhengxiang(temp_result,len_beichengshu_jia1);
        temp_result++;
        chengshu++;
        if(jishu!=0)
        {
            jishu--;
        }
        else
        {
            jinwei_zhengxiang(mark_result,len_beichengshu_jia_xunhuancishu);
            mark_result=temp_result;
            jishu=(NUM_WIDTH)A_E_B(1,WEI)-1;
        }
    }
    jinwei_zhengxiang(mark_result,len_beichengshu_jia_xunhuancishu-1-jishu);
    if(*(mark_result+len_beichengshu_jia_xunhuancishu-2-jishu)==0)
    {
        Num_result->len=(len_result-1);
    }
    else
    {
        Num_result->len=len_result;
    }
}
inline NUM_WIDTH numcmp(const NUM_WIDTH *const a,const NUM_WIDTH *b,const LEN n)
{
    if(n==0)
    {
        return 0;
    }
    const NUM_WIDTH *temp_a=a+n-1;
    b+=n-1;
    while(temp_a!=a&&*temp_a==*b)
    {
        temp_a--;
        b--;
    }
    return *temp_a-*b;
}
inline void jinwei_zhengxiang(NUM_WIDTH *a,LEN n)
{
    while(n>1)
    {
        *(a+1)+=*a/(NUM_WIDTH)A_E_B(1,WEI);
        *(a++)%=(NUM_WIDTH)A_E_B(1,WEI);
        n--;
    }
}
inline void jinwei_fanxiang(NUM_WIDTH *a,LEN n)
{
    while(n>1)
    {
        if(*a<0)
        {
            NUM_WIDTH temp=(-*a-1)/(NUM_WIDTH)A_E_B(1,WEI)+1;
            *a+=temp*(NUM_WIDTH)A_E_B(1,WEI);
            *(++a)-=temp;
        }
        else
        {
            a++;
        }
        n--;
    }
}
void scan(Num *a,Num *b,int8_t *fuhao,LEN *baoliu)
{
    do
    {
        printf("请输入运算符(+ - * /)：\n");
    }
    while( (*fuhao=getchar())=='\n' || (getchar()!='\n'&&(xi(),1)) || !( (*fuhao=='+' && (*fuhao=0,1) ) || (*fuhao=='-' && (*fuhao=1,1) )|| (*fuhao=='*' && (*fuhao=2,1) )|| (*fuhao=='/' && (*fuhao=3,1) ) ) );
    if(*fuhao==3)
    {
        printf("请输入至少保留多少位有效数字：\n");
        while( ( (scanf("%llu",baoliu)!=1 || getchar()!='\n') && (xi(),1))|| ( *baoliu==0 && (printf("请输入大于0的数！\n"),1) ))
        {
            printf("请输入至少保留多少位有效数字：\n");
        }
    }
    LEN x;
    printf("请输入最大位数(不计小数点、负号)：\n");
    printf("比如若你要计算的两个数字为：-123(3位)和3.5685(5位)，则最大位数为5\n");
    while(scanf("%llu",&x)!=1||getchar()!='\n')
    {
        xi();
        printf("请输入最大位数：\n");
    }
    if(x>MALLOC_MAX-2)
    {
        printf("位数过多，无法malloc\n");
        exit(1);
    }
    char *temp_num=(char *)malloc(x+2);
    if(temp_num==NULL)
    {
        printf("内存不足！！\n");
        exit(1);
    }
    char temp;
    char temp_for_scanf[28];
    sprintf(temp_for_scanf,"%%%llu[0-9.]",x+1);
    do
    {
        printf("请输入第一个数字(请输入合法数字，可包含小数点、数字、正负号)：\n");
    }
    while( (((temp=getchar())=='-' && (a->is_negative=1,1)) || (temp=='+' && (a->is_negative=0,1)) || (a->is_negative=0,ungetc(temp,stdin),1) ) &&
            ( (scanf(temp_for_scanf,temp_num)==1 && check(temp_num) )? (xi(),0) : (xi(),1) ) );
    string_to_Num(temp_num,a,*fuhao);
    do
    {
        printf("请输入第二个数字：\n");
    }
    while( (((temp=getchar())=='-' && (b->is_negative=1,1)) || (temp=='+' && (a->is_negative=0,1)) || (b->is_negative=0,ungetc(temp,stdin),1) ) &&
            ( (scanf(temp_for_scanf,temp_num)==1 && check(temp_num) )? (xi(),0) : (xi(),1) ) );
    string_to_Num(temp_num,b,*fuhao);
    free(temp_num);
}
bool check(const char *a)
{
    size_t len=strlen(a);
    if(len<=0)
    {
        return 0;
    }
    if(a[len-1]>'9'||a[len-1]<'0')
    {
        return 0;
    }
    if(a[0]>'9'||a[0]<'0')
    {
        return 0;
    }
    size_t num_of_point=0;
    for(size_t i=1;i<len-1;i++)
    {
        if(a[i]>'9'||a[i]<'0')
        {
            if(a[i]=='.')
            {
                num_of_point++;
                if(num_of_point>1)
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
    }
    return 1;
}
void string_to_Num(char *string,Num *const a,int8_t const fuhao)
{
    while(*string=='0')
    {
        string++;
    }
    size_t len=strlen(string);
    if(len==0)
    {
label1:
        a->num=NULL;
        a->len=0;
        a->float_segment=0;
        a->float_offset=0;
        a->is_negative=0;
        return;
    }
    int64_t float_=0;
    while(string[len-1]=='0')
    {
        len--;
        float_++;
    }
    if(*string=='.')
    {
        string++;
        len--;
        float_=-len;
    }
    else if(string[len-1]=='.')
    {
        float_=0;
        len--;
        while(string[len-1]=='0')
        {
            len--;
            float_++;
        }
    }
    else
    {
        size_t i=1;
        while(i<len-1)
        {
            if(string[i]=='.')
            {
                len--;
                float_=i-len;
                memcpy(string+i,string+i+1,-float_);
                break;
            }
            i++;
        }
    }
    if(len==0)
    {
        goto label1;
    }

    NUM_WIDTH *num;
    if(fuhao<=1)
    {
        int8_t temp=(len+float_)%WEI;
        if(temp==0)
        {
            goto label2;
        }
        if(temp<0)
        {
            temp=WEI+temp;
        }
        if((size_t)temp>=len)
        {
            a->len=1;
            a->num=(NUM_WIDTH *)malloc(sizeof(NUM_WIDTH));
            char temp_for_num[temp+1];
            int8_t i=0;
            for(;i<(int8_t)len;i++)
            {
                temp_for_num[i]=string[i];
            }
            for(;i<temp;i++)
            {
                temp_for_num[i]='0';
            }
            temp_for_num[i]='\0';
            *a->num=0;
            sscanf(temp_for_num,"%lld",(long long *)a->num);
            a->len=1;
            if(float_>=0)
            {
                a->float_segment=0;
            }
            else
            {
                a->float_segment=(float_-temp+len)/WEI;
            }
            a->float_offset=0;
            return;
        }
        a->len=( (len-temp)%WEI==0?(len-temp)/WEI+1:(len-temp)/WEI+2 );
        a->num=(NUM_WIDTH *)malloc(sizeof(NUM_WIDTH)*a->len);
        num=a->num+a->len-1;
        char temp_for_sscanf[6];
        sprintf(temp_for_sscanf,"%%%dlld",(int)temp);
        *num=0;
        sscanf(string,temp_for_sscanf,(long long *)num);
        num--;
        string+=temp;
        len-=temp;
    }
    else
    {
label2:
        a->len=(len%WEI==0?len/WEI:len/WEI+1);
        a->num=(NUM_WIDTH *)malloc(sizeof(NUM_WIDTH)*a->len);
        num=a->num+a->len-1;
    }
    while(len/WEI!=0)
    {
        *num=0;
        sscanf(string,"%" WEI_STR "lld",(long long *)num);
        num--;
        string+=WEI;
        len-=WEI;
    }
    if(len!=0)
    {
        float_-=WEI-len;
        char temp[WEI+1];
        int8_t i=0;
        for(;i<(int8_t)len;i++)
        {
            temp[i]=string[i];
        }
        for(;i<WEI;i++)
        {
            temp[i]='0';
        }
        temp[i]='\0';
        *num=0;
        sscanf(temp,"%" WEI_STR "lld",(long long *)num);
    }
    if(float_%WEI!=0)
    {
        if(float_%WEI<0)
        {
            a->float_offset=WEI+float_%WEI;
            a->float_segment=float_/WEI-1;
        }
        else
        {
            a->float_offset=float_%WEI;
            a->float_segment=float_/WEI;
        }
    }
    else
    {
        a->float_segment=float_/WEI;
        a->float_offset=0;
    }
}
void print(Num const * const a)
{
    if(a->len==0 && a->is_negative==1)
    {
        if(a->is_negative==1)
        {
            printf("error\n");
            exit(1);
        }
        else
        {
            putchar('0');
            return;
        }
    }
    if(a->is_negative)
    {
        putchar('-');
    }
    if(a->float_segment<0)
    {
        if((LEN)-a->float_segment>a->len||((LEN)-a->float_segment==a->len&&a->float_offset==0))
        {
            printf("0.");
            for(LEN i=((LEN)-a->float_segment-a->len)*WEI-a->float_offset;i!=0;)
            {
                i--;
                putchar('0');
            }
            for(LEN i=a->len;i!=0;)
            {
                i--;
                printf("%." WEI_STR "lld",(long long)a->num[i]);
            }
            return;
        }
        LEN i=a->len-1;
        if((LEN)-a->float_segment==a->len)
        {
            char temp[WEI+1];
            sprintf(temp,"%lld",(long long)a->num[i]);
            if(strlen(temp)<=(size_t)(WEI-a->float_offset))
            {
                printf("0.");
                for(uint8_t i=WEI-a->float_offset-strlen(temp);i!=0;)
                {
                    i--;
                    putchar('0');
                }
                printf(temp);
            }
            else
            {
                for(uint8_t i=0;i<strlen(temp)-WEI+a->float_offset;i++)
                {
                    putchar(temp[i]);
                }
                putchar('.');
                printf(temp+strlen(temp)-WEI+a->float_offset);
            }
            goto label1;
        }
        printf("%lld",(long long)a->num[i]);
        i--;
        for(;i!=(LEN)(-a->float_segment-1);i--)
        {
            printf("%." WEI_STR "lld",(long long)a->num[i]);
        }
        char temp[WEI+1];
        sprintf(temp,"%." WEI_STR "lld",(long long)a->num[i]);
        for(uint8_t i=0;i<a->float_offset;i++)
        {
            putchar(temp[i]);
        }
        putchar('.');
        for(uint8_t i=a->float_offset;i<WEI;i++)
        {
            putchar(temp[i]);
        }
label1:
        for(;i!=0;)
        {
            i--;
            printf("%." WEI_STR "lld",(long long)a->num[i]);
        }
    }
    else
    {
        printf("%lld",(long long)a->num[a->len-1]);
        for(LEN i=a->len-1;i!=0;)
        {
            i--;
            printf("%." WEI_STR "lld",(long long)a->num[i]);
        }
        for(FLOAT_WIDTH i=0;i<a->float_segment;i++)
        {
            for(int8_t i=0;i<WEI;i++)
            {
                putchar('0');
            }
        }
        for(int8_t i=0;i<a->float_offset;i++)
        {
            putchar('0');
        }
    }
}
inline void xi()
{
    while(getchar()!='\n')
    {
    }
}
