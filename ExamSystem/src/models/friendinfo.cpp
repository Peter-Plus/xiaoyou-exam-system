#include "friendinfo.h"

FriendInfo::FriendInfo()
    : m_friendId(0)
{
}

FriendInfo::FriendInfo(int friendId, const QString &friendType, const QString &friendName,
                       const QString &friendCollege, const QString &friendGrade)
    : m_friendId(friendId)
    , m_friendType(friendType)
    , m_friendName(friendName)
    , m_friendCollege(friendCollege)
    , m_friendGrade(friendGrade)
    , m_createdTime(QDateTime::currentDateTime())
{
}

QString FriendInfo::getDisplayName() const
{
    if (isStudent()) {
        return QString("%1 (学生)").arg(m_friendName);
    } else {
        return QString("%1 (老师)").arg(m_friendName);
    }
}

QString FriendInfo::getDisplayInfo() const
{
    QString info = m_friendCollege;
    if (isStudent() && !m_friendGrade.isEmpty()) {
        info += QString(" %1级").arg(m_friendGrade);
    }
    return info;
}

QString FriendInfo::getFriendTypeText() const
{
    return m_friendType;
}

bool FriendInfo::isValid() const
{
    return m_friendId > 0 && !m_friendName.isEmpty() && !m_friendType.isEmpty();
}
