#ifndef PAPERCOMPOSER_H
#define PAPERCOMPOSER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>
#include "database.h"
#include "exam.h"
#include "question.h"

class PaperComposer : public QWidget
{
    Q_OBJECT

public:
    explicit PaperComposer(int examId, int teacherId, Database* database, QWidget *parent = nullptr);

private slots:
    void onTabChanged(int index);
    void onCourseFilterChanged();
    void onTypeFilterChanged();
    void onSearchTextChanged();
    void onAddQuestionClicked();
    void onRemoveQuestionClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onSavePaperClicked();
    void onPreviewPaperClicked();
    void onSharePermissionClicked();
    void onQuestionDoubleClicked(int row, int column);
    void onSelectedQuestionChanged();
    void delayedSearch();

private:
    void setupUI();
    void setupLeftPanel();
    void setupRightPanel();
    void loadAvailableQuestions();
    void loadSelectedQuestions();
    void refreshQuestionTable();
    void refreshSelectedList();
    void updateScoreInfo();
    void updateButtonStates();
    bool matchesFilter(const Question& question);
    void checkExamPublishStatus();    // 检查考试发布状态

private:
    int examId;
    int teacherId;
    Database* database;
    Exam currentExam;

    // UI组件
    QHBoxLayout* mainLayout;
    QSplitter* splitter;

    // 左侧题库区域
    QWidget* leftPanel;
    QVBoxLayout* leftLayout;
    QTabWidget* sourceTabWidget;
    QComboBox* courseFilterCombo;
    QComboBox* typeFilterCombo;
    QLineEdit* searchEdit;
    QTableWidget* questionTable;
    QPushButton* addQuestionBtn;

    // 右侧试卷区域
    QWidget* rightPanel;
    QVBoxLayout* rightLayout;
    QGroupBox* examInfoGroup;
    QLabel* examNameLabel;
    QLabel* currentScoreLabel;
    QLabel* questionCountLabel;
    QListWidget* selectedQuestionsList;
    QPushButton* removeQuestionBtn;
    QPushButton* moveUpBtn;
    QPushButton* moveDownBtn;
    QPushButton* savePaperBtn;
    QPushButton* previewPaperBtn;
    QPushButton* sharePermissionBtn;

    // 数据
    QList<Question> availableQuestions;
    QList<Question> selectedQuestions;
    int currentTotalScore;
    int targetTotalScore;

    // 定时器用于延迟搜索
    QTimer* searchTimer;
};

#endif // PAPERCOMPOSER_H
