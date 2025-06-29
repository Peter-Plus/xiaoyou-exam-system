#include "studentanswer.h"

// 构造函数
StudentAnswer::StudentAnswer()
    : examId(0), questionId(0), studentId(0), score(0.0), isGraded(false), fullScore(0)
{
}

StudentAnswer::StudentAnswer(int examId, int questionId, int studentId,
                             const QString &studentAnswer, double score, bool isGraded)
    : examId(examId), questionId(questionId), studentId(studentId),
    studentAnswer(studentAnswer), score(score), isGraded(isGraded), fullScore(0)
{
}

// 是否答对
bool StudentAnswer::isCorrect() const
{
    if (fullScore <= 0) {
        return false;
    }
    return score >= fullScore;
}

// 分数显示文字
QString StudentAnswer::getScoreText() const
{
    if (fullScore <= 0) {
        return QString::number(score, 'f', 1);
    }
    return QString("%1/%2").arg(QString::number(score, 'f', 1)).arg(fullScore);
}

// 得分率
double StudentAnswer::getScorePercentage() const
{
    if (fullScore <= 0) {
        return 0.0;
    }
    return (score / fullScore) * 100.0;
}

// 是否需要人工批改
bool StudentAnswer::needsManualGrading() const
{
    // 主观题需要人工批改
    return questionType == "简答" || questionType == "论述" || questionType == "计算";
}

// 批改状态
QString StudentAnswer::getGradingStatus() const
{
    if (!isGraded) {
        if (needsManualGrading()) {
            return "待批改";
        } else {
            return "待自动批改";
        }
    }

    if (isCorrect()) {
        return "正确";
    } else if (score > 0) {
        return "部分正确";
    } else {
        return "错误";
    }
}

// 基本有效性检查
bool StudentAnswer::isValid() const
{
    return examId > 0 && questionId > 0 && studentId > 0;
}
