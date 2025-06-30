#include "groupmember.h"
#include <QDebug>

GroupMember::GroupMember()
    : m_userId(-1)
    , m_role(MEMBER)
{
}

GroupMember::GroupMember(int userId, const QString &userType, const QString &userName,
                         const QString &userCollege, const QString &userGrade,
                         MemberRole role, const QDateTime &joinTime)
    : m_userId(userId)
    , m_userType(userType)
    , m_userName(userName)
    , m_userCollege(userCollege)
    , m_userGrade(userGrade)
    , m_role(role)
    , m_joinTime(joinTime)
{
}

QString GroupMember::getDisplayName() const
{
    if (m_userName.isEmpty()) {
        return QString("%1 %2").arg(m_userType).arg(m_userId);
    }
    return m_userName;
}

QString GroupMember::getUserInfo() const
{
    QString info = getDisplayName();

    if (!m_userGrade.isEmpty()) {
        // 学生：显示年级和学院
        info += QString("（%1级 %2）").arg(m_userGrade).arg(m_userCollege);
    } else if (!m_userCollege.isEmpty()) {
        // 教师：只显示学院
        info += QString("（%1）").arg(m_userCollege);
    }

    return info;
}

QString GroupMember::getRoleText() const
{
    switch (m_role) {
    case CREATOR:
        return "创建者";
    case ADMIN:
        return "管理员";
    case MEMBER:
        return "成员";
    default:
        return "未知";
    }
}

QString GroupMember::getJoinTimeText() const
{
    return formatTimeDisplay(m_joinTime);
}

QString GroupMember::getFullDescription() const
{
    return QString("%1 · %2 · 加入时间：%3")
        .arg(getUserInfo())
        .arg(getRoleText())
        .arg(getJoinTimeText());
}

bool GroupMember::isCreator() const
{
    return m_role == CREATOR;
}

bool GroupMember::isAdmin() const
{
    return m_role == ADMIN;
}

bool GroupMember::isMember() const
{
    return m_role == MEMBER;
}

bool GroupMember::canBeRemoved() const
{
    // 创建者不能被移除
    return m_role != CREATOR;
}

bool GroupMember::hasManagePermission() const
{
    // 创建者和管理员有管理权限
    return m_role == CREATOR || m_role == ADMIN;
}

bool GroupMember::isValid() const
{
    return m_userId > 0 &&
           !m_userType.isEmpty() &&
           !m_userName.isEmpty() &&
           m_joinTime.isValid();
}

QString GroupMember::formatTimeDisplay(const QDateTime &dateTime) const
{
    if (!dateTime.isValid()) {
        return "未知时间";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secsAgo = dateTime.secsTo(now);

    if (secsAgo < 86400) {
        return "今天";
    } else if (secsAgo < 604800) {
        int daysAgo = secsAgo / 86400;
        return QString("%1天前").arg(daysAgo);
    } else {
        // 超过一周显示具体日期
        return dateTime.toString("yyyy-MM-dd");
    }
}
