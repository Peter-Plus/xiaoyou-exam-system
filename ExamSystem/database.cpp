#include "database.h"

Database::Database()
{
}

bool Database::connectToDatabase()
{
    db = QSqlDatabase::addDatabase("QODBC");

    // 使用正确的64位驱动名称
    QStringList driverNames = {
        "MySQL ODBC 9.3 Unicode Driver",
        "MySQL ODBC 9.3 ANSI Driver"
    };

    bool connected = false;
    for (const QString &driverName : driverNames) {
        QString connectionString = QString("DRIVER={%1};"
                                           "SERVER=localhost;"
                                           "DATABASE=exam_system;"
                                           "UID=root;"
                                           "PWD=Zy202312138;"
                                           "PORT=3306;").arg(driverName);

        db.setDatabaseName(connectionString);

        if (db.open()) {
            qDebug() << "数据库连接成功! 使用驱动:" << driverName;
            connected = true;
            break;
        } else {
            qDebug() << "尝试驱动" << driverName << "失败:" << db.lastError().text();
        }
    }

    if (!connected) {
        qDebug() << "所有驱动尝试失败";
        return false;
    }

    return true;
}

void Database::createTables()
{
    QSqlQuery query;

    QString createUsersTable =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "username VARCHAR(50) UNIQUE NOT NULL,"
        "password VARCHAR(255) NOT NULL,"
        "role ENUM('student', 'teacher', 'admin') DEFAULT 'student',"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(createUsersTable)) {
        qDebug() << "创建用户表失败:" << query.lastError().text();
    } else {
        qDebug() << "用户表创建成功";
    }
}

void Database::closeDatabase()
{
    if (db.isOpen()) {
        db.close();
    }
}
