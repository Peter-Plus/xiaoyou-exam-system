
#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QLabel>
#include <QTimer>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QListWidgetItem>
#include <QTabWidget>

#include "../../core/database.h"
#include "../../models/chatinfo.h"
#include "../../models/messageinfo.h"

// 前向声明
class ChatListWidget;
class ChatWindowWidget;
class MessageBubbleWidget;
class GroupListWidget;
class GroupManageWidget;

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);
    ~ChatPage();

    // 公共方法
    void refreshChatList();
    void openChatWith(int friendId, const QString &friendType, const QString &friendName);
    bool hasActiveChat() const;

    // 新增群聊方法
    void openGroupChat(int groupId, const QString &groupName);
    void refreshGroupList();

public slots:
    void onChatSelected(int chatId, int friendId, const QString &friendName);
    void onMessageSent(int chatId);
    void onAutoRefresh();

    // 新增群聊槽函数
    void onGroupChatSelected(int groupId, const QString &groupName);
    void onGroupMessageSent(int groupId);
    void onTabChanged(int index);
    void onCreateGroupClicked();
    void onJoinGroupClicked();

signals:
    void messageSent(int chatId);
    void chatOpened(int friendId, const QString &friendName);
    void groupChatOpened(int groupId, const QString &groupName);

private slots:
    void updateStatistics();
    void updateGroupStatistics();

private:
    void setupUI();
    void setupStyles();
    void connectSignals();
    void showWelcomePage();

    // 新增私有方法
    void setupTabWidget();
    void createPrivateChatTab();
    void createGroupListTab();
    void createGroupManageTab();

    // 成员变量
    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;

    // UI组件 - 主布局
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧标签页区域
    QTabWidget *m_tabWidget;
    QWidget *m_privateChatTab;
    QWidget *m_groupChatTab;
    QWidget *m_groupManageTab;

    // 私聊标签页组件
    ChatListWidget *m_chatListWidget;
    QGroupBox *m_statsGroupBox;
    QLabel *m_statsLabel;
    QPushButton *m_refreshButton;

    // 群聊标签页组件
    GroupListWidget *m_groupListWidget;

    // 群聊管理标签页组件
    GroupManageWidget *m_groupManageWidget;

    // 右侧聊天区域
    QWidget *m_rightWidget;
    ChatWindowWidget *m_chatWindowWidget;
    QLabel *m_welcomeLabel;

    // 定时器
    QTimer *m_autoRefreshTimer;

    // 状态变量
    QString m_currentChatType;  // "私聊" 或 "群聊"
    int m_currentChatId;        // 私聊ID
    QString m_currentFriendName;
    int m_currentGroupId;       // 群聊ID
    QString m_currentGroupName;
};

// ============================================================================
// ChatListWidget - 聊天列表组件
// ============================================================================

class ChatListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ChatListWidget(Database *db, int userId, const QString &userType, QWidget *parent = nullptr);

    void refreshChatList();
    void updateChatItem(int chatId);

public slots:
    void onChatItemClicked(QListWidgetItem *item);
    void onNewMessageReceived(int chatId);

signals:
    void chatSelected(int chatId, int friendId, const QString &friendName);

private:
    void createChatItem(const ChatInfo &chat);
    void updateUnreadBadge(int chatId, int count);
    QString formatChatItemText(const ChatInfo &chat);

    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;
    QList<ChatInfo> m_chatList;
};

// ============================================================================
// ChatWindowWidget - 聊天窗口组件
// ============================================================================

class ChatWindowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindowWidget(Database *db, int userId, const QString &userType, QWidget *parent = nullptr);

    void openChat(int chatId, int friendId, const QString &friendName);
    void openGroupChat(int groupId, const QString &groupName);
    void clearChat();
    void setGroupChatMode(bool isGroupChat);

public slots:
    void sendMessage();
    void refreshMessages();
    void onInputTextChanged();

signals:
    void messageSent(int chatId);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void setupStyles();
    void loadMessages(bool autoScroll = false);
    void loadGroupMessages(bool autoScroll = false);
    void addMessageBubble(const MessageInfo &message);
    void scrollToBottom();
    bool validateInput();
    void clearMessages();

    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;
    int m_currentChatId;
    int m_friendId;
    QString m_friendName;

    // 群聊相关成员变量
    bool m_isGroupChat;
    int m_currentGroupId;
    QString m_currentGroupName;

    // UI组件
    QVBoxLayout *m_mainLayout;

    // 头部
    QWidget *m_headerWidget;
    QHBoxLayout *m_headerLayout;
    QLabel *m_chatTitleLabel;
    QPushButton *m_refreshMessagesButton;

    // 消息区域
    QScrollArea *m_messageArea;
    QWidget *m_messageContainer;
    QVBoxLayout *m_messageLayout;

    // 输入区域
    QWidget *m_inputWidget;
    QVBoxLayout *m_inputLayout;
    QTextEdit *m_inputEdit;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_sendButton;

    // 状态
    QList<MessageInfo> m_messageList;
};

#endif // CHATPAGE_H
