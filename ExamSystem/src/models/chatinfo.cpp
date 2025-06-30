#include "chatinfo.h"
#include <QTime>

ChatInfo::ChatInfo()
    : chatId(-1)
    , friendId(-1)
    , friendType("")
    , friendName("")
    , friendDisplayInfo("")
    , lastMessage("")
    , lastMessageTime()
    , unreadCount(0)
{
}

ChatInfo::ChatInfo(int id, int fId, const QString &fType, const QString &fName)
    : chatId(id)
    , friendId(fId)
    , friendType(fType)
    , friendName(fName)
    , friendDisplayInfo("")
    , lastMessage("")
    , lastMessageTime()
    , unreadCount(0)
{
    // 生成好友显示信息
    if (friendType == "学生") {
        friendDisplayInfo = QString("%1 (学生)").arg(friendName);
    } else if (friendType == "老师") {
        friendDisplayInfo = QString("%1 (老师)").arg(friendName);
    } else {
        friendDisplayInfo = friendName;
    }
}

QString ChatInfo::getDisplayName() const
{
    if (!friendDisplayInfo.isEmpty()) {
        return friendDisplayInfo;
    }
    return friendName;
}

QString ChatInfo::getLastMessagePreview() const
{
    if (lastMessage.isEmpty()) {
        return "暂无消息";
    }

    // 限制预览长度
    const int maxLength = 30;
    if (lastMessage.length() > maxLength) {
        return lastMessage.left(maxLength) + "...";
    }
    return lastMessage;
}

QString ChatInfo::getTimeDisplay() const
{
    if (!lastMessageTime.isValid()) {
        return "";
    }

    return formatRelativeTime(lastMessageTime);
}

QString ChatInfo::getUnreadBadge() const
{
    if (unreadCount <= 0) {
        return "";
    }

    if (unreadCount > 99) {
        return "99+";
    }

    return QString::number(unreadCount);
}

bool ChatInfo::operator<(const ChatInfo &other) const
{
    // 按最后消息时间降序排序（最新的在前）
    if (!lastMessageTime.isValid() && !other.lastMessageTime.isValid()) {
        return chatId > other.chatId; // 都没有消息时按ID降序
    }
    if (!lastMessageTime.isValid()) {
        return false; // 没有消息的排在后面
    }
    if (!other.lastMessageTime.isValid()) {
        return true; // 有消息的排在前面
    }
    return lastMessageTime > other.lastMessageTime;
}

bool ChatInfo::isValid() const
{
    return chatId > 0 && friendId > 0 && !friendName.isEmpty();
}

QString ChatInfo::formatRelativeTime(const QDateTime &time) const
{
    if (!time.isValid()) {
        return "";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = time.secsTo(now);

    if (secondsAgo < 60) {
        return "刚刚";
    } else if (secondsAgo < 3600) {
        int minutes = secondsAgo / 60;
        return QString("%1分钟前").arg(minutes);
    } else if (secondsAgo < 86400) {
        int hours = secondsAgo / 3600;
        return QString("%1小时前").arg(hours);
    } else if (secondsAgo < 86400 * 7) {
        int days = secondsAgo / 86400;
        return QString("%1天前").arg(days);
    } else {
        // 超过一周显示具体日期
        return time.toString("MM-dd");
    }
}
