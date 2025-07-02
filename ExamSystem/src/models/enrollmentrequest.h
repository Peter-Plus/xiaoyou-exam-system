#ifndef ENROLLMENTREQUEST_H
#define ENROLLMENTREQUEST_H

#include <QString>
#include <QDateTime>

class EnrollmentRequest
{
public:
    enum EnrollmentStatus {
        PENDING,    // 申请中
        APPROVED    // 已通过
    };

private:
    int studentId;
    int courseId;
    EnrollmentStatus status;
    QDateTime enrollmentTime;
    QString studentName;      // 扩展字段
    QString studentGrade;     // 扩展字段
    QString studentCollege;   // 扩展字段
    QString courseName;       // 扩展字段
    QString courseCollege;    // 扩展字段
    QString teacherName;      // 扩展字段

public:
    // 构造函数
    EnrollmentRequest();
    EnrollmentRequest(int studentId, int courseId, EnrollmentStatus status = PENDING);

    // Getter方法
    int getStudentId() const { return studentId; }
    int getCourseId() const { return courseId; }
    EnrollmentStatus getStatus() const { return status; }
    QDateTime getEnrollmentTime() const { return enrollmentTime; }
    QString getStudentName() const { return studentName; }
    QString getStudentGrade() const { return studentGrade; }
    QString getStudentCollege() const { return studentCollege; }
    QString getCourseName() const { return courseName; }
    QString getCourseCollege() const { return courseCollege; }
    QString getTeacherName() const { return teacherName; }

    // Setter方法
    void setStudentId(int id) { studentId = id; }
    void setCourseId(int id) { courseId = id; }
    void setStatus(EnrollmentStatus s) { status = s; }
    void setEnrollmentTime(const QDateTime &time) { enrollmentTime = time; }
    void setStudentName(const QString &name) { studentName = name; }
    void setStudentGrade(const QString &grade) { studentGrade = grade; }
    void setStudentCollege(const QString &college) { studentCollege = college; }
    void setCourseName(const QString &name) { courseName = name; }
    void setCourseCollege(const QString &college) { courseCollege = college; }
    void setTeacherName(const QString &name) { teacherName = name; }

    // 实用方法
    QString getStatusText() const;
    QString getStudentInfo() const;
    QString getCourseInfo() const;
    QString getEnrollmentTimeString() const;
    bool isPending() const;
    bool isApproved() const;
    bool isValid() const;

    // 静态方法
    static EnrollmentStatus stringToStatus(const QString &statusStr);
    static QString statusToString(EnrollmentStatus status);
};

#endif // ENROLLMENTREQUEST_H
