#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
int jisuan(char,char *,char *,short *,int);
int jisuan_b(char,char *,char *,short *,int,int);
int jisuan_part_simp(char **,int *);
int plus_c_f(char *,char *,int,int,int,int,short *,int);
int jian_c_f(char *,char *,int,int,int,int,short *,int);
void chen_c(char *,char *,int,int,short *,int);
void chu_c_f_b(char *,char *,int,int,int,int,short *,int,int);
int chu_part3_s(short *,int,unsigned long long *);
int chu_part3_t_s(short *,const char *,const int,unsigned long long *,short *);
void jinwei_c(char *,int,int);
void jinwei_c_s(short *,int,int);
void fan_jinwei_c(char *,int,int);
void fan_jinwei_c_s(short *,int,int);
void print(short *result,int n,int);
void print_b(short *result,int n,int,int);
void print_b_n(short *result,int n,int,int);
void fake_strcpy(char *,char *);
void fake_strncpy(char *,char *,int);
int fake_strncmp(char *,char *,int);
int fake_strncmp_s(short *,short *,int);
char xi();
inline char xi()
{
    while(getchar()!='\n')
    {
    }
    return 1;
}
int main()
{
    printf("-------------------------------高精度计算器-------------------------------\n");
    printf("可进行大数/高精度的四则运算，支持输入负数\n");
    putchar('\n');
    printf("计算两个两百位以上的随机数相除，保留一亿位小数，仅需20秒(i5-8300H) \n");
    printf("--------------------------------------------------------------------------\n\n\n");
    int n,float_,baoliu;
    char b=0,c=0,if_sishewuru;
    printf("请输入最大位数：\n");
    printf("即要计算的数字中含有数字的个数，可不计小数点和负号。\n");
    printf("比如若你要计算的两个数字为：-123(3位)和3.5685(5位)，则最大位数为5\n");
    while(scanf("%d",&n)!=1||getchar()!='\n')
    {
        xi();
        printf("请输入最大位数：\n");
    }

    do
    {
        printf("请输入运算符(+ - * /)：\n");
    }
    while( (b=getchar())=='\n' || (getchar()!='\n'&&(xi()||1)) || (b!='+'&&b!='-'&&b!='*'&&b!='/') );
    
    

    
    do
    {
        printf("是否选择保留多少位小数(如果选n则输出完全精确数字)？(y/n)\n");
    }
    while((c=getchar())=='\n' || (getchar()!='\n'&&(xi()||1)) || (c!='y'&&c!='n'));

    if(b=='/'&&c=='n')
    {
        printf("除法运算必须选择保留多少位小数！！\n");
        c='y';
    }
    


    if(c=='y')
    {
label1:
        printf("请输入要保留多少位小数：\n");
        while(scanf("%d",&baoliu)!=1||getchar()!='\n')
        {
            xi();
            printf("请输入要保留多少位小数：\n");
        }
        if(baoliu<0)
        {
            printf("保留位数必须>=0！！\n");
            goto label1;
        }
        do
        {
            printf("是否要四舍五入？(y/n)\n");
        }
        while( (if_sishewuru=getchar())=='\n' || (getchar()!='\n'&&(xi()||1)) || (if_sishewuru!='y'&&if_sishewuru!='n') );
    }
    char *x,*y;
    x=(char *)malloc((n+3)*sizeof(char));
    y=(char *)malloc((n+3)*sizeof(char));

    if(b!='/')
    {
        n=n*2+1;
    }
    else
    {
        n=n*2+1+baoliu;
    }


    short *result;
    result=(short *)calloc(n,sizeof(short));
    if( x==NULL || y==NULL || result==NULL )
    {
        free(result);
        free(x);
        free(y);
        printf("内存不足！！\n");
        return 1;
    }
    do
    {
        printf("请输入第一个数字(请输入合法数字，可包含负号，小数点和数字)：\n");
    }
    while( scanf("%[-0-9.]",x)!=1 && (xi()||1) );
    xi();
    do
    {
        printf("请输入第二个数字：\n");
    }
    while( scanf("%[-0-9.]",y)!=1 && (xi()||1) );
    xi();
    printf("计算中。。。\n\n");
    printf("%s\n%c\n%s\n=\n",x,b,y);
    if(c=='y')
    {
        float_=jisuan_b(b,x,y,result,n,baoliu);
    }
    else
    {
        float_=jisuan(b,x,y,result,n);
    }


    if(c=='n')
    {
        print(result,n,float_);
    }
    else if(if_sishewuru=='y')
    {
        print_b(result,n,float_,baoliu);
    }
    else
    {
        print_b_n(result,n,float_,baoliu);
    }
    if(c=='y')
    {
        printf("(保留%d位小数，",baoliu);
        if(if_sishewuru=='y')
        {
            printf("已四舍五入)\n");
        }
        else
        {
            printf("未四舍五入)\n");
        }
    }
    else
    {
        printf("(完全精确输出)\n");
    }

    free(result);
    free(x);
    free(y);
    return 0;
}

