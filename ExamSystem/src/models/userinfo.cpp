#include "userinfo.h"

UserInfo::UserInfo()
    : m_userId(0)
    , m_relationshipStatus(CAN_ADD)
{
}

UserInfo::UserInfo(int userId, const QString &userType, const QString &userName,
                   const QString &userCollege, const QString &userGrade)
    : m_userId(userId)
    , m_userType(userType)
    , m_userName(userName)
    , m_userCollege(userCollege)
    , m_userGrade(userGrade)
    , m_relationshipStatus(CAN_ADD)
{
}

QString UserInfo::getDisplayName() const
{
    if (isStudent()) {
        return QString("%1 (ID:%2)").arg(m_userName).arg(m_userId);
    } else {
        return QString("%1 (老师ID:%2)").arg(m_userName).arg(m_userId);
    }
}

QString UserInfo::getDisplayInfo() const
{
    QString info = m_userCollege;
    if (isStudent() && !m_userGrade.isEmpty()) {
        info += QString(" %1级").arg(m_userGrade);
    }
    return info;
}

QString UserInfo::getRelationshipText() const
{
    switch (m_relationshipStatus) {
    case CAN_ADD:
        return "可添加";
    case ALREADY_FRIEND:
        return "已是好友";
    case REQUEST_SENT:
        return "已发送请求";
    case REQUEST_PENDING:
        return "待处理请求";
    default:
        return "未知状态";
    }
}

QString UserInfo::getActionButtonText() const
{
    switch (m_relationshipStatus) {
    case CAN_ADD:
        return "添加好友";
    case ALREADY_FRIEND:
        return "已是好友";
    case REQUEST_SENT:
        return "已发送";
    case REQUEST_PENDING:
        return "待处理";
    default:
        return "-";
    }
}

bool UserInfo::isValid() const
{
    return m_userId > 0 && !m_userName.isEmpty() && !m_userType.isEmpty();
}
