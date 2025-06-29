#include "studentmainwindow.h"
#include <QMessageBox>
#include <QInputDialog>

// 新的构造函数：接受Student对象和Database
StudentMainWindow::StudentMainWindow(const Student &student, Database *database, QWidget *parent)
    : QMainWindow(parent), currentStudent(student), database(database), examListWindow(nullptr)
{
    setupUI();
    loadStudentInfo();
}

// 原构造函数：保持兼容性
StudentMainWindow::StudentMainWindow(int studentId, QWidget *parent)
    : QMainWindow(parent), database(nullptr), examListWindow(nullptr)
{
    // 设置当前学生ID
    currentStudent.setId(studentId);
    setupUI();
    loadStudentInfo();
}

void StudentMainWindow::setupUI()
{
    setWindowTitle("学生考试系统");
    setFixedSize(400, 300);
    // 创建中央窗口部件
    QWidget *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    centralWidget->setLayout(mainLayout);

    // 欢迎标签
    welcomeLabel = new QLabel("欢迎使用考试系统");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size:16px;font-weight:bold;margin:20px;");

    // 功能按钮
    examButton = new QPushButton("参与考试");
    examButton->setFixedHeight(50);
    examButton->setStyleSheet("font-size:14px");

    scoreButton = new QPushButton("考试成绩分析");
    scoreButton->setFixedHeight(50);
    scoreButton->setStyleSheet("font-size:14px");

    logoutButton = new QPushButton("退出登录");
    logoutButton->setFixedHeight(40);
    logoutButton->setStyleSheet("font-size: 12px;");

    // 添加布局
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addWidget(examButton);
    mainLayout->addWidget(scoreButton);
    mainLayout->addStretch(); // 添加弹性空间
    mainLayout->addWidget(logoutButton);

    // 连接信号槽
    connect(examButton, &QPushButton::clicked, this, &StudentMainWindow::onExamClicked);
    connect(scoreButton, &QPushButton::clicked, this, &StudentMainWindow::onScoreClicked);
    connect(logoutButton, &QPushButton::clicked, this, &StudentMainWindow::onLogoutClicked);
}

// 加载学生信息
void StudentMainWindow::loadStudentInfo()
{
    QString welcomeText;

    // 如果学生有姓名信息，显示姓名；否则显示学号
    if (!currentStudent.getName().isEmpty()) {
        welcomeText = QString("欢迎，%1 同学").arg(currentStudent.getName());

        // 如果还有学院和年级信息，也显示出来
        QString infoText = "";
        if (!currentStudent.getCollege().isEmpty()) {
            infoText += currentStudent.getCollege();
        }
        if (!currentStudent.getGrade().isEmpty()) {
            if (!infoText.isEmpty()) infoText += " ";
            infoText += currentStudent.getGrade() + "级";
        }

        if (!infoText.isEmpty()) {
            welcomeText += QString("\n%1").arg(infoText);
        }
    } else {
        welcomeText = QString("欢迎，学号：%1").arg(currentStudent.getId());
    }

    welcomeLabel->setText(welcomeText);
}

// 参与考试功能槽函数
void StudentMainWindow::onExamClicked()
{
    if (!database) {
        QMessageBox::warning(this, "错误", "数据库连接异常！");
        return;
    }

    // 如果考试列表窗口不存在，创建新的
    if (!examListWindow) {
        examListWindow = new StudentExamList(database, currentStudent.getId());

        // 连接窗口关闭信号，清空指针
        connect(examListWindow, &QObject::destroyed, [this]() {
            examListWindow = nullptr;
        });
    }

    // 显示考试列表窗口
    examListWindow->show();
    examListWindow->raise();
    examListWindow->activateWindow();
}

// 考试成绩分析功能槽函数
void StudentMainWindow::onScoreClicked()
{
    if (!database) {
        QMessageBox::warning(this, "错误", "数据库连接异常！");
        return;
    }

    // 获取学生的考试历史
    QList<Exam> examHistory = database->getStudentExamHistory(currentStudent.getId());

    if (examHistory.isEmpty()) {
        QMessageBox::information(this, "提示", "您还没有参加过任何考试！");
        return;
    }

    // 创建选择考试的对话框
    QStringList examNames;
    QList<int> examIds;

    for (const Exam &exam : examHistory) {
        examNames.append(QString("%1 - %2").arg(exam.getExamName()).arg(exam.getCourseName()));
        examIds.append(exam.getExamId());
    }

    bool ok;
    QString selectedExam = QInputDialog::getItem(this, "选择考试",
                                                 "请选择要查看成绩的考试：",
                                                 examNames, 0, false, &ok);

    if (ok && !selectedExam.isEmpty()) {
        int selectedIndex = examNames.indexOf(selectedExam);
        if (selectedIndex >= 0) {
            int examId = examIds[selectedIndex];

            // 创建并显示成绩查看窗口
            ExamResult *resultWindow = new ExamResult(database, examId, currentStudent.getId());
            resultWindow->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
            resultWindow->show();
        }
    }
}

// 退出登录信号
void StudentMainWindow::onLogoutClicked()
{
    // 关闭考试列表窗口
    if (examListWindow) {
        examListWindow->close();
    }

    emit logoutRequested();
    this->close();
}