inline void fake_strncpy(char *x,char *y,int n)
{
    while((*x++=*y++)&&--n)
    {
        ;
    }
}
inline int fake_strncmp(char *x,char *y,int n)
{
    while(--n&&!(*x-*y))
    {
        x++;
        y++;
    }
    return *x-*y;
}
inline int fake_strncmp_s(short *x,short *y,int n)
{
    while(--n&&!(*x-*y))
    {
        x++;
        y++;
    }
    return *x-*y;
}

inline int chu_part3_s(short *x,int leny,unsigned long long *model)            //lenx<=19
{
    if(leny>4)
    {
        leny--;
        unsigned long long gongx=x[0]*1000000+x[1]*100000+x[2]*10000+x[3]*1000+x[4]*100+x[5]*10+x[6];
        int ix=7;
        for(int i=3;i<leny;i++)
        {
            if((gongx+1)/model[i]==gongx/(model[i]+1))
            {
                return (gongx+1)/model[i];
            }
            gongx=gongx*10+x[ix++];
        }
        return gongx/model[leny];
    }
    else
    {
        int gongx=x[0],ix=1;
        while(ix!=leny+3)
        {
            gongx=gongx*10+x[ix++];
        }
        return gongx/model[leny-1];
    }
}

inline int chu_part3_t_s(short *x,const char *y,const int leny,unsigned long long *model,short *temp)          //lenx>=20
{
    unsigned long long gongx=x[0]*1000000+x[1]*100000+x[2]*10000+x[3]*1000+x[4]*100+x[5]*10+x[6];
    int ix=7,gong;
    for(int i=3;i<16;i++)
    {
        if((gong=(gongx+1)/model[i])==gongx/(model[i]+1))
        {
            return gong;
        }
        gongx=gongx*10+x[ix++];
    }
label1:
    temp[0]=temp[1]=temp[2]=0;
    for(int i=0;i<leny;i++)
    {
        temp[i+3]=y[i]*gong;
    }

    jinwei_c_s(temp,leny+3,leny+3);


    if(fake_strncmp_s(temp,x,leny+3)<=0)
    {
        return gong;
    }
    else
    {
        gong--;
        goto label1;
    }
}

