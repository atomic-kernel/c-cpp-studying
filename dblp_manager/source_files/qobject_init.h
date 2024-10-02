#ifndef QOBJECT_INIT_H
#define QOBJECT_INIT_H

#include <QObject>
#include <QDebug>

class QObject_init : public QObject
{
    Q_OBJECT
public:
    explicit QObject_init(QObject *parent = nullptr):QObject(parent){}

    inline void init(FILE *);
    bool kill=false;
private:
    uint64_t get_essay_num(FILE *);
    inline void remove_item(FILE* fp, const char * const item_title);
    inline void get_item_content(FILE* fp, const char * const item_title, char * buffer);
    inline void get_month_from_content(const char * content, uint32_t &year_month);
    inline void get_tag_content(FILE* fp,char *buffer);
    //更新作者列表，建立作者词索引
    inline void update_author_list_author_index(const uint64_t essay_list_position, const std::vector<std::string>& author_name_temp_list);
    //inline void* update_author_list_author_index_for_pthread(void *);
    //建立标题词索引
    inline void update_essay_index(const uint64_t essay_list_position);
    //inline void* update_essay_index_for_pthread(void *);
    inline void get_top_100();
signals:
    void wancheng();
    void error(const char *);
};

inline uint64_t QObject_init::get_essay_num(FILE *fp)
{
    uint64_t total=0;
    rewind(fp);
    char item_title[5000];
    while (1)
    {
        get_tag_content(fp, item_title);
        if (strcmp(item_title, "dblp")==0)
        {
            break;
        }
        if(kill)
        {
            return 0;
        }
    }
    while (1)
    {
        get_tag_content(fp, item_title);
        char temp_type[300];
        sscanf(item_title, "%s", temp_type);
        if(strcmp(temp_type, "inproceedings")==0 || strcmp(temp_type,"article")==0 || strcmp(temp_type,"phdthesis")==0 || strcmp(temp_type,"incollection")==0 || strcmp(temp_type,"proceedings")==0 || strcmp(temp_type,"book")==0 || strcmp(temp_type,"mastersthesis")==0)
        {
            total++;
            remove_item(fp, temp_type);
        }
        else if(strcmp(temp_type,"www")==0)
        {
            //ignore www
            remove_item(fp, temp_type);
        }
        else if(strcmp(temp_type,"/dblp")==0)
        {
            break;
        }
        else if(!kill)
        {
            emit error("xml解析出错：未知的文献类型");
            kill=true;
        }
        if(kill)
        {
            return 0;
        }
    }
    return total;
}

#include "my_data_struct.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <string.h>
//#include <pthread.h>
#include "hash.h"
#include "split.h"
//#include "my_error.h"
#include <list>

