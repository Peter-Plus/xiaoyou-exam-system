#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class Database
{
public:
    Database();
    bool connectToDatabase();
    void createTables();
    void closeDatabase();

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
