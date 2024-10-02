#ifndef D_ESSAY_H
#define D_ESSAY_H

#include <QDialog>
#include <QTableWidgetItem>


namespace Ui {
class D_essay;
}

class D_essay : public QDialog
{
    Q_OBJECT

public:
    explicit D_essay(const uint64_t essay_list_position, QWidget *parent = nullptr);
    ~D_essay();

private:
    Ui::D_essay *ui;
    const uint64_t essay_list_position;
    const uint64_t author_num;
    QTableWidgetItem *x;
};

#endif // D_ESSAY_H
