#include "courseassignment.h"

CourseAssignment::CourseAssignment()
    : assignmentId(0), courseId(0), maxScore(100), status(OPEN)
{
}

CourseAssignment::CourseAssignment(int assignmentId, int courseId, const QString &title,
                                   const QString &description, const QDateTime &publishTime,
                                   const QDateTime &deadline, int maxScore, AssignmentStatus status)
    : assignmentId(assignmentId), courseId(courseId), title(title),
    description(description), publishTime(publishTime), deadline(deadline),
    maxScore(maxScore), status(status)
{
}

QString CourseAssignment::getDescriptionPreview(int maxLength) const
{
    if (description.length() <= maxLength) {
        return description;
    }
    return description.left(maxLength) + "...";
}

QString CourseAssignment::getStatusText() const
{
    return statusToString(status);
}

QString CourseAssignment::getDeadlineString() const
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secsToDeadline = now.secsTo(deadline);

    if (secsToDeadline < 0) {
        return QString("已过期 (%1)").arg(deadline.toString("MM-dd hh:mm"));
    } else if (secsToDeadline < 3600) {
        return QString("剩余%1分钟").arg(secsToDeadline / 60);
    } else if (secsToDeadline < 86400) {
        return QString("剩余%1小时").arg(secsToDeadline / 3600);
    } else if (secsToDeadline < 604800) {
        return QString("剩余%1天").arg(secsToDeadline / 86400);
    } else {
        return deadline.toString("MM-dd hh:mm截止");
    }
}

bool CourseAssignment::isOverdue() const
{
    return QDateTime::currentDateTime() > deadline;
}

bool CourseAssignment::canSubmit() const
{
    return status == OPEN && !isOverdue();
}

QString CourseAssignment::getTimeRemaining() const
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secsToDeadline = now.secsTo(deadline);

    if (secsToDeadline <= 0) {
        return "已过期";
    } else if (secsToDeadline < 3600) {
        return QString("%1分钟").arg(secsToDeadline / 60);
    } else if (secsToDeadline < 86400) {
        return QString("%1小时").arg(secsToDeadline / 3600);
    } else {
        return QString("%1天").arg(secsToDeadline / 86400);
    }
}

bool CourseAssignment::isValid() const
{
    return assignmentId > 0 &&
           courseId > 0 &&
           !title.isEmpty() &&
           !description.isEmpty() &&
           publishTime.isValid() &&
           deadline.isValid() &&
           maxScore > 0;
}

CourseAssignment::AssignmentStatus CourseAssignment::stringToStatus(const QString &statusStr)
{
    if (statusStr == "开放提交") {
        return OPEN;
    } else if (statusStr == "已截止") {
        return CLOSED;
    }
    return OPEN; // 默认值
}

QString CourseAssignment::statusToString(AssignmentStatus status)
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
