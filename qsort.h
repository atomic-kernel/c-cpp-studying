/*    快速排序模板函数       */


/*    使用方法:
 *    1. qsort(  迭代器begin或数组开头  ,  元素数量n    [,比较函数]  );
 *    2. qsort(  迭代器begin或数组开头  ,  迭代器end或指向数组末尾的下一个元素的指针  [,比较函数]  );
 *
 *    适用于:
 *    int x[100] , int x[a]  , int x[a][100]  ,  类数组
 *    vector deque list
 *    
 *    不适用于:
 *    int x[a][b]
 *
 *    其中a,b为变量
 *  */


/*
 *    比较函数:
 *    int compare(type a,type b)
 *    {
 *      ...
 *    }
 *    返回值<0:a在b左边
 *    返回值>0:a在b右边
 *    返回值=0:a与b位置关系不确定
 * */


/*
 *    示例:
 *    #include "qsort.h"
 *    #include <vector>
 *   
 *    int comp( [const] int [&] a, [const] int [&] b)
 *    {
 *        return a-b;
 *    }
 *
 *
 *    int main()
 *    {
 *        int x[]={3,2,1};
 *        qsort( x , 3 [,comp] );
 *        qsort( x, x+3 [,comp] );
 *
 *        std::vector<int> y {3,2,1};
 *        qsort( y.[r]begin() , y.size() [,comp] );
 *        qsort( y.[r]begin() , y.[r]end() [,comp] );
 *
 *        return 0;
 *    }
 * */


/*
 *  性能:
 *  经测试,略快于<stdlib.h>中的qsort(),原因不明
 *  对100000个位于 0-9999 之间的数字排序,此函数平均耗时0.008s,<stdlib.h>中的qsort()平均耗时0.009s
 *  i5-8300h
 * */


/*    注意事项:
 *    swap函数与std::swap重名，请不要使用using namespace std; 
 *    或者将修改源码将swap换个名字
 * */
#include <stdint.h>

template<typename T>
void qsort(const T begin,const long unsigned int n);

template<typename T>
void qsort(const T begin,T fake_end);

template<typename T>
void qsort(const T begin,const T end,const long unsigned int n);

template<typename T,typename T2,typename T3>
void qsort(const T begin,const long unsigned int n,int (*comp)(T2 a,T3 b));

template<typename T,typename T2,typename T3>
void qsort(const T begin,T fake_end,int (*comp)(T2 a,T3 b));

template<typename T,typename T2,typename T3>
void qsort(const T begin,const T end,const long unsigned int n,int (*comp)(T2 a,T3 b));
#ifndef qsort_sdfsdf
#define qsort_sdfsdf
/*template<typename T>
void swap(T &a,T &b)
{
    uint8_t *temp1=(uint8_t *)&a,*temp2=(uint8_t *)&b;
    long unsigned int i=sizeof(T);
    while(i>=sizeof(uint64_t))
    {
        *(uint64_t *)temp1^=*(uint64_t *)temp2;
        *(uint64_t *)temp2^=*(uint64_t *)temp1;
        *(uint64_t *)temp1^=*(uint64_t *)temp2;
        temp1+=sizeof(uint64_t);
        temp2+=sizeof(uint64_t);
        i-=sizeof(uint64_t);
    }
    while(i>=sizeof(uint32_t))
    {
        *(uint32_t *)temp1^=*(uint32_t *)temp2;
        *(uint32_t *)temp2^=*(uint32_t *)temp1;
        *(uint32_t *)temp1^=*(uint32_t *)temp2;
        temp1+=sizeof(uint32_t);
        temp2+=sizeof(uint32_t);
        i-=sizeof(uint32_t);
    }
    while(i>=sizeof(uint16_t))
    {
        *(uint16_t *)temp1^=*(uint16_t *)temp2;
        *(uint16_t *)temp2^=*(uint16_t *)temp1;
        *(uint16_t *)temp1^=*(uint16_t *)temp2;
        temp1+=sizeof(uint16_t);
        temp2+=sizeof(uint16_t);
        i-=sizeof(uint16_t);
    }
    while(i!=0)
    {
        *temp1^=*temp2;
        *temp2^=*temp1;
        *temp1^=*temp2;
        temp1++;
        temp2++;
        i--;
    }
}*/
template<typename T>
void swap(T &a,T &b)
{
    uint8_t *temp1=(uint8_t *)&a,*temp2=(uint8_t *)&b;
    long unsigned int i=sizeof(T);
    while(i>=sizeof(uint64_t))
    {
        uint64_t temp=*(uint64_t *)temp1;
        *(uint64_t *)temp1=*(uint64_t *)temp2;
        *(uint64_t *)temp2=temp;
        temp1+=sizeof(uint64_t);
        temp2+=sizeof(uint64_t);
        i-=sizeof(uint64_t);
    }
    while(i>=sizeof(uint32_t))
    {
        uint32_t temp=*(uint32_t *)temp1;
        *(uint32_t *)temp1=*(uint32_t *)temp2;
        *(uint32_t *)temp2=temp;
        temp1+=sizeof(uint32_t);
        temp2+=sizeof(uint32_t);
        i-=sizeof(uint32_t);
    }
    while(i>=sizeof(uint16_t))
    {
        uint16_t temp=*(uint16_t *)temp1;
        *(uint16_t *)temp1=*(uint16_t *)temp2;
        *(uint16_t *)temp2=temp;
        temp1+=sizeof(uint16_t);
        temp2+=sizeof(uint16_t);
        i-=sizeof(uint16_t);
    }
    while(i!=0)
    {
        uint8_t temp=*temp1;
        *temp1=*temp2;
        *temp2=temp;
        temp1++;
        temp2++;
        i--;
    }
}

