#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>
#include "database.h"

class LoginWindow:public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

private:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void setupUI();
    QLineEdit *userIdEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QRadioButton *studentRadio;
    QRadioButton *teacherRadio;
    QButtonGroup *userTypeGroup;

    Database database;

signals:
    void loginSuccess(int userId,bool isStudent);
};

#endif // LOGINWINDOW_H
