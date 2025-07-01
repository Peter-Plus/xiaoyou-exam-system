#include "teachermainwindow.h"

TeacherMainWindow::TeacherMainWindow(const Teacher &teacher, Database *database, QWidget *parent)
    : QMainWindow(parent)
    , m_teacher(teacher)
    , m_database(database)
    , m_questionManager(nullptr)
    , m_examManager(nullptr)
    , m_gradingWindow(nullptr)
    , m_statisticsWindow(nullptr)
    , m_friendPage(nullptr)
    , m_chatPage(nullptr)  // 添加这一行
{
    setupUI();
    updateUserInfo();
}

TeacherMainWindow::~TeacherMainWindow()
{
    // 子窗口会在适当时候自动删除
}

void TeacherMainWindow::setupUI()
{
    setWindowTitle("在线考试系统 - 教师端 2.0");

    // 设置窗口大小和位置
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = qMin(1400, screenGeometry.width() - 100);
    int height = qMin(900, screenGeometry.height() - 100);
    resize(width, height);

    // 居中显示
    move((screenGeometry.width() - width) / 2, (screenGeometry.height() - height) / 2);

    // 设置最小尺寸
    setMinimumSize(1000, 700);

    setupHeaderArea();
    setupNavigationBar();
    setupContentArea();

    // 创建主布局
    centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建分割器
    mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->addWidget(navigationFrame);
    mainSplitter->addWidget(contentFrame);

    // 设置分割比例 (导航栏:内容区域 = 1:5)
    mainSplitter->setSizes({220, 1100});
    mainSplitter->setChildrenCollapsible(false);

    mainLayout->addWidget(mainSplitter);

    // 应用现代化样式
    setModernStyle();

    // 默认显示题库管理页面
    switchToPage(PAGE_QUESTION);
}

void TeacherMainWindow::setupHeaderArea()
{
    headerFrame = new QFrame();
    headerFrame->setFixedHeight(70);
    headerFrame->setObjectName("headerFrame");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(15, 10, 15, 10);

    // 用户头像
    userAvatarLabel = new QLabel();
    userAvatarLabel->setFixedSize(45, 45);
    userAvatarLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #e74c3c;"
        "    border-radius: 22px;"
        "    color: white;"
        "    font-weight: bold;"
        "    font-size: 18px;"
        "}"
        );
    userAvatarLabel->setAlignment(Qt::AlignCenter);

    // 用户信息
    QVBoxLayout *userInfoLayout = new QVBoxLayout();
    userInfoLayout->setSpacing(3);

    userNameLabel = new QLabel();
    userNameLabel->setObjectName("userNameLabel");

    userInfoLabel = new QLabel();
    userInfoLabel->setObjectName("userInfoLabel");

    userInfoLayout->addWidget(userNameLabel);
    userInfoLayout->addWidget(userInfoLabel);

    // 右侧按钮
    settingsButton = new QPushButton("⚙");
    settingsButton->setFixedSize(35, 35);
    settingsButton->setObjectName("iconButton");

    logoutButton = new QPushButton("退出登录");
    logoutButton->setObjectName("logoutButton");

    headerLayout->addWidget(userAvatarLabel);
    headerLayout->addLayout(userInfoLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(settingsButton);
    headerLayout->addWidget(logoutButton);

    connect(logoutButton, &QPushButton::clicked, this, &TeacherMainWindow::onLogoutClicked);
}

void TeacherMainWindow::setupNavigationBar()
{
    navigationFrame = new QFrame();
    navigationFrame->setObjectName("navigationFrame");
    navigationFrame->setFixedWidth(220);

    navLayout = new QVBoxLayout(navigationFrame);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(0);

    // 添加顶部用户信息
    navLayout->addWidget(headerFrame);

    // 创建导航列表
    navigationList = new QListWidget();
    navigationList->setObjectName("navigationList");

    // 添加导航项
    QStringList navItems = {
        "💬 聊天",          // PAGE_CHAT
        "👥 好友",          // PAGE_FRIEND
        "📚 课程",          // PAGE_COURSE
        "📝 题库管理",      // PAGE_QUESTION
        "📋 考试管理",      // PAGE_EXAM
        "✏️ 阅卷",          // PAGE_GRADING
        "📊 成绩统计",      // PAGE_STATISTICS
        "⚙️ 设置"           // PAGE_SETTINGS
    };

    for (const QString &item : navItems) {
        QListWidgetItem *listItem = new QListWidgetItem(item);
        listItem->setSizeHint(QSize(220, 55));
        navigationList->addItem(listItem);
    }

    // 设置默认选中项
    navigationList->setCurrentRow(PAGE_QUESTION);

    navLayout->addWidget(navigationList);

    connect(navigationList, &QListWidget::itemClicked, this, &TeacherMainWindow::onNavigationClicked);
}

