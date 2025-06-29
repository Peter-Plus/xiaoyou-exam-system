#include "loginwindow.h"
#include "registerwindow.h"


LoginWindow::LoginWindow(QWidget *parent):QWidget(parent)
{
    setupUI();
    //连接数据库
    if(!database.connectToDatabase())
    {
        QMessageBox::critical(this,"错误","无法连接到数据库");
    }
}

void LoginWindow::setupUI()
{
    setWindowTitle("考试系统登录");
    setFixedSize(450,400);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    //标题
    QLabel *titleLabel =new QLabel("考试系统登录");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size:18px;font-weight:bold;margin:10px;");

    //用户类型选择
    QHBoxLayout *typeLayout = new QHBoxLayout();
    studentRadio = new QRadioButton("学生");
    teacherRadio = new QRadioButton("教师");
    studentRadio->setChecked(true);//默认选择学生

    userTypeGroup = new QButtonGroup(this);
    userTypeGroup->addButton(studentRadio,0);
    userTypeGroup->addButton(teacherRadio,1);

    typeLayout->addWidget(studentRadio);
    typeLayout->addWidget(teacherRadio);

    //用户ID输入
    QHBoxLayout *userIdLayout = new QHBoxLayout();
    QLabel *userIdLabel = new QLabel("学号/工号:");
    userIdEdit = new QLineEdit();
    userIdLayout->addWidget(userIdLabel);
    userIdLayout->addWidget(userIdEdit);

    //密码输入
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    QLabel *passwordLabel = new QLabel("密码:");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordEdit);

    //按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    loginButton = new QPushButton("登录");
    registerButton = new QPushButton("注册");
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);

    //添加到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(typeLayout);
    mainLayout->addLayout(userIdLayout);
    mainLayout->addLayout(passwordLayout);
    mainLayout->addLayout(buttonLayout);

    //连接信号槽
    connect(loginButton,&QPushButton::clicked,this,&LoginWindow::onLoginClicked);
    connect(registerButton,&QPushButton::clicked,this,&LoginWindow::onRegisterClicked);
}

void LoginWindow::onLoginClicked()
{
    QString userIdText = userIdEdit->text();
    QString password = passwordEdit->text();

    if(userIdText.isEmpty()||password.isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入完整的登录信息");
        return;
    }

    int userId = userIdText.toInt();//将学号转为数字int
    bool isStudent = studentRadio->isChecked();
    bool success = false;
    if(isStudent)
    {
        success = database.authenticateStudent(userId,password);
    }
    else
    {
        success = database.authenticateTeacher(userId,password);
    }

    //处理登录结果
    if(success)
    {
        emit loginSuccess(userId,isStudent);
        this->close();
    }
    else
    {
        QMessageBox::warning(this,"登录失败","账号或密码错误");
    }
}

void LoginWindow::onRegisterClicked()
{
    RegisterWindow *registerWindow = new RegisterWindow();
    registerWindow->setAttribute(Qt::WA_DeleteOnClose);
    registerWindow->show();
}




