#ifndef STUDENTMAINWINDOW_H
#define STUDENTMAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include "student.h"
#include "database.h"
#include "studentexamlist.h"
#include "examresult.h"

class StudentMainWindow:public QMainWindow
{
    Q_OBJECT

public:
    // 新的构造函数：接受Student对象和Database
    explicit StudentMainWindow(const Student &student, Database *database, QWidget *parent = nullptr);

    // 保留原构造函数以保持兼容性
    explicit StudentMainWindow(int studentId, QWidget *parent = nullptr);

private slots:
    void onExamClicked();
    void onScoreClicked();
    void onLogoutClicked();

private:
    void setupUI();
    void loadStudentInfo();

    Student currentStudent;
    Database *database;

    // 子窗口指针
    StudentExamList *examListWindow;

    QLabel *welcomeLabel;
    QPushButton *examButton;
    QPushButton *scoreButton;
    QPushButton *logoutButton;

signals:
    void logoutRequested(); // 修改信号名称以匹配main.cpp
};

#endif // STUDENTMAINWINDOW_H
