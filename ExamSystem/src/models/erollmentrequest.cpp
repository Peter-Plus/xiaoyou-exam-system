#include "enrollmentrequest.h"

EnrollmentRequest::EnrollmentRequest()
    : studentId(0), courseId(0), status(PENDING)
{
}

EnrollmentRequest::EnrollmentRequest(int studentId, int courseId, EnrollmentStatus status)
    : studentId(studentId), courseId(courseId), status(status)
{
    enrollmentTime = QDateTime::currentDateTime();
}

QString EnrollmentRequest::getStatusText() const
{
    return statusToString(status);
}

QString EnrollmentRequest::getStudentInfo() const
{
    if (!studentName.isEmpty() && !studentGrade.isEmpty() && !studentCollege.isEmpty()) {
        return QString("%1 (%2级-%3)").arg(studentName, studentGrade, studentCollege);
    } else if (!studentName.isEmpty() && !studentGrade.isEmpty()) {
        return QString("%1 (%2级)").arg(studentName, studentGrade);
    } else if (!studentName.isEmpty()) {
        return studentName;
    }
    return QString("学生%1").arg(studentId);
}

QString EnrollmentRequest::getCourseInfo() const
{
    if (!courseName.isEmpty() && !teacherName.isEmpty()) {
        return QString("%1 (%2)").arg(courseName, teacherName);
    } else if (!courseName.isEmpty()) {
        return courseName;
    }
    return QString("课程%1").arg(courseId);
}

QString EnrollmentRequest::getEnrollmentTimeString() const
{
    if (!enrollmentTime.isValid()) {
        return "未知时间";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = enrollmentTime.secsTo(now);

    if (secondsAgo < 60) {
        return "刚刚申请";
    } else if (secondsAgo < 3600) {
        return QString("%1分钟前申请").arg(secondsAgo / 60);
    } else if (secondsAgo < 86400) {
        return QString("%1小时前申请").arg(secondsAgo / 3600);
    } else if (secondsAgo < 604800) {
        return QString("%1天前申请").arg(secondsAgo / 86400);
    } else {
        return enrollmentTime.toString("MM-dd hh:mm申请");
    }
}

bool EnrollmentRequest::isPending() const
{
    return status == PENDING;
}

bool EnrollmentRequest::isApproved() const
{
    return status == APPROVED;
}

bool EnrollmentRequest::isValid() const
{
    return studentId > 0 &&
           courseId > 0 &&
           enrollmentTime.isValid();
}

EnrollmentRequest::EnrollmentStatus EnrollmentRequest::stringToStatus(const QString &statusStr)
{
    if (statusStr == "申请中") {
        return PENDING;
    } else if (statusStr == "已通过") {
        return APPROVED;
    }
    return PENDING; // 默认值
}

QString EnrollmentRequest::statusToString(EnrollmentStatus status)
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
