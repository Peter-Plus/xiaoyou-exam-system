#include "studentmainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QScreen>

StudentMainWindow::StudentMainWindow(const Student &student, Database *database, QWidget *parent)
    : QMainWindow(parent), currentStudent(student), database(database), examListWindow(nullptr), m_friendPage(nullptr), m_chatPage(nullptr)  // 添加m_chatPage(nullptr)
{
    setupUI();
    updateUserInfo();
}

StudentMainWindow::StudentMainWindow(int studentId, QWidget *parent)
    : QMainWindow(parent), database(nullptr), examListWindow(nullptr), m_friendPage(nullptr), m_chatPage(nullptr)  // 添加m_chatPage(nullptr)
{
    currentStudent.setId(studentId);
    setupUI();
    updateUserInfo();
}

void StudentMainWindow::setupUI()
{
    setWindowTitle("学生考试系统 2.0");

    // 设置窗口大小和位置
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = qMin(1200, screenGeometry.width() - 100);
    int height = qMin(800, screenGeometry.height() - 100);
    resize(width, height);

    // 居中显示
    move((screenGeometry.width() - width) / 2, (screenGeometry.height() - height) / 2);

    // 设置最小尺寸
    setMinimumSize(800, 600);

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

    // 设置分割比例 (导航栏:内容区域 = 1:4)
    mainSplitter->setSizes({200, 800});
    mainSplitter->setChildrenCollapsible(false); // 防止面板被完全折叠

    mainLayout->addWidget(mainSplitter);

    // 应用现代化样式
    setModernStyle();

    // 默认显示考试页面
    switchToPage(PAGE_EXAM);
}

void StudentMainWindow::setupHeaderArea()
{
    headerFrame = new QFrame();
    headerFrame->setFixedHeight(60);
    headerFrame->setObjectName("headerFrame");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(15, 10, 15, 10);

    // 用户头像（暂时用标签代替）
    userAvatarLabel = new QLabel();
    userAvatarLabel->setFixedSize(40, 40);
    userAvatarLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #3498db;"
        "    border-radius: 20px;"
        "    color: white;"
        "    font-weight: bold;"
        "    font-size: 16px;"
        "}"
        );
    userAvatarLabel->setAlignment(Qt::AlignCenter);

    // 用户信息
    QVBoxLayout *userInfoLayout = new QVBoxLayout();
    userInfoLayout->setSpacing(2);

    userNameLabel = new QLabel();
    userNameLabel->setObjectName("userNameLabel");

    userInfoLabel = new QLabel();
    userInfoLabel->setObjectName("userInfoLabel");

    userInfoLayout->addWidget(userNameLabel);
    userInfoLayout->addWidget(userInfoLabel);

    // 右侧按钮
    settingsButton = new QPushButton("⚙");
    settingsButton->setFixedSize(30, 30);
    settingsButton->setObjectName("iconButton");

    logoutButton = new QPushButton("退出");
    logoutButton->setObjectName("logoutButton");

    headerLayout->addWidget(userAvatarLabel);
    headerLayout->addLayout(userInfoLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(settingsButton);
    headerLayout->addWidget(logoutButton);

    connect(logoutButton, &QPushButton::clicked, this, &StudentMainWindow::onLogoutClicked);
}

void StudentMainWindow::setupNavigationBar()
{
    navigationFrame = new QFrame();
    navigationFrame->setObjectName("navigationFrame");
    navigationFrame->setFixedWidth(200);

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
        "💬 聊天",      // PAGE_CHAT
        "👥 好友",      // PAGE_FRIEND
        "📚 课程",      // PAGE_COURSE
        "📝 考试",      // PAGE_EXAM
        "📊 成绩",      // PAGE_SCORE
        "⚙️ 设置"       // PAGE_SETTINGS
    };

    for (const QString &item : navItems) {
        QListWidgetItem *listItem = new QListWidgetItem(item);
        listItem->setSizeHint(QSize(200, 50));
        navigationList->addItem(listItem);
    }

    // 设置默认选中项
    navigationList->setCurrentRow(PAGE_EXAM);

    navLayout->addWidget(navigationList);

    connect(navigationList, &QListWidget::itemClicked, this, &StudentMainWindow::onNavigationClicked);
}

void StudentMainWindow::setupContentArea()
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
    createExamPage();
    createScorePage();
    createSettingsPage();
}

