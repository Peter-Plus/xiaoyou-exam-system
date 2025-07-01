#include "submission.h"
#include <QDebug>

Submission::Submission()
    : m_assignmentId(-1), m_studentId(-1), m_score(0.0), m_status(SUBMITTED), m_maxScore(100)
{
}

Submission::Submission(int assignmentId, int studentId, const QString &content, const QDateTime &submitTime,
                       double score, const QString &feedback, const QDateTime &gradeTime,
                       SubmissionStatus status, const QString &studentName,
                       const QString &assignmentTitle, int maxScore)
    : m_assignmentId(assignmentId), m_studentId(studentId), m_content(content), m_submitTime(submitTime),
    m_score(score), m_feedback(feedback), m_gradeTime(gradeTime), m_status(status),
    m_studentName(studentName), m_assignmentTitle(assignmentTitle), m_maxScore(maxScore)
{
}

Submission::SubmissionStatus Submission::statusFromString(const QString &statusStr)
{
    if (statusStr == "已提交") {
        return SUBMITTED;
    } else if (statusStr == "已批改") {
        return GRADED;
    } else if (statusStr == "逾期提交") {
        return OVERDUE_SUBMITTED;
    }
    return SUBMITTED;  // 默认值
}

QString Submission::statusToString(SubmissionStatus status)
{
    switch (status) {
    case SUBMITTED:
        return "已提交";
    case GRADED:
        return "已批改";
    case OVERDUE_SUBMITTED:
        return "逾期提交";
    default:
        return "未知状态";
    }
}

QString Submission::getContentPreview(int maxLength) const
{
    if (m_content.length() <= maxLength) {
        return m_content;
    }

    return m_content.left(maxLength) + "...";
}

QString Submission::getSubmitTimeText() const
{
    if (!m_submitTime.isValid()) {
        return "未提交";
    }

    return m_submitTime.toString("yyyy-MM-dd hh:mm");
}

QString Submission::getGradeTimeText() const
{
    if (!m_gradeTime.isValid()) {
        return "未批改";
    }

    return m_gradeTime.toString("yyyy-MM-dd hh:mm");
}

QString Submission::getStatusText() const
{
    return statusToString(m_status);
}

QString Submission::getStatusIcon() const
{
    switch (m_status) {
    case SUBMITTED:
        return "📝";      // 已提交
    case GRADED:
        return "✅";      // 已批改
    case OVERDUE_SUBMITTED:
        return "⏰";      // 逾期提交
    default:
        return "❓";
    }
}

QString Submission::getStatusColor() const
{
    switch (m_status) {
    case SUBMITTED:
        return "#3498db";      // 蓝色（已提交）
    case GRADED:
        return "#27ae60";      // 绿色（已批改）
    case OVERDUE_SUBMITTED:
        return "#e67e22";      // 橙色（逾期）
    default:
        return "#95a5a6";      // 灰色（未知）
    }
}

QString Submission::getScoreText() const
{
    if (m_status != GRADED) {
        return "待批改";
    }

    return QString("%1/%2").arg(QString::number(m_score, 'f', 1)).arg(m_maxScore);
}

double Submission::getScorePercentage() const
{
    if (m_maxScore <= 0 || m_status != GRADED) {
        return 0.0;
    }

    return (m_score / m_maxScore) * 100.0;
}

QString Submission::getGrade() const
{
    if (m_status != GRADED) {
        return "待评";
    }

    double percentage = getScorePercentage();

    if (percentage >= 90) {
        return "A";
    } else if (percentage >= 80) {
        return "B";
    } else if (percentage >= 70) {
        return "C";
    } else if (percentage >= 60) {
        return "D";
    } else {
        return "F";
    }
}

QString Submission::getGradeColor() const
{
    QString grade = getGrade();

    if (grade == "A") {
        return "#27ae60";  // 绿色
    } else if (grade == "B") {
        return "#3498db";  // 蓝色
    } else if (grade == "C") {
        return "#f39c12";  // 橙色
    } else if (grade == "D") {
        return "#e67e22";  // 深橙色
    } else if (grade == "F") {
        return "#e74c3c";  // 红色
    } else {
        return "#95a5a6";  // 灰色（待评）
    }
}

bool Submission::isValid() const
{
    return m_assignmentId > 0 && m_studentId > 0 && !m_content.isEmpty() && m_submitTime.isValid();
}

bool Submission::isGraded() const
{
    return m_status == GRADED;
}

bool Submission::isOverdueSubmission() const
{
    return m_status == OVERDUE_SUBMITTED;
}

bool Submission::hasFeedback() const
{
    return !m_feedback.isEmpty();
}

QString Submission::getRelativeSubmitTime() const
{
    if (!m_submitTime.isValid()) {
        return "未提交";
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 seconds = m_submitTime.secsTo(currentTime);

    if (seconds < 0) {
        return "刚刚";
    }

    if (seconds < 60) {
        return "刚刚";
    } else if (seconds < 3600) {
        int minutes = seconds / 60;
        return QString("%1分钟前").arg(minutes);
    } else if (seconds < 86400) {
        int hours = seconds / 3600;
        return QString("%1小时前").arg(hours);
    } else if (seconds < 2592000) {  // 30天
        int days = seconds / 86400;
        return QString("%1天前").arg(days);
    } else {
        return getSubmitTimeText();
    }
}

bool Submission::operator<(const Submission &other) const
{
    // 按提交时间降序排列（最新的在前）
    return m_submitTime > other.m_submitTime;
}

bool Submission::operator==(const Submission &other) const
{
    return m_assignmentId == other.m_assignmentId && m_studentId == other.m_studentId;
}
