#include "mainwidget.h"

#include "./ui_mainwidget.h"
#include <QFileDialog>
#include "d_search_essay.h"
#include "d_search_author.h"
#include <QThread>
#include "qobject_init.h"
#include "d_init.h"
#include "d_author_list.h"
#include "my_data_struct.h"
#include "my_error.h"
#include "ui_d_init.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    {
        FILE *fp;
        {
            QString temp=QFileDialog::getOpenFileName(this,"请选择XML资源文件",".","Xml files (*xml)");
            QByteArray bytes = temp.toLocal8Bit();
            fp=fopen(bytes.data(),"r");
            if (fp==NULL)
            {
                my_error("打开文件失败！");
            }
        }
        //创建子线程：qt1
        QThread qt1;
        {
            //创建自定义QObject：qob_init
            QObject_init qob_init;

            //将qob_init移至子线程运行
            qob_init.moveToThread(&qt1);

            //创建与用户交互的窗口：d_init
            D_init d_init;

            //收到解析完成信息关闭窗口
            connect(&qob_init, &QObject_init::wancheng, &d_init, &QMessageBox::close);

            //子线程发出错误信号，主动退出程序
            connect(&qob_init, &QObject_init::error, this,[&qt1, &d_init, fp](const char * error)
            {
                d_init.close();
                qt1.quit();
                qt1.wait();
                fclose(fp);
                my_error(error);
            });

            //窗口的退出按钮被点击，主动退出程序
            connect(d_init.ui->pushButton, &QPushButton::released, this, [&qob_init, &d_init, &qt1, fp]()
            {
                d_init.close();
                qob_init.kill=true;
                qt1.quit();
                qt1.wait();
                fclose(fp);
                exit(0);
            });

            //开始解析文件
            connect(&qt1, &QThread::started, &qob_init, [&qob_init, fp]()
            {
                qob_init.init(fp);
            });
            qt1.start();

            //开启窗口，阻塞运行，等待解析结束
            d_init.exec();

            qt1.quit();
            qt1.wait();
            fclose(fp);
            //先释放qob_init
        }
        //然后释放qt1
    }
    ui->setupUi(this);
    connect(ui->b1, &QPushButton::released, []()
    {
        D_search_essay d;
        d.exec();
    });
    connect(ui->b2, &QPushButton::released, []()
    {
        D_search_author x;
        x.exec();
    });
    connect(ui->b3, &QPushButton::released, []()
    {
        D_author_list x(author_list_top_100_position_list);
        x.exec();
    });
    connect(ui->b4, &QPushButton::released, [this]()
    {
        close();
    });
    setWindowOpacity(0.88);
    setWindowFlags(Qt::CustomizeWindowHint| Qt::WindowTitleHint);
    setAutoFillBackground(true);//必须有这条语句
    //setPalette(QPalette(QColor(247,247,247)));
    //setPalette(QPalette(QColor(254,252,255)));
    setPalette(QPalette(QColor(254,247,255)));
    ui->l1->setText(QString::number(essay_list.size()));
    ui->l2->setText(QString::number(author_list.size()));
}



MainWidget::~MainWidget()
{
    delete ui;
}