void StudentMainWindow::createExamPage()
{
    examPage = new QWidget();
    examPage->setObjectName("examPage");

    QVBoxLayout *layout = new QVBoxLayout(examPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    // 页面标题
    QLabel *titleLabel = new QLabel("考试管理");
    titleLabel->setObjectName("pageTitle");

    // 考试状态显示
    examStatusLabel = new QLabel("正在加载考试信息...");
    examStatusLabel->setObjectName("statusLabel");

    // 开始考试按钮
    startExamButton = new QPushButton("参与考试");
    startExamButton->setObjectName("primaryButton");
    startExamButton->setFixedHeight(50);

    connect(startExamButton, &QPushButton::clicked, [this]() {
        if (!database) {
            QMessageBox::warning(this, "错误", "数据库连接异常！");
            return;
        }

        // 如果考试列表窗口不存在，创建新的
        if (!examListWindow) {
            examListWindow = new StudentExamList(database, currentStudent.getId());
            connect(examListWindow, &QObject::destroyed, [this]() {
                examListWindow = nullptr;
            });
        }

        examListWindow->show();
        examListWindow->raise();
        examListWindow->activateWindow();
    });

    layout->addWidget(titleLabel);
    layout->addWidget(examStatusLabel);
    layout->addWidget(startExamButton);
    layout->addStretch();

    contentStack->addWidget(examPage);
}

void StudentMainWindow::createScorePage()
{
    scorePage = new QWidget();
    scorePage->setObjectName("scorePage");

    QVBoxLayout *layout = new QVBoxLayout(scorePage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    // 页面标题
    QLabel *titleLabel = new QLabel("成绩查询");
    titleLabel->setObjectName("pageTitle");

    // 成绩信息显示
    scoreInfoLabel = new QLabel("点击下方按钮查看考试成绩");
    scoreInfoLabel->setObjectName("statusLabel");

    // 查看成绩按钮
    viewScoreButton = new QPushButton("考试成绩分析");
    viewScoreButton->setObjectName("primaryButton");
    viewScoreButton->setFixedHeight(50);

    connect(viewScoreButton, &QPushButton::clicked, [this]() {
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
                resultWindow->setAttribute(Qt::WA_DeleteOnClose);
                resultWindow->show();
            }
        }
    });

    layout->addWidget(titleLabel);
    layout->addWidget(scoreInfoLabel);
    layout->addWidget(viewScoreButton);
    layout->addStretch();

    contentStack->addWidget(scorePage);
}

