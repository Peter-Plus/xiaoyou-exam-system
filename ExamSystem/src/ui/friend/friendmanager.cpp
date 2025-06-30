#include "friendmanager.h"
#include <QApplication>
#include <QStyleOption>
#include <QPainter>

FriendManager::FriendManager(Database *database, int currentUserId, const QString &currentUserType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_currentUserId(currentUserId)
    , m_currentUserType(currentUserType)
    , m_friendCount(0)
    , m_pendingRequestCount(0)
{
    setWindowTitle("好友管理");
    setMinimumSize(900, 700);

    initUI();

    // 设置定时刷新（每30秒刷新一次请求状态）
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &FriendManager::refreshRequests);
    m_refreshTimer->start(30000); // 30秒

    // 初始加载数据
    refreshData();
}

FriendManager::~FriendManager()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
    }
}

void FriendManager::initUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    // 创建标签页控件
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setObjectName("FriendManagerTabs");

    // 设置标签页
    setupFriendsTab();
    setupAddFriendTab();
    setupRequestsTab();

    // 仅为学生显示班级成员标签页
    if (m_currentUserType == "学生") {
        setupClassmatesTab();
    }

    m_mainLayout->addWidget(m_tabWidget);

    // 连接标签页切换信号
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &FriendManager::onTabChanged);

    // 设置样式
    setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #cccccc;
            background-color: white;
        }

        QTabWidget::tab-bar {
            alignment: center;
        }

        QTabBar::tab {
            background: #f0f0f0;
            border: 1px solid #cccccc;
            padding: 8px 16px;
            margin-right: 2px;
        }

        QTabBar::tab:selected {
            background: #3498db;
            color: white;
        }

        QTabBar::tab:hover {
            background: #e3f2fd;
        }

        QListWidget {
            border: 1px solid #ddd;
            background-color: white;
            alternate-background-color: #f9f9f9;
        }

        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #eee;
        }

        QListWidget::item:selected {
            background-color: #3498db;
            color: white;
        }

        QListWidget::item:hover {
            background-color: #e3f2fd;
        }

        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #2980b9;
        }

        QPushButton:pressed {
            background-color: #21618c;
        }

        QPushButton:disabled {
            background-color: #bdc3c7;
            color: #7f8c8d;
        }

        QLineEdit {
            border: 2px solid #ddd;
            border-radius: 4px;
            padding: 8px;
            font-size: 14px;
        }

        QLineEdit:focus {
            border-color: #3498db;
        }

        QComboBox {
            border: 2px solid #ddd;
            border-radius: 4px;
            padding: 6px;
            min-width: 120px;
        }

        QComboBox:focus {
            border-color: #3498db;
        }

        QLabel {
            color: #2c3e50;
            font-weight: bold;
        }

        QGroupBox {
            font-weight: bold;
            border: 2px solid #ddd;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
        }
    )");
}

void FriendManager::setupFriendsTab()
{
    m_friendsTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_friendsTab);

    // 顶部信息栏
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_friendsCountLabel = new QLabel("好友: 0人");
    m_friendsCountLabel->setStyleSheet("font-size: 16px; color: #2c3e50; font-weight: bold;");

    m_refreshFriendsBtn = new QPushButton("🔄 刷新");
    m_refreshFriendsBtn->setMaximumWidth(80);
    connect(m_refreshFriendsBtn, &QPushButton::clicked, this, &FriendManager::refreshFriendsList);

    topLayout->addWidget(m_friendsCountLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_refreshFriendsBtn);
    layout->addLayout(topLayout);

    // 好友列表
    m_friendsList = new QListWidget();
    m_friendsList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_friendsList, &QListWidget::itemClicked, this, &FriendManager::onFriendItemClicked);
    connect(m_friendsList, &QListWidget::customContextMenuRequested, this, &FriendManager::onFriendContextMenu);
    layout->addWidget(m_friendsList);

    m_tabWidget->addTab(m_friendsTab, "👥 我的好友");
}

