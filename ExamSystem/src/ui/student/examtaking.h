#ifndef EXAMTAKING_H
#define EXAMTAKING_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QTimer>
#include <QScrollArea>
#include <QMessageBox>
#include <QMap>
#include "database.h"
#include "exam.h"
#include "question.h"

class ExamTaking : public QWidget
{
    Q_OBJECT

public:
    explicit ExamTaking(Database *database, int examId, int studentId, QWidget *parent = nullptr);

private slots:
    void updateCountdown();
    void onQuestionNavigationClicked();
    void onAnswerChanged();
    void onSaveAnswer();
    void onPreviousQuestion();
    void onNextQuestion();
    void onSubmitExam();
    void autoSaveAnswers();

private:
    void setupUI();
    void loadExamData();
    void loadExamQuestions();
    void displayQuestion(int index);
    void createAnswerWidget(const Question &question);
    void saveCurrentAnswer();
    void navigateToQuestion(int index);
    void updateNavigationButtons();
    void updateQuestionProgress();
    QString getCurrentAnswer();
    void setCurrentAnswer(const QString &answer);
    void submitExamConfirm();
    void autoSubmitExam();
    void closeExam();

    Database *database;
    int examId;
    int studentId;
    Exam currentExam;
    QList<Question> examQuestions;
    QMap<int, QString> studentAnswers; // questionId -> answer
    int currentQuestionIndex;
    QDateTime examStartTime;
    QDateTime examEndTime;

    // UI组件
    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QSplitter *contentSplitter;

    // 头部区域
    QLabel *examInfoLabel;
    QLabel *countdownLabel;
    QLabel *progressLabel;
    QPushButton *submitButton;

    // 左侧导航区域
    QWidget *navigationWidget;
    QVBoxLayout *navigationLayout;
    QListWidget *questionNavList;
    QPushButton *prevButton;
    QPushButton *nextButton;

    // 右侧答题区域
    QWidget *answerWidget;
    QVBoxLayout *answerLayout;
    QScrollArea *answerScrollArea;
    QLabel *questionInfoLabel;
    QLabel *questionContentLabel;
    QWidget *answerInputWidget;
    QPushButton *saveAnswerButton;
    QPushButton *markButton;

    // 答题控件（根据题型动态创建）
    QButtonGroup *radioGroup;
    QList<QCheckBox*> checkBoxes;
    QLineEdit *lineEdit;
    QTextEdit *textEdit;

    // 定时器
    QTimer *countdownTimer;
    QTimer *autoSaveTimer;

    // 状态标记
    bool isAnswerChanged;
    bool isExamSubmitted;
};

#endif // EXAMTAKING_H
