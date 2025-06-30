#include "chatpage.h"
#include "grouplistwidget.h"
#include "groupmanagewidget.h"
#include "creategroupdialog.h"
#include "joingroupdialog.h"
#include "messagebubblewidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QScrollBar>
#include <QKeyEvent>
#include <QApplication>
#include <QTimer>
#include <QEvent>
#include <QTabWidget>
#include "groupdetailwidget.h"

// ============================================================================
// ChatPage 主页面实现 - 修复后的构造函数
// ============================================================================

ChatPage::ChatPage(Database *database, int userId, const QString &userType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
    , m_currentChatType("私聊")
    , m_currentChatId(-1)
    , m_currentFriendName("")
    , m_currentGroupId(-1)
    , m_currentGroupName("")
    , m_tabWidget(nullptr)
    , m_privateChatTab(nullptr)
    , m_groupChatTab(nullptr)
    , m_groupManageTab(nullptr)
    , m_chatListWidget(nullptr)
    , m_groupListWidget(nullptr)
    , m_groupManageWidget(nullptr)
    , m_statsGroupBox(nullptr)
    , m_statsLabel(nullptr)
    , m_refreshButton(nullptr)
    , m_rightWidget(nullptr)
    , m_chatWindowWidget(nullptr)
    , m_welcomeLabel(nullptr)
    , m_autoRefreshTimer(nullptr)
    , m_groupDetailWidget(nullptr)
{
    setupUI();
    setupStyles();
    connectSignals();

    // 初始化数据
    refreshChatList();
    updateStatistics();
    showWelcomePage();

    qDebug() << "ChatPage初始化完成，用户:" << userId << userType;
}

ChatPage::~ChatPage()
{
    if (m_autoRefreshTimer) {
        m_autoRefreshTimer->stop();
    }
}

// ============================================================================
// 修复后的setupUI方法
// ============================================================================

void ChatPage::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);

    // 创建左侧标签页区域
    setupTabWidget();

    // 右侧聊天窗口区域
    m_rightWidget = new QWidget();
    m_chatWindowWidget = new ChatWindowWidget(m_database, m_currentUserId, m_currentUserType, this);

    // 欢迎页面
    m_welcomeLabel = new QLabel("选择一个聊天开始对话");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setObjectName("welcomeLabel");

    QVBoxLayout *rightLayout = new QVBoxLayout(m_rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(m_welcomeLabel);
    rightLayout->addWidget(m_chatWindowWidget);

    // 添加到分割器
    m_splitter->addWidget(m_tabWidget);
    m_splitter->addWidget(m_rightWidget);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    // 设置初始比例
    m_splitter->setSizes({350, 650});

    // 定时器
    m_autoRefreshTimer = new QTimer(this);
    m_autoRefreshTimer->setInterval(2000);
}

void ChatPage::setupTabWidget()
{
    m_tabWidget = new QTabWidget();
    m_tabWidget->setObjectName("chatTabWidget");
    m_tabWidget->setMinimumWidth(300);
    m_tabWidget->setMaximumWidth(450);

    // 创建三个标签页
    createPrivateChatTab();
    createGroupListTab();
    createGroupManageTab();

    // 连接标签切换信号
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &ChatPage::onTabChanged);
}

void ChatPage::createPrivateChatTab()
{
    m_privateChatTab = new QWidget();

    QVBoxLayout *privateChatLayout = new QVBoxLayout(m_privateChatTab);
    privateChatLayout->setContentsMargins(8, 8, 8, 8);
    privateChatLayout->setSpacing(8);

    // 标题
    QLabel *titleLabel = new QLabel("💬 私聊");
    titleLabel->setObjectName("chatTitle");

    // 聊天列表
    m_chatListWidget = new ChatListWidget(m_database, m_currentUserId, m_currentUserType, this);

    // 统计信息
    m_statsGroupBox = new QGroupBox("📊 统计信息");
    m_statsLabel = new QLabel("正在加载...");
    m_statsLabel->setObjectName("statsLabel");

    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroupBox);
    statsLayout->addWidget(m_statsLabel);

    // 刷新按钮
    m_refreshButton = new QPushButton("🔄 刷新");
    m_refreshButton->setObjectName("refreshButton");

    // 组装布局
    privateChatLayout->addWidget(titleLabel);
    privateChatLayout->addWidget(m_chatListWidget, 1);
    privateChatLayout->addWidget(m_statsGroupBox);
    privateChatLayout->addWidget(m_refreshButton);

    m_tabWidget->addTab(m_privateChatTab, "💬 私聊");
}