void FriendManager::setupAddFriendTab()
{
    m_addFriendTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_addFriendTab);

    // 搜索区域
    QGroupBox *searchGroup = new QGroupBox("搜索用户");
    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);

    // 搜索方式选择
    QHBoxLayout *methodLayout = new QHBoxLayout();
    QLabel *methodLabel = new QLabel("搜索方式:");
    m_searchMethodCombo = new QComboBox();
    m_searchMethodCombo->addItem("按姓名搜索", "name");
    m_searchMethodCombo->addItem("按ID搜索", "id");
    connect(m_searchMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FriendManager::onSearchMethodChanged);

    methodLayout->addWidget(methodLabel);
    methodLayout->addWidget(m_searchMethodCombo);
    methodLayout->addStretch();
    searchLayout->addLayout(methodLayout);

    // 搜索输入框
    QHBoxLayout *inputLayout = new QHBoxLayout();
    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("输入姓名或关键词搜索用户...");
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &FriendManager::onSearchTextChanged);
    connect(m_searchLineEdit, &QLineEdit::returnPressed, this, &FriendManager::onSearchButtonClicked);

    m_searchButton = new QPushButton("🔍 搜索");
    m_searchButton->setMaximumWidth(80);
    connect(m_searchButton, &QPushButton::clicked, this, &FriendManager::onSearchButtonClicked);

    inputLayout->addWidget(m_searchLineEdit);
    inputLayout->addWidget(m_searchButton);
    searchLayout->addLayout(inputLayout);

    layout->addWidget(searchGroup);

    // 搜索状态标签
    m_searchStatusLabel = new QLabel("请输入关键词开始搜索");
    m_searchStatusLabel->setStyleSheet("color: #7f8c8d; font-style: italic;");
    layout->addWidget(m_searchStatusLabel);

    // 搜索结果列表
    m_searchResultsList = new QListWidget();
    connect(m_searchResultsList, &QListWidget::itemClicked, this, &FriendManager::onAddFriendClicked);
    layout->addWidget(m_searchResultsList);

    m_tabWidget->addTab(m_addFriendTab, "➕ 添加好友");
}

void FriendManager::setupRequestsTab()
{
    m_requestsTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_requestsTab);

    // 申请管理子标签页
    m_requestsTabWidget = new QTabWidget();

    // 收到的申请
    QWidget *receivedTab = new QWidget();
    QVBoxLayout *receivedLayout = new QVBoxLayout(receivedTab);

    m_receivedCountLabel = new QLabel("收到的申请: 0条");
    m_receivedCountLabel->setStyleSheet("font-size: 14px; color: #e74c3c; font-weight: bold;");
    receivedLayout->addWidget(m_receivedCountLabel);

    m_receivedRequestsList = new QListWidget();
    connect(m_receivedRequestsList, &QListWidget::itemClicked, this, &FriendManager::onReceivedRequestClicked);
    receivedLayout->addWidget(m_receivedRequestsList);

    m_requestsTabWidget->addTab(receivedTab, "📨 收到的申请");

    // 发送的申请
    QWidget *sentTab = new QWidget();
    QVBoxLayout *sentLayout = new QVBoxLayout(sentTab);

    m_sentCountLabel = new QLabel("发送的申请: 0条");
    m_sentCountLabel->setStyleSheet("font-size: 14px; color: #3498db; font-weight: bold;");
    sentLayout->addWidget(m_sentCountLabel);

    m_sentRequestsList = new QListWidget();
    connect(m_sentRequestsList, &QListWidget::itemClicked, this, &FriendManager::onSentRequestClicked);
    sentLayout->addWidget(m_sentRequestsList);

    m_requestsTabWidget->addTab(sentTab, "📤 发送的申请");

    layout->addWidget(m_requestsTabWidget);

    m_tabWidget->addTab(m_requestsTab, "📋 申请管理");
}