template<typename T>
void qsort(const T begin,const long unsigned int n)
{
    if(n>2)
    {
        T mid=begin;
        for(long unsigned int i=0,max=n/2;i<max;i++)
        {
            mid++;
        }
        T end=mid;
        for(long unsigned int i=0,max=n-n/2-1;i<max;i++)
        {
            end++;
        }
        T i=begin,j=end;
        long unsigned int i_c=1,j_c=n;
        do
        {
            if(!(*mid<*i))
            {
                i++;
                i_c++;
            }
            else
            {
                break;
            }
        }
        while(i_c!=n);
        do
        {
            if(!(*j<*mid))
            {
                j--;
                j_c--;
            }
            else
            {
                break;
            }
        }
        while(j_c!=1);
        if(i_c<j_c)
        {
            swap(*i,*j);
            i++;
            i_c++;
            j--;
            j_c--;
            while(i_c<=j_c)
            {
                while(!(*mid<*i))
                {
                    i++;
                    i_c++;
                }
                while(!(*j<*mid))
                {
                    j--;
                    j_c--;
                }
                if(i_c>j_c)
                {
                    break;
                }
                else
                {
                    swap(*i,*j);
                    i++;
                    i_c++;
                    j--;
                    j_c--;
                }
            }
        }
        else if(i_c==j_c)
        {
            if(i_c==n)
            {
                swap(*mid,*end);
                j--;
                j_c--;
            }
            else
            {
                swap(*mid,*begin);
                i++;
                i_c++;
            }
        }
        qsort(begin,j,j_c);
        qsort(i,end,n-i_c+1);
    }
    else if(2==n)
    {
        T end=begin;
        end++;
        if(*end<*begin)
        {
            swap(*begin,*end);
        }
    }
}
template<typename T>
void qsort(const T begin,T end)
{
    end--;                        //fake end to true end
    long unsigned int n=1;
    T mid;
    {
        T i=begin,j=end;
        while(i!=j)
        {
            i++;
            n++;
            if(i!=j)
            {
                j--;
                n++;
            }
            else
            {
                break;
            }
        }
        mid=i;
    }
    if(n>2)
    {
        T i=begin,j=end;
        long unsigned int i_c=1,j_c=n;
        do
        {
            if(!(*mid<*i))
            {
                i++;
                i_c++;
            }
            else
            {
                break;
            }
        }
        while(i_c!=n);
        do
        {
            if(!(*j<*mid))
            {
                j--;
                j_c--;
            }
            else
            {
                break;
            }
        }
        while(j_c!=1);
        if(i_c<j_c)
        {
            swap(*i,*j);
            i++;
            i_c++;
            j--;
            j_c--;
            while(i_c<=j_c)
            {
                while(!(*mid<*i))
                {
                    i++;
                    i_c++;
                }
                while(!(*j<*mid))
                {
                    j--;
                    j_c--;
                }
                if(i_c>j_c)
                {
                    break;
                }
                else
                {
                    swap(*i,*j);
                    i++;
                    i_c++;
                    j--;
                    j_c--;
                }
            }
        }
        else if(i_c==j_c)
        {
            if(i_c==n)
            {
                swap(*mid,*end);
                j--;
                j_c--;
            }
            else
            {
                swap(*mid,*begin);
                i++;
                i_c++;
            }
        }
        qsort(begin,j,j_c);
        qsort(i,end,n-i_c+1);
    }
    else if(2==n)
    {
        if(*end<*begin)
        {
            swap(*begin,*end);
        }
    }
}
template<typename T>
void qsort(const T begin,const T end,const long unsigned int n)
{
    if(n>2)
    {
        T mid=end;
        for(long unsigned int i=0,max=n-n/2-1;i<max;i++)
        {
            mid--;
        }
        T i=begin,j=end;
        long unsigned int i_c=1,j_c=n;
        do
        {
            if(!(*mid<*i))
            {
                i++;
                i_c++;
            }
            else
            {
                break;
            }
        }
        while(i_c!=n);
        do
        {
            if(!(*j<*mid))
            {
                j--;
                j_c--;
            }
            else
            {
                break;
            }
        }
        while(j_c!=1);
        if(i_c<j_c)
        {
            swap(*i,*j);
            i++;
            i_c++;
            j--;
            j_c--;
            while(i_c<=j_c)
            {
                while(!(*mid<*i))
                {
                    i++;
                    i_c++;
                }
                while(!(*j<*mid))
                {
                    j--;
                    j_c--;
                }
                if(i_c>j_c)
                {
                    break;
                }
                else
                {
                    swap(*i,*j);
                    i++;
                    i_c++;
                    j--;
                    j_c--;
                }
            }
        }
        else if(i_c==j_c)
        {
            if(i_c==n)
            {
                swap(*mid,*end);
                j--;
                j_c--;
            }
            else
            {
                swap(*mid,*begin);
                i++;
                i_c++;
            }
        }
        qsort(begin,j,j_c);
        qsort(i,end,n-i_c+1);
    }
    else if(2==n)
    {
        if(*end<*begin)
        {
            swap(*begin,*end);
        }
    }
}