void ChatPage::createGroupListTab()
{
    m_groupChatTab = new QWidget();

    QVBoxLayout *groupChatLayout = new QVBoxLayout(m_groupChatTab);
    groupChatLayout->setContentsMargins(8, 8, 8, 8);
    groupChatLayout->setSpacing(8);

    // 标题
    QLabel *titleLabel = new QLabel("👥 群聊");
    titleLabel->setObjectName("chatTitle");

    // 群聊列表
    m_groupListWidget = new GroupListWidget(m_database, m_currentUserId, m_currentUserType, this);

    // 操作按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *createGroupBtn = new QPushButton("➕ 创建群聊");
    QPushButton *joinGroupBtn = new QPushButton("🔍 搜索群聊");
    createGroupBtn->setObjectName("actionButton");
    joinGroupBtn->setObjectName("actionButton");

    buttonLayout->addWidget(createGroupBtn);
    buttonLayout->addWidget(joinGroupBtn);

    // 统计信息
    QGroupBox *groupStatsBox = new QGroupBox("📊 群聊统计");
    QLabel *groupStatsLabel = new QLabel("我的群聊: 0个\n我创建的: 0个");
    groupStatsLabel->setObjectName("groupStatsLabel");

    QVBoxLayout *groupStatsLayout = new QVBoxLayout(groupStatsBox);
    groupStatsLayout->addWidget(groupStatsLabel);

    // 刷新按钮
    QPushButton *refreshGroupBtn = new QPushButton("🔄 刷新");
    refreshGroupBtn->setObjectName("refreshButton");

    // 组装布局
    groupChatLayout->addWidget(titleLabel);
    groupChatLayout->addWidget(m_groupListWidget, 1);
    groupChatLayout->addLayout(buttonLayout);
    groupChatLayout->addWidget(groupStatsBox);
    groupChatLayout->addWidget(refreshGroupBtn);

    // 连接信号
    connect(createGroupBtn, &QPushButton::clicked,
            this, &ChatPage::onCreateGroupClicked);
    connect(joinGroupBtn, &QPushButton::clicked,
            this, &ChatPage::onJoinGroupClicked);
    connect(refreshGroupBtn, &QPushButton::clicked,
            this, &ChatPage::refreshGroupList);
    connect(m_groupListWidget, &GroupListWidget::groupSelected,
            this, &ChatPage::onGroupChatSelected);

    m_tabWidget->addTab(m_groupChatTab, "👥 群聊");
}


void ChatPage::createGroupManageTab()
{
    m_groupManageTab = new QWidget();

    // 群聊管理组件 - 只占用第二栏，不使用分割器
    m_groupManageWidget = new GroupManageWidget(m_database, m_currentUserId, m_currentUserType, this);

    QVBoxLayout *manageLayout = new QVBoxLayout(m_groupManageTab);
    manageLayout->setContentsMargins(0, 0, 0, 0);
    manageLayout->addWidget(m_groupManageWidget);

    // 连接信号 - 群聊选中时在第三栏显示详情
    connect(m_groupManageWidget, &GroupManageWidget::groupRequestProcessed,
            this, &ChatPage::refreshGroupList);

    // 新增：连接群聊选择信号到第三栏显示
    connect(m_groupManageWidget, &GroupManageWidget::groupSelected,
            this, &ChatPage::showGroupDetailInThirdColumn);
    connect(m_groupManageWidget, &GroupManageWidget::noGroupSelected,
            this, &ChatPage::hideGroupDetailInThirdColumn);

    m_tabWidget->addTab(m_groupManageTab, "⚙️ 群聊管理");
}

void ChatPage::onTabChanged(int index)
{
    qDebug() << "标签页切换到索引:" << index;

    QString tabText = m_tabWidget->tabText(index);

    if (tabText.contains("私聊")) {
        m_currentChatType = "私聊";

        // 切换到私聊时，清理第三栏的所有组件
        clearThirdColumn();

        if (m_currentGroupId > 0) {
            m_currentGroupId = -1;
        }

    } else if (tabText.contains("群聊")) {
        m_currentChatType = "群聊";

        // 切换到群聊时，清理第三栏的所有组件
        clearThirdColumn();

        if (m_currentChatId > 0) {
            m_currentChatId = -1;
        }

        refreshGroupList();

    } else if (tabText.contains("管理")) {
        m_currentChatType = "管理";

        // 切换到群聊管理时，清理第三栏的聊天组件
        clearThirdColumn();

        m_groupManageWidget->refreshData();
    }
}

void ChatPage::onGroupChatSelected(int groupId, const QString &groupName)
{
    qDebug() << "选择群聊:" << groupId << groupName;

    m_currentGroupId = groupId;
    m_currentGroupName = groupName;
    m_currentChatType = "群聊";

    // 先清理第三栏的所有组件
    clearThirdColumn();

    // 隐藏欢迎页面，显示聊天窗口
    m_welcomeLabel->hide();
    m_chatWindowWidget->show();
    m_chatWindowWidget->openGroupChat(groupId, groupName);

    emit groupChatOpened(groupId, groupName);
}