void FriendManager::setupClassmatesTab()
{
    m_classmatesTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_classmatesTab);

    // 顶部信息栏
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_classmatesCountLabel = new QLabel("同班同学: 0人");
    m_classmatesCountLabel->setStyleSheet("font-size: 16px; color: #2c3e50; font-weight: bold;");

    m_refreshClassmatesBtn = new QPushButton("🔄 刷新");
    m_refreshClassmatesBtn->setMaximumWidth(80);
    connect(m_refreshClassmatesBtn, &QPushButton::clicked, this, &FriendManager::updateClassmates);

    topLayout->addWidget(m_classmatesCountLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_refreshClassmatesBtn);
    layout->addLayout(topLayout);

    // 说明文字
    QLabel *infoLabel = new QLabel("以下是与您选修相同课程的同学：");
    infoLabel->setStyleSheet("color: #7f8c8d; font-style: italic;");
    layout->addWidget(infoLabel);

    // 同班同学列表
    m_classmatesList = new QListWidget();
    connect(m_classmatesList, &QListWidget::itemClicked, this, &FriendManager::onClassmateClicked);
    layout->addWidget(m_classmatesList);

    m_tabWidget->addTab(m_classmatesTab, "🎓 班级成员");
}

void FriendManager::refreshData()
{
    updateStatistics();
    updateFriendsList();
    updateReceivedRequests();
    updateSentRequests();
    if (m_currentUserType == "学生") {
        updateClassmates();
    }
}

void FriendManager::refreshFriendsList()
{
    updateFriendsList();
    updateStatistics();
}

void FriendManager::refreshRequests()
{
    updateReceivedRequests();
    updateSentRequests();
    updateStatistics();
}

void FriendManager::onTabChanged(int index)
{
    Q_UNUSED(index)
    // 每次切换标签页时刷新当前数据
    switch (m_tabWidget->currentIndex()) {
    case 0: // 好友列表
        refreshFriendsList();
        break;
    case 1: // 添加好友
        // 清空搜索结果
        m_searchResultsList->clear();
        m_searchLineEdit->clear();
        m_searchStatusLabel->setText("请输入关键词开始搜索");
        break;
    case 2: // 申请管理
        refreshRequests();
        break;
    case 3: // 班级成员（仅学生）
        if (m_currentUserType == "学生") {
            updateClassmates();
        }
        break;
    }
}

void FriendManager::updateFriendsList()
{
    m_friendsList->clear();
    m_friends.clear();

    QList<QVariantMap> friendsData = m_database->getFriendsList(m_currentUserId, m_currentUserType);

    for (const QVariantMap &friendData : friendsData) {
        FriendInfo friendInfo;
        friendInfo.setFriendId(friendData["friend_id"].toInt());
        friendInfo.setFriendType(friendData["friend_type"].toString());
        friendInfo.setFriendName(friendData["friend_name"].toString());
        friendInfo.setFriendCollege(friendData["friend_college"].toString());
        friendInfo.setFriendGrade(friendData["friend_grade"].toString());
        friendInfo.setCreatedTime(friendData["created_time"].toDateTime());

        m_friends.append(friendInfo);
        QListWidgetItem *item = createFriendItem(friendInfo);
        m_friendsList->addItem(item);
    }

    m_friendsCountLabel->setText(QString("好友: %1人").arg(m_friends.size()));
}

void FriendManager::updateSearchResults()
{
    m_searchResultsList->clear();
    m_searchResults.clear();

    QString searchText = m_searchLineEdit->text().trimmed();
    if (searchText.isEmpty()) {
        m_searchStatusLabel->setText("请输入关键词开始搜索");
        return;
    }

    QList<QVariantMap> results;
    QString searchMethod = m_searchMethodCombo->currentData().toString();

    if (searchMethod == "id") {
        bool ok;
        int userId = searchText.toInt(&ok);
        if (ok) {
            results = m_database->searchUsersById(userId, m_currentUserId, m_currentUserType);
        }
    } else {
        results = m_database->searchUsersByKeyword(searchText, m_currentUserId, m_currentUserType);
    }

    if (results.isEmpty()) {
        m_searchStatusLabel->setText("未找到匹配的用户");
        return;
    }

    for (const QVariantMap &userData : results) {
        UserInfo userInfo;
        userInfo.setUserId(userData["user_id"].toInt());
        userInfo.setUserType(userData["user_type"].toString());
        userInfo.setUserName(userData["name"].toString());
        userInfo.setUserCollege(userData["college"].toString());
        userInfo.setUserGrade(userData["grade"].toString());

        // 设置关系状态
        QString relationship = userData["relationship"].toString();
        if (relationship == "已是好友") {
            userInfo.setRelationshipStatus(UserInfo::ALREADY_FRIEND);
        } else if (relationship == "已发送请求") {
            userInfo.setRelationshipStatus(UserInfo::REQUEST_SENT);
        } else if (relationship == "待处理请求") {
            userInfo.setRelationshipStatus(UserInfo::REQUEST_PENDING);
        } else {
            userInfo.setRelationshipStatus(UserInfo::CAN_ADD);
        }

        m_searchResults.append(userInfo);
        QListWidgetItem *item = createUserItem(userInfo);
        m_searchResultsList->addItem(item);
    }

    m_searchStatusLabel->setText(QString("找到 %1 个用户").arg(results.size()));
}

