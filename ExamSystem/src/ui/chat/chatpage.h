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

#include "../../core/database.h"
#include "../../models/chatinfo.h"
#include "../../models/messageinfo.h"
#include "messagebubblewidget.h"

class ChatListWidget;
class ChatWindowWidget;
class MessageBubbleWidget;

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);
    ~ChatPage();

    // 公共方法
    void refreshChatList();
    void openChatWith(int friendId, const QString &friendType, const QString &friendName);
    //NEW

    bool hasActiveChat() const { return m_currentChatId > 0; }

public slots:
    void onChatSelected(int chatId, int friendId, const QString &friendName);
    void onMessageSent(int chatId);
    void onAutoRefresh();

signals:
    void messageSent(int chatId);
    void chatOpened(int friendId, const QString &friendName);

private slots:
    void updateStatistics();

private:
    void setupUI();
    void setupStyles();
    void connectSignals();
    void showWelcomePage();

    // 成员变量
    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;

    // UI组件
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧区域
    QWidget *m_leftWidget;
    QVBoxLayout *m_leftLayout;
    QLabel *m_titleLabel;
    ChatListWidget *m_chatListWidget;
    QGroupBox *m_statsGroupBox;
    QLabel *m_statsLabel;
    QPushButton *m_refreshButton;

    // 右侧区域
    QWidget *m_rightWidget;
    ChatWindowWidget *m_chatWindowWidget;
    QLabel *m_welcomeLabel;

    // 定时器
    QTimer *m_autoRefreshTimer;

    // 状态
    int m_currentChatId;
    QString m_currentFriendName;
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
    void clearChat();

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
