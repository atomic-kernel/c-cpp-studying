#pragma once
#include "my_data_struct.h"
#include "split.h"
#include "hash.h"
#include <string.h>
#include <string>
#include <algorithm>

namespace search {
    //快速求交集
    static inline void intersection(std::vector<uint64_t> &result, const std::vector<std::vector<uint64_t> const*> &uint64_t_vector_p_vector);
}

static inline void word_search_author(std::vector<uint64_t> &result, char *const search_content);
static inline void name_search_author(std::vector<uint64_t> &result, const char * const search_content);
static inline void name_search_author_strstr(std::vector<uint64_t> &result, const char * const search_content);
static inline void word_search_essay(std::vector<uint64_t> &result, char *const search_content);
static inline void title_search_essay(std::vector<uint64_t> &result, const char * const search_content);
static inline void title_search_essay_strstr(std::vector<uint64_t> &result, const char * const search_content);

inline void name_search_author_strstr(std::vector<uint64_t> &result, const char * const search_content)
{
    result.clear();
    result.reserve(author_list.size()/strlen(search_content)/10);
    for (uint64_t i=author_list.size(); i!=0;)
    {
        i--;
        if (strstr(author_list[i].name.c_str(), search_content)!=NULL)
        {
            result.emplace_back(i);
        }
    }
}

inline void title_search_essay_strstr(std::vector<uint64_t> &result, const char * const search_content)
{
    result.clear();
    result.reserve(essay_list.size()/strlen(search_content)/10);
    for (uint64_t i=essay_list.size(); i!=0;)
    {
        i--;
        if (strstr(essay_list[i].title.c_str(), search_content)!=NULL)
        {
            result.emplace_back(i);
            continue;
        }
        if (strstr(essay_list[i].booktitle.c_str(), search_content)!=NULL)
        {
            result.emplace_back(i);
            continue;
        }
    }
}

inline void title_search_essay(std::vector<uint64_t> &result, const char * const search_content)
{
    result.clear();
    result.reserve(10);
    const uint64_t temp_x=md5_ELFhash_32(search_content)%essay_list_title_booktitle_position_hash_table.size();
    for(uint64_t temp_y=essay_list_title_booktitle_position_hash_table[temp_x].size(); temp_y!=0;)
    {
        temp_y--;
        if (strcmp(essay_list[essay_list_title_booktitle_position_hash_table[temp_x][temp_y]].title.c_str(),search_content)==0||strcmp(essay_list[essay_list_title_booktitle_position_hash_table[temp_x][temp_y]].booktitle.c_str(), search_content)==0)
        {
            for(uint64_t i=result.size(); i!=0;)
            {
                i--;
                if (result[i]==essay_list_title_booktitle_position_hash_table[temp_x][temp_y])
                {
                    goto label_continue;
                }
            }
            result.emplace_back(essay_list_title_booktitle_position_hash_table[temp_x][temp_y]);
        }
label_continue:
        ;
    }
}

inline void name_search_author(std::vector<uint64_t> &result, const char *const search_content)
{
    result.clear();
    result.reserve(10);
    const uint64_t temp_x=md5_ELFhash_32(search_content)%author_list_name_position_hash_table.size();
    for (uint64_t temp_y=author_list_name_position_hash_table[temp_x].size(); temp_y!=0;)
    {
        temp_y--;
        if (strcmp(author_list[author_list_name_position_hash_table[temp_x][temp_y]].name.c_str(), search_content)==0)
        {
            result.emplace_back(author_list_name_position_hash_table[temp_x][temp_y]);
        }
    }
}

inline void word_search_author(std::vector<uint64_t> &result, char *const search_content)
{
    using namespace search;
    result.clear();
    {
        char *src=search_content;
        while (1)
        {
            if ('A'<=*src&&*src<='Z')
            {
                *src|=0b100000;
            }
            else if (*src=='\0')
            {
                break;
            }
            src++;
        }
    }
    std::vector<std::vector<uint64_t> const *> author_position_list_p_list;
    {
        std::vector<std::string> word_content_temp_list;
        word_content_temp_list.reserve(32);
        std::vector<uint32_t> word_content_hash_result_temp_list;
        word_content_hash_result_temp_list.reserve(32);
        unique_split(search_content, md5_ELFhash_32, word_content_temp_list, word_content_hash_result_temp_list);
        author_position_list_p_list.reserve(word_content_temp_list.size());
        for (uint64_t i=word_content_temp_list.size(); i!=0;)
        {
            i--;
            //transform(word_content_temp_list[i].begin(), word_content_temp_list[i].end(), word_content_temp_list[i].begin(), ::tolower);
            const uint64_t temp_x=word_content_hash_result_temp_list[i]%author_name_word_list_content_position_hash_table.size();
            for (uint64_t temp_y=author_name_word_list_content_position_hash_table[temp_x].size(); temp_y!=0;)
            {
                temp_y--;
                if (author_name_word_list[author_name_word_list_content_position_hash_table[temp_x][temp_y]].content==word_content_temp_list[i])
                {
                    author_position_list_p_list.emplace_back(&author_name_word_list[author_name_word_list_content_position_hash_table[temp_x][temp_y]].position_list);
                    goto label_continue;
                }
            }
            //此单词未找到
            return ;
label_continue:
            ;
        }
    }
    intersection(result, author_position_list_p_list);
}