template<typename T,typename T2,typename T3>
void qsort(const T begin,const long unsigned int n,int (*comp)(T2 a,T3 b))
{
    if(n>2)
    {
        T mid=begin;
        for(long unsigned int i=0,max=n/2;i<max;i++)
        {
            mid++;
        }
        T end=mid;
        for(long unsigned int i=0,max=n-n/2-1;i<max;i++)
        {
            end++;
        }
        T i=begin,j=end;
        long unsigned int i_c=1,j_c=n;
        do
        {
            if(comp(*i,*mid)<=0)
            {
                i++;
                i_c++;
            }
            else
            {
                break;
            }
        }
        while(i_c!=n);
        do
        {
            if(comp(*mid,*j)<=0)
            {
                j--;
                j_c--;
            }
            else
            {
                break;
            }
        }
        while(j_c!=1);
        if(i_c<j_c)
        {
            swap(*i,*j);
            i++;
            i_c++;
            j--;
            j_c--;
            while(i_c<=j_c)
            {
                while(comp(*i,*mid)<=0)
                {
                    i++;
                    i_c++;
                }
                while(comp(*mid,*j)<=0)
                {
                    j--;
                    j_c--;
                }
                if(i_c>j_c)
                {
                    break;
                }
                else
                {
                    swap(*i,*j);
                    i++;
                    i_c++;
                    j--;
                    j_c--;
                }
            }
        }
        else if(i_c==j_c)
        {
            if(i_c==n)
            {
                swap(*mid,*end);
                j--;
                j_c--;
            }
            else
            {
                swap(*mid,*begin);
                i++;
                i_c++;
            }
        }
        qsort(begin,j,j_c,comp);
        qsort(i,end,n-i_c+1,comp);
    }
    else if(2==n)
    {
        T end=begin;
        end++;
        if(comp(*begin,*end)>0)
        {
            swap(*begin,*end);
        }
    }
}
template<typename T,typename T2,typename T3>
void qsort(const T begin,T end,int (*comp)(T2 a,T3 b))
{
    end--;                        //fake end to true end
    long unsigned int n=1;
    T mid;
    {
        T i=begin,j=end;
        while(i!=j)
        {
            i++;
            n++;
            if(i!=j)
            {
                j--;
                n++;
            }
            else
            {
                break;
            }
        }
        mid=i;
    }
    if(n>2)
    {
        T i=begin,j=end;
        long unsigned int i_c=1,j_c=n;
        do
        {
            if(comp(*i,*mid)<=0)
            {
                i++;
                i_c++;
            }
            else
            {
                break;
            }
        }
        while(i_c!=n);
        do
        {
            if(comp(*mid,*j)<=0)
            {
                j--;
                j_c--;
            }
            else
            {
                break;
            }
        }
        while(j_c!=1);
        if(i_c<j_c)
        {
            swap(*i,*j);
            i++;
            i_c++;
            j--;
            j_c--;
            while(i_c<=j_c)
            {
                while(comp(*i,*mid)<=0)
                {
                    i++;
                    i_c++;
                }
                while(comp(*mid,*j)<=0)
                {
                    j--;
                    j_c--;
                }
                if(i_c>j_c)
                {
                    break;
                }
                else
                {
                    swap(*i,*j);
                    i++;
                    i_c++;
                    j--;
                    j_c--;
                }
            }
        }
        else if(i_c==j_c)
        {
            if(i_c==n)
            {
                swap(*mid,*end);
                j--;
                j_c--;
            }
            else
            {
                swap(*mid,*begin);
                i++;
                i_c++;
            }
        }
        qsort(begin,j,j_c,comp);
        qsort(i,end,n-i_c+1,comp);
    }
    else if(2==n)
    {
        if(comp(*begin,*end)>0)
        {
            swap(*begin,*end);
        }
    }
}
template<typename T,typename T2,typename T3>
void qsort(const T begin,const T end,const long unsigned int n,int (*comp)(T2 a,T3 b))
{
    if(n>2)
    {
        T mid=end;
        for(long unsigned int i=0,max=n-n/2-1;i<max;i++)
        {
            mid--;
        }
        T i=begin,j=end;
        long unsigned int i_c=1,j_c=n;
        do
        {
            if(comp(*i,*mid)<=0)
            {
                i++;
                i_c++;
            }
            else
            {
                break;
            }
        }
        while(i_c!=n);
        do
        {
            if(comp(*mid,*j)<=0)
            {
                j--;
                j_c--;
            }
            else
            {
                break;
            }
        }
        while(j_c!=1);
        if(i_c<j_c)
        {
            swap(*i,*j);
            i++;
            i_c++;
            j--;
            j_c--;
            while(i_c<=j_c)
            {
                while(comp(*i,*mid)<=0)
                {
                    i++;
                    i_c++;
                }
                while(comp(*mid,*j)<=0)
                {
                    j--;
                    j_c--;
                }
                if(i_c>j_c)
                {
                    break;
                }
                else
                {
                    swap(*i,*j);
                    i++;
                    i_c++;
                    j--;
                    j_c--;
                }
            }
        }
        else if(i_c==j_c)
        {
            if(i_c==n)
            {
                swap(*mid,*end);
                j--;
                j_c--;
            }
            else
            {
                swap(*mid,*begin);
                i++;
                i_c++;
            }
        }
        qsort(begin,j,j_c,comp);
        qsort(i,end,n-i_c+1,comp);
    }
    else if(2==n)
    {
        if(comp(*begin,*end)>0)
        {
            swap(*begin,*end);
        }
    }
}
#endif