inline void QObject_init::init(FILE *fp)
{
    const uint64_t essay_num=get_essay_num(fp);
    if(kill)
    {
        return;
    }
    //printf("total%lu\n",essay_num);
    essay_list.reserve(essay_num);
    essay_list_title_booktitle_position_hash_table.resize(essay_num*5);
    author_list.reserve(essay_num*3/4);
    author_list_name_position_hash_table.resize(essay_num*2);
    essay_title_booktitle_word_list.reserve(essay_num*4/35);
    essay_title_booktitle_word_list_content_position_hash_table.resize(essay_num*167/500);
    author_name_word_list.reserve(essay_num/6);
    author_name_word_list_content_position_hash_table.resize(essay_num/2);

    //pthread_t pt1, pt2;
    uint64_t essay_list_position=0;
    std::vector<std::string> author_name_temp_list;
    author_name_temp_list.reserve(20);

    rewind(fp);
    char temp_content[5000];
    while (1)
    {
        get_tag_content(fp, temp_content);
        if (strcmp(temp_content, "dblp")==0)
        {
            break;
        }
        if(kill)
        {
            return;
        }
    }
    while (1)
    {
        get_tag_content(fp, temp_content);
        char temp_type[30];
        sscanf(temp_content, "%s", temp_type);
        if(strcmp(temp_type,"inproceedings")==0)
        {
            essay_list.emplace_back(INPROCEEDINGS);
        }
        else if(strcmp(temp_type,"www")==0)
        {
            //ignore www
            remove_item(fp, "www");
            if(kill)
            {
                return;
            }
            continue;
        }
        else if(strcmp(temp_type,"article")==0)
        {
            essay_list.emplace_back(ARTICLE);
        }
        else if(strcmp(temp_type,"phdthesis")==0)
        {
            essay_list.emplace_back(PHDTHESIS);
        }
        else if(strcmp(temp_type,"incollection")==0)
        {
            essay_list.emplace_back(INCOLLECTION);
        }
        else if(strcmp(temp_type,"proceedings")==0)
        {
            essay_list.emplace_back(PROCEEDINGS);
        }
        else if(strcmp(temp_type,"book")==0)
        {
            essay_list.emplace_back(BOOK);
        }
        else if(strcmp(temp_type,"mastersthesis")==0)
        {
            essay_list.emplace_back(MASTERSTHESIS);
        }
        else if(strcmp(temp_type,"/dblp")==0)
        {
            if(kill)
            {
                return;
            }
            break;
        }
        else
        {
            if(!kill)
            {
                emit error("xml解析出错：未知的文献类型");
            }
            return;
        }
        while (1)
        {
            get_tag_content(fp, temp_content);
            if(strcmp(temp_content, "author") == 0)
            {
                get_item_content(fp, "author", temp_content);
                // chuli(temp_content);
                if(temp_content[0]!='\0')
                {
                    author_name_temp_list.emplace_back(temp_content);
                }
            }
            else if(strcmp(temp_content, "title") == 0)
            {
                get_item_content(fp, "title", temp_content);
                // chuli(temp_content);
                if(temp_content[0]!='\0')
                {
                    if(!essay_list[essay_list_position].title.empty())
                    {
                        if(!kill)
                        {
                            emit error("xml解析出错：一个文献中有多个title");
                        }
                        return;
                    }
                    essay_list[essay_list_position].title = temp_content;
                }
            }
            else if(strcmp(temp_content, "booktitle") == 0)
            {
                get_item_content(fp, "booktitle", temp_content);
                // chuli(temp_content);
                if(temp_content[0]!='\0')
                {
                    if(!essay_list[essay_list_position].booktitle.empty())
                    {
                        if(!kill)
                        {
                            emit error("xml解析出错：一个文献中有多个booktitle");
                        }
                        return;
                    }
                    essay_list[essay_list_position].booktitle = temp_content;
                }
            }
            else if(strcmp(temp_content, "journal") == 0)
            {
                get_item_content(fp, "journal", temp_content);
                // chuli(temp_content);
                if(temp_content[0]!='\0')
                {
                    if(!essay_list[essay_list_position].journal.empty())
                    {
                        if(!kill)
                        {
                            emit error("more than one journal in one essay!");
                        }
                        return;
                    }
                    essay_list[essay_list_position].journal = temp_content;
                }
            }
            else if(strcmp(temp_content, "year") == 0)
            {
                uint16_t temp;
                char temp2[8];
                if (fscanf(fp, "%hu", &temp)!=1 || (fgets(temp2, 8, fp), strcmp(temp2, "</year>")!=0))
                {
                    if(!kill)
                    {
                        emit error("error in get year");
                    }
                    return;
                }
                if(essay_list[essay_list_position].year_month>>8==0)
                {
                    *((uint16_t *)((char *)&essay_list[essay_list_position].year_month+1)) = temp;
                }
            }
            else if(strcmp(temp_content, "month") == 0)
            {
                get_item_content(fp, "month", temp_content);
                if ((uint8_t)essay_list[essay_list_position].year_month==0)
                {
                    get_month_from_content(temp_content, essay_list[essay_list_position].year_month);
                }
            }
            else if(strcmp(temp_content, "volume")==0)
            {
                get_item_content(fp, "volume", temp_content);
                // chuli(temp_content);
                if (temp_content[0]!='\0')
                {
                    if(!essay_list[essay_list_position].volume.empty())
                    {
                        if(!kill)
                        {
                            emit error("more than one volume in an essay!");
                        }
                        return;
                    }
                    essay_list[essay_list_position].volume = temp_content;
                }
            }
            else if(temp_content[0] == '/' && strcmp(&temp_content[1], temp_type)==0)
            {
                if(kill)
                {
                    return;
                }
                break;
            }
            if(kill)
            {
                return;
            }
        }
        if (essay_list[essay_list_position].title.size()==0&&essay_list[essay_list_position].booktitle.size()==0)
        {
            essay_list.pop_back();
        }
        else
        {
            /*{
                struct Temp_can
                {
                    uint64_t essay_list_position;
                    std::vector<std::string>& author_name_temp_list;
                };
                Temp_can x={essay_list_position, author_name_temp_list};
                pthread_create(&pt1,NULL,update_author_list_author_index_for_pthread, (void *)&x);
            }*/
            //pthread_create(&pt2, NULL, update_essay_index_for_pthread, *(void **)&essay_list_position);
            update_author_list_author_index(essay_list_position, author_name_temp_list);
            update_essay_index(essay_list_position);
            //pthread_join(pt1, NULL);
            //pthread_join(pt2, NULL);
            essay_list_position++;
        }
        author_name_temp_list.clear();
    }
    get_top_100();
    emit wancheng();
}