void TeacherMainWindow::setupContentArea()
{
    contentFrame = new QFrame();
    contentFrame->setObjectName("contentFrame");

    QVBoxLayout *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // 创建堆叠窗口
    contentStack = new QStackedWidget();
    contentLayout->addWidget(contentStack);

    // 创建各个页面
    createChatPage();
    createFriendPage();
    createCoursePage();
    createQuestionPage();
    createExamPage();
    createGradingPage();
    createStatisticsPage();
    createSettingsPage();
}

QPushButton* TeacherMainWindow::createFeatureCard(const QString &title, const QString &subtitle, const QString &buttonText)
{
    QPushButton *card = new QPushButton();
    card->setObjectName("featureCard");
    card->setFixedSize(280, 160);

    // 使用简单的文本格式，通过样式表控制外观
    QString cardText = QString("%1\n\n%2\n\n%3").arg(title, subtitle, buttonText);
    card->setText(cardText);

    return card;
}

void TeacherMainWindow::createQuestionPage()
{
    questionPage = new QWidget();
    questionPage->setObjectName("questionPage");

    QVBoxLayout *layout = new QVBoxLayout(questionPage);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(30);

    // 页面标题
    QLabel *titleLabel = new QLabel("题库管理");
    titleLabel->setObjectName("pageTitle");

    // 功能卡片
    QPushButton *questionCard = createFeatureCard(
        "题库管理",
        "创建题目、管理题库、题目分类\n支持多种题型和智能搜索",
        "进入题库管理"
        );

    connect(questionCard, &QPushButton::clicked, this, &TeacherMainWindow::onQuestionManagementClicked);

    // 布局
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->addWidget(questionCard);
    cardLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(cardLayout);
    layout->addStretch();

    contentStack->addWidget(questionPage);
}

void TeacherMainWindow::createExamPage()
{
    examPage = new QWidget();
    examPage->setObjectName("examPage");

    QVBoxLayout *layout = new QVBoxLayout(examPage);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(30);

    // 页面标题
    QLabel *titleLabel = new QLabel("考试管理");
    titleLabel->setObjectName("pageTitle");

    // 功能卡片
    QPushButton *examCard = createFeatureCard(
        "考试管理",
        "创建考试、智能组卷、发布考试\n支持权限分享和状态管理",
        "进入考试管理"
        );

    connect(examCard, &QPushButton::clicked, this, &TeacherMainWindow::onExamManagementClicked);

    // 布局
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->addWidget(examCard);
    cardLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(cardLayout);
    layout->addStretch();

    contentStack->addWidget(examPage);
}

void TeacherMainWindow::createGradingPage()
{
    gradingPage = new QWidget();
    gradingPage->setObjectName("gradingPage");

    QVBoxLayout *layout = new QVBoxLayout(gradingPage);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(30);

    // 页面标题
    QLabel *titleLabel = new QLabel("阅卷管理");
    titleLabel->setObjectName("pageTitle");

    // 功能卡片
    QPushButton *gradingCard = createFeatureCard(
        "智能阅卷",
        "自动批改客观题、手工批改主观题\n实时跟踪阅卷进度",
        "开始阅卷"
        );

    connect(gradingCard, &QPushButton::clicked, this, &TeacherMainWindow::onGradingClicked);

    // 布局
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->addWidget(gradingCard);
    cardLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(cardLayout);
    layout->addStretch();

    contentStack->addWidget(gradingPage);
}

