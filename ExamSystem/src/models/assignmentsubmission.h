#ifndef ASSIGNMENTSUBMISSION_H
#define ASSIGNMENTSUBMISSION_H

#include <QString>
#include <QDateTime>

class AssignmentSubmission
{
public:
    enum SubmissionStatus {
        SUBMITTED,     // 已提交
        GRADED,        // 已批改
        OVERDUE        // 逾期提交
    };

private:
    int assignmentId;
    int studentId;
    QString content;
    QDateTime submitTime;
    double score;
    QString feedback;
    QDateTime gradeTime;
    SubmissionStatus status;
    QString studentName;     // 扩展字段
    QString studentGrade;    // 扩展字段

public:
    // 构造函数
    AssignmentSubmission();
    AssignmentSubmission(int assignmentId, int studentId, const QString &content,
                         const QDateTime &submitTime, SubmissionStatus status = SUBMITTED);

    // Getter方法
    int getAssignmentId() const { return assignmentId; }
    int getStudentId() const { return studentId; }
    QString getContent() const { return content; }
    QDateTime getSubmitTime() const { return submitTime; }
    double getScore() const { return score; }
    QString getFeedback() const { return feedback; }
    QDateTime getGradeTime() const { return gradeTime; }
    SubmissionStatus getStatus() const { return status; }
    QString getStudentName() const { return studentName; }
    QString getStudentGrade() const { return studentGrade; }

    // Setter方法
    void setAssignmentId(int id) { assignmentId = id; }
    void setStudentId(int id) { studentId = id; }
    void setContent(const QString &c) { content = c; }
    void setSubmitTime(const QDateTime &time) { submitTime = time; }
    void setScore(double s) { score = s; }
    void setFeedback(const QString &f) { feedback = f; }
    void setGradeTime(const QDateTime &time) { gradeTime = time; }
    void setStatus(SubmissionStatus s) { status = s; }
    void setStudentName(const QString &name) { studentName = name; }
    void setStudentGrade(const QString &grade) { studentGrade = grade; }

    // 实用方法
    QString getContentPreview(int maxLength = 200) const;
    QString getStatusText() const;
    QString getSubmitTimeString() const;
    QString getGradeTimeString() const;
    bool isGraded() const;
    bool hasScore() const;
    QString getScoreText() const;
    QString getStudentInfo() const;
    bool isValid() const;

    // 静态方法
    static SubmissionStatus stringToStatus(const QString &statusStr);
    static QString statusToString(SubmissionStatus status);
};

#endif // ASSIGNMENTSUBMISSION_H