inline void QObject_init::update_author_list_author_index(const uint64_t essay_list_position, const std::vector<std::string>& author_name_temp_list)
{
    //printf("1:%lu\n", essay_list_position);
    const uint64_t author_name_temp_list_size=author_name_temp_list.size();
    const uint64_t author_list_name_position_hash_table_size=author_list_name_position_hash_table.size();
    const uint64_t author_name_word_list_content_position_hash_table_size=author_name_word_list_content_position_hash_table.size();
    uint64_t author_list_position_temp_list[author_name_temp_list_size];
    {
        std::string temp_name;
        std::vector<std::string> word_content_temp_list;
        word_content_temp_list.reserve(32);
        std::vector<uint32_t> word_hash_result_list;
        word_hash_result_list.reserve(32);
        for (uint64_t i=author_name_temp_list_size; i!=0;)
        {
            i--;
            const uint64_t temp_x=md5_ELFhash_32(author_name_temp_list[i].c_str())%author_list_name_position_hash_table_size;
            for (uint64_t temp_y=author_list_name_position_hash_table[temp_x].size(); temp_y!=0;)
            {
                temp_y--;
                if (author_name_temp_list[i] == author_list[author_list_name_position_hash_table[temp_x][temp_y]].name )
                {
                    author_list_position_temp_list[i]=author_list_name_position_hash_table[temp_x][temp_y];
                    goto label1;
                }
            }
            author_list_position_temp_list[i]=author_list.size();
            author_list_name_position_hash_table[temp_x].emplace_back(author_list_position_temp_list[i]);
            author_list.emplace_back(author_name_temp_list[i]);
            //新作者，更新作者词索引
            {
                temp_name.resize(author_list[author_list_position_temp_list[i]].name.size());
                transform(author_list[author_list_position_temp_list[i]].name.begin(), author_list[author_list_position_temp_list[i]].name.end(), temp_name.begin(), ::tolower);
                unique_split(temp_name.c_str(), md5_ELFhash_32, word_content_temp_list, word_hash_result_list);
                for(uint64_t i2=word_content_temp_list.size(); i2!=0;)
                {
                    i2--;
                    const uint64_t temp_x=word_hash_result_list[i2]%author_name_word_list_content_position_hash_table_size;
                    for(uint64_t temp_y=author_name_word_list_content_position_hash_table[temp_x].size(); temp_y!=0;)
                    {
                        temp_y--;
                        if(word_content_temp_list[i2]==author_name_word_list[author_name_word_list_content_position_hash_table[temp_x][temp_y]].content)
                        {
                            author_name_word_list[author_name_word_list_content_position_hash_table[temp_x][temp_y]].position_list.emplace_back(author_list_position_temp_list[i]);
                            goto labelx1;
                        }
                    }
                    {
                        const uint64_t temp_size=author_name_word_list.size();
                        author_name_word_list_content_position_hash_table[temp_x].emplace_back(temp_size);
                        author_name_word_list.emplace_back(word_content_temp_list[i2]);
                        author_name_word_list[temp_size].position_list.emplace_back(author_list_position_temp_list[i]);
                    }
labelx1:
                    ;
                }
            }
label1:
            essay_list[essay_list_position].author_position_list.emplace_back(author_list_position_temp_list[i]);
            author_list[author_list_position_temp_list[i]].essay_position_list.emplace_back(essay_list_position);
        }
    }
    //更新合作者列表，可以优化，在temp_size1后面建立一个临时哈希表
    for (uint64_t i=author_name_temp_list_size; i>1;)
    {
        i--;
        uint64_t temp_size1=author_list[author_list_position_temp_list[i]].partner_position_list.size();
        for (uint64_t i2=i; i2!=0;)
        {
            i2--;
            const uint64_t temp_size2=author_list[author_list_position_temp_list[i2]].partner_position_list.size();
            if (temp_size1<=temp_size2)
            {
                for (uint64_t i3=temp_size1; i3!=0;)
                {
                    i3--;
                    if (author_list[author_list_position_temp_list[i]].partner_position_list[i3] == author_list_position_temp_list[i2])
                    {
                        goto label2;
                    }
                }
            }
            else
            {
                for (uint64_t i3=temp_size2; i3!=0;)
                {
                    i3--;
                    if (author_list[author_list_position_temp_list[i2]].partner_position_list[i3] == author_list_position_temp_list[i])
                    {
                        goto label2;
                    }
                }
            }
            temp_size1++;
            author_list[author_list_position_temp_list[i]].partner_position_list.emplace_back(author_list_position_temp_list[i2]);
            author_list[author_list_position_temp_list[i2]].partner_position_list.emplace_back(author_list_position_temp_list[i]);
label2:
            ;
        }
    }
}

