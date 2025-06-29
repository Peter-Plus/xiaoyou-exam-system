#ifndef CLASSSTATISTICS_H
#define CLASSSTATISTICS_H

#include <QString>
#include <QList>

class StudentScore
{
private:
    QString studentName;
    QString studentId;
    double totalScore;
    double percentage;
    QString statusText;         // "及格"/"不及格"
    int rank;                   // 排名

public:
    // 构造函数
    StudentScore();
    StudentScore(const QString &name, const QString &id, double score,
                 double percent, const QString &status, int studentRank = 0);

    // Getter方法
    QString getStudentName() const { return studentName; }
    QString getStudentId() const { return studentId; }
    double getTotalScore() const { return totalScore; }
    double getPercentage() const { return percentage; }
    QString getStatusText() const { return statusText; }
    int getRank() const { return rank; }

    // Setter方法
    void setStudentName(const QString &name) { studentName = name; }
    void setStudentId(const QString &id) { studentId = id; }
    void setTotalScore(double score) { totalScore = score; }
    void setPercentage(double percent) { percentage = percent; }
    void setStatusText(const QString &status) { statusText = status; }
    void setRank(int studentRank) { rank = studentRank; }

    // 实用方法
    bool isPassing() const { return percentage >= 60.0; }
    QString getScoreText() const { return QString::number(totalScore, 'f', 1); }
    QString getPercentageText() const { return QString::number(percentage, 'f', 1) + "%"; }
};

class ClassStatistics
{
private:
    int examId;
    QString examName;
    QString courseName;
    int totalStudents;          // 参考人数
    double highestScore;        // 最高分
    double lowestScore;         // 最低分
    double averageScore;        // 平均分
    int passingCount;           // 及格人数
    double passingRate;         // 及格率
    int totalScore;             // 考试总分
    QList<StudentScore> failingStudents; // 不及格学生列表

public:
    // 构造函数
    ClassStatistics();
    ClassStatistics(int examId, const QString &examName);

    // Getter方法
    int getExamId() const { return examId; }
    QString getExamName() const { return examName; }
    QString getCourseName() const { return courseName; }
    int getTotalStudents() const { return totalStudents; }
    double getHighestScore() const { return highestScore; }
    double getLowestScore() const { return lowestScore; }
    double getAverageScore() const { return averageScore; }
    int getPassingCount() const { return passingCount; }
    double getPassingRate() const { return passingRate; }
    int getTotalScore() const { return totalScore; }
    QList<StudentScore> getFailingStudents() const { return failingStudents; }

    // Setter方法
    void setExamId(int id) { examId = id; }
    void setExamName(const QString &name) { examName = name; }
    void setCourseName(const QString &name) { courseName = name; }
    void setTotalStudents(int count) { totalStudents = count; }
    void setHighestScore(double score) { highestScore = score; }
    void setLowestScore(double score) { lowestScore = score; }
    void setAverageScore(double score) { averageScore = score; }
    void setPassingCount(int count) { passingCount = count; }
    void setPassingRate(double rate) { passingRate = rate; }
    void setTotalScore(int score) { totalScore = score; }
    void setFailingStudents(const QList<StudentScore> &students) { failingStudents = students; }

    // 实用方法
    QString getPassingRateText() const { return QString::number(passingRate, 'f', 1) + "%"; }
    QString getScoreRangeText() const {
        return QString::number(lowestScore, 'f', 1) + " - " + QString::number(highestScore, 'f', 1);
    }
    QString getAverageScoreText() const { return QString::number(averageScore, 'f', 1); }
    int getFailingCount() const { return failingStudents.size(); }
    bool hasStatistics() const { return totalStudents > 0; }

    // 数据验证
    bool isValid() const {
        return examId > 0 && !examName.isEmpty() && totalStudents >= 0;
    }
};

#endif // CLASSSTATISTICS_H
