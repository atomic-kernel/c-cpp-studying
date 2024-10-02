#ifndef D_SEARCH_ESSAY_H
#define D_SEARCH_ESSAY_H

#include <QDialog>

namespace Ui {
class D_search_essay;
}

class D_search_essay : public QDialog
{
    Q_OBJECT

public:
    explicit D_search_essay(QWidget *parent = nullptr);
    ~D_search_essay();

private:
    Ui::D_search_essay *ui;
};

#endif // D_SEARCH_ESSAY_H