inline void QObject_init::update_essay_index(const uint64_t essay_list_position)
{
    //temp_title= "$title|$booktitle"，并且大写转为小写
    std::string temp_title;
    temp_title.resize(essay_list[essay_list_position].title.size()+essay_list[essay_list_position].booktitle.size()+1);
    transform(essay_list[essay_list_position].title.begin(), essay_list[essay_list_position].title.end(), temp_title.begin(), ::tolower);
    temp_title[essay_list[essay_list_position].title.size()]='|';
    transform(essay_list[essay_list_position].booktitle.begin(), essay_list[essay_list_position].booktitle.end(), temp_title.begin()+essay_list[essay_list_position].title.size()+1 , ::tolower);

    const uint64_t essay_title_booktitle_word_list_content_position_hash_table_size=essay_title_booktitle_word_list_content_position_hash_table.size();
    //拆词
    std::vector<std::string> word_content_temp_list;
    word_content_temp_list.reserve(64);
    std::vector<uint32_t> word_hash_result_temp_list;
    word_hash_result_temp_list.reserve(64);
    unique_split(temp_title.c_str(), md5_ELFhash_32, word_content_temp_list, word_hash_result_temp_list);
    for(uint64_t i=word_content_temp_list.size(); i!=0;)
    {
        i--;
        const uint64_t temp_x=word_hash_result_temp_list[i]%essay_title_booktitle_word_list_content_position_hash_table_size;
        for(uint64_t temp_y=essay_title_booktitle_word_list_content_position_hash_table[temp_x].size(); temp_y!=0;)
        {
            temp_y--;
            if(word_content_temp_list[i]==essay_title_booktitle_word_list[essay_title_booktitle_word_list_content_position_hash_table[temp_x][temp_y]].content)
            {
                essay_title_booktitle_word_list[essay_title_booktitle_word_list_content_position_hash_table[temp_x][temp_y]].position_list.emplace_back(essay_list_position);
                goto label1;
            }
        }
        //新词
        {
            const uint64_t temp_size=essay_title_booktitle_word_list.size();
            essay_title_booktitle_word_list_content_position_hash_table[temp_x].emplace_back(temp_size);
            essay_title_booktitle_word_list.emplace_back(word_content_temp_list[i]);
            essay_title_booktitle_word_list[temp_size].position_list.emplace_back(essay_list_position);
        }
label1:
        ;
    }
    if (essay_list[essay_list_position].title.size()!=0)
    {
        uint64_t temp_x=md5_ELFhash_32(essay_list[essay_list_position].title.c_str())%essay_list_title_booktitle_position_hash_table.size();
        essay_list_title_booktitle_position_hash_table[temp_x].emplace_back(essay_list_position);
        if (essay_list[essay_list_position].booktitle.size()!=0)
        {
            uint64_t temp_x2=md5_ELFhash_32(essay_list[essay_list_position].booktitle.c_str())%essay_list_title_booktitle_position_hash_table.size();
            if(temp_x!=temp_x2)
            {
                essay_list_title_booktitle_position_hash_table[temp_x2].emplace_back(essay_list_position);
            }
        }
    }
    else if (essay_list[essay_list_position].booktitle.size()!=0)
    {
        essay_list_title_booktitle_position_hash_table[md5_ELFhash_32(essay_list[essay_list_position].booktitle.c_str())%essay_list_title_booktitle_position_hash_table.size()].emplace_back(essay_list_position);
    }
    //大写转小写的c语言实现
    /*char temp_title[2000];
    {
        const char *src=essay_list[position_of_essay].title.c_str();
        char *dest=temp_title;
        while (1)
        {
            char tmp=*src;
            if ('A'<=tmp&&tmp<='Z')
            {
                *(dest++)=(tmp|0b100000);
                src++;
            }
            else
            {
                *(dest++)=tmp;
                if(tmp=='\0')
                {
                    break;
                }
                src++;
            }
        }
    }
    */
}