void TeacherMainWindow::createStatisticsPage()
{
    statisticsPage = new QWidget();
    statisticsPage->setObjectName("statisticsPage");

    QVBoxLayout *layout = new QVBoxLayout(statisticsPage);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(30);

    // 页面标题
    QLabel *titleLabel = new QLabel("成绩统计");
    titleLabel->setObjectName("pageTitle");

    // 功能卡片
    QPushButton *statsCard = createFeatureCard(
        "班级成绩分析",
        "班级统计、不及格分析、成绩导出\n支持多维度数据分析",
        "查看统计"
        );

    connect(statsCard, &QPushButton::clicked, this, &TeacherMainWindow::onScoreAnalysisClicked);

    // 布局
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->addWidget(statsCard);
    cardLayout->addStretch();

    layout->addWidget(titleLabel);
    layout->addLayout(cardLayout);
    layout->addStretch();

    contentStack->addWidget(statisticsPage);
}

void TeacherMainWindow::createChatPage()
{
    // 数据库连接检查
    if (!m_database) {
        // 创建错误提示页面
        chatPage = new QWidget();
        chatPage->setObjectName("chatPage");

        QVBoxLayout *layout = new QVBoxLayout(chatPage);
        layout->setContentsMargins(40, 40, 40, 40);

        QLabel *titleLabel = new QLabel("教师聊天");
        titleLabel->setObjectName("pageTitle");

        QLabel *errorLabel = new QLabel("聊天功能暂时不可用\n请检查数据库连接");
        errorLabel->setObjectName("comingSoonLabel");
        errorLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleLabel);
        layout->addStretch();
        layout->addWidget(errorLabel);
        layout->addStretch();

        contentStack->addWidget(chatPage);
        qDebug() << "聊天功能不可用：数据库连接无效";
        return;
    }

    // 创建真正的聊天页面
    m_chatPage = new ChatPage(m_database, m_teacher.getId(), "老师", this);

    // 连接信号槽
    connect(m_chatPage, &ChatPage::messageSent,
            this, &TeacherMainWindow::onMessageSent);
    connect(m_chatPage, &ChatPage::chatOpened,
            this, &TeacherMainWindow::onChatOpened);

    // 添加到内容栈
    contentStack->addWidget(m_chatPage);
    chatPage = m_chatPage; // 保持兼容性

    qDebug() << "教师端聊天页面创建成功";
}

void TeacherMainWindow::createFriendPage()
{
    // 检查数据库连接是否有效
    if (!m_database) {
        // 如果没有数据库连接，显示占位页面
        friendPage = new QWidget();
        friendPage->setObjectName("friendPage");

        QVBoxLayout *layout = new QVBoxLayout(friendPage);
        layout->setContentsMargins(40, 40, 40, 40);

        QLabel *titleLabel = new QLabel("好友管理");
        titleLabel->setObjectName("pageTitle");

        QLabel *errorLabel = new QLabel("数据库连接异常\n请重新登录后使用好友功能");
        errorLabel->setObjectName("comingSoonLabel");
        errorLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleLabel);
        layout->addStretch();
        layout->addWidget(errorLabel);
        layout->addStretch();

        contentStack->addWidget(friendPage);
        return;
    }

    // 创建真正的好友管理页面
    m_friendPage = new FriendPage(m_database, m_teacher.getId(), "老师", this);

    // 连接信号槽
    connect(m_friendPage, &FriendPage::friendAdded, this, &TeacherMainWindow::onFriendAdded);
    connect(m_friendPage, &FriendPage::friendRemoved, this, &TeacherMainWindow::onFriendRemoved);
    connect(m_friendPage, &FriendPage::requestProcessed, this, &TeacherMainWindow::onRequestProcessed);
    // 新增：连接好友双击信号
    connect(m_friendPage, &FriendPage::friendDoubleClicked,
            this, &TeacherMainWindow::onFriendDoubleClickedToChat);

    // 添加到内容栈
    contentStack->addWidget(m_friendPage);
    friendPage = m_friendPage; // 保持兼容性
}