void chu_c_f_b(char *x,char *y,int lenx,int leny,int float_x,int float_y,short *result,int result_long,int baoliu)
{
    if(lenx==0)
    {
        return;
    }
    for(int i=0;i<leny;i++)
    {
        y[i]-='0';
    }


    int float_gong=result_long-baoliu-2+leny-lenx+float_x-float_y,float_first=float_gong;
    if(float_gong>=result_long)
    {
        return;
    }

    int fake_lenx=lenx-float_x+float_y+baoliu+1;
    short *p_fake_x,*p_first;
    if(fake_lenx==leny)
    {
        p_first=p_fake_x=(short *)malloc((fake_lenx+3)*sizeof(short));
        if(p_fake_x==NULL)
        {
            printf("内存不足！！\n");
            exit(1);
        }
        p_fake_x[0]=p_fake_x[1]=p_fake_x[2]=0;
        if(lenx>=fake_lenx)
        {
            for(int i=0;i<fake_lenx;i++)
            {
                p_fake_x[i+3]=x[i]-'0';
            }
        }
        else
        {
            for(int i=0;i<lenx;i++)
            {
                p_fake_x[i+3]=x[i]-'0';
            }
            memset(p_fake_x+lenx+3,0,(fake_lenx-lenx)*sizeof(short));
        }


        short *temp;
        temp=(short *)malloc((leny+3)*sizeof(short));
        if(temp==NULL)
        {
            printf("内存不足！！\n");
            exit(1);
        }
        unsigned long long model[16];
        memset(model,0,16*sizeof(unsigned long long));
        if(leny>=16)
        {
            for(int i1=0;i1<16;i1++)
            {
                for(int i2=0;i2<=i1;i2++)
                {
                    model[i1]+=y[i2]*(unsigned long long)pow(10,i1-i2);
                }
            }
        }
        else
        {
            for(int i1=0;i1<leny;i1++)
            {
                for(int i2=0;i2<=i1;i2++)
                {
                    model[i1]+=y[i2]*(unsigned long long)pow(10,i1-i2);
                }
            }
        }


        result[float_gong]=chu_part3_t_s(p_fake_x,y,leny,model,temp);
        free(temp);
    }
    else
    {

        p_first=p_fake_x=(short *)malloc((fake_lenx+2)*sizeof(short));
        if(p_fake_x==NULL)
        {
            printf("内存不足！！\n");
            exit(1);
        }


        p_fake_x[0]=0;
        *(++p_fake_x)=0;
        if(lenx>=fake_lenx)
        {
            for(int i=0;i<fake_lenx;i++)
            {
                p_fake_x[i+1]=x[i]-'0';
            }
        }
        else
        {
            for(int i=0;i<lenx;i++)
            {
                p_fake_x[i+1]=x[i]-'0';
            }
            memset(p_fake_x+lenx+1,0,(fake_lenx-lenx)*sizeof(short));
        }


        float_gong+=2;

        if(leny>=17)
        {
            short *temp;
            temp=(short *)malloc((leny+3)*sizeof(short));
            if(temp==NULL)
            {
                printf("内存不足！！\n");
                exit(1);
            }

            unsigned long long model[16];
            memset(model,0,16*sizeof(unsigned long long));
            for(int i1=0;i1<16;i1++)
            {
                for(int i2=0;i2<=i1;i2++)
                {
                    model[i1]+=y[i2]*(unsigned long long)pow(10,i1-i2);
                }
            }

            while(float_gong<result_long)
            {
                result[float_gong]=chu_part3_t_s(p_fake_x,y,leny,model,temp);
                for(int i=0;i<leny;i++)
                {
                    p_fake_x[i+3]-=y[i]*result[float_gong];
                }
                fan_jinwei_c_s(p_fake_x,leny+3,leny+3);
                p_fake_x+=3;
                float_gong+=3;
            }
            float_gong--;
            if(float_gong<result_long)
            {
                p_fake_x--;
                result[float_gong]=chu_part3_t_s(p_fake_x,y,leny,model,temp);
            }
            else
            {
                float_gong--;
                if(float_gong<result_long)
                {
                    p_fake_x-=2;
                    result[float_gong]=chu_part3_t_s(p_fake_x,y,leny,model,temp);
                }
                else
                {
                    float_gong--;
                }
            }
            free(temp);
        }
        else
        {
            unsigned long long model[leny];
            memset(model,0,leny*sizeof(unsigned long long));
            for(int i1=0;i1<leny;i1++)
            {
                for(int i2=0;i2<=i1;i2++)
                {
                    model[i1]+=y[i2]*(unsigned long long)pow(10,i1-i2);
                }
            }

            while(float_gong<result_long)
            {
                result[float_gong]=chu_part3_s(p_fake_x,leny,model);
                for(int i=0;i<leny;i++)
                {
                    p_fake_x[i+3]-=y[i]*result[float_gong];
                }
                fan_jinwei_c_s(p_fake_x,leny+3,leny+3);
                p_fake_x+=3;
                float_gong+=3;
            }
            float_gong--;
            if(float_gong<result_long)
            {
                p_fake_x--;
                result[float_gong]=chu_part3_s(p_fake_x,leny,model);
            }
            else
            {
                float_gong--;
                if(float_gong<result_long)
                {
                    p_fake_x-=2;
                    result[float_gong]=chu_part3_s(p_fake_x,leny,model);
                }
                else
                {
                    float_gong--;
                }
            }
        }
        jinwei_c_s(result,result_long,result_long-float_first);
    }
    free(p_first);
}

