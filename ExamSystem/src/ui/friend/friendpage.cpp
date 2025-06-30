#include "friendpage.h"
#include <QMessageBox>
#include <QApplication>
#include <QFrame>

FriendPage::FriendPage(Database *database, int currentUserId, const QString &currentUserType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_currentUserId(currentUserId)
    , m_currentUserType(currentUserType)
    , m_friendCount(0)
    , m_pendingRequestCount(0)
    , m_currentPage(0)
{
    initUI();

    // 设置定时刷新（每60秒自动刷新申请状态）
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &FriendPage::autoRefresh);
    m_refreshTimer->start(60000); // 60秒

    // 搜索延迟定时器
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, &QTimer::timeout, this, &FriendPage::updateSearchResults);

    // 初始加载数据
    refreshAll();
}

FriendPage::~FriendPage()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
    }
}

void FriendPage::initUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    setupNavigation();

    // 创建内容区域
    m_contentStack = new QStackedWidget();
    m_contentStack->setObjectName("FriendContentStack");

    setupFriendsPage();
    setupAddFriendPage();
    setupRequestsPage();

    m_mainLayout->addWidget(m_navigationWidget, 0);
    m_mainLayout->addWidget(m_contentStack, 1);

    // 设置样式
    setStyleSheet(R"(
        QWidget#FriendNavigation {
            background-color: #2c3e50;
            border-right: 1px solid #34495e;
        }

        QWidget#FriendContentStack {
            background-color: white;
        }

        QPushButton#NavButton {
            background-color: transparent;
            color: #ecf0f1;
            border: none;
            text-align: left;
            padding: 15px 20px;
            font-size: 14px;
            font-weight: bold;
        }

        QPushButton#NavButton:hover {
            background-color: #34495e;
        }

        QPushButton#NavButton:checked {
            background-color: #3498db;
            color: white;
        }

        QPushButton#ActionButton {
            background-color: #3498db;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }

        QPushButton#ActionButton:hover {
            background-color: #2980b9;
        }

        QPushButton#ActionButton:pressed {
            background-color: #21618c;
        }

        QPushButton#ActionButton:disabled {
            background-color: #bdc3c7;
            color: #7f8c8d;
        }

        QListWidget {
            border: 1px solid #ddd;
            background-color: white;
        }

        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #eee;
        }

        QListWidget::item:selected {
            background-color: #3498db;
            color: white;
        }

        QListWidget::item:hover {
            background-color: #e3f2fd;
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

        QLabel#StatisticsLabel {
            color: #ecf0f1;
            font-size: 12px;
            padding: 10px;
        }

        QLabel#CountLabel {
            color: #2c3e50;
            font-size: 16px;
            font-weight: bold;
        }

        QLabel#StatusLabel {
            color: #7f8c8d;
            font-style: italic;
            padding: 10px;
        }

        QScrollArea {
            border: 1px solid #ddd;
            background-color: white;
        }

        QFrame#UserCard {
            border: 1px solid #ddd;
            border-radius: 8px;
            background-color: #f8f9fa;
            margin: 5px;
            padding: 10px;
        }

        QFrame#UserCard:hover {
            border-color: #3498db;
            background-color: #e3f2fd;
        }
    )");
}

void FriendPage::setupNavigation()
{
    m_navigationWidget = new QWidget();
    m_navigationWidget->setObjectName("FriendNavigation");
    m_navigationWidget->setFixedWidth(200);

    m_navigationLayout = new QVBoxLayout(m_navigationWidget);
    m_navigationLayout->setContentsMargins(0, 0, 0, 0);
    m_navigationLayout->setSpacing(0);

    // 导航按钮
    m_friendsListBtn = new QPushButton("👥 我的好友");
    m_friendsListBtn->setObjectName("NavButton");
    m_friendsListBtn->setCheckable(true);
    m_friendsListBtn->setChecked(true);
    connect(m_friendsListBtn, &QPushButton::clicked, this, &FriendPage::onNavigationClicked);

    m_addFriendBtn = new QPushButton("➕ 添加好友");
    m_addFriendBtn->setObjectName("NavButton");
    m_addFriendBtn->setCheckable(true);
    connect(m_addFriendBtn, &QPushButton::clicked, this, &FriendPage::onNavigationClicked);

    m_requestsBtn = new QPushButton("📋 申请管理");
    m_requestsBtn->setObjectName("NavButton");
    m_requestsBtn->setCheckable(true);
    connect(m_requestsBtn, &QPushButton::clicked, this, &FriendPage::onNavigationClicked);

    m_navigationLayout->addWidget(m_friendsListBtn);
    m_navigationLayout->addWidget(m_addFriendBtn);
    m_navigationLayout->addWidget(m_requestsBtn);

    // 分隔线
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #34495e;");
    m_navigationLayout->addWidget(separator);

    // 统计信息
    m_statisticsLabel = new QLabel();
    m_statisticsLabel->setObjectName("StatisticsLabel");
    m_statisticsLabel->setWordWrap(true);
    m_navigationLayout->addWidget(m_statisticsLabel);

    m_navigationLayout->addStretch();
}