void TeacherMainWindow::createCoursePage()
{
    // 检查数据库连接
    if (!m_database) {
        // 创建错误提示页面
        coursePage = new QWidget();
        coursePage->setObjectName("coursePage");

        QVBoxLayout *layout = new QVBoxLayout(coursePage);
        layout->setContentsMargins(40, 40, 40, 40);

        QLabel *titleLabel = new QLabel("课程管理");
        titleLabel->setObjectName("pageTitle");

        QLabel *errorLabel = new QLabel("课程管理功能暂时不可用\n请检查数据库连接");
        errorLabel->setObjectName("comingSoonLabel");
        errorLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleLabel);
        layout->addStretch();
        layout->addWidget(errorLabel);
        layout->addStretch();

        contentStack->addWidget(coursePage);
        qDebug() << "课程管理功能不可用：数据库连接无效";
        return;
    }

    // 创建真正的课程管理页面
    m_coursePage = new CoursePage(m_database, m_teacher.getId(), CoursePage::TEACHER, this);

    // 连接信号槽
    connect(m_coursePage, &CoursePage::courseEnrolled,
            this, &TeacherMainWindow::onCourseEnrolled);
    connect(m_coursePage, &CoursePage::enrollmentProcessed,
            this, &TeacherMainWindow::onEnrollmentProcessed);
    connect(m_coursePage, &CoursePage::noticePublished,
            this, &TeacherMainWindow::onNoticePublished);
    connect(m_coursePage, &CoursePage::assignmentPublished,
            this, &TeacherMainWindow::onAssignmentPublished);

    // 添加到内容栈
    contentStack->addWidget(m_coursePage);
    coursePage = m_coursePage; // 保持兼容性

    qDebug() << "教师端课程管理页面创建成功";
}

void TeacherMainWindow::createSettingsPage()
{
    settingsPage = new QWidget();
    settingsPage->setObjectName("settingsPage");

    QVBoxLayout *layout = new QVBoxLayout(settingsPage);
    layout->setContentsMargins(40, 40, 40, 40);

    QLabel *titleLabel = new QLabel("系统设置");
    titleLabel->setObjectName("pageTitle");

    QLabel *infoLabel = new QLabel("教师端设置功能开发中...\n包括：个人信息设置、权限管理、系统偏好");
    infoLabel->setObjectName("comingSoonLabel");
    infoLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(infoLabel);
    layout->addStretch();

    contentStack->addWidget(settingsPage);
}

void TeacherMainWindow::switchToPage(int pageIndex)
{
    if (pageIndex >= 0 && pageIndex < contentStack->count()) {
        contentStack->setCurrentIndex(pageIndex);
        navigationList->setCurrentRow(pageIndex);
    }
}

void TeacherMainWindow::onNavigationClicked()
{
    int currentRow = navigationList->currentRow();

    // 特殊处理聊天页面
    if (currentRow == PAGE_CHAT && m_chatPage) {
        // 刷新聊天数据
        m_chatPage->refreshChatList();
    }

    // 特殊处理好友页面
    if (currentRow == PAGE_FRIEND && m_friendPage) {
        // 刷新好友数据
        m_friendPage->refreshAll();
    }

    switchToPage(currentRow);
}

void TeacherMainWindow::updateUserInfo()
{
    userNameLabel->setText(QString("%1 老师").arg(m_teacher.getName()));
    userAvatarLabel->setText(m_teacher.getName().left(1).toUpper());

    QString infoText = QString("学院：%1").arg(m_teacher.getCollege());
    userInfoLabel->setText(infoText);
}

// 功能按钮槽函数（保持原有逻辑）
void TeacherMainWindow::onQuestionManagementClicked()
{
    if (!m_questionManager) {
        m_questionManager = new QuestionManager(m_teacher.getId(), m_database);
        connect(m_questionManager, &QObject::destroyed, [this]() {
            m_questionManager = nullptr;
        });
    }

    m_questionManager->show();
    m_questionManager->raise();
    m_questionManager->activateWindow();
}