void ChatPage::onGroupMessageSent(int groupId)
{
    qDebug() << "群聊消息发送完成:" << groupId;
    m_groupListWidget->refreshGroupList();
    emit messageSent(groupId);
}

void ChatPage::refreshGroupList()
{
    qDebug() << "刷新群聊列表";
    if (m_groupListWidget) {
        m_groupListWidget->refreshGroupList();
    }
    updateGroupStatistics();
}

void ChatPage::openGroupChat(int groupId, const QString &groupName)
{
    // 切换到群聊标签页
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (m_tabWidget->tabText(i).contains("群聊")) {
            m_tabWidget->setCurrentIndex(i);
            break;
        }
    }

    // 打开群聊
    onGroupChatSelected(groupId, groupName);
}

void ChatPage::updateGroupStatistics()
{
    // 获取群聊统计信息
    QList<QVariantMap> userGroups = m_database->getUserGroups(m_currentUserId, m_currentUserType);

    int totalGroups = userGroups.size();
    int createdGroups = 0;

    for (const QVariantMap &group : userGroups) {
        if (group["user_role"].toString() == "创建者") {
            createdGroups++;
        }
    }

    // 更新群聊标签页中的统计信息
    QString statsText = QString("我的群聊: %1个\n我创建的: %2个")
                            .arg(totalGroups)
                            .arg(createdGroups);

    // 查找并更新统计标签
    if (m_groupChatTab) {
        QLabel *groupStatsLabel = m_groupChatTab->findChild<QLabel*>("groupStatsLabel");
        if (groupStatsLabel) {
            groupStatsLabel->setText(statsText);
        }
    }
}

void ChatPage::onCreateGroupClicked()
{
    CreateGroupDialog dialog(m_database, m_currentUserId, m_currentUserType, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshGroupList();
        QMessageBox::information(this, "成功", "群聊创建成功！");
    }
}

void ChatPage::onJoinGroupClicked()
{
    JoinGroupDialog dialog(m_database, m_currentUserId, m_currentUserType, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshGroupList();
        QMessageBox::information(this, "申请已发送", "加群申请已发送，等待群主审核。");
    }
}


void ChatPage::connectSignals()
{
    // 私聊相关信号
    connect(m_chatListWidget, &ChatListWidget::chatSelected,
            this, &ChatPage::onChatSelected);

    connect(m_chatWindowWidget, &ChatWindowWidget::messageSent,
            this, &ChatPage::onMessageSent);

    connect(m_refreshButton, &QPushButton::clicked,
            this, &ChatPage::refreshChatList);

    // 定时器
    connect(m_autoRefreshTimer, &QTimer::timeout,
            this, &ChatPage::onAutoRefresh);

    m_autoRefreshTimer->start();
}

void ChatPage::refreshChatList()
{
    qDebug() << "刷新聊天列表";
    m_chatListWidget->refreshChatList();
    updateStatistics();
}

void ChatPage::openChatWith(int friendId, const QString &friendType, const QString &friendName)
{
    qDebug() << "打开与" << friendName << "的聊天";

    // 检查是否可以聊天（是否为好友）
    if (!m_database->canChat(m_currentUserId, m_currentUserType, friendId, friendType)) {
        QMessageBox::warning(this, "无法聊天", "只能与好友进行私聊");
        return;
    }

    // 获取或创建私聊关系
    int chatId = m_database->getOrCreatePrivateChat(m_currentUserId, m_currentUserType, friendId, friendType);
    if (chatId <= 0) {
        QMessageBox::critical(this, "错误", "无法创建聊天关系");
        return;
    }

    // 打开聊天窗口
    onChatSelected(chatId, friendId, friendName);

    // 刷新聊天列表（可能新增了聊天项）
    refreshChatList();
}

void ChatPage::onChatSelected(int chatId, int friendId, const QString &friendName)
{
    qDebug() << "选择聊天:" << chatId << friendName;

    m_currentChatId = chatId;
    m_currentFriendName = friendName;
    m_currentChatType = "私聊";

    // 先清理第三栏的所有组件
    clearThirdColumn();

    // 隐藏欢迎页面，显示聊天窗口
    m_welcomeLabel->hide();
    m_chatWindowWidget->show();
    m_chatWindowWidget->openChat(chatId, friendId, friendName);

    emit chatOpened(friendId, friendName);
}

void ChatPage::onMessageSent(int chatId)
{
    qDebug() << "消息发送完成:" << chatId;

    // 刷新聊天列表（更新最后消息）
    m_chatListWidget->refreshChatList();

    emit messageSent(chatId);
}

