#ifndef D_INIT_H
#define D_INIT_H

#include <QDialog>

namespace Ui {
class D_init;
}

class D_init : public QDialog
{
    Q_OBJECT

public:
    explicit D_init(QWidget *parent = nullptr);
    ~D_init();

private:
    Ui::D_init *ui;
    friend class MainWidget;
};

#endif // D_INIT_H
