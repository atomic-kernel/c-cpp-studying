#pragma once
#include <vector>
#include <string>
#include <stdint.h>

template<typename T>
static inline void unique_split(const char *sentence, T (*const hash_fun)(const char *), std::vector<std::string> &word_list, std::vector<T> &hash_list);
static inline void split(const char *sentence, std::vector<std::string> &word_list);

template<typename T>
inline void unique_split(const char *sentence, T (*const hash_fun)(const char *), std::vector<std::string> &word_list, std::vector<T> &hash_list)
{
    word_list.clear();
    hash_list.clear();
    if(sentence==NULL)
    {
        return;
    }
    char temp_c=*sentence;
    while (!('a'<=temp_c&&temp_c<='z') || ('A'<=temp_c&&temp_c<='Z'))
    {
        if (temp_c=='\0')
        {
            return;
        }
        temp_c=*(++sentence);
    }
    std::string temp_word;
    temp_word.reserve(50);
    std::vector<std::vector<uint64_t>> word_list_position_hash_table(128);
    goto label1;
    while (1)
    {
        do
        {
            if (temp_c=='\0')
            {
                return;
            }
            temp_c=*(++sentence);
        }
        while (!(('a'<=temp_c&&temp_c<='z') || ('A'<=temp_c&&temp_c<='Z')));
        temp_word.clear();
label1:
        do
        {
            temp_word+=temp_c;
            temp_c=*(++sentence);
        }
        while (('a'<=temp_c&&temp_c<='z') || ('A'<=temp_c&&temp_c<='Z'));
        if (temp_word.size()>1)
        {
            const T hash_result=hash_fun(temp_word.c_str());
            const uint64_t temp_x=hash_result%128;
            for (uint64_t temp_y=word_list_position_hash_table[temp_x].size(); temp_y!=0;)
            {
                temp_y--;
                if (temp_word==word_list[word_list_position_hash_table[temp_x][temp_y]])
                {
                    goto label_continue;
                }
            }
            word_list_position_hash_table[temp_x].emplace_back(word_list.size());
            word_list.emplace_back(temp_word);
            hash_list.emplace_back(hash_result);
        }
label_continue:
        ;
    }
}

inline void split(const char *sentence, std::vector<std::string> &word_list)
{
    word_list.clear();
    if(sentence==NULL)
    {
        return;
    }
    uint64_t i=0;
    char temp=*sentence;
    while (1)
    {
        if (('a'<=temp&&temp<='z') || ('A'<=temp&&temp<='Z'))
        {
//label1:
            word_list.emplace_back();
            word_list[i].reserve(20);
label3:
            word_list[i]+=temp;
            while (1)
            {
                temp=*(++sentence);
                if (('a'<=temp&&temp<='z') || ('A'<=temp&&temp<='Z'))
                {
                    word_list[i]+=temp;
                }
                else
                {
                    break;
                }
            }
            if (word_list[i].size()==1)
            {
                goto label4;
                while (1)
                {
                    temp=*(++sentence);
                    if (('a'<=temp&&temp<='z') || ('A'<=temp&&temp<='Z'))
                    {
                        word_list[i].clear();
                        goto label3;
                    }
label4:
                    if (temp=='\0')
                    {
                        word_list.pop_back();
                        return;
                    }
                }
            }
            i++;
            goto label2;
        }
        else
        {
label2:
            if(temp=='\0')
            {
                return;
            }
        }
        temp=*(++sentence);
    }
}
/*void split(const char *sentence, std::vector<std::string> &word_list)
{
    word_list.clear();
    if(sentence==NULL)
    {
        return;
    }
    uint64_t i=0;
    char temp=*sentence;
    while (1)
    {
        if (('a'<=temp&&temp<='z') || ('A'<=temp&&temp<='Z'))
        {
label1:
            word_list.emplace_back();
            word_list[i].reserve(20);
label3:
            word_list[i]+=temp;
            while (1)
            {
                temp=*(++sentence);
                if (('a'<=temp&&temp<='z') || ('A'<=temp&&temp<='Z'))
                {
                    word_list[i]+=temp;
                    continue;
                }
                if(temp=='\'')
                {
                    temp=*(++sentence);
                    if (('a'<=temp&&temp<='z') || ('A'<=temp&&temp<='Z'))
                    {
                        if ((temp=='s' || temp=='m' || temp=='S' || temp=='M') && !(('a'<=sentence[1]&&sentence[1]<='z') || ('A'<=sentence[1]&&sentence[1]<='Z')))
                        {
                            word_list[i]+='\'';
                            word_list[i]+=temp;
                            temp=*(++sentence);
                        }
                        else if (word_list[i].size()==1)
                        {
                            word_list[i].clear();
                            goto label3;
                        }
                        else
                        {
                            i++;
                            goto label1;
                        }
                    }
                }
                break;
            }
            if (word_list[i].size()==1)
            {
                goto label4;
                while (1)
                {
                    temp=*(++sentence);
                    if (('a'<=temp&&temp<='z') || ('A'<=temp&&temp<='Z'))
                    {
                        word_list[i].clear();
                        goto label3;
                    }
label4:
                    if (temp=='\0')
                    {
                        word_list.pop_back();
                        return;
                    }
                }
            }
            i++;
            goto label2;
        }
        else
        {
label2:
            if(temp=='\0')
            {
                return;
            }
        }
        temp=*(++sentence);
    }
}*/