void ChatPage::onAutoRefresh()
{
    // 私聊自动刷新
    if (m_chatListWidget) {
        m_chatListWidget->refreshChatList();
    }

    // 群聊自动刷新
    if (m_groupListWidget) {
        m_groupListWidget->refreshGroupList();
    }

    // 聊天窗口刷新
    if (hasActiveChat()) {
        m_chatWindowWidget->refreshMessages();
    }
}

void ChatPage::updateStatistics()
{
    int friendCount = m_database->getFriendCount(m_currentUserId, m_currentUserType);
    int pendingCount = m_database->getPendingRequestCount(m_currentUserId, m_currentUserType);

    QString statsText = QString("好友总数: %1人\n待处理申请: %2条")
                            .arg(friendCount)
                            .arg(pendingCount);

    m_statsLabel->setText(statsText);
}

void ChatPage::showWelcomePage()
{
    // 确保只显示欢迎页面
    m_welcomeLabel->show();
    m_chatWindowWidget->hide();
    m_chatWindowWidget->clearChat();

    // 如果有群聊详情组件，也要隐藏
    if (m_groupDetailWidget) {
        QVBoxLayout *rightLayout = qobject_cast<QVBoxLayout*>(m_rightWidget->layout());
        if (rightLayout) {
            rightLayout->removeWidget(m_groupDetailWidget);
        }
        m_groupDetailWidget->hide();
    }
}

// ============================================================================
// ChatListWidget 聊天列表组件实现
// ============================================================================

ChatListWidget::ChatListWidget(Database *db, int userId, const QString &userType, QWidget *parent)
    : QListWidget(parent)
    , m_database(db)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
{
    setObjectName("chatListWidget");
    setMinimumHeight(200);

    // 设置样式
    QString listStyle = R"(
        QListWidget {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            outline: none;
        }

        QListWidget::item {
            padding: 12px;
            border-bottom: 1px solid #f0f0f0;
            min-height: 60px;
        }

        QListWidget::item:selected {
            background-color: #e3f2fd;
            border-left: 4px solid #2196F3;
        }

        QListWidget::item:hover {
            background-color: #f5f5f5;
        }
    )";
    setStyleSheet(listStyle);

    // 连接信号
    connect(this, &QListWidget::itemClicked,
            this, &ChatListWidget::onChatItemClicked);
}

void ChatListWidget::refreshChatList()
{
    qDebug() << "ChatListWidget刷新聊天列表";

    clear();
    m_chatList.clear();

    // 获取私聊列表
    QList<QVariantMap> chats = m_database->getPrivateChats(m_currentUserId, m_currentUserType);

    for (const QVariantMap &chatData : chats) {
        ChatInfo chat;
        chat.setChatId(chatData["chat_id"].toInt());
        chat.setFriendId(chatData["friend_id"].toInt());
        chat.setFriendType(chatData["friend_type"].toString());
        chat.setFriendName(chatData["friend_name"].toString());

        if (chatData.contains("last_message")) {
            chat.setLastMessage(chatData["last_message"].toString());
        }
        if (chatData.contains("last_message_time")) {
            chat.setLastMessageTime(chatData["last_message_time"].toDateTime());
        }

        m_chatList.append(chat);
        createChatItem(chat);
    }

    qDebug() << "聊天列表刷新完成，共" << m_chatList.size() << "个聊天";
}

void ChatListWidget::createChatItem(const ChatInfo &chat)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(formatChatItemText(chat));
    item->setData(Qt::UserRole, chat.getChatId());
    item->setData(Qt::UserRole + 1, chat.getFriendId());
    item->setData(Qt::UserRole + 2, chat.getFriendName());

    addItem(item);
}

QString ChatListWidget::formatChatItemText(const ChatInfo &chat)
{
    QString displayName = chat.getDisplayName();
    QString lastMessage = chat.getLastMessagePreview();
    QString timeDisplay = chat.getTimeDisplay();

    QString itemText = QString("%1\n%2").arg(displayName, lastMessage);

    if (!timeDisplay.isEmpty()) {
        itemText += QString("  %1").arg(timeDisplay);
    }

    return itemText;
}

void ChatListWidget::onChatItemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int chatId = item->data(Qt::UserRole).toInt();
    int friendId = item->data(Qt::UserRole + 1).toInt();
    QString friendName = item->data(Qt::UserRole + 2).toString();

    qDebug() << "聊天列表项被点击:" << chatId << friendName;
    emit chatSelected(chatId, friendId, friendName);
}

void ChatListWidget::updateChatItem(int chatId)
{
    // 查找并更新指定聊天项
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *item = this->item(i);
        if (item && item->data(Qt::UserRole).toInt() == chatId) {
            // 重新获取聊天信息并更新
            refreshChatList();
            break;
        }
    }
}

void ChatListWidget::onNewMessageReceived(int chatId)
{
    updateChatItem(chatId);
}

