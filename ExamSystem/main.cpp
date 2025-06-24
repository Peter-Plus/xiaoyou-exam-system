#include "mainwindow.h"
#include "database.h"
#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 检查可用驱动
    QStringList drivers = QSqlDatabase::drivers();
    qDebug() << "可用驱动:" << drivers;

    // 测试数据库连接
    Database database;
    if (database.connectToDatabase()) {
        qDebug() << "开始创建数据库表...";
        database.createTables();
    } else {
        qDebug() << "数据库连接失败，程序继续运行";
    }

    MainWindow w;
    w.show();

    return a.exec();
}