inline void word_search_essay(std::vector<uint64_t> &result, char *const search_content)
{
    using namespace search;
    result.clear();
    {
        char *src=search_content;
        while (1)
        {
            if ('A'<=*src&&*src<='Z')
            {
                *src|=0b100000;
            }
            else if (*src=='\0')
            {
                break;
            }
            src++;
        }
    }
    std::vector<std::vector<uint64_t> const *> essay_position_list_p_list;
    {
        std::vector<std::string> word_content_temp_list;
        word_content_temp_list.reserve(32);
        std::vector<uint32_t> word_content_hash_result_temp_list;
        word_content_hash_result_temp_list.reserve(32);
        unique_split(search_content, md5_ELFhash_32, word_content_temp_list, word_content_hash_result_temp_list);
        essay_position_list_p_list.reserve(word_content_temp_list.size());
        for (uint64_t i=word_content_temp_list.size(); i!=0;)
        {
            i--;
            //transform(word_content_temp_list[i].begin(), word_content_temp_list[i].end(), word_content_temp_list[i].begin(), ::tolower);
            const uint64_t temp_x=word_content_hash_result_temp_list[i]%essay_title_booktitle_word_list_content_position_hash_table.size();
            for (uint64_t temp_y=essay_title_booktitle_word_list_content_position_hash_table[temp_x].size(); temp_y!=0;)
            {
                temp_y--;
                if (essay_title_booktitle_word_list[essay_title_booktitle_word_list_content_position_hash_table[temp_x][temp_y]].content==word_content_temp_list[i])
                {
                    essay_position_list_p_list.emplace_back(&essay_title_booktitle_word_list[essay_title_booktitle_word_list_content_position_hash_table[temp_x][temp_y]].position_list);
                    goto label_continue;
                }
            }
            //此单词未找到
            return ;
label_continue:
            ;
        }
    }
    intersection(result, essay_position_list_p_list);
}
inline void search::intersection(std::vector<uint64_t> &result, const std::vector<std::vector<uint64_t> const*> &uint64_t_vector_p_vector)
{
    if (uint64_t_vector_p_vector.size()<=1)
    {
        if (uint64_t_vector_p_vector.size()==1)
        {
            result=*uint64_t_vector_p_vector[0];
        }
        return;
    }
    const uint64_t end_point=uint64_t_vector_p_vector.size()-1;
    //求最短的数组，以及它包含的元素数量
    uint64_t min_size=uint64_t_vector_p_vector[end_point]->size(), min_position=end_point;
    for (uint64_t i=end_point; i!=0;)
    {
        i--;
        if (uint64_t_vector_p_vector[i]->size()<min_size)
        {
            min_size=uint64_t_vector_p_vector[i]->size();
            min_position=i;
        }
    }
    //标记
    std::vector<uint64_t> mark(min_size, 0);
    {
        //用最小的那一组初始化哈希表
        const uint64_t min_list_position_hash_table_size=min_size<<1;
        std::vector<std::vector<uint64_t>> min_list_position_hash_table(min_list_position_hash_table_size);
        for (uint64_t i=0; i<min_size; i++)
        {
            min_list_position_hash_table[(*uint64_t_vector_p_vector[min_position])[i]%min_list_position_hash_table_size].emplace_back(i);
        }

        //用剩余的组在哈希表中查找
        for (uint64_t i=end_point; i!=min_position; i--)
        {
            for (uint64_t i2=uint64_t_vector_p_vector[i]->size(); i2!=0;)
            {
                i2--;
                const uint64_t temp_x=(*uint64_t_vector_p_vector[i])[i2]%min_list_position_hash_table_size;
                for (uint64_t temp_y=min_list_position_hash_table[temp_x].size(); temp_y!=0;)
                {
                    temp_y--;
                    if ((*uint64_t_vector_p_vector[min_position])[min_list_position_hash_table[temp_x][temp_y]]==(*uint64_t_vector_p_vector[i])[i2])
                    {
                        mark[min_list_position_hash_table[temp_x][temp_y]]++;
                    }
                }
            }
        }
        for (uint64_t i=min_position; i!=0;)
        {
            i--;
            for (uint64_t i2=uint64_t_vector_p_vector[i]->size(); i2!=0;)
            {
                i2--;
                const uint64_t temp_x=(*uint64_t_vector_p_vector[i])[i2]%min_list_position_hash_table_size;
                for (uint64_t temp_y=min_list_position_hash_table[temp_x].size(); temp_y!=0;)
                {
                    temp_y--;
                    if ((*uint64_t_vector_p_vector[min_position])[min_list_position_hash_table[temp_x][temp_y]]==(*uint64_t_vector_p_vector[i])[i2])
                    {
                        mark[min_list_position_hash_table[temp_x][temp_y]]++;
                    }
                }
            }
        }
    }
    result.reserve(min_size);
    for (uint64_t i=min_size; i!=0;)
    {
        i--;
        if (mark[i]==end_point)
        {
            result.emplace_back((*uint64_t_vector_p_vector[min_position])[i]);
        }
    }
}