void FriendPage::setupFriendsPage()
{
    m_friendsPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_friendsPage);
    layout->setContentsMargins(20, 20, 20, 20);

    // 顶部信息栏
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_friendsCountLabel = new QLabel("好友: 0人");
    m_friendsCountLabel->setObjectName("CountLabel");

    m_refreshFriendsBtn = new QPushButton("🔄 刷新");
    m_refreshFriendsBtn->setObjectName("ActionButton");
    m_refreshFriendsBtn->setMaximumWidth(80);
    connect(m_refreshFriendsBtn, &QPushButton::clicked, this, &FriendPage::updateFriendsList);

    topLayout->addWidget(m_friendsCountLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_refreshFriendsBtn);
    layout->addLayout(topLayout);

    // 好友列表
    m_friendsList = new QListWidget();
    connect(m_friendsList, &QListWidget::itemDoubleClicked, this, &FriendPage::onFriendDoubleClicked);
    layout->addWidget(m_friendsList);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *removeFriendBtn = new QPushButton("❌ 删除好友");
    removeFriendBtn->setObjectName("ActionButton");
    removeFriendBtn->setStyleSheet("QPushButton#ActionButton { background-color: #e74c3c; } QPushButton#ActionButton:hover { background-color: #c0392b; }");
    connect(removeFriendBtn, &QPushButton::clicked, this, &FriendPage::onRemoveFriendClicked);

    buttonLayout->addStretch();
    buttonLayout->addWidget(removeFriendBtn);
    layout->addLayout(buttonLayout);

    m_contentStack->addWidget(m_friendsPage);
}

void FriendPage::setupAddFriendPage()
{
    m_addFriendPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_addFriendPage);
    layout->setContentsMargins(20, 20, 20, 20);

    // 搜索区域
    QGroupBox *searchGroup = new QGroupBox("搜索用户");
    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);

    // 搜索方式选择
    QHBoxLayout *methodLayout = new QHBoxLayout();
    QLabel *methodLabel = new QLabel("搜索方式:");
    m_searchMethodCombo = new QComboBox();
    m_searchMethodCombo->addItem("按姓名搜索", "name");
    m_searchMethodCombo->addItem("按ID搜索", "id");

    methodLayout->addWidget(methodLabel);
    methodLayout->addWidget(m_searchMethodCombo);
    methodLayout->addStretch();
    searchLayout->addLayout(methodLayout);

    // 搜索输入框
    QHBoxLayout *inputLayout = new QHBoxLayout();
    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("输入姓名或关键词搜索用户...");
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &FriendPage::onSearchTextChanged);
    connect(m_searchLineEdit, &QLineEdit::returnPressed, this, &FriendPage::onSearchButtonClicked);

    m_searchButton = new QPushButton("🔍 搜索");
    m_searchButton->setObjectName("ActionButton");
    m_searchButton->setMaximumWidth(80);
    connect(m_searchButton, &QPushButton::clicked, this, &FriendPage::onSearchButtonClicked);

    inputLayout->addWidget(m_searchLineEdit);
    inputLayout->addWidget(m_searchButton);
    searchLayout->addLayout(inputLayout);

    layout->addWidget(searchGroup);

    // 搜索状态标签
    m_searchStatusLabel = new QLabel("请输入关键词开始搜索");
    m_searchStatusLabel->setObjectName("StatusLabel");
    layout->addWidget(m_searchStatusLabel);

    // 搜索结果区域
    m_searchResultsArea = new QScrollArea();
    m_searchResultsWidget = new QWidget();
    m_searchResultsLayout = new QVBoxLayout(m_searchResultsWidget);
    m_searchResultsLayout->setAlignment(Qt::AlignTop);
    m_searchResultsArea->setWidget(m_searchResultsWidget);
    m_searchResultsArea->setWidgetResizable(true);
    layout->addWidget(m_searchResultsArea);

    m_contentStack->addWidget(m_addFriendPage);
}

