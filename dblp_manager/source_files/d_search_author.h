#ifndef D_SEARCH_AUTHOR_H
#define D_SEARCH_AUTHOR_H

#include <QDialog>

namespace Ui {
class D_search_author;
}

class D_search_author : public QDialog
{
    Q_OBJECT

public:
    explicit D_search_author(QWidget *parent = nullptr);
    ~D_search_author();

private:
    Ui::D_search_author *ui;
};

#endif // D_SEARCH_AUTHOR_H
