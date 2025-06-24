#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QDockWidget>
#include <QDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QDebug>
#include <QMovie>
#include <QToolBar>        // 添加这行
#include <QMenuBar>        // 添加这行
#include <QMenu>           // 添加这行
#include <QAction>
#include <QLabel>
#include <QColor>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建菜单栏
    QMenuBar* bar = menuBar();
    this->setMenuBar(bar);

    // 创建菜单
    QMenu* menuFile = bar->addMenu("文件");
    QMenu* menuEdit = bar->addMenu("编辑");

    // 添加动作
    QAction* actionNew = menuFile->addAction("新建");
    menuFile->addSeparator();
    QAction* actionOpen = menuFile->addAction("打开");

    // 连接信号和槽（建议添加）
    // connect(actionNew, &QAction::triggered, this, &MainWindow::newFile);
    // connect(actionOpen, &QAction::triggered, this, &MainWindow::openFile);

    // 创建工具栏
    QToolBar* toolbar = new QToolBar(this);
    this->addToolBar(Qt::LeftToolBarArea, toolbar);

    // 工具栏设置
    toolbar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
    toolbar->setFloatable(true);  // 如果想禁止浮动
    toolbar->setMovable(true);

    // 添加动作到工具栏（建议添加）
    toolbar->addAction(actionNew);
    toolbar->addAction(actionOpen);

    //创建状态栏
    QStatusBar* stbar = statusBar();
    setStatusBar(stbar);
    //创建文字
    QLabel* leftLabel = new QLabel("@wlinword.cn",this);
    stbar->addWidget(leftLabel);
    QLabel* rightLabel = new QLabel("2027.1.3",this);
    stbar->addPermanentWidget(rightLabel);


    // //错误对话框
    // QMessageBox::critical(this,"错误","critical");
    // //信息对话框
    // QMessageBox::information(this,"错误","information");
    // //警告对话框
    // QMessageBox::warning(this,"警告","warning");

    // int code = QMessageBox::question(this,"问题","question",QMessageBox::Save|QMessageBox::Cancel);
    // if(code==QMessageBox::Save)
    // {
    //     qDebug()<<"保存";
    // }else
    // {
    //     qDebug()<<"取消";
    // }

    QColor color = QColorDialog::getColor(QColor(255,0,0));
    qDebug()<<color.red()<<color.green()<<color.blue();

    QString path =QFileDialog::getOpenFileName(this,"打开文件","S:\\","*.txt *.png");
    qDebug()<<path;

}
MainWindow::~MainWindow()
{
    delete ui;
}
