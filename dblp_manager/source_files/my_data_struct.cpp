#include "my_data_struct.h"
std::vector<Essay> essay_list;
std::vector<std::vector<uint64_t>> essay_list_title_booktitle_position_hash_table;
std::vector<Word> essay_title_booktitle_word_list;
std::vector<std::vector<uint64_t>> essay_title_booktitle_word_list_content_position_hash_table;

std::vector<Author> author_list;
std::vector<std::vector<uint64_t>> author_list_name_position_hash_table;
std::vector<Word> author_name_word_list;
std::vector<std::vector<uint64_t>> author_name_word_list_content_position_hash_table;

std::vector<uint64_t> author_list_top_100_position_list;