void ChatListWidget::updateUnreadBadge(int chatId, int count)
{
    // 暂时不实现未读消息角标，留待后续扩展
    Q_UNUSED(chatId)
    Q_UNUSED(count)
}

// ============================================================================
// ChatWindowWidget 聊天窗口组件实现
// ============================================================================

ChatWindowWidget::ChatWindowWidget(Database *db, int userId, const QString &userType, QWidget *parent)
    : QWidget(parent)
    , m_database(db)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
    , m_currentChatId(-1)
    , m_friendId(-1)
    , m_friendName("")
    , m_isGroupChat(false)          // 新增
    , m_currentGroupId(-1)          // 新增
    , m_currentGroupName("")        // 新增
{
    setupUI();
    setupStyles();
}

void ChatWindowWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 头部区域
    m_headerWidget = new QWidget();
    m_headerWidget->setObjectName("chatHeader");
    m_headerWidget->setFixedHeight(50);

    m_headerLayout = new QHBoxLayout(m_headerWidget);
    m_headerLayout->setContentsMargins(16, 8, 16, 8);

    m_chatTitleLabel = new QLabel("选择聊天");
    m_chatTitleLabel->setObjectName("chatTitleLabel");

    m_refreshMessagesButton = new QPushButton("🔄");
    m_refreshMessagesButton->setObjectName("refreshMessagesButton");
    m_refreshMessagesButton->setFixedSize(32, 32);

    m_headerLayout->addWidget(m_chatTitleLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_refreshMessagesButton);

    // 消息区域
    m_messageArea = new QScrollArea();
    m_messageArea->setObjectName("messageArea");
    m_messageArea->setWidgetResizable(true);
    m_messageArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_messageArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_messageContainer = new QWidget();
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setContentsMargins(16, 16, 16, 16);
    m_messageLayout->setSpacing(12);
    m_messageLayout->addStretch(); // 让消息从底部开始

    m_messageArea->setWidget(m_messageContainer);

    // 输入区域
    m_inputWidget = new QWidget();
    m_inputWidget->setObjectName("inputWidget");
    m_inputWidget->setFixedHeight(120);

    m_inputLayout = new QVBoxLayout(m_inputWidget);
    m_inputLayout->setContentsMargins(16, 8, 16, 8);
    m_inputLayout->setSpacing(8);

    m_inputEdit = new QTextEdit();
    m_inputEdit->setObjectName("messageInput");
    m_inputEdit->setMaximumHeight(80);
    m_inputEdit->setPlaceholderText("在此输入消息...");

    m_buttonLayout = new QHBoxLayout();
    m_sendButton = new QPushButton("发送");
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setFixedSize(80, 32);
    m_sendButton->setEnabled(false);

    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_sendButton);

    m_inputLayout->addWidget(m_inputEdit);
    m_inputLayout->addLayout(m_buttonLayout);

    // 组装主布局
    m_mainLayout->addWidget(m_headerWidget);
    m_mainLayout->addWidget(m_messageArea, 1);
    m_mainLayout->addWidget(m_inputWidget);

    // 连接信号
    connect(m_sendButton, &QPushButton::clicked,
            this, &ChatWindowWidget::sendMessage);
    connect(m_refreshMessagesButton, &QPushButton::clicked,
            this, &ChatWindowWidget::refreshMessages);
    connect(m_inputEdit, &QTextEdit::textChanged,
            this, &ChatWindowWidget::onInputTextChanged);

    // 支持Enter发送
    m_inputEdit->installEventFilter(this);
}



void ChatWindowWidget::openChat(int chatId, int friendId, const QString &friendName)
{
    qDebug() << "打开聊天窗口:" << chatId << friendName;

    m_currentChatId = chatId;
    m_friendId = friendId;
    m_friendName = friendName;

    m_chatTitleLabel->setText(QString("与 %1 的聊天").arg(friendName));

    loadMessages();
}

void ChatWindowWidget::clearChat()
{
    m_currentChatId = -1;
    m_friendId = -1;
    m_friendName = "";
    m_isGroupChat = false;         // 新增
    m_currentGroupId = -1;         // 新增
    m_currentGroupName = "";       // 新增
    m_chatTitleLabel->setText("选择聊天");
    clearMessages();
}

void ChatWindowWidget::loadMessages(bool autoScroll)
{
    if (m_currentChatId <= 0) return;

    qDebug() << "加载聊天记录:" << m_currentChatId << "群聊模式:" << m_isGroupChat;

    clearMessages();
    m_messageList.clear();

    // 根据聊天类型获取消息
    QString chatType = m_isGroupChat ? "群聊" : "私聊";
    QList<QVariantMap> messages = m_database->getChatMessages(m_currentChatId, chatType, 50, 0);

    for (const QVariantMap &msgData : messages) {
        MessageInfo message;
        message.setMessageId(msgData["message_id"].toInt());
        message.setSenderId(msgData["sender_id"].toInt());
        message.setSenderType(msgData["sender_type"].toString());
        message.setSenderName(msgData["sender_name"].toString());
        message.setContent(msgData["content"].toString());
        message.setSendTime(msgData["send_time"].toDateTime());

        // 判断是否为自己发送的消息
        bool isFromMe = (message.getSenderId() == m_currentUserId &&
                         message.getSenderType() == m_currentUserType);
        message.setIsFromMe(isFromMe);

        m_messageList.append(message);
        addMessageBubble(message);
    }

    if (autoScroll) {
        scrollToBottom();
    }
}

