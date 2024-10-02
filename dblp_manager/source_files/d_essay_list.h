#ifndef D_ESSAY_LIST_H
#define D_ESSAY_LIST_H

#include <QDialog>

#include <QTableWidgetItem>

namespace Ui {
class D_essay_list;
}

class D_essay_list : public QDialog
{
    Q_OBJECT

public:
    explicit D_essay_list(const std::vector<uint64_t> &t_essay_position_list, QWidget *parent = nullptr);
    ~D_essay_list();

private:
    Ui::D_essay_list *ui;
    void flush();
    const std::vector<uint64_t> &essay_position_list;
    uint64_t ye=0;
    const uint64_t last_ye;
    QTableWidgetItem x[10][3];
};

#endif // D_ESSAY_LIST_H
