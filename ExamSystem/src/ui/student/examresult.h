#ifndef EXAMRESULT_H
#define EXAMRESULT_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QProgressBar>
#include <QMessageBox>
#include "database.h"
#include "exam.h"
#include "question.h"
#include "studentanswer.h"

class ExamResult : public QWidget
{
    Q_OBJECT

public:
    explicit ExamResult(Database *database, int examId, int studentId, QWidget *parent = nullptr);

private slots:
    void onPrintResult();
    void onBackToList();

private:
    void setupUI();
    void loadExamData();
    void loadStudentAnswers();
    void displayBasicInfo();
    void displayAnswerDetails();
    void displayStatistics();
    void calculateScores();

    Database *database;
    int examId;
    int studentId;
    Exam currentExam;
    QList<Question> examQuestions;
    QList<StudentAnswer> studentAnswers;

    // 成绩统计
    double totalScore;
    double maxScore;
    double percentage;
    int correctCount;
    int totalCount;
    double objectiveScore;
    double subjectiveScore;
    double objectiveMaxScore;
    double subjectiveMaxScore;

    // UI组件
    QVBoxLayout *mainLayout;

    // 基本信息区域
    QGroupBox *basicInfoGroup;
    QGridLayout *basicInfoLayout;
    QLabel *examNameLabel;
    QLabel *examTimeLabel;
    QLabel *totalScoreLabel;
    QLabel *percentageLabel;
    QProgressBar *scoreProgressBar;

    // 操作按钮
    QHBoxLayout *buttonLayout;
    QPushButton *printButton;
    QPushButton *backButton;

    // 答题详情表格
    QGroupBox *detailGroup;
    QTableWidget *detailTable;

    // 统计信息区域
    QGroupBox *statisticsGroup;
    QGridLayout *statisticsLayout;
    QLabel *correctCountLabel;
    QLabel *objectiveScoreLabel;
    QLabel *subjectiveScoreLabel;
};

#endif // EXAMRESULT_H
