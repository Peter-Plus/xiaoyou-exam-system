#include "enrollmentrequest.h"
#include <QDebug>

EnrollmentRequest::EnrollmentRequest()
    : m_studentId(-1), m_courseId(-1), m_status(PENDING)
{
}

EnrollmentRequest::EnrollmentRequest(int studentId, int courseId, const QDateTime &enrollmentTime,
                                     RequestStatus status, const QString &studentName,
                                     const QString &studentGrade, const QString &studentCollege,
                                     const QString &courseName, const QString &courseCollege,
                                     const QString &teacherName)
    : m_studentId(studentId), m_courseId(courseId), m_enrollmentTime(enrollmentTime),
    m_status(status), m_studentName(studentName), m_studentGrade(studentGrade),
    m_studentCollege(studentCollege), m_courseName(courseName),
    m_courseCollege(courseCollege), m_teacherName(teacherName)
{
}

EnrollmentRequest::RequestStatus EnrollmentRequest::statusFromString(const QString &statusStr)
{
    if (statusStr == "申请中") {
        return PENDING;
    } else if (statusStr == "已通过") {
        return APPROVED;
    }
    return PENDING;  // 默认值
}

QString EnrollmentRequest::statusToString(RequestStatus status)
{
    switch (status) {
    case PENDING:
        return "申请中";
    case APPROVED:
        return "已通过";
    default:
        return "未知状态";
    }
}

QString EnrollmentRequest::getEnrollmentTimeText() const
{
    if (!m_enrollmentTime.isValid()) {
        return "未知时间";
    }

    return m_enrollmentTime.toString("yyyy-MM-dd hh:mm");
}

QString EnrollmentRequest::getRelativeTimeText() const
{
    if (!m_enrollmentTime.isValid()) {
        return "未知时间";
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 seconds = m_enrollmentTime.secsTo(currentTime);

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
        return getEnrollmentTimeText();
    }
}

QString EnrollmentRequest::getStatusText() const
{
    return statusToString(m_status);
}

QString EnrollmentRequest::getStatusIcon() const
{
    switch (m_status) {
    case PENDING:
        return "⏳";      // 申请中
    case APPROVED:
        return "✅";      // 已通过
    default:
        return "❓";
    }
}

QString EnrollmentRequest::getStatusColor() const
{
    switch (m_status) {
    case PENDING:
        return "#f39c12";  // 橙色（待处理）
    case APPROVED:
        return "#27ae60";  // 绿色（已通过）
    default:
        return "#95a5a6";  // 灰色（未知）
    }
}

QString EnrollmentRequest::getStudentInfo() const
{
    if (m_studentName.isEmpty()) {
        return QString("学生ID: %1").arg(m_studentId);
    }

    QString info = m_studentName;
    if (!m_studentGrade.isEmpty() && !m_studentCollege.isEmpty()) {
        info += QString(" (%1级 %2)").arg(m_studentGrade, m_studentCollege);
    } else if (!m_studentGrade.isEmpty()) {
        info += QString(" (%1级)").arg(m_studentGrade);
    } else if (!m_studentCollege.isEmpty()) {
        info += QString(" (%1)").arg(m_studentCollege);
    }

    return info;
}

QString EnrollmentRequest::getCourseInfo() const
{
    if (m_courseName.isEmpty()) {
        return QString("课程ID: %1").arg(m_courseId);
    }

    QString info = m_courseName;
    if (!m_teacherName.isEmpty()) {
        info += QString(" - %1").arg(m_teacherName);
    }
    if (!m_courseCollege.isEmpty()) {
        info += QString(" (%1)").arg(m_courseCollege);
    }

    return info;
}

int EnrollmentRequest::getPriority() const
{
    int priority = 0;

    // 状态优先级：待处理 > 已通过
    if (m_status == PENDING) {
        priority += 1000;
    }

    // 时间优先级：越新的申请优先级越高
    if (m_enrollmentTime.isValid()) {
        qint64 hoursAgo = m_enrollmentTime.secsTo(QDateTime::currentDateTime()) / 3600;
        priority += (1000 - qMin(999LL, hoursAgo));  // 最多减999
    }

    return priority;
}

bool EnrollmentRequest::isValid() const
{
    return m_studentId > 0 && m_courseId > 0 && m_enrollmentTime.isValid();
}

bool EnrollmentRequest::isPending() const
{
    return m_status == PENDING;
}

bool EnrollmentRequest::isApproved() const
{
    return m_status == APPROVED;
}

bool EnrollmentRequest::isNew() const
{
    if (!m_enrollmentTime.isValid()) {
        return false;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 hours = m_enrollmentTime.secsTo(currentTime) / 3600;

    return hours <= 24;
}

QString EnrollmentRequest::getRequestDescription() const
{
    QString description;

    description += QString("学生 %1 申请选修课程 %2")
                       .arg(getStudentInfo(), getCourseInfo());

    description += QString("\n申请时间：%1").arg(getEnrollmentTimeText());
    description += QString("\n当前状态：%1").arg(getStatusText());

    if (isNew()) {
        description += " [新申请]";
    }

    return description;
}

bool EnrollmentRequest::operator<(const EnrollmentRequest &other) const
{
    // 按优先级降序排列（优先级高的在前）
    return getPriority() > other.getPriority();
}

bool EnrollmentRequest::operator==(const EnrollmentRequest &other) const
{
    return m_studentId == other.m_studentId && m_courseId == other.m_courseId;
}
