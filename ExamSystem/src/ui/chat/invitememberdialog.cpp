#include "invitememberdialog.h"
#include <QDebug>

InviteMemberDialog::InviteMemberDialog(Database *database, int groupId, const QString &groupName,
                                       int userId, const QString &userType, QWidget *parent)
    : QDialog(parent)
    , m_database(database)
    , m_groupId(groupId)
    , m_groupName(groupName)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
    , m_selectedUserId(-1)
{
    setWindowTitle(QString("邀请成员 - %1").arg(groupName));
    setModal(true);
    resize(500, 600);

    setupUI();
    setupStyles();

    // 加载当前群聊成员（用于过滤）
    QList<QVariantMap> members = m_database->getGroupMembers(groupId);
    for (const QVariantMap &member : members) {
        m_currentMembers.append({
            member["user_id"].toInt(),
            member["user_type"].toString()
        });
    }

    loadFriendsList();
}

void InviteMemberDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 16, 16, 16);
    m_mainLayout->setSpacing(16);

    // 标题
    QLabel *titleLabel = new QLabel(QString("邀请好友加入群聊：%1").arg(m_groupName));
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    // 搜索区域
    m_searchBox = new QGroupBox("🔍 搜索用户");
    QHBoxLayout *searchLayout = new QHBoxLayout(m_searchBox);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("输入姓名或ID搜索用户...");
    m_searchEdit->setObjectName("searchEdit");

    m_searchButton = new QPushButton("搜索");
    m_searchButton->setObjectName("searchButton");

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);

    // 好友列表区域
    m_friendsBox = new QGroupBox("👥 我的好友");
    QVBoxLayout *friendsLayout = new QVBoxLayout(m_friendsBox);

    m_friendsList = new QListWidget();
    m_friendsList->setObjectName("friendsList");
    friendsLayout->addWidget(m_friendsList);

    // 搜索结果区域
    m_resultsBox = new QGroupBox("🔍 搜索结果");
    QVBoxLayout *resultsLayout = new QVBoxLayout(m_resultsBox);

    m_resultsList = new QListWidget();
    m_resultsList->setObjectName("resultsList");
    resultsLayout->addWidget(m_resultsList);

    // 按钮区域
    m_buttonLayout = new QHBoxLayout();
    m_inviteButton = new QPushButton("📧 邀请选中用户");
    m_inviteButton->setObjectName("inviteButton");
    m_inviteButton->setEnabled(false);

    m_closeButton = new QPushButton("关闭");
    m_closeButton->setObjectName("closeButton");

    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_inviteButton);
    m_buttonLayout->addWidget(m_closeButton);

    // 组装布局
    m_mainLayout->addWidget(titleLabel);
    m_mainLayout->addWidget(m_searchBox);
    m_mainLayout->addWidget(m_friendsBox);
    m_mainLayout->addWidget(m_resultsBox);
    m_mainLayout->addLayout(m_buttonLayout);

    // 搜索防抖定时器
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(800);

    // 连接信号
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &InviteMemberDialog::onSearchTextChanged);
    connect(m_searchButton, &QPushButton::clicked,
            this, &InviteMemberDialog::onSearchButtonClicked);
    connect(m_friendsList, &QListWidget::itemClicked,
            this, &InviteMemberDialog::onUserItemClicked);
    connect(m_resultsList, &QListWidget::itemClicked,
            this, &InviteMemberDialog::onUserItemClicked);
    connect(m_inviteButton, &QPushButton::clicked,
            this, &InviteMemberDialog::onInviteButtonClicked);
    connect(m_closeButton, &QPushButton::clicked,
            this, &InviteMemberDialog::onCloseButtonClicked);
    connect(m_searchTimer, &QTimer::timeout,
            this, &InviteMemberDialog::performSearch);
}

void InviteMemberDialog::setupStyles()
{
    QString styles = R"(
        QDialog {
            background-color: #f8f9fa;
            font-family: 'Microsoft YaHei', Arial, sans-serif;
        }

        #titleLabel {
            font-size: 16px;
            font-weight: bold;
            color: #212529;
            padding: 8px;
            background-color: #e9ecef;
            border-radius: 6px;
        }

        QGroupBox {
            font-weight: bold;
            border: 2px solid #dee2e6;
            border-radius: 8px;
            margin-top: 8px;
            padding-top: 8px;
            background-color: white;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px 0 8px;
            color: #495057;
        }

        #searchEdit {
            padding: 8px;
            border: 1px solid #ced4da;
            border-radius: 4px;
            font-size: 14px;
        }

        #searchEdit:focus {
            border-color: #80bdff;
            outline: none;
        }

        #searchButton {
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 14px;
        }

        #searchButton:hover {
            background-color: #0056b3;
        }

        #friendsList, #resultsList {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 4px;
        }

        #friendsList::item, #resultsList::item {
            padding: 10px;
            border-bottom: 1px solid #e9ecef;
        }

        #friendsList::item:selected, #resultsList::item:selected {
            background-color: #cce5ff;
            border-left: 4px solid #007bff;
        }

        #friendsList::item:hover, #resultsList::item:hover {
            background-color: #e9ecef;
        }

        #inviteButton {
            background-color: #28a745;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
        }

        #inviteButton:hover {
            background-color: #218838;
        }

        #inviteButton:disabled {
            background-color: #6c757d;
        }

        #closeButton {
            background-color: #6c757d;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
        }

        #closeButton:hover {
            background-color: #5a6268;
        }
    )";

    setStyleSheet(styles);
}