void FriendPage::setupRequestsPage()
{
    m_requestsPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_requestsPage);
    layout->setContentsMargins(20, 20, 20, 20);

    // 顶部信息栏
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_requestsCountLabel = new QLabel("待处理申请: 0条");
    m_requestsCountLabel->setObjectName("CountLabel");

    m_refreshRequestsBtn = new QPushButton("🔄 刷新");
    m_refreshRequestsBtn->setObjectName("ActionButton");
    m_refreshRequestsBtn->setMaximumWidth(80);
    connect(m_refreshRequestsBtn, &QPushButton::clicked, this, &FriendPage::updatePendingRequests);

    topLayout->addWidget(m_requestsCountLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_refreshRequestsBtn);
    layout->addLayout(topLayout);

    // 说明文字
    QLabel *infoLabel = new QLabel("收到的好友申请：");
    infoLabel->setObjectName("StatusLabel");
    layout->addWidget(infoLabel);

    // 申请列表
    m_pendingRequestsList = new QListWidget();
    connect(m_pendingRequestsList, &QListWidget::itemClicked, this, &FriendPage::onRequestItemClicked);
    layout->addWidget(m_pendingRequestsList);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *acceptBtn = new QPushButton("✅ 接受申请");
    acceptBtn->setObjectName("ActionButton");
    acceptBtn->setStyleSheet("QPushButton#ActionButton { background-color: #27ae60; } QPushButton#ActionButton:hover { background-color: #229954; }");
    connect(acceptBtn, &QPushButton::clicked, this, &FriendPage::onAcceptRequestClicked);

    QPushButton *rejectBtn = new QPushButton("❌ 拒绝申请");
    rejectBtn->setObjectName("ActionButton");
    rejectBtn->setStyleSheet("QPushButton#ActionButton { background-color: #e74c3c; } QPushButton#ActionButton:hover { background-color: #c0392b; }");
    connect(rejectBtn, &QPushButton::clicked, this, &FriendPage::onRejectRequestClicked);

    buttonLayout->addStretch();
    buttonLayout->addWidget(acceptBtn);
    buttonLayout->addWidget(rejectBtn);
    layout->addLayout(buttonLayout);

    m_contentStack->addWidget(m_requestsPage);
}

void FriendPage::refreshAll()
{
    updateFriendsList();
    updatePendingRequests();
    updateNavigationBadges();
}

void FriendPage::showFriendsList()
{
    m_currentPage = 0;
    m_contentStack->setCurrentIndex(0);

    // 更新导航按钮状态
    m_friendsListBtn->setChecked(true);
    m_addFriendBtn->setChecked(false);
    m_requestsBtn->setChecked(false);

    updateFriendsList();
}

void FriendPage::showAddFriend()
{
    m_currentPage = 1;
    m_contentStack->setCurrentIndex(1);

    // 更新导航按钮状态
    m_friendsListBtn->setChecked(false);
    m_addFriendBtn->setChecked(true);
    m_requestsBtn->setChecked(false);

    // 清空搜索结果
    m_searchLineEdit->clear();
    m_searchStatusLabel->setText("请输入关键词开始搜索");
    clearSearchResults();
}

void FriendPage::showRequests()
{
    m_currentPage = 2;
    m_contentStack->setCurrentIndex(2);

    // 更新导航按钮状态
    m_friendsListBtn->setChecked(false);
    m_addFriendBtn->setChecked(false);
    m_requestsBtn->setChecked(true);

    updatePendingRequests();
}

void FriendPage::onNavigationClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (button == m_friendsListBtn) {
        showFriendsList();
    } else if (button == m_addFriendBtn) {
        showAddFriend();
    } else if (button == m_requestsBtn) {
        showRequests();
    }
}