void FriendManager::updateReceivedRequests()
{
    m_receivedRequestsList->clear();
    m_receivedRequests.clear();

    QList<QVariantMap> requestsData = m_database->getReceivedFriendRequests(m_currentUserId, m_currentUserType);

    for (const QVariantMap &requestData : requestsData) {
        FriendRequest request;
        request.setRequestId(requestData["request_id"].toInt());
        request.setUserId(requestData["requester_id"].toInt());
        request.setUserType(requestData["requester_type"].toString());
        request.setUserName(requestData["requester_name"].toString());
        request.setUserCollege(requestData["requester_college"].toString());
        request.setUserGrade(requestData["requester_grade"].toString());
        request.setRequestType(FriendRequest::RECEIVED);
        request.setStatus(FriendRequest::PENDING);
        request.setRequestTime(requestData["request_time"].toDateTime());

        m_receivedRequests.append(request);
        QListWidgetItem *item = createRequestItem(request, true);
        m_receivedRequestsList->addItem(item);
    }

    m_receivedCountLabel->setText(QString("收到的申请: %1条").arg(m_receivedRequests.size()));
}

void FriendManager::updateSentRequests()
{
    m_sentRequestsList->clear();
    m_sentRequests.clear();

    QList<QVariantMap> requestsData = m_database->getSentFriendRequests(m_currentUserId, m_currentUserType);

    for (const QVariantMap &requestData : requestsData) {
        FriendRequest request;
        request.setRequestId(requestData["request_id"].toInt());
        request.setUserId(requestData["target_id"].toInt());
        request.setUserType(requestData["target_type"].toString());
        request.setUserName(requestData["target_name"].toString());
        request.setUserCollege(requestData["target_college"].toString());
        request.setRequestType(FriendRequest::SENT);

        QString status = requestData["status"].toString();
        if (status == "已同意") {
            request.setStatus(FriendRequest::ACCEPTED);
        } else {
            request.setStatus(FriendRequest::PENDING);
        }

        request.setRequestTime(requestData["request_time"].toDateTime());

        m_sentRequests.append(request);
        QListWidgetItem *item = createRequestItem(request, false);
        m_sentRequestsList->addItem(item);
    }

    m_sentCountLabel->setText(QString("发送的申请: %1条").arg(m_sentRequests.size()));
}

void FriendManager::updateClassmates()
{
    if (m_currentUserType != "学生") return;

    m_classmatesList->clear();
    m_classmates.clear();

    QList<QVariantMap> classmatesData = m_database->getClassmates(m_currentUserId);

    for (const QVariantMap &classmateData : classmatesData) {
        UserInfo classmate;
        classmate.setUserId(classmateData["user_id"].toInt());
        classmate.setUserType(classmateData["user_type"].toString());
        classmate.setUserName(classmateData["name"].toString());
        classmate.setUserCollege(classmateData["college"].toString());
        classmate.setUserGrade(classmateData["grade"].toString());

        // 检查关系状态
        classmate.setRelationshipStatus(getUserRelationshipStatus(classmate.getUserId(), classmate.getUserType()));

        m_classmates.append(classmate);
        QListWidgetItem *item = createUserItem(classmate);
        m_classmatesList->addItem(item);
    }

    m_classmatesCountLabel->setText(QString("同班同学: %1人").arg(m_classmates.size()));
}

