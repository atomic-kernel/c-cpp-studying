#ifndef D_AUTHOR_H
#define D_AUTHOR_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class D_author;
}

class D_author : public QDialog
{
    Q_OBJECT

public:
    explicit D_author(const uint64_t position, QWidget *parent = nullptr);
    ~D_author();

private:
    Ui::D_author *ui;
    const uint64_t position;
    const uint64_t essay_num;
    const uint64_t author_num;
    QTableWidgetItem (*essay)[3];
    QTableWidgetItem (*author)[1];
};

#endif // D_AUTHOR_H
