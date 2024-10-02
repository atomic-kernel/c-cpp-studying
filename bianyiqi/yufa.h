#include <vector>
void yufa();
#ifndef Tree_sdfsdfds
#define Tree_sdfsdfds
class Tree
{
public:
    Tree(uint8_t x):zhongbie(x)
    {
    }
    Tree(uint8_t x,uint64_t y):zhongbie(x),num(y)
    {
    }
    uint8_t zhongbie;
    uint64_t num;
    std::vector<Tree> list;
};
#endif
extern Tree root;