inline void QObject_init::get_top_100()
{
    std::list<uint64_t> temp_author_list_top_100_position_list;
    const uint64_t author_list_size=author_list.size();
    uint64_t i=0;
    while (i!=author_list_size)
    {
        for (std::list<uint64_t>::iterator p=temp_author_list_top_100_position_list.begin(), end=temp_author_list_top_100_position_list.end(); p!=end; p++)
        {
            if(author_list[*p].essay_position_list.size()<author_list[i].essay_position_list.size())
            {
                continue;
            }
            temp_author_list_top_100_position_list.insert(p, i);
            goto label1;
        }
        temp_author_list_top_100_position_list.emplace_back(i);
label1:
        i++;
        if (i == 100)
        {
            break;
        }
    }
    while (i!=author_list_size)
    {
        std::list<uint64_t>::iterator p=temp_author_list_top_100_position_list.begin();
        if (author_list[i].essay_position_list.size()<=author_list[*p].essay_position_list.size())
        {
            i++;
            continue;
        }
        for (uint64_t count=99; count!=0; count--)
        {
            p++;
            if (author_list[i].essay_position_list.size()<=author_list[*p].essay_position_list.size())
            {
                temp_author_list_top_100_position_list.insert(p, i);
                goto label2;
            }
        }
        temp_author_list_top_100_position_list.emplace_back(i);
label2:
        temp_author_list_top_100_position_list.pop_front();
        i++;
    }
    author_list_top_100_position_list.reserve(100);
    author_list_top_100_position_list.assign(temp_author_list_top_100_position_list.rbegin(), temp_author_list_top_100_position_list.rend());
}

