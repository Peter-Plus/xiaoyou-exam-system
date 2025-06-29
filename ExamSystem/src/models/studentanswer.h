#ifndef STUDENTANSWER_H
#define STUDENTANSWER_H

#include <QString>
#include <QDateTime>

class StudentAnswer
{
private:
    // 基本属性（对应数据库字段）
    int examId;
    int questionId;
    int studentId;
    QString studentAnswer;
    double score;
    bool isGraded;

    // 扩展字段（用于显示和处理）
    QString questionContent;
    QString correctAnswer;
    QString questionType;
    int fullScore;
    QString studentName;
    QDateTime answerTime;

public:
    // 构造函数
    StudentAnswer();
    StudentAnswer(int examId, int questionId, int studentId,
                  const QString &studentAnswer, double score = 0.0, bool isGraded = false);

    // Getter方法
    int getExamId() const { return examId; }
    int getQuestionId() const { return questionId; }
    int getStudentId() const { return studentId; }
    QString getStudentAnswer() const { return studentAnswer; }
    double getScore() const { return score; }
    bool getIsGraded() const { return isGraded; }
    QString getQuestionContent() const { return questionContent; }
    QString getCorrectAnswer() const { return correctAnswer; }
    QString getQuestionType() const { return questionType; }
    int getFullScore() const { return fullScore; }
    QString getStudentName() const { return studentName; }
    QDateTime getAnswerTime() const { return answerTime; }

    // Setter方法
    void setExamId(int id) { examId = id; }
    void setQuestionId(int id) { questionId = id; }
    void setStudentId(int id) { studentId = id; }
    void setStudentAnswer(const QString &answer) { studentAnswer = answer; }
    void setScore(double s) { score = s; }
    void setIsGraded(bool graded) { isGraded = graded; }
    void setQuestionContent(const QString &content) { questionContent = content; }
    void setCorrectAnswer(const QString &answer) { correctAnswer = answer; }
    void setQuestionType(const QString &type) { questionType = type; }
    void setFullScore(int s) { fullScore = s; }
    void setStudentName(const QString &name) { studentName = name; }
    void setAnswerTime(const QDateTime &time) { answerTime = time; }

    // 实用方法
    bool isCorrect() const; // 是否答对
    QString getScoreText() const; // 分数显示文字
    double getScorePercentage() const; // 得分率
    bool needsManualGrading() const; // 是否需要人工批改
    QString getGradingStatus() const; // 批改状态
    bool isValid() const; // 基本有效性检查
};

#endif // STUDENTANSWER_H
