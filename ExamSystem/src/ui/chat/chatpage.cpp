#include "chatpage.h"
#include "messagebubblewidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QScrollBar>
#include <QKeyEvent>
#include <QApplication>
#include <QTimer>
#include <QEvent>

// ============================================================================
// ChatPage 主页面实现
// ============================================================================

ChatPage::ChatPage(Database *database, int userId, const QString &userType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
    , m_currentChatId(-1)
    , m_currentFriendName("")
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

void ChatPage::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);

    // 左侧聊天列表区域
    m_leftWidget = new QWidget();
    m_leftWidget->setMinimumWidth(250);
    m_leftWidget->setMaximumWidth(400);

    m_leftLayout = new QVBoxLayout(m_leftWidget);
    m_leftLayout->setContentsMargins(8, 8, 8, 8);
    m_leftLayout->setSpacing(8);

    // 标题
    m_titleLabel = new QLabel("💬 私聊");
    m_titleLabel->setObjectName("chatTitle");

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

    // 组装左侧布局
    m_leftLayout->addWidget(m_titleLabel);
    m_leftLayout->addWidget(m_chatListWidget, 1);
    m_leftLayout->addWidget(m_statsGroupBox);
    m_leftLayout->addWidget(m_refreshButton);

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
    m_splitter->addWidget(m_leftWidget);
    m_splitter->addWidget(m_rightWidget);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    // 设置初始比例
    m_splitter->setSizes({300, 700});

    // 定时器
    m_autoRefreshTimer = new QTimer(this);
    m_autoRefreshTimer->setInterval(2000); // 2秒
}

void ChatPage::setupStyles()
{
    QString styles = R"(
        QWidget {
            background-color: #f5f5f5;
            font-family: 'Microsoft YaHei', Arial, sans-serif;
        }

        #chatTitle {
            font-size: 16px;
            font-weight: bold;
            color: #333;
            padding: 8px;
            background-color: #e3f2fd;
            border-radius: 6px;
        }

        #statsLabel {
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

void ChatPage::connectSignals()
{
    // 聊天列表信号
    connect(m_chatListWidget, &ChatListWidget::chatSelected,
            this, &ChatPage::onChatSelected);

    // 聊天窗口信号
    connect(m_chatWindowWidget, &ChatWindowWidget::messageSent,
            this, &ChatPage::onMessageSent);

    // 刷新按钮
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
    // 静默刷新聊天列表
    m_chatListWidget->refreshChatList();

    //NEW
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
    m_welcomeLabel->show();
    m_chatWindowWidget->hide();
    m_chatWindowWidget->clearChat();
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
    m_chatTitleLabel->setText("选择聊天");
    clearMessages();
}

void ChatWindowWidget::loadMessages(bool autoScroll)
{
    if (m_currentChatId <= 0) return;

    qDebug() << "加载聊天记录:" << m_currentChatId;

    clearMessages();
    m_messageList.clear();

    // 获取聊天记录
    QList<QVariantMap> messages = m_database->getChatMessages(m_currentChatId, "私聊", 50, 0);

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
        scrollToBottom();        // 只有明确要求时才滚动
    }
}

void ChatWindowWidget::addMessageBubble(const MessageInfo &message)
{
    MessageBubbleWidget *bubbleWidget = new MessageBubbleWidget(message, this);

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
    qDebug() << "发送消息:" << content;

    // 发送消息到数据库
    int messageId = m_database->sendMessage(m_currentChatId, "私聊",
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
        QMessageBox::warning(this, "提示", "请先选择聊天对象");
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

#include "chatpage.moc"
