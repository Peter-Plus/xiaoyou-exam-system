#ifndef EXAMGRADING_H
#define EXAMGRADING_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QComboBox>
#include <QTimer>
#include "database.h"
#include "exam.h"
#include "Question.h"
#include "studentanswer.h"
#include <QInputDialog>

class ExamGrading : public QWidget
{
    Q_OBJECT

public:
    explicit ExamGrading(Database *database, int teacherId, QWidget *parent = nullptr);

private slots:
    void onExamSelectionChanged();
    void onQuestionTypeSelected();
    void onPreviousAnswer();
    void onNextAnswer();
    void onSaveGrading();
    void onBatchGrading();
    void refreshGradingData();

private:
    void setupUI();
    void loadExamList();
    void loadGradingData();
    void loadQuestionTypes();
    void loadAnswersForGrading();
    void displayCurrentAnswer();
    void updateProgressInfo();
    void updateNavigationButtons();
    void saveCurrentGrading();
    QString formatAnswerPreview(const QString &answer, int maxLength = 100);

    Database *database;
    int teacherId;
    int currentExamId;
    QString currentQuestionType;
    QList<Exam> gradingExams;
    QList<StudentAnswer> currentAnswers;
    int currentAnswerIndex;

    // UI组件
    QVBoxLayout *mainLayout;
    QHBoxLayout *topLayout;
    QSplitter *mainSplitter;

    // 顶部信息栏
    QComboBox *examComboBox;
    QLabel *examInfoLabel;
    QLabel *progressLabel;
    QPushButton *refreshButton;

    // 左侧题目筛选区域（25%）
    QWidget *filterWidget;
    QVBoxLayout *filterLayout;
    QTreeWidget *questionTypeTree;
    QLabel *filterInfoLabel;

    // 右侧批改区域（75%）
    QWidget *gradingWidget;
    QVBoxLayout *gradingLayout;

    // 学生信息区域
    QGroupBox *studentInfoGroup;
    QHBoxLayout *studentInfoLayout;
    QLabel *studentNameLabel;
    QLabel *questionInfoLabel;
    QPushButton *prevAnswerButton;
    QPushButton *nextAnswerButton;

    // 题目内容区域
    QGroupBox *questionGroup;
    QVBoxLayout *questionLayout;
    QScrollArea *questionScrollArea;
    QLabel *questionContentLabel;
    QLabel *referenceAnswerLabel;

    // 学生答案区域
    QGroupBox *answerGroup;
    QVBoxLayout *answerLayout;
    QScrollArea *answerScrollArea;
    QLabel *studentAnswerLabel;

    // 批改区域
    QGroupBox *gradingGroup;
    QHBoxLayout *gradingControlLayout;
    QLabel *scoreLabel;
    QDoubleSpinBox *scoreSpinBox;
    QLabel *maxScoreLabel;
    QLineEdit *commentEdit;
    QPushButton *saveGradingButton;
    QPushButton *batchButton;

    // 进度显示
    QProgressBar *gradingProgressBar;

    // 状态变量
    bool isGradingChanged;
};

#endif // EXAMGRADING_H
