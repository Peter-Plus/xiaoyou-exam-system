#ifndef EXAMMANAGER_H
#define EXAMMANAGER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>
#include <QMenu>
#include <QStatusBar>
#include "database.h"
#include "exam.h"
#include "course.h"
#include "examstatusmanager.h"
#include <QCloseEvent>


class ExamManager : public QWidget
{
    Q_OBJECT

public:
    explicit ExamManager(int teacherId, Database* database, QWidget *parent = nullptr);
    ~ExamManager();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNewExamClicked();
    void onPublishExamClicked();
    void onTerminateExamClicked();
    void onDeleteExamClicked();
    void onRefreshClicked();
    void onCourseFilterChanged();
    void onStatusFilterChanged();
    void onSearchTextChanged();
    void onExamDoubleClicked(int row, int column);
    void onExamRightClicked(const QPoint &pos);
    void onExamSelectionChanged();
    void autoRefresh();
    // 状态管理器相关槽函数
    void onExamStatusChanged(int examId, const QString &oldStatus, const QString &newStatus);
    void onExamAutoPublished(int examId, const QString &examName);
    void onExamAutoTerminated(int examId, const QString &examName);
    void onStatusCheckCompleted(int totalExams, int changedExams);

private:
    void setupUI();
    void setupTable();
    void setupFilters();
    void setupButtons();
    void setupStatusBar();
    void loadExams();
    void loadCourses();
    void refreshTable();
    void updateButtonStates();
    void updateStatusBar();
    QColor getStatusColor(const QString &status);
    int getSelectedExamId();
    Exam getSelectedExam();

private:
    int teacherId;
    Database* database;
    ExamStatusManager* statusManager;

    // UI组件
    QVBoxLayout* mainLayout;
    QHBoxLayout* buttonLayout;
    QHBoxLayout* filterLayout;

    // 操作按钮
    QPushButton* newExamBtn;
    QPushButton* publishExamBtn;
    QPushButton* terminateExamBtn;
    QPushButton* deleteExamBtn;
    QPushButton* refreshBtn;

    // 筛选组件
    QComboBox* courseFilterCombo;
    QComboBox* statusFilterCombo;
    QLineEdit* searchEdit;

    // 表格
    QTableWidget* examTable;

    // 状态栏
    QLabel* statusLabel;

    // 定时器
    QTimer* refreshTimer;

    // 数据
    QList<Exam> exams;
    QList<Course> courses;
};

#endif // EXAMMANAGER_H
