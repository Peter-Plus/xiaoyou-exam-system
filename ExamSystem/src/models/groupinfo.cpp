#include "groupinfo.h"
#include <QDebug>

GroupInfo::GroupInfo()
    : m_groupId(-1)
    , m_creatorId(-1)
    , m_memberCount(0)
    , m_userRole(MEMBER)
{
}

GroupInfo::GroupInfo(int groupId, const QString &groupName, int creatorId, const QString &creatorType,
                     const QString &creatorName, int memberCount, const QDateTime &createdTime,
                     const QDateTime &lastMessageTime, GroupRole userRole)
    : m_groupId(groupId)
    , m_groupName(groupName)
    , m_creatorId(creatorId)
    , m_creatorType(creatorType)
    , m_creatorName(creatorName)
    , m_memberCount(memberCount)
    , m_createdTime(createdTime)
    , m_lastMessageTime(lastMessageTime)
    , m_userRole(userRole)
{
}

QString GroupInfo::getDisplayName() const
{
    if (m_groupName.isEmpty()) {
        return QString("群聊 #%1").arg(m_groupId);
    }
    return m_groupName;
}

QString GroupInfo::getMemberCountText() const
{
    if (m_memberCount <= 0) {
        return "暂无成员";
    } else if (m_memberCount == 1) {
        return "1人";
    } else {
        return QString("%1人").arg(m_memberCount);
    }
}

QString GroupInfo::getLastActiveText() const
{
    if (!m_lastMessageTime.isValid()) {
        // 使用创建时间
        return getTimeDisplayText(m_createdTime);
    }
    return getTimeDisplayText(m_lastMessageTime);
}

QString GroupInfo::getRoleText() const
{
    switch (m_userRole) {
    case CREATOR:
        return "创建者";
    case MEMBER:
        return "成员";
    default:
        return "未知";
    }
}

QString GroupInfo::getCreatorDisplayText() const
{
    if (m_creatorName.isEmpty()) {
        return QString("%1 %2").arg(m_creatorType).arg(m_creatorId);
    }
    return QString("%1 %2").arg(m_creatorName).arg(m_creatorType);
}

QString GroupInfo::getGroupDescription() const
{
    return QString("%1 · %2 · 创建者：%3")
        .arg(getDisplayName())
        .arg(getMemberCountText())
        .arg(getCreatorDisplayText());
}

bool GroupInfo::isValid() const
{
    return m_groupId > 0 &&
           !m_groupName.isEmpty() &&
           m_creatorId > 0 &&
           !m_creatorType.isEmpty() &&
           m_memberCount >= 0;
}

bool GroupInfo::isUserCreator() const
{
    return m_userRole == CREATOR;
}

bool GroupInfo::hasRecentActivity() const
{
    if (!m_lastMessageTime.isValid()) {
        return false;
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 hoursAgo = m_lastMessageTime.secsTo(now) / 3600;
    return hoursAgo <= 24; // 24小时内有活动
}

// 私有辅助方法：时间显示格式化
QString GroupInfo::getTimeDisplayText(const QDateTime &dateTime) const
{
    if (!dateTime.isValid()) {
        return "未知时间";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secsAgo = dateTime.secsTo(now);

    if (secsAgo < 60) {
        return "刚刚";
    } else if (secsAgo < 3600) {
        int minutesAgo = secsAgo / 60;
        return QString("%1分钟前").arg(minutesAgo);
    } else if (secsAgo < 86400) {
        int hoursAgo = secsAgo / 3600;
        return QString("%1小时前").arg(hoursAgo);
    } else if (secsAgo < 604800) {
        int daysAgo = secsAgo / 86400;
        return QString("%1天前").arg(daysAgo);
    } else {
        // 超过一周显示具体日期
        return dateTime.toString("MM-dd hh:mm");
    }
}
