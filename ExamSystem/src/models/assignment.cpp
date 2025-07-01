#include "assignment.h"
#include <QDebug>

Assignment::Assignment()
    : m_assignmentId(-1), m_courseId(-1), m_maxScore(100), m_status(OPEN)
{
}

Assignment::Assignment(int assignmentId, int courseId, const QString &title, const QString &description,
                       const QDateTime &publishTime, const QDateTime &deadline, int maxScore,
                       AssignmentStatus status, const QString &courseName)
    : m_assignmentId(assignmentId), m_courseId(courseId), m_title(title), m_description(description),
    m_publishTime(publishTime), m_deadline(deadline), m_maxScore(maxScore),
    m_status(status), m_courseName(courseName)
{
}

Assignment::AssignmentStatus Assignment::statusFromString(const QString &statusStr)
{
    if (statusStr == "开放提交") {
        return OPEN;
    } else if (statusStr == "已截止") {
        return CLOSED;
    }
    return OPEN;  // 默认值
}

QString Assignment::statusToString(AssignmentStatus status)
{
    switch (status) {
    case OPEN:
        return "开放提交";
    case CLOSED:
        return "已截止";
    default:
        return "未知状态";
    }
}

QString Assignment::getDescriptionPreview(int maxLength) const
{
    if (m_description.length() <= maxLength) {
        return m_description;
    }

    return m_description.left(maxLength) + "...";
}

QString Assignment::getPublishTimeText() const
{
    if (!m_publishTime.isValid()) {
        return "未知时间";
    }

    return m_publishTime.toString("yyyy-MM-dd hh:mm");
}

QString Assignment::getDeadlineText() const
{
    if (!m_deadline.isValid()) {
        return "无截止时间";
    }

    return m_deadline.toString("yyyy-MM-dd hh:mm");
}

QString Assignment::getRemainingTimeText() const
{
    if (!m_deadline.isValid()) {
        return "无截止时间";
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 seconds = currentTime.secsTo(m_deadline);

    if (seconds <= 0) {
        return "已截止";
    }

    if (seconds < 3600) {
        int minutes = seconds / 60;
        return QString("还有%1分钟").arg(minutes);
    } else if (seconds < 86400) {
        int hours = seconds / 3600;
        return QString("还有%1小时").arg(hours);
    } else {
        int days = seconds / 86400;
        return QString("还有%1天").arg(days);
    }
}

QString Assignment::getStatusText() const
{
    return statusToString(m_status);
}

QString Assignment::getStatusIcon() const
{
    switch (m_status) {
    case OPEN:
        if (isNearDeadline()) {
            return "⚠️";  // 即将截止警告
        }
        return "📝";      // 开放提交
    case CLOSED:
        return "🔒";      // 已截止
    default:
        return "❓";
    }
}

QString Assignment::getStatusColor() const
{
    switch (m_status) {
    case OPEN:
        if (isNearDeadline()) {
            return "#f39c12";  // 橙色（警告）
        }
        return "#27ae60";      // 绿色（正常）
    case CLOSED:
        return "#e74c3c";      // 红色（截止）
    default:
        return "#95a5a6";      // 灰色（未知）
    }
}

bool Assignment::isValid() const
{
    return m_assignmentId > 0 && m_courseId > 0 && !m_title.isEmpty() &&
           m_publishTime.isValid() && m_deadline.isValid() && m_maxScore > 0;
}

bool Assignment::isNearDeadline() const
{
    if (!m_deadline.isValid() || m_status == CLOSED) {
        return false;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 hours = currentTime.secsTo(m_deadline) / 3600;

    return hours <= 24 && hours > 0;
}

bool Assignment::isOverdue() const
{
    if (!m_deadline.isValid()) {
        return false;
    }

    return QDateTime::currentDateTime() > m_deadline;
}

bool Assignment::canSubmit() const
{
    return m_status == OPEN && !isOverdue();
}

int Assignment::getUrgencyLevel() const
{
    if (!m_deadline.isValid() || m_status == CLOSED) {
        return 0;  // 无紧急性
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 hours = currentTime.secsTo(m_deadline) / 3600;

    if (hours <= 0) {
        return 3;  // 已过期，最高紧急
    } else if (hours <= 6) {
        return 3;  // 6小时内，最高紧急
    } else if (hours <= 24) {
        return 2;  // 24小时内，高紧急
    } else if (hours <= 72) {
        return 1;  // 3天内，中等紧急
    } else {
        return 0;  // 超过3天，无紧急性
    }
}

bool Assignment::operator<(const Assignment &other) const
{
    // 首先按紧急程度排序（紧急的在前）
    int thisUrgency = getUrgencyLevel();
    int otherUrgency = other.getUrgencyLevel();

    if (thisUrgency != otherUrgency) {
        return thisUrgency > otherUrgency;
    }

    // 相同紧急程度下，按截止时间升序排列
    return m_deadline < other.m_deadline;
}

bool Assignment::operator==(const Assignment &other) const
{
    return m_assignmentId == other.m_assignmentId && m_courseId == other.m_courseId;
}
