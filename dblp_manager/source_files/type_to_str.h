#include "my_data_struct.h"
static inline const char * type_to_str(int x);
const char * type_to_str(int x)
{
    switch(x)
    {
        case PHDTHESIS: return "phdthesis";
        case INCOLLECTION: return "incollection";
        case MASTERSTHESIS: return "mastersthesis";
    //WWW,
        case INPROCEEDINGS: return "inproceedings";
        case PROCEEDINGS: return "proceedings";
        case BOOK: return "book";
        case ARTICLE: return "article";
        default: return "error";
    }
}
