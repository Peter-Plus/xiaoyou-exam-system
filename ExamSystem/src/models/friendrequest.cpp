#include "friendrequest.h"

FriendRequest::FriendRequest()
    : m_requestId(0)
    , m_userId(0)
    , m_requestType(RECEIVED)
    , m_status(PENDING)
    , m_requestTime(QDateTime::currentDateTime())
{
}

FriendRequest::FriendRequest(int requestId, int userId, const QString &userType,
                             const QString &userName, const QString &userCollege,
                             const QString &userGrade)
    : m_requestId(requestId)
    , m_userId(userId)
    , m_userType(userType)
    , m_userName(userName)
    , m_userCollege(userCollege)
    , m_userGrade(userGrade)
    , m_requestType(RECEIVED)
    , m_status(PENDING)
    , m_requestTime(QDateTime::currentDateTime())
{
}

QString FriendRequest::getDisplayName() const
{
    if (isStudent()) {
        return QString("%1 (学生)").arg(m_userName);
    } else {
        return QString("%1 (老师)").arg(m_userName);
    }
}

QString FriendRequest::getDisplayInfo() const
{
    QString info = m_userCollege;
    if (isStudent() && !m_userGrade.isEmpty()) {
        info += QString(" %1级").arg(m_userGrade);
    }
    return info;
}

QString FriendRequest::getStatusText() const
{
    switch (m_status) {
    case PENDING:
        return "申请中";
    case ACCEPTED:
        return "已同意";
    case REJECTED:
        return "已拒绝";
    default:
        return "未知状态";
    }
}

QString FriendRequest::getRequestTypeText() const
{
    switch (m_requestType) {
    case RECEIVED:
        return "收到的请求";
    case SENT:
        return "发送的请求";
    default:
        return "未知类型";
    }
}

QString FriendRequest::getTimeText() const
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = m_requestTime.secsTo(now);

    if (secondsAgo < 60) {
        return "刚刚";
    } else if (secondsAgo < 3600) {
        return QString("%1分钟前").arg(secondsAgo / 60);
    } else if (secondsAgo < 86400) {
        return QString("%1小时前").arg(secondsAgo / 3600);
    } else if (secondsAgo < 2592000) { // 30天
        return QString("%1天前").arg(secondsAgo / 86400);
    } else {
        return m_requestTime.toString("yyyy-MM-dd");
    }
}

bool FriendRequest::isValid() const
{
    return m_requestId > 0 && m_userId > 0 && !m_userName.isEmpty() && !m_userType.isEmpty();
}
