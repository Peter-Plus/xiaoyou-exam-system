#ifndef COURSEASSIGNMENT_H
#define COURSEASSIGNMENT_H

#include <QString>
#include <QDateTime>

class CourseAssignment
{
public:
    enum AssignmentStatus {
        OPEN,       // 开放提交
        CLOSED      // 已截止
    };

private:
    int assignmentId;
    int courseId;
    QString title;
    QString description;
    QDateTime publishTime;
    QDateTime deadline;
    int maxScore;
    AssignmentStatus status;
    QString courseName;      // 扩展字段
    QString teacherName;     // 扩展字段

public:
    // 构造函数
    CourseAssignment();
    CourseAssignment(int assignmentId, int courseId, const QString &title,
                     const QString &description, const QDateTime &publishTime,
                     const QDateTime &deadline, int maxScore, AssignmentStatus status = OPEN);

    // Getter方法
    int getAssignmentId() const { return assignmentId; }
    int getCourseId() const { return courseId; }
    QString getTitle() const { return title; }
    QString getDescription() const { return description; }
    QDateTime getPublishTime() const { return publishTime; }
    QDateTime getDeadline() const { return deadline; }
    int getMaxScore() const { return maxScore; }
    AssignmentStatus getStatus() const { return status; }
    QString getCourseName() const { return courseName; }
    QString getTeacherName() const { return teacherName; }

    // Setter方法
    void setAssignmentId(int id) { assignmentId = id; }
    void setCourseId(int id) { courseId = id; }
    void setTitle(const QString &t) { title = t; }
    void setDescription(const QString &desc) { description = desc; }
    void setPublishTime(const QDateTime &time) { publishTime = time; }
    void setDeadline(const QDateTime &time) { deadline = time; }
    void setMaxScore(int score) { maxScore = score; }
    void setStatus(AssignmentStatus s) { status = s; }
    void setCourseName(const QString &name) { courseName = name; }
    void setTeacherName(const QString &name) { teacherName = name; }

    // 实用方法
    QString getDescriptionPreview(int maxLength = 100) const;
    QString getStatusText() const;
    QString getDeadlineString() const;
    bool isOverdue() const;
    bool canSubmit() const;
    QString getTimeRemaining() const;
    bool isValid() const;

    // 静态方法
    static AssignmentStatus stringToStatus(const QString &statusStr);
    static QString statusToString(AssignmentStatus status);
};

#endif // COURSEASSIGNMENT_H