void StudentMainWindow::createChatPage()
{
    // 数据库连接检查
    if (!database) {
        // 创建错误提示页面
        chatPage = new QWidget();
        chatPage->setObjectName("chatPage");

        QVBoxLayout *layout = new QVBoxLayout(chatPage);
        layout->setContentsMargins(30, 30, 30, 30);

        QLabel *titleLabel = new QLabel("聊天功能");
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
    m_chatPage = new ChatPage(database, currentStudent.getId(), "学生", this);

    // 连接信号槽
    connect(m_chatPage, &ChatPage::messageSent,
            this, &StudentMainWindow::onMessageSent);
    connect(m_chatPage, &ChatPage::chatOpened,
            this, &StudentMainWindow::onChatOpened);

    // 添加到内容栈
    contentStack->addWidget(m_chatPage);
    chatPage = m_chatPage; // 保持兼容性

    qDebug() << "学生端聊天页面创建成功";
}

void StudentMainWindow::createFriendPage()
{
    // 检查数据库连接是否有效
    if (!database) {
        // 如果没有数据库连接，显示占位页面
        friendPage = new QWidget();
        friendPage->setObjectName("friendPage");

        QVBoxLayout *layout = new QVBoxLayout(friendPage);
        layout->setContentsMargins(30, 30, 30, 30);

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
    m_friendPage = new FriendPage(database, currentStudent.getId(), "学生", this);

    // 连接信号槽
    connect(m_friendPage, &FriendPage::friendAdded, this, &StudentMainWindow::onFriendAdded);
    connect(m_friendPage, &FriendPage::friendRemoved, this, &StudentMainWindow::onFriendRemoved);
    connect(m_friendPage, &FriendPage::requestProcessed, this, &StudentMainWindow::onRequestProcessed);
    // 新增：连接好友双击信号
    connect(m_friendPage, &FriendPage::friendDoubleClicked,
            this, &StudentMainWindow::onFriendDoubleClickedToChat);

    // 添加到内容栈
    contentStack->addWidget(m_friendPage);
    friendPage = m_friendPage; // 保持兼容性
}

void StudentMainWindow::createCoursePage()
{
    coursePage = new QWidget();
    coursePage->setObjectName("coursePage");

    QVBoxLayout *layout = new QVBoxLayout(coursePage);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("课程管理");
    titleLabel->setObjectName("pageTitle");

    QLabel *comingSoonLabel = new QLabel("即将推出...\n敬请期待 2.0 版本的课程管理功能！");
    comingSoonLabel->setObjectName("comingSoonLabel");
    comingSoonLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(comingSoonLabel);
    layout->addStretch();

    contentStack->addWidget(coursePage);
}

void StudentMainWindow::createSettingsPage()
{
    settingsPage = new QWidget();
    settingsPage->setObjectName("settingsPage");

    QVBoxLayout *layout = new QVBoxLayout(settingsPage);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("系统设置");
    titleLabel->setObjectName("pageTitle");

    QLabel *infoLabel = new QLabel("设置功能开发中...");
    infoLabel->setObjectName("comingSoonLabel");
    infoLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(infoLabel);
    layout->addStretch();

    contentStack->addWidget(settingsPage);
}

void StudentMainWindow::switchToPage(int pageIndex)
{
    if (pageIndex >= 0 && pageIndex < contentStack->count()) {
        contentStack->setCurrentIndex(pageIndex);
        navigationList->setCurrentRow(pageIndex);
    }
}

void StudentMainWindow::onNavigationClicked()
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

void StudentMainWindow::updateUserInfo()
{
    if (!currentStudent.getName().isEmpty()) {
        userNameLabel->setText(currentStudent.getName());
        userAvatarLabel->setText(currentStudent.getName().left(1).toUpper());

        QString infoText = "";
        if (!currentStudent.getCollege().isEmpty()) {
            infoText += currentStudent.getCollege();
        }
        if (!currentStudent.getGrade().isEmpty()) {
            if (!infoText.isEmpty()) infoText += " ";
            infoText += currentStudent.getGrade() + "级";
        }
        userInfoLabel->setText(infoText);
    } else {
        userNameLabel->setText(QString("学号：%1").arg(currentStudent.getId()));
        userAvatarLabel->setText("S");
        userInfoLabel->setText("学生用户");
    }
}

void StudentMainWindow::onLogoutClicked()
{
    // 关闭考试列表窗口
    if (examListWindow) {
        examListWindow->close();
    }

    // 友管理页面会随主窗口自动销毁，无需手动关闭

    emit logoutRequested();
    this->close();
}

void StudentMainWindow::setModernStyle()
{
    setStyleSheet(
        // 主窗口样式
        "QMainWindow {"
        "    background-color: #f5f5f5;"
        "}"

        // 头部区域样式
        "#headerFrame {"
        "    background-color: #3498db;"
        "    border: none;"
        "}"
        "#userNameLabel {"
        "    color: white;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "#userInfoLabel {"
        "    color: rgba(255, 255, 255, 0.8);"
        "    font-size: 12px;"
        "}"
        "#iconButton {"
        "    background-color: rgba(255, 255, 255, 0.2);"
        "    border: none;"
        "    border-radius: 15px;"
        "    color: white;"
        "    font-size: 14px;"
        "}"
        "#iconButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.3);"
        "}"
        "#logoutButton {"
        "    background-color: #e74c3c;"
        "    border: none;"
        "    border-radius: 5px;"
        "    color: white;"
        "    padding: 5px 15px;"
        "    font-size: 12px;"
        "}"
        "#logoutButton:hover {"
        "    background-color: #c0392b;"
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
        "    padding: 15px 20px;"
        "    border-bottom: 1px solid #34495e;"
        "    font-size: 14px;"
        "}"
        "#navigationList::item:selected {"
        "    background-color: #3498db;"
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
        "    font-size: 24px;"
        "    font-weight: bold;"
        "    margin-bottom: 20px;"
        "}"
        "#statusLabel {"
        "    color: #7f8c8d;"
        "    font-size: 14px;"
        "    margin-bottom: 20px;"
        "}"
        "#primaryButton {"
        "    background-color: #3498db;"
        "    border: none;"
        "    border-radius: 8px;"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 15px 30px;"
        "}"
        "#primaryButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "#primaryButton:pressed {"
        "    background-color: #21618c;"
        "}"
        "#comingSoonLabel {"
        "    color: #95a5a6;"
        "    font-size: 16px;"
        "    font-style: italic;"
        "}"

        // 分割器样式
        "QSplitter::handle {"
        "    background-color: #bdc3c7;"
        "    width: 1px;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #3498db;"
        "}"
        );
}

void StudentMainWindow::onFriendAdded(int friendId, const QString &friendType)
{
    Q_UNUSED(friendId)
    Q_UNUSED(friendType)
    qDebug() << "学生端：新好友添加成功";

    // 可以在这里添加其他相关界面的刷新逻辑
    // 例如刷新聊天列表等（为2.0后续功能预留）
}

void StudentMainWindow::onFriendRemoved(int friendId, const QString &friendType)
{
    Q_UNUSED(friendId)
    Q_UNUSED(friendType)
    qDebug() << "学生端：好友删除成功";

    // 可以在这里添加其他相关界面的刷新逻辑
}

void StudentMainWindow::onRequestProcessed()
{
    qDebug() << "学生端：好友申请处理完成";

    // 可以在这里添加通知逻辑
    // 例如显示系统通知或更新状态栏
}

void StudentMainWindow::onMessageSent(int chatId)
{
    qDebug() << "学生端收到消息发送完成信号:" << chatId;
    // 可以在这里添加额外的处理逻辑，比如更新通知等
}

void StudentMainWindow::onChatOpened(int friendId, const QString &friendName)
{
    qDebug() << "学生端打开与" << friendName << "的聊天";
    // 可以在这里添加额外的处理逻辑
}

void StudentMainWindow::onFriendDoubleClickedToChat(int friendId, const QString &friendType, const QString &friendName)
{
    qDebug() << "学生端收到好友双击信号：" << friendName << "(" << friendType << ")" << "ID:" << friendId;

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

    // 5. 显示用户反馈
    QMessageBox::information(this, "聊天已开启",
                             QString("已为您打开与 %1 的聊天窗口").arg(friendName));

    qDebug() << "好友双击切换聊天功能执行完成";
}