void ChatWindowWidget::addMessageBubble(const MessageInfo &message)
{
    MessageBubbleWidget *bubbleWidget = new MessageBubbleWidget(message, this);

    // 如果是群聊模式，设置群聊标识
    if (m_isGroupChat) {
        bubbleWidget->setGroupChatMode(true);
    }

    // 插入到消息布局中（在stretch之前）
    int insertIndex = m_messageLayout->count() - 1; // stretch是最后一个
    m_messageLayout->insertWidget(insertIndex, bubbleWidget);
}

void ChatWindowWidget::sendMessage()
{
    if (!validateInput()) {
        return;
    }

    QString content = m_inputEdit->toPlainText().trimmed();
    qDebug() << "发送消息:" << content << "群聊模式:" << m_isGroupChat;

    // 确定聊天类型
    QString chatType = m_isGroupChat ? "群聊" : "私聊";

    // 发送消息到数据库
    int messageId = m_database->sendMessage(m_currentChatId, chatType,
                                            m_currentUserId, m_currentUserType, content);

    if (messageId <= 0) {
        QMessageBox::critical(this, "发送失败", "消息发送失败，请重试");
        return;
    }

    // 清空输入框
    m_inputEdit->clear();

    // 刷新消息列表
    refreshMessages();

    // 发出信号
    emit messageSent(m_currentChatId);

    qDebug() << "消息发送成功，ID:" << messageId;
}

void ChatWindowWidget::refreshMessages()
{
    if (m_currentChatId <= 0) return;

    // 记录刷新前最后一条消息的 ID
    int oldLastId = m_messageList.isEmpty() ? -1
                                            : m_messageList.last().getMessageId();

    loadMessages(false);         // 安静地更新

    int newLastId = m_messageList.isEmpty() ? -1
                                            : m_messageList.last().getMessageId();

    if (newLastId != oldLastId) {
        scrollToBottom();        // 只有收到了新消息才滚动
    }
}

void ChatWindowWidget::onInputTextChanged()
{
    QString text = m_inputEdit->toPlainText().trimmed();
    m_sendButton->setEnabled(!text.isEmpty());
}

void ChatWindowWidget::scrollToBottom()
{
    QScrollBar *scrollBar = m_messageArea->verticalScrollBar();
    QTimer::singleShot(100, [scrollBar]() {
        scrollBar->setValue(scrollBar->maximum());
    });
}

bool ChatWindowWidget::validateInput()
{
    if (m_currentChatId <= 0) {
        QString message = m_isGroupChat ? "请先选择群聊" : "请先选择聊天对象";
        QMessageBox::warning(this, "提示", message);
        return false;
    }

    QString content = m_inputEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        QMessageBox::warning(this, "提示", "消息内容不能为空");
        return false;
    }

    if (content.length() > 1000) {
        QMessageBox::warning(this, "提示", "消息内容过长，请控制在1000字符以内");
        return false;
    }

    // 群聊权限检查
    if (m_isGroupChat) {
        if (!m_database->isGroupMember(m_currentGroupId, m_currentUserId, m_currentUserType)) {
            QMessageBox::warning(this, "提示", "您不是该群聊的成员，无法发送消息");
            return false;
        }
    }

    return true;
}

