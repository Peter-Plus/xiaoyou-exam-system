#include "exam.h"

Exam::Exam()
    : examId(0), courseId(0), totalScore(0), isPublished(false), isCompleted(false), questionCount(0)
{
}

Exam::Exam(int examId, const QString &examName, int courseId, const QDate &examDate,
           const QTime &startTime, const QTime &endTime, int totalScore)
    : examId(examId), examName(examName), courseId(courseId), examDate(examDate),
    startTime(startTime), endTime(endTime), totalScore(totalScore),
    isPublished(false), isCompleted(false), questionCount(0)
{
}

QString Exam::getStatusText() const
{
    if (!status.isEmpty()) {
        return status;
    }

    // 根据时间计算状态
    QDateTime examStart = QDateTime(examDate, startTime);
    QDateTime examEnd = QDateTime(examDate, endTime);
    QDateTime now = QDateTime::currentDateTime();

    if (!isPublished) {
        return "未发布";
    }
    if (now < examStart) {
        return "已发布";
    }
    if (now >= examStart && now <= examEnd) {
        return "进行中";
    }
    return "已结束";
}

QString Exam::getTimeRange() const
{
    return QString("%1 %2-%3")
    .arg(examDate.toString("yyyy-MM-dd"))
        .arg(startTime.toString("hh:mm"))
        .arg(endTime.toString("hh:mm"));
}

bool Exam::canModify() const
{
    // 未发布的考试可以修改
    return !isPublished;
}

bool Exam::canPublish() const
{
    // 未发布且组卷完成的考试可以发布
    return !isPublished && isCompleted && questionCount > 0;
}

bool Exam::isValid() const
{
    return !examName.isEmpty() &&
           courseId > 0 &&
           examDate.isValid() &&
           startTime.isValid() &&
           endTime.isValid() &&
           startTime < endTime &&
           totalScore > 0;
}