void TeacherMainWindow::onExamManagementClicked()
{
    if (!m_examManager) {
        m_examManager = new ExamManager(m_teacher.getId(), m_database);
        connect(m_examManager, &QObject::destroyed, [this]() {
            m_examManager = nullptr;
        });
    }

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

    if (!m_gradingWindow) {
        m_gradingWindow = new ExamGrading(m_database, m_teacher.getId());
        connect(m_gradingWindow, &QObject::destroyed, [this]() {
            m_gradingWindow = nullptr;
        });
    }

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

    if (!m_statisticsWindow) {
        m_statisticsWindow = new ClassStatisticsWindow(m_database, m_teacher.getId());
        connect(m_statisticsWindow, &QObject::destroyed, [this]() {
            m_statisticsWindow = nullptr;
        });
    }

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
        // 关闭所有子窗口
        if (m_questionManager) m_questionManager->close();
        if (m_examManager) m_examManager->close();
        if (m_gradingWindow) m_gradingWindow->close();
        if (m_statisticsWindow) m_statisticsWindow->close();

        // 好友管理页面会随主窗口自动销毁，无需手动关闭

        emit logoutRequested();
    }
}

void TeacherMainWindow::setModernStyle()
{
    setStyleSheet(
        // 主窗口样式
        "QMainWindow {"
        "    background-color: #f5f5f5;"
        "}"

        // 头部区域样式（教师版用红色系）
        "#headerFrame {"
        "    background-color: #e74c3c;"
        "    border: none;"
        "}"
        "#userNameLabel {"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        "#userInfoLabel {"
        "    color: rgba(255, 255, 255, 0.9);"
        "    font-size: 13px;"
        "}"
        "#iconButton {"
        "    background-color: rgba(255, 255, 255, 0.2);"
        "    border: none;"
        "    border-radius: 17px;"
        "    color: white;"
        "    font-size: 16px;"
        "}"
        "#iconButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.3);"
        "}"
        "#logoutButton {"
        "    background-color: #c0392b;"
        "    border: none;"
        "    border-radius: 6px;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: bold;"
        "}"
        "#logoutButton:hover {"
        "    background-color: #a93226;"
        "}"

        // 导航栏样式
        "#navigationFrame {"
        "    background-color: #2c3e50;"
        "    border-right: 1px solid #34495e;"
        "}"
        "#navigationList {"
        "    background-color: transparent;"
        "    border: none;"
        "    outline: none;"
        "}"
        "#navigationList::item {"
        "    color: #ecf0f1;"
        "    padding: 18px 25px;"
        "    border-bottom: 1px solid #34495e;"
        "    font-size: 15px;"
        "    font-weight: 500;"
        "}"
        "#navigationList::item:selected {"
        "    background-color: #e74c3c;"
        "    color: white;"
        "}"
        "#navigationList::item:hover {"
        "    background-color: #34495e;"
        "}"

        // 内容区域样式
        "#contentFrame {"
        "    background-color: white;"
        "    border: none;"
        "}"
        "#pageTitle {"
        "    color: #2c3e50;"
        "    font-size: 28px;"
        "    font-weight: bold;"
        "    margin-bottom: 30px;"
        "}"
        "#comingSoonLabel {"
        "    color: #95a5a6;"
        "    font-size: 18px;"
        "    font-style: italic;"
        "    line-height: 1.6;"
        "}"

        // 功能卡片样式
        "#featureCard {"
        "    background-color: white;"
        "    border: 2px solid #ecf0f1;"
        "    border-radius: 15px;"
        "    text-align: center;"
        "    padding: 20px;"
        "    font-size: 14px;"
        "    color: #2c3e50;"
        "    line-height: 1.4;"
        "}"
        "#featureCard:hover {"
        "    border-color: #3498db;"
        "    background-color: #f8f9fa;"
        "    color: #2980b9;"
        "}"
        "#featureCard:pressed {"
        "    background-color: #e9ecef;"
        "    border-color: #2980b9;"
        "}"

        // 分割器样式
        "QSplitter::handle {"
        "    background-color: #bdc3c7;"
        "    width: 1px;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #e74c3c;"
        "}"
        );
}

