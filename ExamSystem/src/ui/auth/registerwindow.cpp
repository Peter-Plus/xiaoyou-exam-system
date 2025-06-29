#include "registerwindow.h"

RegisterWindow::RegisterWindow(QWidget *parent):QWidget(parent)
{
    setupUI();
    //连接数据库
    if(!database.connectToDatabase())
    {
        QMessageBox::critical(this,"错误","无法连接到数据库");
    }
}

void RegisterWindow::setupUI()
{
    setWindowTitle("用户注册");
    setFixedSize(450,400);
    QVBoxLayout *mainLayout=new QVBoxLayout();
    setLayout(mainLayout);
    //标题
    QLabel *titleLabel = new QLabel("用户注册");
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

    //姓名输入
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel("姓名:");
    nameEdit = new QLineEdit();
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);

    //年级输入  仅学生显示
    QHBoxLayout *gradeLayout = new QHBoxLayout();
    gradeLabel = new QLabel("年级:");
    gradeEdit = new QLineEdit();
    gradeEdit->setPlaceholderText("如：2023");
    gradeLayout->addWidget(gradeLabel);
    gradeLayout->addWidget(gradeEdit);

    // 学院输入
    QHBoxLayout *collegeLayout = new QHBoxLayout();
    QLabel *collegeLabel = new QLabel("学院:");
    collegeEdit = new QLineEdit();
    collegeLayout->addWidget(collegeLabel);
    collegeLayout->addWidget(collegeEdit);

    // 密码输入
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    QLabel *passwordLabel = new QLabel("密码:");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordEdit);
    //确认密码
    QHBoxLayout *confirmLayout = new QHBoxLayout();
    QLabel *confirmLabel = new QLabel("确认密码:");
    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmLayout->addWidget(confirmLabel);
    confirmLayout->addWidget(confirmPasswordEdit);
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    registerButton = new QPushButton("注册");
    cancelButton = new QPushButton("取消");
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(cancelButton);

    // 添加到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(typeLayout);
    mainLayout->addLayout(userIdLayout);
    mainLayout->addLayout(nameLayout);
    mainLayout->addLayout(gradeLayout);
    mainLayout->addLayout(collegeLayout);
    mainLayout->addLayout(passwordLayout);
    mainLayout->addLayout(confirmLayout);
    mainLayout->addLayout(buttonLayout);

    //连接信号槽
    connect(registerButton,&QPushButton::clicked,this,&RegisterWindow::onRegisterClicked);
    connect(cancelButton,&QPushButton::clicked, this, &RegisterWindow::onCancelClicked);
    connect(studentRadio, &QRadioButton::clicked, this, &RegisterWindow::onUserTypeChanged);
    connect(teacherRadio, &QRadioButton::clicked, this, &RegisterWindow::onUserTypeChanged);

    //初始化界面状态
    onUserTypeChanged();
}


void RegisterWindow::onUserTypeChanged()
{
    bool isStudent = studentRadio->isChecked();//是否是学生
    gradeLabel->setVisible(isStudent);//根据是否是学生决定这两个年级标签是否显示
    gradeEdit->setVisible(isStudent);
}

void RegisterWindow::onRegisterClicked()
{
    // 获取输入数据
    QString userIdText = userIdEdit->text();
    QString name = nameEdit->text();
    QString grade = gradeEdit->text();
    QString college = collegeEdit->text();
    QString password = passwordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    // 基本验证
    if (userIdText.isEmpty() || name.isEmpty() || college.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请填写完整信息");
        return;
    }
    //验证密码是否一致
    if (password != confirmPassword)
    {
        QMessageBox::warning(this, "提示", "两次输入的密码不一致");
        return;
    }

    int userId = userIdText.toInt();//学号转化为数字
    bool isStudent = studentRadio->isChecked();

    // 学生必须填写年级
    if (isStudent && grade.isEmpty())
    {
        QMessageBox::warning(this, "提示", "学生必须填写年级");
        return;
    }

    // 执行注册  调用数据库注册方法
    bool success = false;
    if (isStudent)
    {
        success = database.registerStudent(userId, name, grade, college, password);
    } else {
        success = database.registerTeacher(userId, name, college, password);
    }
    //处理注册结果
    if (success) {
        QMessageBox::information(this, "成功", "注册成功！");
        this->close();
    } else {
        QMessageBox::critical(this, "失败", "注册失败，请检查输入信息");
    }
}

void RegisterWindow::onCancelClicked()
{
    this->close();
}

