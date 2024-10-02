#include <stdint.h>
#include <vector>
#include <string>
typedef struct Danci Danci;
typedef struct BiaoShiFu BiaoShiFu;
//单词流(所有单词)
extern std::vector<Danci> list_Danci;
//标识符列表
extern std::vector<BiaoShiFu> list_BiaoShiFu;
//整型常量列表
extern std::vector<int64_t> list_ConstInt;
//字符常量列表
extern std::vector<std::string> list_ConstStr;
#ifndef Danci_sdf
#define Danci_sdf
struct Danci
{
    uint8_t zhongbie;
    uint64_t num;
};
//标识符
//type==0:integer type==1:char type==2:bool type==3:程序名 type==4:不确定
struct BiaoShiFu
{
    uint8_t type;
    std::string zhi;
};
#endif
void cifa(FILE*);
