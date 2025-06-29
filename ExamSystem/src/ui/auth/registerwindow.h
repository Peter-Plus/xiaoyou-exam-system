#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

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

class RegisterWindow:public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent=nullptr);

private slots:
    void onRegisterClicked();//注册处理函数
    void onCancelClicked();//取消处理函数
    void onUserTypeChanged();//当用户选择学生或教师时，决定是否显示年级输入框

private:
    void setupUI();

    QLineEdit *userIdEdit;
    QLineEdit *nameEdit;
    QLineEdit *gradeEdit;
    QLineEdit *collegeEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *registerButton;
    QPushButton *cancelButton;
    QRadioButton *studentRadio;
    QRadioButton *teacherRadio;
    QButtonGroup *userTypeGroup;
    QLabel *gradeLabel;

    Database database;

};


#endif // REGISTERWINDOW_H