void FriendPage::onFriendDoubleClicked(QListWidgetItem *item)
{
    if (!item) {
        qDebug() << "好友双击：item为空";
        return;
    }

    // 从item中获取好友信息
    int friendId = item->data(Qt::UserRole).toInt();
    QString friendType = item->data(Qt::UserRole + 1).toString();
    QString friendName = item->data(Qt::UserRole + 2).toString();

    qDebug() << "好友列表双击：" << friendName << "(" << friendType << ")" << "ID:" << friendId;

    // 验证数据有效性
    if (friendId <= 0 || friendType.isEmpty() || friendName.isEmpty()) {
        qDebug() << "好友信息无效，取消打开聊天";
        showMessage("好友信息异常，无法打开聊天", true);
        return;
    }

    // 检查是否仍为好友关系
    if (!m_database->areFriends(m_currentUserId, m_currentUserType, friendId, friendType)) {
        showMessage("您与该用户已不是好友关系", true);
        updateFriendsList(); // 刷新好友列表
        return;
    }

    // 发射信号，通知主界面切换到聊天
    emit friendDoubleClicked(friendId, friendType, friendName);

    qDebug() << "已发射好友双击信号";
}

void FriendPage::onRemoveFriendClicked()
{
    QListWidgetItem *item = m_friendsList->currentItem();
    if (!item) {
        showMessage("请先选择要删除的好友", true);
        return;
    }

    int friendId = item->data(Qt::UserRole).toInt();
    QString friendType = item->data(Qt::UserRole + 1).toString();
    QString friendName = item->data(Qt::UserRole + 2).toString();

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除好友 \"%1\" 吗？\n\n删除后将无法看到对方的动态信息。").arg(friendName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->removeFriendship(m_currentUserId, m_currentUserType, friendId, friendType)) {
            showMessage("已删除好友关系");
            updateFriendsList();
            updateNavigationBadges();
            emit friendRemoved(friendId, friendType);
        } else {
            showMessage("删除失败，请重试", true);
        }
    }
}

void FriendPage::onSearchTextChanged()
{
    // 延迟搜索，避免频繁查询
    m_searchTimer->stop();
    m_searchTimer->start(800); // 800ms延迟
}

void FriendPage::onSearchButtonClicked()
{
    updateSearchResults();
}

void FriendPage::onAddFriendClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int userId = button->property("userId").toInt();
    QString userType = button->property("userType").toString();
    QString userName = button->property("userName").toString();

    int ret = QMessageBox::question(this, "确认添加",
                                    QString("确定要向 \"%1\" 发送好友申请吗？").arg(userName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->sendFriendRequest(m_currentUserId, m_currentUserType, userId, userType)) {
            showMessage("好友申请已发送，等待对方确认");
            updateSearchResults(); // 刷新搜索结果状态
        } else {
            showMessage("发送失败，请重试", true);
        }
    }
}

void FriendPage::onRequestItemClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    // 选择申请项目，准备处理
}

void FriendPage::onAcceptRequestClicked()
{
    QListWidgetItem *item = m_pendingRequestsList->currentItem();
    if (!item) {
        showMessage("请先选择要处理的申请", true);
        return;
    }

    int requestId = item->data(Qt::UserRole).toInt();
    QString requesterName = item->data(Qt::UserRole + 2).toString();

    int ret = QMessageBox::question(this, "确认接受",
                                    QString("确定要接受 \"%1\" 的好友申请吗？").arg(requesterName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->acceptFriendRequest(requestId)) {
            showMessage("已接受好友申请！");
            updatePendingRequests();
            updateFriendsList();
            updateNavigationBadges();
            emit requestProcessed();
        } else {
            showMessage("接受失败，请重试", true);
        }
    }
}

void FriendPage::onRejectRequestClicked()
{
    QListWidgetItem *item = m_pendingRequestsList->currentItem();
    if (!item) {
        showMessage("请先选择要处理的申请", true);
        return;
    }

    int requestId = item->data(Qt::UserRole).toInt();
    QString requesterName = item->data(Qt::UserRole + 2).toString();

    int ret = QMessageBox::question(this, "确认拒绝",
                                    QString("确定要拒绝 \"%1\" 的好友申请吗？").arg(requesterName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->rejectFriendRequest(requestId)) {
            showMessage("已拒绝好友申请");
            updatePendingRequests();
            updateNavigationBadges();
            emit requestProcessed();
        } else {
            showMessage("拒绝失败，请重试", true);
        }
    }
}

void FriendPage::autoRefresh()
{
    // 自动刷新申请状态
    updatePendingRequests();
    updateNavigationBadges();
}