int jisuan_b(char a,char *x,char *y,short *result,int result_long,int baoliu)
{
    if(a!='/')
    {
        return jisuan(a,x,y,result,result_long);
    }
    int lenx=strlen(x),leny=strlen(y),float_x,float_y;
    if(x[0]=='-')
    {
        lenx--;
        x++;
        if(y[0]=='-')               //(-x)/(-y)
        {
            leny--;
            y++;
        }
        else                        //(-x)/y
        {
            result[0]='-';
        }
    }
    else
    {
        if(y[0]=='-')               //x/(-y)
        {
            leny--;
            y++;
            result[0]='-';
        }
    }
    float_x=jisuan_part_simp(&x,&lenx);
    float_y=jisuan_part_simp(&y,&leny);
    chu_c_f_b(x,y,lenx,leny,float_x,float_y,result,result_long,baoliu);
    return baoliu+1;
}


int jisuan_part_simp(char **x,int *lenx)
{
    int float_=0;
    while(*lenx>0&&**x=='0')
    {
        (*x)++;
        (*lenx)--;
    }
    if(*lenx==0)
    {
        return 0;
    }
    else if(**x=='.')
    {
        (*x)++;
        float_=--(*lenx);
        while(*lenx>0&&**x=='0')
        {
            (*x)++;
            (*lenx)--;
        }
        if(*lenx==0)
        {
            return 0;
        }
    }
    if(float_==0)
    {
        while((*x)[*lenx-1]=='0')
        {
            (*lenx)--;
            float_--;
        }
        if((*x)[*lenx-1]=='.')
        {
            (*lenx)--;
            float_=0;
            while((*x)[*lenx-1]=='0')
            {
                (*lenx)--;
                float_--;
            }
        }
        else
        {
            if(*lenx<=2)
            {
                return float_;
            }
            int i=*lenx-2;
            for(;i>0;i--)
            {
                if((*x)[i]=='.')
                {
                    break;
                }
            }
            if(i!=0)
            {
                fake_strncpy((*x)+i,(*x)+i+1,--(*lenx)-i);
                float_=*lenx-i;
            }
        }
    }
    else
    {
        while((*x)[*lenx-1]=='0')
        {
            (*lenx)--;
            float_--;
        }
    }
    return float_;
}


int jisuan(char a,char *x,char *y,short *result,int result_long)
{
    int lenx=strlen(x),leny=strlen(y),float_x,float_y;
    if(x[0]=='-')
    {
        x++;
        lenx--;
        float_x=jisuan_part_simp(&x,&lenx);
        if(y[0]=='-')             //(-x) (-y)
        {
            y++;
            leny--;
            float_y=jisuan_part_simp(&y,&leny);
            if(a=='+')
            {
                result[0]='-';
                return plus_c_f(x,y,lenx,leny,float_x,float_y,result,result_long);
            }
            else if(a=='-')
            {
                return jian_c_f(y,x,leny,lenx,float_y,float_x,result,result_long);
            }
        }
        else                    //(-x)  y
        {
            float_y=jisuan_part_simp(&y,&leny);
            if(a=='+')
            {
                return jian_c_f(y,x,leny,lenx,float_y,float_x,result,result_long);
            }
            else if(a=='-')
            {
                result[0]='-';
                return plus_c_f(x,y,lenx,leny,float_x,float_y,result,result_long);
            }
            else
            {
                result[0]='-';
            }
        }
    }
    else
    {
        float_x=jisuan_part_simp(&x,&lenx);
        if(y[0]=='-')             //x (-y)
        {
            y++;
            leny--;
            float_y=jisuan_part_simp(&y,&leny);
            if(a=='+')
            {
                return jian_c_f(x,y,lenx,leny,float_x,float_y,result,result_long);
            }
            else if(a=='-')
            {
                return plus_c_f(x,y,lenx,leny,float_x,float_y,result,result_long);
            }
            else
            {
                result[0]='-';
            }
        }
        else                 // x y
        {
            float_y=jisuan_part_simp(&y,&leny);
            if(a=='+')
            {
                return plus_c_f(x,y,lenx,leny,float_x,float_y,result,result_long);
            }
            else if(a=='-')
            {
                return jian_c_f(x,y,lenx,leny,float_x,float_y,result,result_long);
            }
        }
    }
    chen_c(x,y,lenx,leny,result,result_long);
    return float_x+float_y;
}