void FriendManager::updateStatistics()
{
    m_friendCount = m_database->getFriendCount(m_currentUserId, m_currentUserType);
    m_pendingRequestCount = m_database->getPendingRequestCount(m_currentUserId, m_currentUserType);

    // 更新标签页标题中的数字提示
    QString friendsTabText = QString("👥 我的好友 (%1)").arg(m_friendCount);
    QString requestsTabText = QString("📋 申请管理 (%1)").arg(m_pendingRequestCount);

    m_tabWidget->setTabText(0, friendsTabText);
    m_tabWidget->setTabText(2, requestsTabText);
}

QListWidgetItem* FriendManager::createFriendItem(const FriendInfo &friendInfo)
{
    QListWidgetItem *item = new QListWidgetItem();

    QString displayText = QString("%1\n%2")
                              .arg(friendInfo.getDisplayName())
                              .arg(friendInfo.getDisplayInfo());

    item->setText(displayText);
    item->setData(Qt::UserRole, friendInfo.getFriendId());
    item->setData(Qt::UserRole + 1, friendInfo.getFriendType());

    // 设置图标
    if (friendInfo.isStudent()) {
        item->setIcon(QIcon(":/icons/student.png")); // 如果有图标资源
    } else {
        item->setIcon(QIcon(":/icons/teacher.png"));
    }

    return item;
}

QListWidgetItem* FriendManager::createUserItem(const UserInfo &userInfo)
{
    QListWidgetItem *item = new QListWidgetItem();

    QString displayText = QString("%1\n%2\n状态: %3")
                              .arg(userInfo.getDisplayName())
                              .arg(userInfo.getDisplayInfo())
                              .arg(userInfo.getRelationshipText());

    item->setText(displayText);
    item->setData(Qt::UserRole, userInfo.getUserId());
    item->setData(Qt::UserRole + 1, userInfo.getUserType());
    item->setData(Qt::UserRole + 2, static_cast<int>(userInfo.getRelationshipStatus()));

    // 根据状态设置不同颜色
    switch (userInfo.getRelationshipStatus()) {
    case UserInfo::CAN_ADD:
        item->setForeground(QColor("#27ae60"));
        break;
    case UserInfo::ALREADY_FRIEND:
        item->setForeground(QColor("#95a5a6"));
        break;
    case UserInfo::REQUEST_SENT:
        item->setForeground(QColor("#f39c12"));
        break;
    case UserInfo::REQUEST_PENDING:
        item->setForeground(QColor("#e74c3c"));
        break;
    }

    return item;
}

QListWidgetItem* FriendManager::createRequestItem(const FriendRequest &request, bool isReceived)
{
    QListWidgetItem *item = new QListWidgetItem();

    QString displayText;
    if (isReceived) {
        displayText = QString("来自: %1\n%2\n时间: %3")
                          .arg(request.getDisplayName())
                          .arg(request.getDisplayInfo())
                          .arg(request.getTimeText());
    } else {
        displayText = QString("发送给: %1\n%2\n状态: %3\n时间: %4")
                          .arg(request.getDisplayName())
                          .arg(request.getDisplayInfo())
                          .arg(request.getStatusText())
                          .arg(request.getTimeText());
    }

    item->setText(displayText);
    item->setData(Qt::UserRole, request.getRequestId());
    item->setData(Qt::UserRole + 1, request.getUserId());
    item->setData(Qt::UserRole + 2, request.getUserType());

    // 根据状态设置颜色
    if (isReceived) {
        item->setForeground(QColor("#e74c3c")); // 红色表示需要处理
    } else {
        if (request.getStatus() == FriendRequest::ACCEPTED) {
            item->setForeground(QColor("#27ae60")); // 绿色表示已同意
        } else {
            item->setForeground(QColor("#f39c12")); // 橙色表示待处理
        }
    }

    return item;
}

void FriendManager::onFriendItemClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    // 可以在这里添加查看好友详情的功能
}

void FriendManager::onFriendContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_friendsList->itemAt(pos);
    if (!item) return;

    QMenu contextMenu(this);
    QAction *removeAction = contextMenu.addAction("❌ 删除好友");

    QAction *selectedAction = contextMenu.exec(m_friendsList->mapToGlobal(pos));

    if (selectedAction == removeAction) {
        removeFriend();
    }
}