void FriendPage::updateFriendsList()
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

        QListWidgetItem *item = new QListWidgetItem();
        QString displayText = QString("%1\n%2")
                                  .arg(friendInfo.getDisplayName())
                                  .arg(friendInfo.getDisplayInfo());
        item->setText(displayText);
        item->setData(Qt::UserRole, friendInfo.getFriendId());
        item->setData(Qt::UserRole + 1, friendInfo.getFriendType());
        item->setData(Qt::UserRole + 2, friendInfo.getFriendName());

        m_friendsList->addItem(item);
    }

    m_friendCount = m_friends.size();
    m_friendsCountLabel->setText(QString("好友: %1人").arg(m_friendCount));
}

void FriendPage::updateSearchResults()
{
    clearSearchResults();

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

    m_searchResults.clear();
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

        QWidget *userWidget = createUserWidget(userInfo);
        m_searchResultsLayout->addWidget(userWidget);
    }

    m_searchStatusLabel->setText(QString("找到 %1 个用户").arg(results.size()));
}

void FriendPage::updatePendingRequests()
{
    m_pendingRequestsList->clear();
    m_pendingRequests.clear();

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

        m_pendingRequests.append(request);

        QListWidgetItem *item = new QListWidgetItem();
        QString displayText = QString("来自: %1\n%2\n时间: %3")
                                  .arg(request.getDisplayName())
                                  .arg(request.getDisplayInfo())
                                  .arg(request.getTimeText());
        item->setText(displayText);
        item->setData(Qt::UserRole, request.getRequestId());
        item->setData(Qt::UserRole + 1, request.getUserId());
        item->setData(Qt::UserRole + 2, request.getUserName());

        m_pendingRequestsList->addItem(item);
    }

    m_pendingRequestCount = m_pendingRequests.size();
    m_requestsCountLabel->setText(QString("待处理申请: %1条").arg(m_pendingRequestCount));
}

void FriendPage::updateNavigationBadges()
{
    // 更新导航按钮文字中的数字提示
    m_friendsListBtn->setText(QString("👥 我的好友 (%1)").arg(m_friendCount));
    m_requestsBtn->setText(QString("📋 申请管理 (%1)").arg(m_pendingRequestCount));

    // 更新统计信息
    QString statisticsText = QString("好友总数: %1人\n待处理申请: %2条")
                                 .arg(m_friendCount)
                                 .arg(m_pendingRequestCount);
    m_statisticsLabel->setText(statisticsText);
}

QWidget* FriendPage::createUserWidget(const UserInfo &userInfo)
{
    QFrame *userCard = new QFrame();
    userCard->setObjectName("UserCard");

    QHBoxLayout *layout = new QHBoxLayout(userCard);

    // 左侧用户信息
    QVBoxLayout *infoLayout = new QVBoxLayout();

    QLabel *nameLabel = new QLabel(userInfo.getDisplayName());
    nameLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");

    QLabel *infoLabel = new QLabel(userInfo.getDisplayInfo());
    infoLabel->setStyleSheet("color: #7f8c8d; font-size: 14px;");

    QLabel *statusLabel = new QLabel(QString("状态: %1").arg(userInfo.getRelationshipText()));
    statusLabel->setStyleSheet("color: #95a5a6; font-size: 12px;");

    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(infoLabel);
    infoLayout->addWidget(statusLabel);

    layout->addLayout(infoLayout);
    layout->addStretch();

    // 右侧操作按钮
    if (userInfo.canAddFriend()) {
        QPushButton *addBtn = new QPushButton("➕ 添加好友");
        addBtn->setObjectName("ActionButton");
        addBtn->setMaximumWidth(100);
        addBtn->setProperty("userId", userInfo.getUserId());
        addBtn->setProperty("userType", userInfo.getUserType());
        addBtn->setProperty("userName", userInfo.getUserName());
        connect(addBtn, &QPushButton::clicked, this, &FriendPage::onAddFriendClicked);
        layout->addWidget(addBtn);
    } else {
        QLabel *statusLabel = new QLabel(userInfo.getActionButtonText());
        statusLabel->setStyleSheet("color: #95a5a6; font-weight: bold;");
        layout->addWidget(statusLabel);
    }

    return userCard;
}

void FriendPage::clearSearchResults()
{
    // 清空搜索结果布局中的所有子控件
    QLayoutItem *item;
    while ((item = m_searchResultsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_searchResults.clear();
}

void FriendPage::showMessage(const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "错误", message);
    } else {
        QMessageBox::information(this, "提示", message);
    }
}

UserInfo::RelationshipStatus FriendPage::checkUserRelationship(int userId, const QString &userType)
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
