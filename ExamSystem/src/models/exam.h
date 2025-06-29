#ifndef EXAM_H
#define EXAM_H

#include <QString>
#include <QDate>
#include <QTime>
#include <QDateTime>

class Exam
{
private:
    int examId;
    QString examName;
    int courseId;
    QDate examDate;
    QTime startTime;
    QTime endTime;
    int totalScore;
    bool isPublished;
    bool isCompleted;

    // 扩展字段
    QString courseName;
    QString teacherName;
    QString status;
    int questionCount;

public:
    // 构造函数
    Exam();
    Exam(int examId, const QString &examName, int courseId, const QDate &examDate,
         const QTime &startTime, const QTime &endTime, int totalScore);

    // Getter方法
    int getExamId() const { return examId; }
    QString getExamName() const { return examName; }
    int getCourseId() const { return courseId; }
    QDate getExamDate() const { return examDate; }
    QTime getStartTime() const { return startTime; }
    QTime getEndTime() const { return endTime; }
    int getTotalScore() const { return totalScore; }
    bool getIsPublished() const { return isPublished; }
    bool getIsCompleted() const { return isCompleted; }
    QString getCourseName() const { return courseName; }
    QString getTeacherName() const { return teacherName; }
    QString getStatus() const { return status; }
    int getQuestionCount() const { return questionCount; }

    // Setter方法
    void setExamId(int id) { examId = id; }
    void setExamName(const QString &name) { examName = name; }
    void setCourseId(int id) { courseId = id; }
    void setExamDate(const QDate &date) { examDate = date; }
    void setStartTime(const QTime &time) { startTime = time; }
    void setEndTime(const QTime &time) { endTime = time; }
    void setTotalScore(int score) { totalScore = score; }
    void setIsPublished(bool published) { isPublished = published; }
    void setIsCompleted(bool completed) { isCompleted = completed; }
    void setCourseName(const QString &name) { courseName = name; }
    void setTeacherName(const QString &name) { teacherName = name; }
    void setStatus(const QString &s) { status = s; }
    void setQuestionCount(int count) { questionCount = count; }

    // 实用方法
    QString getStatusText() const;
    QString getTimeRange() const;
    bool canModify() const;
    bool canPublish() const;
    bool isValid() const;
};

#endif // EXAM_H
