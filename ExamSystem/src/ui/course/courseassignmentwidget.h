#ifndef COURSEASSIGNMENTWIDGET_H
#define COURSEASSIGNMENTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTextEdit>
#include <QSplitter>
#include <QListWidgetItem>
#include <QDialog>
#include <QDateTimeEdit>
#include <QTableWidget>
#include <QSpinBox>
#include <QScrollArea>
#include <QProgressBar>
#include <QTimer>

#include "../../core/database.h"

// 前向声明，避免循环包含
class AssignmentDetailWidget;
class PublishAssignmentDialog;
class SubmitAssignmentDialog;
class GradeAssignmentDialog;  // 新增批改对话框
class CourseAssignment;
class AssignmentSubmission;

class CourseAssignmentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CourseAssignmentWidget(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);

    void refreshData();

public slots:
    void onAssignmentSelected(QListWidgetItem *item);
    void onPublishAssignment();      // 发布作业（教师）
    void onSubmitAssignment();       // 提交作业（学生）
    void onGradeAssignment();        // 批改作业（教师）
    void onRefreshClicked();
    void onEditAssignment();         // 编辑作业（教师）
    void onDeleteAssignment();       // 删除作业（教师）

signals:
    void assignmentPublished(int courseId);
    void assignmentSubmitted(int assignmentId);
    void assignmentGraded(int assignmentId);

private slots:
    void onCourseFilterChanged();
    void onStatusFilterChanged();
    void onAutoRefresh();            // 自动刷新
    void onSearchTextChanged();      // 搜索文本变化

private:
    void setupUI();
    void setupStyles();
    void setupTeacherUI();          // 教师端界面
    void setupStudentUI();          // 学生端界面
    void updateAssignmentList();
    void updateCourseFilter();
    void createAssignmentListItem(const QVariantMap &assignment);
    void showAssignmentDetail(const QVariantMap &assignment);
    void showMessage(const QString &message, bool isError = false);
    void updateButtonStates();      // 更新按钮状态
    void updateStatistics();        // 更新统计信息
    QList<QVariantMap> getFilteredAssignments();

    // 成员变量
    Database *m_database;
    int m_userId;
    QString m_userType;
    bool m_isTeacher;

    // UI组件 - 主布局
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧作业列表区域
    QWidget *m_leftWidget;
    QVBoxLayout *m_leftLayout;

    // 筛选区域（增强版）
    QGroupBox *m_filterGroup;
    QComboBox *m_courseFilterCombo;
    QComboBox *m_statusFilterCombo;
    QLineEdit *m_searchLineEdit;     // 新增搜索框
    QPushButton *m_refreshButton;

    // 作业列表
    QGroupBox *m_listGroup;
    QListWidget *m_assignmentList;
    QLabel *m_assignmentCountLabel;

    // 统计信息（新增）
    QGroupBox *m_statsGroup;
    QLabel *m_totalAssignmentsLabel;
    QLabel *m_openAssignmentsLabel;
    QLabel *m_submittedLabel;        // 学生端：已提交数量
    QLabel *m_gradedLabel;           // 教师端：已批改数量

    // 操作按钮
    QGroupBox *m_actionGroup;
    QPushButton *m_publishButton;    // 教师：发布作业
    QPushButton *m_editButton;       // 教师：编辑作业（新增）
    QPushButton *m_deleteButton;     // 教师：删除作业（新增）
    QPushButton *m_submitButton;     // 学生：提交作业
    QPushButton *m_gradeButton;      // 教师：批改作业

    // 右侧作业详情区域
    AssignmentDetailWidget *m_assignmentDetailWidget;

    // 数据缓存
    QList<QVariantMap> m_assignments;
    QList<QVariantMap> m_teacherCourses;
    QList<QVariantMap> m_studentCourses;
    int m_selectedAssignmentId;

    // 自动刷新定时器
    QTimer *m_refreshTimer;
};

// ============================================================================
// AssignmentDetailWidget - 作业详情组件（增强版）
// ============================================================================

class AssignmentDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AssignmentDetailWidget(QWidget *parent = nullptr);

    void showAssignment(const QVariantMap &assignment, bool isTeacher = false);
    void clearContent();
    void setStudentId(int studentId) { m_currentStudentId = studentId; }  // 新增方法
    Database *m_database;

private slots:
    void onViewSubmissionsClicked(); // 查看提交列表

private:
    void setupUI();
    void setupStyles();
    void showTeacherView(const QVariantMap &assignment);
    void showStudentView(const QVariantMap &assignment);
    int getCurrentStudentId() const { return m_currentStudentId; }  // 新增方法

    // UI组件
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_mainLayout;

    QLabel *m_titleLabel;
    QLabel *m_infoLabel;
    QTextEdit *m_descriptionEdit;

    // 学生端相关
    QGroupBox *m_submissionGroup;
    QTextEdit *m_submissionEdit;     // 显示提交内容
    QLabel *m_gradeLabel;            // 显示分数和反馈

    // 教师端相关
    QGroupBox *m_statsGroup;
    QLabel *m_submissionStatsLabel;  // 提交统计
    QProgressBar *m_gradeProgressBar; // 批改进度
    QPushButton *m_viewSubmissionsButton; // 查看提交按钮

    // 欢迎页面
    QWidget *m_welcomeWidget;
    QLabel *m_welcomeLabel;

    // 数据
    QVariantMap m_currentAssignment;
    bool m_isTeacherMode;

    int m_currentStudentId;  // 新增成员变量
};

// ============================================================================
// GradeAssignmentDialog - 批改作业对话框（新增）
// ============================================================================

class GradeAssignmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GradeAssignmentDialog(Database *database, int assignmentId, QWidget *parent = nullptr);
    Database *m_database;

private slots:
    void onStudentSelected();
    void onGradeSubmitted();
    void onBatchGrade();
    void onRefreshSubmissions();

private:
    void setupUI();
    void loadSubmissions();
    void loadStudentSubmission(int studentId);
    void updateGradeProgress();


    int m_assignmentId;
    QString m_assignmentTitle;
    int m_maxScore;

    // UI组件
    QSplitter *m_splitter;

    // 左侧：学生列表
    QTableWidget *m_studentTable;
    QLabel *m_progressLabel;
    QProgressBar *m_progressBar;
    QPushButton *m_refreshButton;
    QPushButton *m_batchGradeButton;

    // 右侧：批改界面
    QGroupBox *m_gradeGroup;
    QLabel *m_studentInfoLabel;
    QTextEdit *m_submissionContentEdit;
    QSpinBox *m_scoreSpinBox;
    QTextEdit *m_feedbackEdit;
    QPushButton *m_submitGradeButton;

    // 数据
    QList<QVariantMap> m_submissions;
    int m_currentStudentId;


};

// ============================================================================
// PublishAssignmentDialog - 发布作业对话框（保持原有功能）
// ============================================================================

class PublishAssignmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PublishAssignmentDialog(Database *database, int teacherId, QWidget *parent = nullptr);
    Database *m_database;

private slots:
    void onPublishClicked();
    void onCancelClicked();

private:
    void setupUI();
    bool validateInput();


    int m_teacherId;

    QComboBox *m_courseCombo;
    QLineEdit *m_titleLineEdit;
    QTextEdit *m_descriptionEdit;
    QDateTimeEdit *m_deadlineEdit;
    QLineEdit *m_maxScoreEdit;
};

// ============================================================================
// SubmitAssignmentDialog - 提交作业对话框（保持原有功能）
// ============================================================================

class SubmitAssignmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubmitAssignmentDialog(int assignmentId, const QString &assignmentTitle,
                                    Database *database, int studentId, QWidget *parent = nullptr);
    Database *m_database;

private slots:
    void onSubmitClicked();
    void onCancelClicked();

private:
    void setupUI();
    bool validateInput();

    int m_assignmentId;

    int m_studentId;
    QString m_assignmentTitle;

    QTextEdit *m_contentEdit;
};

#endif // COURSEASSIGNMENTWIDGET_H
