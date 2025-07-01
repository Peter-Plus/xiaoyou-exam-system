#ifndef ASSIGNMENTWIDGET_H
#define ASSIGNMENTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QTimer>
#include <QMessageBox>
#include <QDialog>
#include "../../core/database.h"
#include "assignment.h"
#include "submission.h"

class AssignmentEditDialog;

class AssignmentWidget : public QWidget
{
    Q_OBJECT

public:
    enum UserType {
        STUDENT = 0,
        TEACHER = 1
    };

    AssignmentWidget(Database *database, int userId, UserType userType, int courseId = -1, QWidget *parent = nullptr);

public slots:
    void refreshData();
    void setCourseId(int courseId);

signals:
    void assignmentPublished(int courseId, const QString &title);
    void assignmentSubmitted(int assignmentId, int studentId);

private slots:
    void onAssignmentSelectionChanged();
    void onCourseSelectionChanged();
    void onPublishAssignment();
    void onSubmitAssignment();
    void onGradeAssignment();
    void onViewSubmissions();

private:
    void setupUI();
    void setupStudentUI();
    void setupTeacherUI();
    void loadAssignments();
    void loadCourseList();
    void updateAssignmentTable();
    void updateStatistics();
    void showAssignmentDetails();
    void showSubmissionForm();
    void showGradingInterface();

    // UI组件
    QVBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧作业列表
    QWidget *m_listWidget;
    QComboBox *m_courseCombo;        // 教师端课程选择
    QPushButton *m_publishButton;    // 教师端发布按钮
    QTableWidget *m_assignmentTable;

    // 右侧详情/操作区域
    QWidget *m_detailWidget;
    QGroupBox *m_detailGroup;
    QLabel *m_titleLabel;
    QLabel *m_infoLabel;
    QTextEdit *m_descriptionText;
    QTextEdit *m_submissionText;     // 学生提交内容
    QPushButton *m_submitButton;     // 学生提交按钮
    QPushButton *m_gradeButton;      // 教师批改按钮
    QPushButton *m_viewButton;       // 教师查看提交按钮

    // 统计信息
    QGroupBox *m_statsGroup;
    QLabel *m_statsLabel;

    // 数据
    Database *m_database;
    int m_userId;
    UserType m_userType;
    int m_currentCourseId;

    QList<QVariantMap> m_assignments;
    QList<QVariantMap> m_courseList;
    QVariantMap m_selectedAssignment;
};

// 作业编辑对话框
class AssignmentEditDialog : public QDialog
{
    Q_OBJECT

public:
    AssignmentEditDialog(int courseId, QWidget *parent = nullptr);

    QString getTitle() const { return m_title; }
    QString getDescription() const { return m_description; }
    QDateTime getDeadline() const { return m_deadline; }
    int getMaxScore() const { return m_maxScore; }

private slots:
    void onAccept();

private:
    void setupUI();

    QLineEdit *m_titleEdit;
    QTextEdit *m_descriptionEdit;
    QDateTimeEdit *m_deadlineEdit;
    QSpinBox *m_scoreSpinBox;

    QString m_title;
    QString m_description;
    QDateTime m_deadline;
    int m_maxScore;
    int m_courseId;
};

#endif // ASSIGNMENTWIDGET_H
