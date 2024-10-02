#include "d_init.h"
#include "ui_d_init.h"

D_init::D_init(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::D_init)
{
    ui->setupUi(this);
    setWindowOpacity(0.93);
    setPalette(QPalette(QColor(254,247,255)));
    setWindowFlags(Qt::CustomizeWindowHint| Qt::WindowTitleHint);
}

D_init::~D_init()
{
    delete ui;
}