void InviteMemberDialog::loadFriendsList()
{
    m_friendsList->clear();

    QList<QVariantMap> friends = m_database->getFriendsList(m_currentUserId, m_currentUserType);

    for (const QVariantMap &friendData : friends) {
        int friendId = friendData["friend_id"].toInt();
        QString friendType = friendData["friend_type"].toString();

        // 过滤已经是群聊成员的用户
        if (isUserAlreadyMember(friendId, friendType)) {
            continue;
        }

        UserInfo user;
        user.setUserId(friendId);
        user.setUserType(friendType);
        user.setUserName(friendData["friend_name"].toString());
        user.setUserCollege(friendData["friend_college"].toString());
        if (friendData.contains("friend_grade")) {
            user.setUserGrade(friendData["friend_grade"].toString());
        }

        addUserToList(user);
    }

    qDebug() << "加载好友列表完成，可邀请" << m_friendsList->count() << "人";
}

void InviteMemberDialog::addUserToList(const UserInfo &user)
{
    QString itemText = QString("%1 (%2)")
    .arg(user.getDisplayName())
        .arg(user.getUserType());

    if (!user.getUserCollege().isEmpty()) {
        itemText += QString(" - %1").arg(user.getUserCollege());
    }

    QListWidgetItem *item = new QListWidgetItem(itemText);
    item->setData(Qt::UserRole, user.getUserId());
    item->setData(Qt::UserRole + 1, user.getUserType());
    item->setData(Qt::UserRole + 2, user.getUserName());

    m_friendsList->addItem(item);
}

void InviteMemberDialog::onSearchTextChanged()
{
    m_searchTimer->stop();
    m_searchTimer->start();
}

void InviteMemberDialog::onSearchButtonClicked()
{
    m_searchTimer->stop();
    performSearch();
}

void InviteMemberDialog::performSearch()
{
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        clearSearchResults();
        return;
    }

    clearSearchResults();

    QList<QVariantMap> searchResults = m_database->searchUsersByKeyword(keyword, m_currentUserId, m_currentUserType);

    for (const QVariantMap &userData : searchResults) {
        int userId = userData["user_id"].toInt();
        QString userType = userData["user_type"].toString();

        // 过滤已经是群聊成员的用户
        if (isUserAlreadyMember(userId, userType)) {
            continue;
        }

        // 只显示可以邀请的用户（好友或可添加）
        QString relationshipStatus = userData["relationship_status"].toString();
        if (relationshipStatus != "可添加" && relationshipStatus != "已是好友") {
            continue;
        }

        QString itemText = QString("%1 (%2)")
                               .arg(userData["user_name"].toString())
                               .arg(userType);

        if (userData.contains("user_college")) {
            itemText += QString(" - %1").arg(userData["user_college"].toString());
        }

        // 添加关系状态标识
        if (relationshipStatus == "已是好友") {
            itemText += " ✓好友";
        }

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, userId);
        item->setData(Qt::UserRole + 1, userType);
        item->setData(Qt::UserRole + 2, userData["user_name"].toString());

        m_resultsList->addItem(item);
    }

    qDebug() << "搜索完成，找到" << m_resultsList->count() << "个可邀请用户";
}

void InviteMemberDialog::clearSearchResults()
{
    m_resultsList->clear();
}

void InviteMemberDialog::onUserItemClicked()
{
    QListWidget *sender = qobject_cast<QListWidget*>(QObject::sender());
    if (!sender) return;

    QListWidgetItem *item = sender->currentItem();
    if (!item) return;

    m_selectedUserId = item->data(Qt::UserRole).toInt();
    m_selectedUserType = item->data(Qt::UserRole + 1).toString();
    m_selectedUserName = item->data(Qt::UserRole + 2).toString();

    // 清除另一个列表的选择
    if (sender == m_friendsList) {
        m_resultsList->clearSelection();
    } else {
        m_friendsList->clearSelection();
    }

    m_inviteButton->setEnabled(true);
    m_inviteButton->setText(QString("📧 邀请 %1").arg(m_selectedUserName));

    qDebug() << "选择邀请用户:" << m_selectedUserName;
}

void InviteMemberDialog::onInviteButtonClicked()
{
    if (m_selectedUserId <= 0) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认邀请",
        QString("确定要邀请 %1 加入群聊吗？").arg(m_selectedUserName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // TODO: 实现邀请逻辑
        // 方案1: 直接添加到群聊
        // 方案2: 发送群聊申请

        // 这里使用方案1：直接添加（如果是好友的话）
        bool success = m_database->addGroupMember(m_groupId, m_selectedUserId, m_selectedUserType);

        if (success) {
            QMessageBox::information(this, "邀请成功",
                                     QString("%1 已加入群聊！").arg(m_selectedUserName));

            // 刷新界面
            loadFriendsList();
            clearSearchResults();
            m_inviteButton->setEnabled(false);
            m_inviteButton->setText("📧 邀请选中用户");

            accept(); // 关闭对话框并返回Accepted
        } else {
            QMessageBox::critical(this, "邀请失败", "邀请用户失败，请重试");
        }
    }
}

void InviteMemberDialog::onCloseButtonClicked()
{
    reject();
}

bool InviteMemberDialog::isUserAlreadyMember(int userId, const QString &userType)
{
    for (const auto &member : m_currentMembers) {
        if (member.first == userId && member.second == userType) {
            return true;
        }
    }
    return false;
}
