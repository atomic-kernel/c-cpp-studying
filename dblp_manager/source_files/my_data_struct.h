#pragma once

#include <string>
#include <vector>
#include <stdint.h>

//文章类型
enum
{
    PHDTHESIS,
    INCOLLECTION,
    MASTERSTHESIS,
    //WWW,
    INPROCEEDINGS,
    PROCEEDINGS,
    BOOK,
    ARTICLE
};

//月份
enum
{
    JANUARY=1,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
};

//文献
struct Essay
{
    //文献类型
    uint8_t type;
    //作者
    std::vector<uint64_t> author_position_list;
    //文献名/书名
    std::string booktitle;
    std::string title;
    //出版社
    std::string journal;
    //版号
    std::string volume;
    //出版年月。低8位表示月
    uint32_t year_month=0;
};

//作者
struct Author
{
    //作者名(不可重复)
    std::string name;
    //著作
    std::vector<uint64_t> essay_position_list;
    //合作者
    std::vector<uint64_t> partner_position_list;
};

//词语(拆词后的结果)
struct Word
{
    std::string content;
    std::vector<uint64_t> position_list;
};

extern std::vector<Essay> essay_list;
extern std::vector<std::vector<uint64_t>> essay_list_title_booktitle_position_hash_table;
extern std::vector<Word> essay_title_booktitle_word_list;
extern std::vector<std::vector<uint64_t>> essay_title_booktitle_word_list_content_position_hash_table;
extern std::vector<Author> author_list;
extern std::vector<std::vector<uint64_t>> author_list_name_position_hash_table;
extern std::vector<Word> author_name_word_list;
extern std::vector<std::vector<uint64_t>> author_name_word_list_content_position_hash_table;
extern std::vector<uint64_t> author_list_top_100_position_list;