void ChatWindowWidget::clearMessages()
{
    // 清空消息布局中的所有消息组件（保留stretch）
    while (m_messageLayout->count() > 1) {
        QLayoutItem *item = m_messageLayout->takeAt(0);
        if (item && item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_messageList.clear();
}

// 重写事件过滤器以支持Enter发送
bool ChatWindowWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_inputEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() == Qt::ShiftModifier) {
                // Shift+Enter 换行
                return false;
            } else {
                // Enter 发送消息
                if (m_sendButton->isEnabled()) {
                    sendMessage();
                }
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}



// 新增方法：打开群聊
void ChatWindowWidget::openGroupChat(int groupId, const QString &groupName)
{
    qDebug() << "打开群聊窗口:" << groupId << groupName;

    // 切换到群聊模式
    m_isGroupChat = true;
    m_currentGroupId = groupId;
    m_currentGroupName = groupName;
    m_currentChatId = groupId;  // 群聊使用 groupId 作为 chatId

    // 清理私聊相关状态
    m_friendId = -1;
    m_friendName = "";

    m_chatTitleLabel->setText(QString("群聊: %1").arg(groupName));

    loadGroupMessages();
}

// 新增方法：设置群聊模式
void ChatWindowWidget::setGroupChatMode(bool isGroupChat)
{
    m_isGroupChat = isGroupChat;
    if (!isGroupChat) {
        m_currentGroupId = -1;
        m_currentGroupName = "";
    }
}

void ChatWindowWidget::loadGroupMessages(bool autoScroll)
{
    loadMessages(autoScroll);  // 复用现有逻辑
}

bool ChatPage::hasActiveChat() const
{
    return m_currentChatId > 0 || m_currentGroupId > 0;
}

void ChatPage::setupStyles()
{
    QString styles = R"(
        QWidget {
            background-color: #f5f5f5;
            font-family: 'Microsoft YaHei', Arial, sans-serif;
        }

        #chatTabWidget {
            background-color: white;
            border: none;
        }

        #chatTabWidget::pane {
            border: 1px solid #ddd;
            border-radius: 8px;
            background-color: white;
        }

        #chatTabWidget::tab-bar {
            left: 8px;
        }

        #chatTabWidget QTabBar::tab {
            background-color: #f0f0f0;
            border: 1px solid #ddd;
            border-bottom: none;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
        }

        #chatTabWidget QTabBar::tab:selected {
            background-color: white;
            border-bottom: 1px solid white;
        }

        #chatTabWidget QTabBar::tab:hover {
            background-color: #e8e8e8;
        }

        #chatTitle {
            font-size: 16px;
            font-weight: bold;
            color: #333;
            padding: 8px;
            background-color: #e3f2fd;
            border-radius: 6px;
        }

        #actionButton {
            padding: 6px 12px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 12px;
        }

        #actionButton:hover {
            background-color: #45a049;
        }

        #statsLabel, #groupStatsLabel {
            font-size: 12px;
            color: #666;
            padding: 4px;
        }

        #refreshButton {
            padding: 8px 16px;
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
        }

        #refreshButton:hover {
            background-color: #1976D2;
        }

        #refreshButton:pressed {
            background-color: #0D47A1;
        }

        #welcomeLabel {
            font-size: 18px;
            color: #999;
            background-color: white;
        }

        QGroupBox {
            font-weight: bold;
            border: 2px solid #ddd;
            border-radius: 8px;
            margin-top: 8px;
            padding-top: 8px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px 0 8px;
        }

        QSplitter::handle {
            background-color: #ddd;
            width: 2px;
        }

        QSplitter::handle:hover {
            background-color: #bbb;
        }
    )";

    setStyleSheet(styles);
}

void ChatWindowWidget::setupStyles()
{
    QString styles = R"(
        #chatHeader {
            background-color: white;
            border-bottom: 1px solid #ddd;
        }

        #chatTitleLabel {
            font-size: 16px;
            font-weight: bold;
            color: #333;
        }

        #refreshMessagesButton {
            border: 1px solid #ddd;
            border-radius: 16px;
            background-color: #f5f5f5;
        }

        #refreshMessagesButton:hover {
            background-color: #e0e0e0;
        }

        #messageArea {
            background-color: #f8f9fa;
            border: none;
        }

        #inputWidget {
            background-color: white;
            border-top: 1px solid #ddd;
        }

        #messageInput {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 8px;
            font-size: 14px;
            background-color: white;
        }

        #messageInput:focus {
            border-color: #2196F3;
        }

        #sendButton {
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 14px;
        }

        #sendButton:hover {
            background-color: #1976D2;
        }

        #sendButton:disabled {
            background-color: #ccc;
        }
    )";

    setStyleSheet(styles);
}

// 新增：显示群聊详情
void ChatPage::showGroupDetail(int groupId, bool isCreator)
{
    qDebug() << "ChatPage显示群聊详情:" << groupId << "是否创建者:" << isCreator;

    if (m_groupDetailWidget) {
        m_groupDetailWidget->showGroupDetail(groupId, isCreator);
    }
}

// 新增：隐藏群聊详情
void ChatPage::hideGroupDetail()
{
    qDebug() << "ChatPage隐藏群聊详情";

    if (m_groupDetailWidget) {
        m_groupDetailWidget->clearDetail();
    }
}

// 新增：群聊被解散
void ChatPage::onGroupDisbanded(int groupId)
{
    qDebug() << "群聊被解散:" << groupId;

    // 刷新群聊管理数据
    if (m_groupManageWidget) {
        m_groupManageWidget->refreshData();
    }

    // 刷新群聊列表
    refreshGroupList();

    // 如果当前正在群聊中，需要关闭
    if (m_currentGroupId == groupId) {
        showWelcomePage();
        m_currentGroupId = -1;
        m_currentGroupName = "";
    }
}

