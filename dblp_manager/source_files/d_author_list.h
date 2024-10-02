#ifndef D_AUTHOR_LIST_H
#define D_AUTHOR_LIST_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class D_author_list;
}

class D_author_list : public QDialog
{
    Q_OBJECT

public:
    explicit D_author_list(const std::vector<uint64_t> &author_position_list, QWidget *parent = nullptr);
    ~D_author_list();

private:
    Ui::D_author_list *ui;
    void flush();
    const std::vector<uint64_t> &author_position_list;
    uint64_t ye=0;
    const uint64_t last_ye;
    QTableWidgetItem x[10][1];
};

#endif // D_AUTHOR_LIST_H