void FriendManager::removeFriend()
{
    QListWidgetItem *item = m_friendsList->currentItem();
    if (!item) return;

    int friendId = item->data(Qt::UserRole).toInt();
    QString friendType = item->data(Qt::UserRole + 1).toString();

    QString friendName;
    for (const FriendInfo &friendInfo : m_friends) {
        if (friendInfo.getFriendId() == friendId && friendInfo.getFriendType() == friendType) {
            friendName = friendInfo.getFriendName();
            break;
        }
    }

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除好友 \"%1\" 吗？").arg(friendName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->removeFriendship(m_currentUserId, m_currentUserType, friendId, friendType)) {
            showMessage("删除成功", "已删除好友关系");
            refreshFriendsList();
        } else {
            showError("删除失败，请重试");
        }
    }
}

void FriendManager::onSearchTextChanged()
{
    // 延迟搜索，避免频繁查询
    static QTimer *searchTimer = nullptr;
    if (!searchTimer) {
        searchTimer = new QTimer(this);
        searchTimer->setSingleShot(true);
        connect(searchTimer, &QTimer::timeout, this, &FriendManager::updateSearchResults);
    }

    searchTimer->stop();
    searchTimer->start(500); // 500ms延迟
}

void FriendManager::onSearchMethodChanged()
{
    QString method = m_searchMethodCombo->currentData().toString();
    if (method == "id") {
        m_searchLineEdit->setPlaceholderText("输入用户ID搜索...");
    } else {
        m_searchLineEdit->setPlaceholderText("输入姓名或关键词搜索用户...");
    }

    // 清空当前搜索结果
    m_searchResultsList->clear();
    m_searchStatusLabel->setText("请输入关键词开始搜索");
}

void FriendManager::onSearchButtonClicked()
{
    updateSearchResults();
}

void FriendManager::onAddFriendClicked()
{
    QListWidgetItem *item = m_searchResultsList->currentItem();
    if (!item) return;

    int userId = item->data(Qt::UserRole).toInt();
    QString userType = item->data(Qt::UserRole + 1).toString();
    UserInfo::RelationshipStatus status = static_cast<UserInfo::RelationshipStatus>(
        item->data(Qt::UserRole + 2).toInt());

    if (status != UserInfo::CAN_ADD) {
        showMessage("无法添加", "该用户状态不允许添加好友");
        return;
    }

    QString userName;
    for (const UserInfo &userInfo : m_searchResults) {
        if (userInfo.getUserId() == userId && userInfo.getUserType() == userType) {
            userName = userInfo.getUserName();
            break;
        }
    }

    int ret = QMessageBox::question(this, "确认添加",
                                    QString("确定要向 \"%1\" 发送好友申请吗？").arg(userName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->sendFriendRequest(m_currentUserId, m_currentUserType, userId, userType)) {
            showMessage("发送成功", "好友申请已发送，等待对方确认");
            updateSearchResults(); // 刷新搜索结果状态
        } else {
            showError("发送失败，请重试");
        }
    }
}

void FriendManager::onReceivedRequestClicked(QListWidgetItem *item)
{
    if (!item) return;

    int requestId = item->data(Qt::UserRole).toInt();
    int requesterId = item->data(Qt::UserRole + 1).toInt();
    QString requesterType = item->data(Qt::UserRole + 2).toString();

    QString requesterName;
    for (const FriendRequest &request : m_receivedRequests) {
        if (request.getRequestId() == requestId) {
            requesterName = request.getUserName();
            break;
        }
    }

    int ret = QMessageBox::question(this, "处理好友申请",
                                    QString("用户 \"%1\" 请求添加您为好友\n\n您的选择：").arg(requesterName),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes);

    if (ret == QMessageBox::Yes) {
        acceptRequest();
    } else {
        rejectRequest();
    }
}

void FriendManager::onSentRequestClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    // 发送的申请暂时只显示，不提供操作
}

