#include <vector>
#include <stdint.h>
#include "cifa.h"
#include "yufa.h"
void siyuan();
typedef struct Siyuan Siyuan;
#ifndef siyuan_sdgfsdf
#define siyuan_sdgfsdf
struct Siyuan
{
    //0:label 1:jmp 2:jl 3:jle 4:jne 5:je 6:jg 7:jge 8:mov 9:add 10:sub 11:mul 12:div 14:&& 15:|| 16:! 17:print
    uint8_t zhongbie;
    //zhongbie:    0: 变量   1：整型常量    2：字符常量    3：临时整型变量    4：临时布尔变量    5:true    6:false
    uint8_t zb1;
    uint64_t num1;
    uint8_t zb2;
    uint64_t num2;
    uint8_t zb3;
    uint64_t num3;
};
#endif
extern std::vector<Siyuan> list_Siyuan;
extern uint64_t num_tmp_i;
extern uint64_t num_tmp_b;
extern uint64_t label;
