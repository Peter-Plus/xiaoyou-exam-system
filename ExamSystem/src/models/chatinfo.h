#ifndef CHATINFO_H
#define CHATINFO_H

#include <QString>
#include <QDateTime>

class ChatInfo
{
public:
    // 构造函数
    ChatInfo();
    ChatInfo(int id, int fId, const QString &fType, const QString &fName);

    // 属性访问器
    int getChatId() const { return chatId; }
    void setChatId(int id) { chatId = id; }

    int getFriendId() const { return friendId; }
    void setFriendId(int id) { friendId = id; }

    QString getFriendType() const { return friendType; }
    void setFriendType(const QString &type) { friendType = type; }

    QString getFriendName() const { return friendName; }
    void setFriendName(const QString &name) { friendName = name; }

    QString getFriendDisplayInfo() const { return friendDisplayInfo; }
    void setFriendDisplayInfo(const QString &info) { friendDisplayInfo = info; }

    QString getLastMessage() const { return lastMessage; }
    void setLastMessage(const QString &message) { lastMessage = message; }

    QDateTime getLastMessageTime() const { return lastMessageTime; }
    void setLastMessageTime(const QDateTime &time) { lastMessageTime = time; }

    int getUnreadCount() const { return unreadCount; }
    void setUnreadCount(int count) { unreadCount = count; }

    // 显示格式化方法
    QString getDisplayName() const;
    QString getLastMessagePreview() const;
    QString getTimeDisplay() const;
    QString getUnreadBadge() const;

    // 比较排序方法（按最后消息时间排序）
    bool operator<(const ChatInfo &other) const;

    // 有效性检查
    bool isValid() const;

private:
    int chatId;
    int friendId;
    QString friendType;
    QString friendName;
    QString friendDisplayInfo;
    QString lastMessage;
    QDateTime lastMessageTime;
    int unreadCount;

    // 辅助方法
    QString formatRelativeTime(const QDateTime &time) const;
};

#endif // CHATINFO_H