void FriendManager::acceptRequest()
{
    QListWidgetItem *item = m_receivedRequestsList->currentItem();
    if (!item) return;

    int requestId = item->data(Qt::UserRole).toInt();

    if (m_database->acceptFriendRequest(requestId)) {
        showMessage("接受成功", "已成为好友！");
        refreshRequests();
        refreshFriendsList();
    } else {
        showError("接受失败，请重试");
    }
}

void FriendManager::rejectRequest()
{
    QListWidgetItem *item = m_receivedRequestsList->currentItem();
    if (!item) return;

    int requestId = item->data(Qt::UserRole).toInt();

    if (m_database->rejectFriendRequest(requestId)) {
        showMessage("已拒绝", "已拒绝该好友申请");
        refreshRequests();
    } else {
        showError("拒绝失败，请重试");
    }
}

void FriendManager::onClassmateClicked(QListWidgetItem *item)
{
    if (!item) return;

    onAddClassmateClicked();
}

void FriendManager::onAddClassmateClicked()
{
    QListWidgetItem *item = m_classmatesList->currentItem();
    if (!item) return;

    int userId = item->data(Qt::UserRole).toInt();
    QString userType = item->data(Qt::UserRole + 1).toString();
    UserInfo::RelationshipStatus status = static_cast<UserInfo::RelationshipStatus>(
        item->data(Qt::UserRole + 2).toInt());

    if (status != UserInfo::CAN_ADD) {
        showMessage("无法添加", "该同学状态不允许添加好友");
        return;
    }

    QString userName;
    for (const UserInfo &userInfo : m_classmates) {
        if (userInfo.getUserId() == userId && userInfo.getUserType() == userType) {
            userName = userInfo.getUserName();
            break;
        }
    }

    int ret = QMessageBox::question(this, "确认添加",
                                    QString("确定要向同学 \"%1\" 发送好友申请吗？").arg(userName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->sendFriendRequest(m_currentUserId, m_currentUserType, userId, userType)) {
            showMessage("发送成功", "好友申请已发送，等待对方确认");
            updateClassmates(); // 刷新状态
        } else {
            showError("发送失败，请重试");
        }
    }
}

void FriendManager::showMessage(const QString &title, const QString &message)
{
    QMessageBox::information(this, title, message);
}

void FriendManager::showError(const QString &message)
{
    QMessageBox::warning(this, "错误", message);
}

UserInfo::RelationshipStatus FriendManager::getUserRelationshipStatus(int userId, const QString &userType)
{
    if (m_database->areFriends(m_currentUserId, m_currentUserType, userId, userType)) {
        return UserInfo::ALREADY_FRIEND;
    } else if (m_database->hasPendingFriendRequest(m_currentUserId, m_currentUserType, userId, userType)) {
        return UserInfo::REQUEST_SENT;
    } else if (m_database->hasPendingFriendRequest(userId, userType, m_currentUserId, m_currentUserType)) {
        return UserInfo::REQUEST_PENDING;
    } else {
        return UserInfo::CAN_ADD;
    }
}

QVariantMap FriendManager::getSelectedFriend()
{
    QListWidgetItem *item = m_friendsList->currentItem();
    if (!item) return QVariantMap();

    QVariantMap friendData;
    friendData["friend_id"] = item->data(Qt::UserRole);
    friendData["friend_type"] = item->data(Qt::UserRole + 1);

    return friendData;
}

QVariantMap FriendManager::getSelectedUser()
{
    QListWidgetItem *item = m_searchResultsList->currentItem();
    if (!item) return QVariantMap();

    QVariantMap userData;
    userData["user_id"] = item->data(Qt::UserRole);
    userData["user_type"] = item->data(Qt::UserRole + 1);
    userData["relationship_status"] = item->data(Qt::UserRole + 2);

    return userData;
}

QVariantMap FriendManager::getSelectedRequest()
{
    QListWidgetItem *item = m_receivedRequestsList->currentItem();
    if (!item) return QVariantMap();

    QVariantMap requestData;
    requestData["request_id"] = item->data(Qt::UserRole);
    requestData["user_id"] = item->data(Qt::UserRole + 1);
    requestData["user_type"] = item->data(Qt::UserRole + 2);

    return requestData;
}
