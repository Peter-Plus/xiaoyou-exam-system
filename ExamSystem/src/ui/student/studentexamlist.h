#ifndef STUDENTEXAMLIST_H
#define STUDENTEXAMLIST_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QTimer>
#include <QMessageBox>
#include "database.h"
#include "exam.h"
#include "examtaking.h"      // 添加这一行
#include "examresult.h"      // 添加这一行

class StudentExamList : public QWidget
{
    Q_OBJECT

public:
    explicit StudentExamList(Database *database, int studentId, QWidget *parent = nullptr);

private slots:
    void refreshExamList();
    void onStatusFilterChanged();
    void onSearchTextChanged();
    void onStartExam();
    void onViewResult();
    void updateCountdown();
    void onViewSubmittedAnswers(); // 查看已提交的答案


private:
    void setupUI();
    void loadExamData();
    void applyFilters();
    void populateTable(const QList<Exam> &exams);
    QString getExamStatusForStudent(const Exam &exam);
    QString getStudentScore(int examId);
    void updateStatistics();

    Database *database;
    int studentId;

    // UI组件
    QVBoxLayout *mainLayout;
    QHBoxLayout *filterLayout;
    QComboBox *statusComboBox;
    QLineEdit *searchLineEdit;
    QPushButton *refreshButton;

    QTableWidget *examTable;
    QLabel *statisticsLabel;

    QTimer *countdownTimer;

    // 数据
    QList<Exam> allExams;
    QList<Exam> filteredExams;
};

#endif // STUDENTEXAMLIST_H
