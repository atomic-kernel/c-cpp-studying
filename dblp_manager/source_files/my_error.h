#include <QMessageBox>
#include <stdlib.h>
#include <QDebug>
static inline void my_error(const char *const error_info);
inline void my_error(const char *const error_info)
{
    QMessageBox::critical(NULL, "错误", error_info);
    exit(1);
}