inline void QObject_init::get_month_from_content(const char * content, uint32_t &year_month)
{
    char month[30];
    sscanf(content, "%[^-/<]",month);
    if (strcmp(month, "January") == 0)
    {
        *(uint8_t *)&year_month=JANUARY;
    }
    else if (strcmp(month, "February") == 0)
    {
        *(uint8_t *)&year_month=FEBRUARY;
    }
    else if (strcmp(month, "March") == 0)
    {
        *(uint8_t *)&year_month=MARCH;
    }
    else if (strcmp(month, "April") == 0)
    {
        *(uint8_t *)&year_month=APRIL;
    }
    else if (strcmp(month, "May") == 0)
    {
        *(uint8_t *)&year_month=MAY;
    }
    else if (strcmp(month, "June") == 0)
    {
        *(uint8_t *)&year_month=JUNE;
    }
    else if (strcmp(month, "July") == 0)
    {
        *(uint8_t *)&year_month=JULY;
    }
    else if (strcmp(month, "August") == 0)
    {
        *(uint8_t *)&year_month=AUGUST;
    }
    else if (strcmp(month, "September") == 0)
    {
        *(uint8_t *)&year_month=SEPTEMBER;
    }
    else if (strcmp(month, "October") == 0)
    {
        *(uint8_t *)&year_month=OCTOBER;
    }
    else if (strcmp(month, "November") == 0)
    {
        *(uint8_t *)&year_month=NOVEMBER;
    }
    else if (strcmp(month, "December") == 0)
    {
        *(uint8_t *)&year_month=DECEMBER;
    }
    else
    {
        if(!kill)
        {
            emit error("xml解析出错：未知月份类型");
            kill=true;
        }
        return;
    }
}

inline void QObject_init::remove_item(FILE* fp, const char * const item_title)
{
    const uint64_t title_len=strlen(item_title);
    while (1)
    {
        fscanf(fp,"%*[^<]");
        if(getc(fp)==EOF)
        {
            if(!kill)
            {
                emit error("xml解析出错：不是标准的xml文件！");
                kill=true;
            }
            return;
        }
        if (getc(fp) != '/')
        {
            continue;
        }
        for (uint64_t i=0; i<title_len; i++)
        {
            if ( getc(fp) != item_title[i])
            {
                goto label_continue;
            }
        }
        if (getc(fp)=='>')
        {
            return;
        }
label_continue:
        ;
    }
}

inline void QObject_init::get_item_content(FILE* fp, const char * const item_title, char * buffer)
{
    const uint64_t title_len=strlen(item_title);
    while (1)
    {
        while (( *buffer = getc(fp)) != '<')
        {
            if (*buffer==EOF)
            {
                if(!kill)
                {
                    emit error("xml解析出错：不是标准的xml文件！");
                    kill=true;
                }
                return;
            }
            buffer++;
        }
        buffer++;
        if (( *(buffer++) = getc(fp)) != '/')
        {
            continue;
        }
        for (uint64_t i=0; i<title_len; i++)
        {
            if (( *(buffer++) = getc(fp)) != item_title[i])
            {
                goto label_continue;
            }
        }
        if (( *buffer = getc(fp)) != '>')
        {
            buffer++;
            continue;
        }
        buffer[-(int64_t)title_len-2]='\0';
        return;
label_continue:
        ;
    }
}

//        |
// <xxxxx>
inline void QObject_init::get_tag_content(FILE* fp, char *buffer)
{
    fscanf(fp,"%*[^<]");
    getc(fp);
    while (1)
    {
        *buffer=getc(fp);
        if ( *buffer == '>')
        {
            break;
        }
        else if( *buffer == '<' )
        {
            if(!kill)
            {
                emit error("find  '<' in tag_content");
                kill=true;
            }
            return;
        }
        else if( *buffer == EOF)
        {
            if(!kill)
            {
                emit error("xml解析出错：不是标准的xml文件！");
                kill=true;
            }
            return;
        }
        buffer++;
    }
    *buffer='\0';
    /*if (fscanf(fp,"%[^>]",tag) != 1)
    {
        tag[0] = '\0';
    }
    getc(fp);*/
}


#endif // QOBJECT_INIT_H
