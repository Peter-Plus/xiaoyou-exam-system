#include "teachermainwindow.h"
#include "exammanager.h"

TeacherMainWindow::TeacherMainWindow(const Teacher &teacher, Database *database, QWidget *parent)
    : QMainWindow(parent)
    , m_teacher(teacher)
    , m_database(database)
    , m_questionManager(nullptr)
    , m_examManager(nullptr)
    , m_gradingWindow(nullptr)
    , m_statisticsWindow(nullptr)  // 初始化班级成绩统计窗口指针
{
    setupUI();
}

TeacherMainWindow::~TeacherMainWindow()
{
    // 窗口会在适当时候自动删除
}

void TeacherMainWindow::setupUI()
{
    setWindowTitle("在线考试系统 - 教师端");
    setMinimumSize(800, 600);

    // 创建中央窗口
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    setupWelcomeArea();
    setupFunctionArea();
    setupBottomArea();

    // 欢迎区域框架
    QFrame *welcomeFrame = new QFrame();
    welcomeFrame->setFrameStyle(QFrame::Box);
    welcomeFrame->setStyleSheet("QFrame { background-color: #ecf0f1; border-radius: 10px; padding: 20px; }");
    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeFrame);
    welcomeLayout->addWidget(m_welcomeLabel);
    welcomeLayout->addWidget(m_infoLabel);

    // 功能区域框架
    QFrame *functionFrame = new QFrame();
    functionFrame->setFrameStyle(QFrame::Box);
    functionFrame->setStyleSheet("QFrame { background-color: #f8f9fa; border-radius: 10px; padding: 30px; }");
    QGridLayout *gridLayout = new QGridLayout(functionFrame);
    gridLayout->setSpacing(30);
    gridLayout->setAlignment(Qt::AlignCenter);
    gridLayout->addWidget(m_questionMgmtBtn, 0, 0);
    gridLayout->addWidget(m_examMgmtBtn, 0, 1);
    gridLayout->addWidget(m_gradingBtn, 1, 0);
    gridLayout->addWidget(m_scoreAnalysisBtn, 1, 1);

    // 底部区域框架
    QFrame *bottomFrame = new QFrame();
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomFrame);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_logoutBtn);

    mainLayout->addWidget(welcomeFrame);
    mainLayout->addWidget(functionFrame);
    mainLayout->addStretch();
    mainLayout->addWidget(bottomFrame);
}

void TeacherMainWindow::setupWelcomeArea()
{
    // 欢迎信息
    m_welcomeLabel = new QLabel(QString("欢迎您，%1 老师！").arg(m_teacher.getName()));
    m_welcomeLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #2c3e50; }");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);

    // 教师信息
    QString infoText = QString("学院：%1").arg(m_teacher.getCollege());
    m_infoLabel = new QLabel(infoText);
    m_infoLabel->setStyleSheet("QLabel { font-size: 14px; color: #7f8c8d; }");
    m_infoLabel->setAlignment(Qt::AlignCenter);
}

void TeacherMainWindow::setupFunctionArea()
{
    // 创建功能按钮
    m_questionMgmtBtn = createFunctionButton("题库管理", "管理试题、创建题目、题目分类");
    m_examMgmtBtn = createFunctionButton("考试管理", "创建考试、组卷、发布考试");
    m_gradingBtn = createFunctionButton("阅卷", "批改试卷、评分、查看答题情况");
    m_scoreAnalysisBtn = createFunctionButton("班级成绩分析", "班级成绩统计、不及格学生分析");

    // 连接信号槽
    connect(m_questionMgmtBtn, &QPushButton::clicked, this, &TeacherMainWindow::onQuestionManagementClicked);
    connect(m_examMgmtBtn, &QPushButton::clicked, this, &TeacherMainWindow::onExamManagementClicked);
    connect(m_gradingBtn, &QPushButton::clicked, this, &TeacherMainWindow::onGradingClicked);
    connect(m_scoreAnalysisBtn, &QPushButton::clicked, this, &TeacherMainWindow::onScoreAnalysisClicked);
}

