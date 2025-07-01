#ifndef SUBMISSION_H
#define SUBMISSION_H

#include <QString>
#include <QDateTime>

class Submission
{
public:
    enum SubmissionStatus {
        SUBMITTED = 0,        // 已提交
        GRADED = 1,          // 已批改
        OVERDUE_SUBMITTED = 2 // 逾期提交
    };

    Submission();
    Submission(int assignmentId, int studentId, const QString &content, const QDateTime &submitTime,
               double score = 0.0, const QString &feedback = "", const QDateTime &gradeTime = QDateTime(),
               SubmissionStatus status = SUBMITTED, const QString &studentName = "",
               const QString &assignmentTitle = "", int maxScore = 100);

    // Getter methods
    int getAssignmentId() const { return m_assignmentId; }
    int getStudentId() const { return m_studentId; }
    QString getContent() const { return m_content; }
    QDateTime getSubmitTime() const { return m_submitTime; }
    double getScore() const { return m_score; }
    QString getFeedback() const { return m_feedback; }
    QDateTime getGradeTime() const { return m_gradeTime; }
    SubmissionStatus getStatus() const { return m_status; }
    QString getStudentName() const { return m_studentName; }
    QString getAssignmentTitle() const { return m_assignmentTitle; }
    int getMaxScore() const { return m_maxScore; }

    // Setter methods
    void setAssignmentId(int assignmentId) { m_assignmentId = assignmentId; }
    void setStudentId(int studentId) { m_studentId = studentId; }
    void setContent(const QString &content) { m_content = content; }
    void setSubmitTime(const QDateTime &submitTime) { m_submitTime = submitTime; }
    void setScore(double score) { m_score = score; }
    void setFeedback(const QString &feedback) { m_feedback = feedback; }
    void setGradeTime(const QDateTime &gradeTime) { m_gradeTime = gradeTime; }
    void setStatus(SubmissionStatus status) { m_status = status; }
    void setStudentName(const QString &studentName) { m_studentName = studentName; }
    void setAssignmentTitle(const QString &assignmentTitle) { m_assignmentTitle = assignmentTitle; }
    void setMaxScore(int maxScore) { m_maxScore = maxScore; }

    // Static methods
    static SubmissionStatus statusFromString(const QString &statusStr);
    static QString statusToString(SubmissionStatus status);

    // Utility methods
    QString getContentPreview(int maxLength = 100) const;
    QString getSubmitTimeText() const;
    QString getGradeTimeText() const;
    QString getStatusText() const;
    QString getStatusIcon() const;
    QString getStatusColor() const;
    QString getScoreText() const;
    double getScorePercentage() const;
    QString getGrade() const;
    QString getGradeColor() const;
    bool isValid() const;
    bool isGraded() const;
    bool isOverdueSubmission() const;
    bool hasFeedback() const;
    QString getRelativeSubmitTime() const;

    // Operators
    bool operator<(const Submission &other) const;
    bool operator==(const Submission &other) const;

private:
    int m_assignmentId;           // 作业ID
    int m_studentId;              // 学生ID
    QString m_content;            // 提交内容
    QDateTime m_submitTime;       // 提交时间
    double m_score;               // 得分
    QString m_feedback;           // 评语
    QDateTime m_gradeTime;        // 批改时间
    SubmissionStatus m_status;    // 提交状态
    QString m_studentName;        // 学生姓名（用于教师查看）
    QString m_assignmentTitle;    // 作业标题（用于学生查看）
    int m_maxScore;               // 最大分数
};

#endif // SUBMISSION_H
