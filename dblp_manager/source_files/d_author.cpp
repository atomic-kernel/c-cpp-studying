#include "d_author.h"
#include "ui_d_author.h"
#include "my_data_struct.h"
#include "type_to_str.h"
#include "d_essay.h"

D_author::D_author(const uint64_t position, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::D_author),
    position(position),
    essay_num(author_list[position].essay_position_list.size()),
    author_num(author_list[position].partner_position_list.size())
{
    ui->setupUi(this);
    setWindowOpacity(0.88);
    setWindowFlags(Qt::CustomizeWindowHint| Qt::WindowTitleHint);
    setPalette(QPalette(QColor(254,247,255)));
    ui->tw_essay->setRowCount(essay_num);
    ui->tw_essay->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tw_essay->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_essay->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_partner->setRowCount(author_num);
    ui->tw_partner->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tw_partner->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_partner->setSelectionMode(QAbstractItemView::SingleSelection);
    essay=new QTableWidgetItem [essay_num][3];
    author=new QTableWidgetItem [author_num][1];
    for(uint64_t i=essay_num; i!=0; )
    {
        i--;
        ui->tw_essay->setItem(i,0,&essay[i][0]);
        ui->tw_essay->setItem(i,1,&essay[i][1]);
        ui->tw_essay->setItem(i,2,&essay[i][2]);
        essay[i][0].setText(type_to_str(essay_list[author_list[position].essay_position_list[i]].type));
        essay[i][1].setText(essay_list[author_list[position].essay_position_list[i]].title.c_str());
        essay[i][2].setText(essay_list[author_list[position].essay_position_list[i]].booktitle.c_str());
    }
    for (uint64_t i=author_num; i!=0; )
    {
        i--;
        ui->tw_partner->setItem(i,0,&author[i][0]);
        author[i][0].setText(author_list[author_list[position].partner_position_list[i]].name.c_str());
    }
    ui->l_name->setText(author_list[position].name.c_str());
    connect(ui->tw_essay, &QTableWidget::cellDoubleClicked, [position](int i, int)
    {
        D_essay x(author_list[position].essay_position_list[i]);
        x.exec();
    });
    connect(ui->tw_partner, &QTableWidget::cellDoubleClicked, [position](int i, int)
    {
        D_author x(author_list[position].partner_position_list[i]);
        x.exec();
    });
    connect(ui->b1, &QPushButton::released, [this]()
    {
        close();
    });
}

D_author::~D_author()
{
    delete ui;
    delete []essay;
    delete [] author;
}