void chen_c(char *x,char *y,int lenx, int leny,short *result,int result_long)
{
    if(lenx>=leny)
    {
        for(int iy=leny-1;iy>=0;iy--)
        {
            int gong=result_long-leny+iy-lenx+1;
            for(int ix=0,chenshu=y[iy]-'0';ix<lenx;ix++)
            {
                result[gong++]+=chenshu*(x[ix]-'0');
            }
            jinwei_c_s(result,gong,lenx+1);
        }
    }
    else
    {
        for(int ix=lenx-1;ix>=0;ix--)
        {
            int gong=result_long-lenx+ix-leny+1;
            for(int iy=0,chenshu=x[ix]-'0';iy<leny;iy++)
            {
                result[gong++]+=chenshu*(y[iy]-'0');
            }
            jinwei_c_s(result,gong,leny+1);
        }
    }
}


inline void fake_strcpy(char *a,char *b)
{
    while( (*a++=*b++)!=0 )
    {
        ;
    }
}

int plus_c_f(char *x,char *y,int lenx,int leny,int float_x,int float_y,short *result,int result_long)
{
    if(float_x>=float_y)
    {
        for(int i=lenx-1,gong=result_long-1;i>=0;i--,gong--)
        {
            result[gong]=x[i]-'0';
        }
        for(int i=leny-1,gong=result_long-float_x+float_y-1;i>=0;i--,gong--)
        {
            result[gong]+=y[i]-'0';
        }
        jinwei_c_s(result,result_long,((lenx-float_x)>(leny-float_y)?lenx-float_x:leny-float_y)+float_x+1);
        return float_x;
    }
    else
    {
        for(int i=leny-1,gong=result_long-1;i>=0;i--,gong--)
        {
            result[gong]=y[i]-'0';
        }
        for(int i=lenx-1,gong=result_long-float_y+float_x-1;i>=0;i--,gong--)
        {
            result[gong]+=x[i]-'0';
        }
        jinwei_c_s(result,result_long,((lenx-float_x)>(leny-float_y)?lenx-float_x:leny-float_y)+float_y+1);
        return float_y;
    }
}


int jian_c_f(char *x,char *y,int lenx,int leny,const int float_x,const int float_y,short *result,int result_long)
{
    if(leny==0)
    {
        goto labelx;
    }
    if(lenx==0)
    {
        goto labely;
    }
    if(lenx-float_x>leny-float_y)
    {
labelx:
        if(float_x>=float_y)
        {
            for(int i=lenx-1,gong=result_long-1;i>=0;i--,gong--)
            {
                result[gong]=x[i]-'0';
            }
            for(int i=leny-1,gong=result_long-float_x+float_y-1;i>=0;i--,gong--)
            {
                result[gong]-=y[i]-'0';
            }
            fan_jinwei_c_s(result,result_long,lenx);
        }
        else
        {
            for(int i=leny-1,gong=result_long-1;i>=0;i--,gong--)
            {
                result[gong]='0'-y[i];
            }
            for(int i=lenx-1,gong=result_long-float_y+float_x-1;i>=0;i--,gong--)
            {
                result[gong]+=x[i]-'0';
            }
            fan_jinwei_c_s(result,result_long,lenx-float_x+float_y);
        }
    }
    else if(lenx-float_x<leny-float_y)
    {
labely:
        if(float_y>=float_x)
        {
            for(int i=leny-1,gong=result_long-1;i>=0;i--,gong--)
            {
                result[gong]=y[i]-'0';
            }
            for(int i=lenx-1,gong=result_long-float_y+float_x-1;i>=0;i--,gong--)
            {
                result[gong]-=x[i]-'0';
            }
            fan_jinwei_c_s(result,result_long,leny);
        }
        else
        {
            for(int i=lenx-1,gong=result_long-1;i>=0;i--,gong--)
            {
                result[gong]='0'-x[i];
            }
            for(int i=leny-1,gong=result_long-float_x+float_y-1;i>=0;i--,gong--)
            {
                result[gong]+=y[i]-'0';
            }
            fan_jinwei_c_s(result,result_long,leny-float_y+float_x);
        }
        result[0]='-';
    }
    else
    {
        if(lenx>=leny)
        {
            int gong;
            if((gong=strncmp(x,y,leny))>0)
            {
                goto labelx;
            }
            else if(gong<0)
            {
                goto labely;
            }
            else if(lenx>leny)
            {
                goto labelx;
            }
        }
        else if(lenx<leny)
        {
            int gong;
            if((gong=strncmp(x,y,lenx))>0)
            {
                goto labelx;
            }
            else if(gong<0)
            {
                goto labely;
            }
            else
            {
                goto labely;
            }
        }
    }
    return float_x>float_y?float_x:float_y;
}