void TeacherMainWindow::setupBottomArea()
{
    m_logoutBtn = new QPushButton("退出登录");
    m_logoutBtn->setFixedSize(120, 40);
    m_logoutBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #e74c3c;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #c0392b;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #a93226;"
        "}"
        );

    connect(m_logoutBtn, &QPushButton::clicked, this, &TeacherMainWindow::onLogoutClicked);
}

QPushButton* TeacherMainWindow::createFunctionButton(const QString &text, const QString &description)
{
    QPushButton *button = new QPushButton();
    button->setFixedSize(200, 120);
    button->setText(QString("%1\n\n%2").arg(text, description));
    button->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    text-align: center;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
        );

    return button;
}

// 功能按钮槽函数
void TeacherMainWindow::onQuestionManagementClicked()
{
    // 如果题库管理窗口不存在，创建新的
    if (!m_questionManager) {
        m_questionManager = new QuestionManager(m_teacher.getId(), m_database);

        // 连接窗口关闭信号，清空指针
        connect(m_questionManager, &QObject::destroyed, [this]() {
            m_questionManager = nullptr;
        });
    }

    // 显示题库管理窗口
    m_questionManager->show();
    m_questionManager->raise();
    m_questionManager->activateWindow();
}

void TeacherMainWindow::onExamManagementClicked()
{
    // 如果考试管理窗口不存在，创建新的
    if (!m_examManager) {
        m_examManager = new ExamManager(m_teacher.getId(), m_database);

        // 连接窗口关闭信号，清空指针
        connect(m_examManager, &QObject::destroyed, [this]() {
            m_examManager = nullptr;
        });
    }

    // 显示考试管理窗口
    m_examManager->show();
    m_examManager->raise();
    m_examManager->activateWindow();
}

void TeacherMainWindow::onGradingClicked()
{
    if (!m_database) {
        QMessageBox::warning(this, "错误", "数据库连接异常！");
        return;
    }

    // 如果阅卷窗口不存在，创建新的
    if (!m_gradingWindow) {
        m_gradingWindow = new ExamGrading(m_database, m_teacher.getId());

        // 连接窗口关闭信号，清空指针
        connect(m_gradingWindow, &QObject::destroyed, [this]() {
            m_gradingWindow = nullptr;
        });
    }

    // 显示阅卷窗口
    m_gradingWindow->show();
    m_gradingWindow->raise();
    m_gradingWindow->activateWindow();
}

void TeacherMainWindow::onScoreAnalysisClicked()
{
    if (!m_database) {
        QMessageBox::warning(this, "错误", "数据库连接异常！");
        return;
    }

    // 如果班级成绩统计窗口不存在，创建新的
    if (!m_statisticsWindow) {
        m_statisticsWindow = new ClassStatisticsWindow(m_database, m_teacher.getId());

        // 连接窗口关闭信号，清空指针
        connect(m_statisticsWindow, &QObject::destroyed, [this]() {
            m_statisticsWindow = nullptr;
        });
    }

    // 显示班级成绩统计窗口
    m_statisticsWindow->show();
    m_statisticsWindow->raise();
    m_statisticsWindow->activateWindow();
}

void TeacherMainWindow::onLogoutClicked()
{
    int ret = QMessageBox::question(this, "确认退出",
                                    "确定要退出登录吗？",
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // 关闭题库管理窗口
        if (m_questionManager) {
            m_questionManager->close();
        }

        // 关闭考试管理窗口
        if (m_examManager) {
            m_examManager->close();
        }

        // 关闭阅卷窗口
        if (m_gradingWindow) {
            m_gradingWindow->close();
        }

        // 关闭班级成绩统计窗口
        if (m_statisticsWindow) {
            m_statisticsWindow->close();
        }

        // 发送退出登录信号
        emit logoutRequested();
    }
}
