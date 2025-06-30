#include "grouprequest.h"
#include <QDebug>

GroupRequest::GroupRequest()
    : m_requestId(-1)
    , m_groupId(-1)
    , m_requesterId(-1)
    , m_creatorId(-1)
    , m_status(PENDING)
    , m_type(RECEIVED)
{
}

GroupRequest::GroupRequest(int requestId, int groupId, const QString &groupName,
                           int requesterId, const QString &requesterType, const QString &requesterName,
                           const QString &requesterCollege, const QString &requesterGrade,
                           int creatorId, const QString &creatorType, const QString &creatorName,
                           RequestStatus status, const QDateTime &requestTime, RequestType type)
    : m_requestId(requestId)
    , m_groupId(groupId)
    , m_groupName(groupName)
    , m_requesterId(requesterId)
    , m_requesterType(requesterType)
    , m_requesterName(requesterName)
    , m_requesterCollege(requesterCollege)
    , m_requesterGrade(requesterGrade)
    , m_creatorId(creatorId)
    , m_creatorType(creatorType)
    , m_creatorName(creatorName)
    , m_status(status)
    , m_requestTime(requestTime)
    , m_type(type)
{
}

QString GroupRequest::getRequesterDisplayText() const
{
    if (m_requesterName.isEmpty()) {
        return QString("%1 %2").arg(m_requesterType).arg(m_requesterId);
    }

    QString displayText = m_requesterName;

    // 添加身份信息
    if (!m_requesterGrade.isEmpty()) {
        // 学生：显示年级和学院
        displayText += QString("（%1级 %2）").arg(m_requesterGrade).arg(m_requesterCollege);
    } else {
        // 教师：只显示学院
        displayText += QString("（%1）").arg(m_requesterCollege);
    }

    return displayText;
}

QString GroupRequest::getGroupDisplayText() const
{
    if (m_groupName.isEmpty()) {
        return QString("群聊 #%1").arg(m_groupId);
    }
    return m_groupName;
}

QString GroupRequest::getStatusText() const
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

QString GroupRequest::getRequestTimeText() const
{
    return formatTimeDisplay(m_requestTime);
}

QString GroupRequest::getTypeText() const
{
    switch (m_type) {
    case SENT:
        return "发送的申请";
    case RECEIVED:
        return "收到的申请";
    default:
        return "未知类型";
    }
}

QString GroupRequest::getFullDescription() const
{
    if (m_type == SENT) {
        // 我发送的申请
        return QString("申请加入群聊「%1」").arg(getGroupDisplayText());
    } else {
        // 我收到的申请
        return QString("%1 申请加入群聊「%2」")
            .arg(getRequesterDisplayText())
            .arg(getGroupDisplayText());
    }
}

bool GroupRequest::isPending() const
{
    return m_status == PENDING;
}

bool GroupRequest::isAccepted() const
{
    return m_status == ACCEPTED;
}

bool GroupRequest::isRejected() const
{
    return m_status == REJECTED;
}

bool GroupRequest::isSentByMe() const
{
    return m_type == SENT;
}

bool GroupRequest::isReceivedByMe() const
{
    return m_type == RECEIVED;
}

bool GroupRequest::canBeProcessed() const
{
    // 只有收到的、待处理的申请可以被处理
    return isReceivedByMe() && isPending();
}

bool GroupRequest::isValid() const
{
    return m_requestId > 0 &&
           m_groupId > 0 &&
           m_requesterId > 0 &&
           !m_requesterType.isEmpty() &&
           !m_groupName.isEmpty() &&
           m_requestTime.isValid();
}

QString GroupRequest::getActionDescription() const
{
    if (isSentByMe()) {
        switch (m_status) {
        case PENDING:
            return "等待群主审核";
        case ACCEPTED:
            return "申请已通过";
        case REJECTED:
            return "申请被拒绝";
        default:
            return "状态未知";
        }
    } else {
        switch (m_status) {
        case PENDING:
            return "等待您处理";
        case ACCEPTED:
            return "您已同意";
        case REJECTED:
            return "您已拒绝";
        default:
            return "状态未知";
        }
    }
}

QString GroupRequest::formatTimeDisplay(const QDateTime &dateTime) const
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
