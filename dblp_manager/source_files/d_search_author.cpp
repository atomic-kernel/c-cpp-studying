#include "d_search_author.h"
#include "ui_d_search_author.h"
#include "search.h"
#include "d_essay_list.h"
#include "d_author_list.h"

D_search_author::D_search_author(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::D_search_author)
{
    ui->setupUi(this);
    setWindowOpacity(0.88);
    setWindowFlags(Qt::CustomizeWindowHint| Qt::WindowTitleHint);
    setPalette(QPalette(QColor(254,247,255)));
    connect(ui->b2, &QPushButton::released, [this]()
    {
        close();
    });
    connect(ui->b1, &QPushButton::released, [this]()
    {
        QString temp=ui->le1->text();
        QByteArray search_content = temp.toLocal8Bit();
        if(search_content.data()==NULL || search_content.data()[0] =='\0' )
        {
            return;
        }
        uint64_t a=ui->cb1->currentIndex();
        std::vector<uint64_t> result;
        if(a==0)
        {
            word_search_author(result, search_content.data());
        }
        else if(a==1)
        {
            name_search_author(result, search_content.data());
        }
        else if(a==2)
        {
            name_search_author_strstr(result, search_content.data());
        }
        D_author_list d_result(result);
        d_result.exec();
    });
}

D_search_author::~D_search_author()
{
    delete ui;
}