void jinwei_c(char *a,int a_long,int n)
{
    for(int i=a_long-1,gong=i-n+1;i>gong;i--)
    {
        a[i-1]+=a[i]/10;
        a[i]%=10;
    }
}

void jinwei_c_s(short *a,int a_long,int n)
{
    for(int i=a_long-1,gong=i-n+1;i>gong;i--)
    {
        a[i-1]+=a[i]/10;
        a[i]%=10;
    }
}

void fan_jinwei_c(char *a,int a_long,int n)
{
    for(int i=a_long-1,gong=a_long-n,gong2;i>gong;i--)
    {
        if(a[i]<0)
        {
            gong2=(-a[i]-1)/10+1;
            a[i-1]-=gong2;
            a[i]+=10*gong2;
        }
    }
}
void fan_jinwei_c_s(short *a,int a_long,int n)
{
    for(int i=a_long-1,gong=a_long-n,gong2;i>gong;i--)
    {
        if(a[i]<0)
        {
            gong2=(-a[i]-1)/10+1;
            a[i-1]-=gong2;
            a[i]+=10*gong2;
        }
    }
}

void print_b_n(short *result,int result_long,int float_,int baoliu)
{
    if(float_>baoliu)
    {
        print(result,result_long-float_+baoliu,baoliu);
    }
    else
    {
        print(result,result_long,float_);
    }
}


void print_b(short *result,int result_long,int float_,int baoliu)
{
    if(float_>baoliu)
    {
        int gong=result_long-float_+baoliu;
        if(result[gong]>=5)
        {
            result[gong-1]+=1;
        }
        if(result[gong-1]==10)
        {
            result[gong-1]=0;
            result[gong-2]+=1;
            if(result[gong-2]==10)
            {
                int i=0;
                while(result[++i]==0)
                {
                    ;
                }
                jinwei_c_s(result,gong-1,gong-i);
            }
        }
        print(result,gong,baoliu);
    }
    else
    {
        print(result,result_long,float_);
    }
}


void print(short *result,int result_long,int float_)
{
    int i=1;
    for(;i<result_long;i++)
    {
        if(result[i]!=0)
        {
            break;
        }
    }


    if(i!=result_long)
    {
        if(result[0]=='-')
        {
            putchar('-');
        }
        int gong=result_long;
        while(result[--gong]==0)
        {
            ;
        }
        if(result_long-gong<=float_)
        {
            if(float_>=result_long-i)
            {
                printf("0.");
                for(int i1=float_-result_long+i;i1>0;i1--)
                {
                    putchar('0');
                }
                do
                {
                    printf("%d",result[i++]);
                }
                while(i<=gong);
            }
            else
            {
                do
                {
                    printf("%d",result[i++]);
                }
                while(result_long-i>float_);
                putchar('.');
                do
                {
                    printf("%d",result[i++]);
                }
                while(i<=gong);
            }
        }
        else
        {
            do
            {
                printf("%d",result[i++]);
            }
            while(i<=gong);
            for(int i=result_long-gong-float_-1;i>0;i--)
            {
                putchar('0');
            }
        }
        putchar('\n');
    }
    else
    {
        printf("0\n");
    }
}
