#include "assignmentsubmission.h"

AssignmentSubmission::AssignmentSubmission()
    : assignmentId(0), studentId(0), score(-1), status(SUBMITTED)
{
}

AssignmentSubmission::AssignmentSubmission(int assignmentId, int studentId, const QString &content,
                                           const QDateTime &submitTime, SubmissionStatus status)
    : assignmentId(assignmentId), studentId(studentId), content(content),
    submitTime(submitTime), score(-1), status(status)
{
}

QString AssignmentSubmission::getContentPreview(int maxLength) const
{
    if (content.length() <= maxLength) {
        return content;
    }
    return content.left(maxLength) + "...";
}

QString AssignmentSubmission::getStatusText() const
{
    return statusToString(status);
}

QString AssignmentSubmission::getSubmitTimeString() const
{
    if (!submitTime.isValid()) {
        return "未提交";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = submitTime.secsTo(now);

    if (secondsAgo < 60) {
        return "刚刚提交";
    } else if (secondsAgo < 3600) {
        return QString("%1分钟前提交").arg(secondsAgo / 60);
    } else if (secondsAgo < 86400) {
        return QString("%1小时前提交").arg(secondsAgo / 3600);
    } else if (secondsAgo < 604800) {
        return QString("%1天前提交").arg(secondsAgo / 86400);
    } else {
        return submitTime.toString("MM-dd hh:mm提交");
    }
}

QString AssignmentSubmission::getGradeTimeString() const
{
    if (!gradeTime.isValid()) {
        return "未批改";
    }
    return gradeTime.toString("MM-dd hh:mm批改");
}

bool AssignmentSubmission::isGraded() const
{
    return status == GRADED && gradeTime.isValid();
}

bool AssignmentSubmission::hasScore() const
{
    return score >= 0;
}

QString AssignmentSubmission::getScoreText() const
{
    if (!hasScore()) {
        return "未评分";
    }
    return QString::number(score, 'f', 1);
}

QString AssignmentSubmission::getStudentInfo() const
{
    if (!studentName.isEmpty() && !studentGrade.isEmpty()) {
        return QString("%1 (%2)").arg(studentName, studentGrade);
    } else if (!studentName.isEmpty()) {
        return studentName;
    }
    return QString("学生%1").arg(studentId);
}

bool AssignmentSubmission::isValid() const
{
    return assignmentId > 0 &&
           studentId > 0 &&
           !content.isEmpty() &&
           submitTime.isValid();
}

AssignmentSubmission::SubmissionStatus AssignmentSubmission::stringToStatus(const QString &statusStr)
{
    if (statusStr == "已提交") {
        return SUBMITTED;
    } else if (statusStr == "已批改") {
        return GRADED;
    } else if (statusStr == "逾期提交") {
        return OVERDUE;
    }
    return SUBMITTED; // 默认值
}

QString AssignmentSubmission::statusToString(SubmissionStatus status)
{
    switch (status) {
    case SUBMITTED:
        return "已提交";
    case GRADED:
        return "已批改";
    case OVERDUE:
        return "逾期提交";
    default:
        return "未知状态";
    }
}
