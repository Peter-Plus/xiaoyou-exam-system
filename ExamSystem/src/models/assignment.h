#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include <QString>
#include <QDateTime>

class Assignment
{
public:
    enum AssignmentStatus {
        OPEN = 0,    // 开放提交
        CLOSED = 1   // 已截止
    };

    Assignment();
    Assignment(int assignmentId, int courseId, const QString &title, const QString &description,
               const QDateTime &publishTime, const QDateTime &deadline, int maxScore,
               AssignmentStatus status = OPEN, const QString &courseName = "");

    // Getter methods
    int getAssignmentId() const { return m_assignmentId; }
    int getCourseId() const { return m_courseId; }
    QString getTitle() const { return m_title; }
    QString getDescription() const { return m_description; }
    QDateTime getPublishTime() const { return m_publishTime; }
    QDateTime getDeadline() const { return m_deadline; }
    int getMaxScore() const { return m_maxScore; }
    AssignmentStatus getStatus() const { return m_status; }
    QString getCourseName() const { return m_courseName; }

    // Setter methods
    void setAssignmentId(int assignmentId) { m_assignmentId = assignmentId; }
    void setCourseId(int courseId) { m_courseId = courseId; }
    void setTitle(const QString &title) { m_title = title; }
    void setDescription(const QString &description) { m_description = description; }
    void setPublishTime(const QDateTime &publishTime) { m_publishTime = publishTime; }
    void setDeadline(const QDateTime &deadline) { m_deadline = deadline; }
    void setMaxScore(int maxScore) { m_maxScore = maxScore; }
    void setStatus(AssignmentStatus status) { m_status = status; }
    void setCourseName(const QString &courseName) { m_courseName = courseName; }

    // Static methods
    static AssignmentStatus statusFromString(const QString &statusStr);
    static QString statusToString(AssignmentStatus status);

    // Utility methods
    QString getDescriptionPreview(int maxLength = 100) const;
    QString getPublishTimeText() const;
    QString getDeadlineText() const;
    QString getRemainingTimeText() const;
    QString getStatusText() const;
    QString getStatusIcon() const;
    QString getStatusColor() const;
    bool isValid() const;
    bool isNearDeadline() const;
    bool isOverdue() const;
    bool canSubmit() const;
    int getUrgencyLevel() const;

    // Operators
    bool operator<(const Assignment &other) const;
    bool operator==(const Assignment &other) const;

private:
    int m_assignmentId;           // 作业ID
    int m_courseId;               // 课程ID
    QString m_title;              // 作业标题
    QString m_description;        // 作业描述
    QDateTime m_publishTime;      // 发布时间
    QDateTime m_deadline;         // 截止时间
    int m_maxScore;               // 最大分数
    AssignmentStatus m_status;    // 作业状态
    QString m_courseName;         // 课程名称（用于显示）
};

#endif // ASSIGNMENT_H