void TeacherMainWindow::onFriendAdded(int friendId, const QString &friendType)
{
    Q_UNUSED(friendId)
    Q_UNUSED(friendType)
    qDebug() << "教师端：新好友添加成功";

    // 可以在这里添加其他相关界面的刷新逻辑
    // 例如更新学生列表、刷新聊天列表等（为2.0后续功能预留）
}

void TeacherMainWindow::onFriendRemoved(int friendId, const QString &friendType)
{
    Q_UNUSED(friendId)
    Q_UNUSED(friendType)
    qDebug() << "教师端：好友删除成功";

    // 可以在这里添加其他相关界面的刷新逻辑
}

void TeacherMainWindow::onRequestProcessed()
{
    qDebug() << "教师端：好友申请处理完成";

    // 可以在这里添加通知逻辑
    // 例如显示系统通知或更新状态栏
}

void TeacherMainWindow::onMessageSent(int chatId)
{
    qDebug() << "教师端收到消息发送完成信号:" << chatId;
    // 可以在这里添加额外的处理逻辑
}

void TeacherMainWindow::onChatOpened(int friendId, const QString &friendName)
{
    qDebug() << "教师端打开与" << friendName << "的聊天";
    // 可以在这里添加额外的处理逻辑
}

// 新增：好友双击切换聊天的槽函数实现
void TeacherMainWindow::onFriendDoubleClickedToChat(int friendId, const QString &friendType, const QString &friendName)
{
    qDebug() << "教师端收到好友双击信号：" << friendName << "(" << friendType << ")" << "ID:" << friendId;

    // 1. 确保聊天页面已创建
    if (!m_chatPage) {
        qDebug() << "聊天页面未创建，正在创建...";
        createChatPage();

        // 如果创建失败，显示错误
        if (!m_chatPage) {
            QMessageBox::critical(this, "错误", "无法创建聊天页面，请检查系统设置");
            return;
        }
    }

    // 2. 切换到聊天页面
    qDebug() << "切换到聊天页面";
    contentStack->setCurrentWidget(m_chatPage);

    // 3. 更新导航栏选中状态
    navigationList->setCurrentRow(PAGE_CHAT);

    // 4. 启动与指定好友的聊天
    qDebug() << "启动与好友的聊天：" << friendName;
    m_chatPage->startChatFromFriendList(friendId, friendType, friendName);

    // 5. 显示用户反馈（可选）
    // QMessageBox::information(this, "聊天已开启",
    //                         QString("已为您打开与 %1 的聊天窗口").arg(friendName));

    qDebug() << "教师端好友双击切换聊天功能执行完成";
}

// 槽函数实现
void TeacherMainWindow::onCourseEnrolled(int courseId)
{
    Q_UNUSED(courseId)
    qDebug() << "教师端：收到选课事件";
}

void TeacherMainWindow::onEnrollmentProcessed(int studentId, int courseId, bool approved)
{
    Q_UNUSED(studentId)
    Q_UNUSED(courseId)
    QString action = approved ? "批准" : "拒绝";
    qDebug() << "教师端：选课申请已" << action;
    // 可以显示操作结果通知
}

void TeacherMainWindow::onNoticePublished(int courseId, const QString &title)
{
    Q_UNUSED(courseId)
    qDebug() << "教师端：通知发布成功:" << title;
    // 可以显示发布成功提示
}

void TeacherMainWindow::onAssignmentPublished(int courseId, const QString &title)
{
    Q_UNUSED(courseId)
    qDebug() << "教师端：作业发布成功:" << title;
    // 可以显示发布成功提示
}

// 在导航点击事件中添加课程页面的特殊处理
void TeacherMainWindow::onNavigationClicked()
{
    int currentRow = navigationList->currentRow();

    // 特殊处理聊天页面
    if (currentRow == PAGE_CHAT && m_chatPage) {
        m_chatPage->refreshChatList();
    }

    // 特殊处理好友页面
    if (currentRow == PAGE_FRIEND && m_friendPage) {
        m_friendPage->refreshAll();
    }

    // 特殊处理课程页面
    if (currentRow == PAGE_COURSE && m_coursePage) {
        m_coursePage->refreshData();
    }

    switchToPage(currentRow);
}