// 新增：退出群聊
void ChatPage::onGroupLeft(int groupId)
{
    qDebug() << "退出群聊:" << groupId;

    // 刷新群聊管理数据
    if (m_groupManageWidget) {
        m_groupManageWidget->refreshData();
    }

    // 刷新群聊列表
    refreshGroupList();

    // 如果当前正在这个群聊中，需要关闭
    if (m_currentGroupId == groupId) {
        showWelcomePage();
        m_currentGroupId = -1;
        m_currentGroupName = "";
    }
}

// 新增：邀请成员
void ChatPage::onMemberInvited(int groupId)
{
    qDebug() << "邀请成员到群聊:" << groupId;

    // 刷新群聊详情
    if (m_groupDetailWidget) {
        m_groupDetailWidget->refreshGroupInfo();
    }

    // 刷新群聊管理数据
    if (m_groupManageWidget) {
        m_groupManageWidget->refreshData();
    }
}

// 新增方法：在第三栏显示群聊详情
void ChatPage::showGroupDetailInThirdColumn(int groupId, bool isCreator)
{
    qDebug() << "在第三栏显示群聊详情:" << groupId << "是否创建者:" << isCreator;

    // 确保群聊详情组件存在
    if (!m_groupDetailWidget) {
        m_groupDetailWidget = new GroupDetailWidget(m_database, m_currentUserId, m_currentUserType, this);

        // 连接群聊操作信号
        connect(m_groupDetailWidget, &GroupDetailWidget::groupDisbanded,
                this, &ChatPage::onGroupDisbanded);
        connect(m_groupDetailWidget, &GroupDetailWidget::groupLeft,
                this, &ChatPage::onGroupLeft);
        connect(m_groupDetailWidget, &GroupDetailWidget::memberInvited,
                this, &ChatPage::onMemberInvited);
    }

    // 先清理第三栏
    clearThirdColumn();

    // 将群聊详情组件添加到第三栏
    QVBoxLayout *rightLayout = qobject_cast<QVBoxLayout*>(m_rightWidget->layout());
    if (rightLayout) {
        // 隐藏欢迎页面
        m_welcomeLabel->hide();

        // 添加群聊详情组件
        rightLayout->addWidget(m_groupDetailWidget);
        m_groupDetailWidget->show();
        m_groupDetailWidget->showGroupDetail(groupId, isCreator);
    }
}

void ChatPage::hideGroupDetailInThirdColumn()
{
    qDebug() << "隐藏第三栏群聊详情";
    clearThirdColumn();
}


void ChatPage::clearThirdColumn()
{
    qDebug() << "清理第三栏组件";

    // 获取右侧布局
    QVBoxLayout *rightLayout = qobject_cast<QVBoxLayout*>(m_rightWidget->layout());
    if (!rightLayout) {
        qWarning() << "无法获取右侧布局";
        return;
    }

    // 隐藏聊天窗口并清空聊天
    if (m_chatWindowWidget) {
        m_chatWindowWidget->hide();
        m_chatWindowWidget->clearChat();
    }

    // 移除并隐藏群聊详情组件
    if (m_groupDetailWidget) {
        rightLayout->removeWidget(m_groupDetailWidget);
        m_groupDetailWidget->hide();
        m_groupDetailWidget->clearDetail();
    }

    // 显示欢迎页面
    m_welcomeLabel->show();
}

void ChatPage::startChatFromFriendList(int friendId, const QString &friendType, const QString &friendName)
{
    qDebug() << "从好友列表启动聊天：" << friendName << "(" << friendType << ")";

    // 1. 切换到私聊标签页
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (m_tabWidget->tabText(i).contains("私聊")) {
            m_tabWidget->setCurrentIndex(i);
            qDebug() << "切换到私聊标签页";
            break;
        }
    }

    // 2. 检查聊天权限
    if (!m_database->canChat(m_currentUserId, m_currentUserType, friendId, friendType)) {
        QMessageBox::warning(this, "无法聊天", "只能与好友进行私聊");
        qDebug() << "聊天权限检查失败";
        return;
    }

    // 3. 获取或创建私聊关系
    int chatId = m_database->getOrCreatePrivateChat(m_currentUserId, m_currentUserType, friendId, friendType);
    if (chatId <= 0) {
        QMessageBox::critical(this, "错误", "无法创建聊天关系");
        qDebug() << "私聊关系创建失败";
        return;
    }

    qDebug() << "私聊ID：" << chatId;

    // 4. 刷新聊天列表
    refreshChatList();

    // 5. 自动选择该聊天并打开聊天窗口
    onChatSelected(chatId, friendId, friendName);

    qDebug() << "好友聊天启动完成";
}
#include "chatpage.moc"
