#include "d_essay.h"
#include "ui_d_essay.h"
#include "my_data_struct.h"
#include "type_to_str.h"
#include "d_author.h"

D_essay::D_essay(const uint64_t essay_list_position, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::D_essay),
    essay_list_position(essay_list_position),
    author_num(essay_list[essay_list_position].author_position_list.size())
{
    ui->setupUi(this);
    setWindowOpacity(0.88);
    setWindowFlags(Qt::CustomizeWindowHint| Qt::WindowTitleHint);
    setPalette(QPalette(QColor(254,247,255)));
    ui->tw1->setRowCount(author_num);
    ui->tw1->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tw1->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw1->setSelectionMode(QAbstractItemView::SingleSelection);
    x=new QTableWidgetItem [author_num];
    for (uint64_t i=0; i<author_num; i++)
    {
        ui->tw1->setItem(i,0,&x[i]);
        x[i].setText(author_list[essay_list[essay_list_position].author_position_list[i]].name.c_str());
    }
    connect(ui->tw1, &QTableWidget::cellDoubleClicked, [this,essay_list_position](int i,int)
    {
        D_author x(essay_list[essay_list_position].author_position_list[i]);
        x.exec();
    });
    connect(ui->b1, &QPushButton::released, [this]()
    {
        close();
    });
    ui->l_title->setText(essay_list[essay_list_position].title.c_str());
    ui->l_booktitle->setText(essay_list[essay_list_position].booktitle.c_str());
    ui->l_journal->setText(essay_list[essay_list_position].journal.c_str());
    ui->l_volume->setText(essay_list[essay_list_position].volume.c_str());
    if((uint16_t)(essay_list[essay_list_position].year_month>>8)!=0)
    {
        QString date=QString::number((uint16_t)(essay_list[essay_list_position].year_month>>8));
        if((uint8_t)(essay_list[essay_list_position].year_month)!=0)
        {
            date+='-'+QString::number((uint8_t)(essay_list[essay_list_position].year_month));
        }
        ui->l_data->setText(date);
    }
    else if((uint8_t)(essay_list[essay_list_position].year_month)!=0)
    {
        ui->l_data->setText(QString::number((uint8_t)(essay_list[essay_list_position].year_month)));
    }
    ui->l_type->setText(type_to_str(essay_list[essay_list_position].type));
}

D_essay::~D_essay()
{
    delete ui;
    delete []x;
}
