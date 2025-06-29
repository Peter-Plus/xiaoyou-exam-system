#include "classstatistics.h"

// StudentScore类实现
StudentScore::StudentScore()
    : studentName(""), studentId(""), totalScore(0.0), percentage(0.0),
    statusText("未知"), rank(0)
{
}

StudentScore::StudentScore(const QString &name, const QString &id, double score,
                           double percent, const QString &status, int studentRank)
    : studentName(name), studentId(id), totalScore(score), percentage(percent),
    statusText(status), rank(studentRank)
{
}

// ClassStatistics类实现
ClassStatistics::ClassStatistics()
    : examId(0), examName(""), courseName(""), totalStudents(0),
    highestScore(0.0), lowestScore(0.0), averageScore(0.0),
    passingCount(0), passingRate(0.0), totalScore(0)
{
}

ClassStatistics::ClassStatistics(int examId, const QString &examName)
    : examId(examId), examName(examName), courseName(""), totalStudents(0),
    highestScore(0.0), lowestScore(0.0), averageScore(0.0),
    passingCount(0), passingRate(0.0), totalScore(0)
{
}
