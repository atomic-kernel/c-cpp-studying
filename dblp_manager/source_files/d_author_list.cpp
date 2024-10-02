#include "d_author_list.h"
#include "ui_d_author_list.h"
#include "my_data_struct.h"
#include "d_author.h"

D_author_list::D_author_list(const std::vector<uint64_t> &author_position_list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::D_author_list),
    author_position_list(author_position_list),
    last_ye(author_position_list.size()==0?0:((author_position_list.size()-1)/10))

{
    ui->setupUi(this);
    setWindowOpacity(0.88);
    setWindowFlags(Qt::CustomizeWindowHint| Qt::WindowTitleHint);
    setPalette(QPalette(QColor(254,247,255)));
    flush();
    ui->tw1->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tw1->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw1->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->tw1, &QTableWidget::cellDoubleClicked, [this](int a, int)
    {
        if (a+ye*10<this->author_position_list.size())
        {
            D_author x(this->author_position_list[a+ye*10]);
            x.exec();
        }
    });
    connect(ui->p10, &QPushButton::released , [this]()
    {
        if(ye!=0)
        {
            ye--;
            flush();
        }
    });
    connect(ui->p11, &QPushButton::released , [this]()
    {
        if (ye!=last_ye)
        {
            ye++;
            flush();
        }
    });
    connect(ui->p1, &QPushButton::released, [this]()
    {
        close();
    });
    for(uint64_t i=0; i<=9; i++)
    {
        ui->tw1->setItem(i,0,&x[i][0]);
    }
}

void D_author_list::flush()
{
    ui->l1->setText(QString::number(ye+1)+'/'+QString::number(last_ye+1));

    if (ye!=last_ye)
    {
        for(uint64_t i=0,i2=ye*10; i<=9; i++,i2++)
        {
            x[i][0].setText(author_list[author_position_list[i2]].name.c_str());
        }
    }
    else
    {
        uint64_t i=0;
        for(uint64_t i2=ye*10; i2<author_position_list.size(); i++,i2++)
        {
            x[i][0].setText(author_list[author_position_list[i2]].name.c_str());
        }
        for (; i<=9; i++)
        {
            x[i][0].setText(nullptr);
        }
    }
}

D_author_list::~D_author_list()
{
    delete ui;
}
