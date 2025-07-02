#ifndef MYCOURSESWIDGET_H
#define MYCOURSESWIDGET_H

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
#include <QTableWidget>
#include <QHeaderView>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialog>
#include <QScrollArea>

#include "../../core/database.h"

// 前向声明
class CourseDetailWidget;
class EditCourseDialog;

class MyCoursesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyCoursesWidget(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);

    void refreshData();

public slots:
    void onCourseSelected(QListWidgetItem *item);
    void onEditCourse();         // 编辑课程（教师专用）
    void onRefreshClicked();

signals:
    void courseUpdated(int courseId);

private slots:
    void onSearchTextChanged();

private:
    void setupUI();
    void setupStyles();
    void setupTeacherUI();       // 教师端界面
    void setupStudentUI();       // 学生端界面
    void updateCourseList();
    void createCourseListItem(const QVariantMap &course);
    void showCourseDetail(const QVariantMap &course);
    void updateButtonStates();
    void showMessage(const QString &message, bool isError = false);

    // 成员变量
    Database *m_database;
    int m_userId;
    QString m_userType;
    bool m_isTeacher;

    // UI组件 - 主布局
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧课程列表区域
    QWidget *m_leftWidget;
    QVBoxLayout *m_leftLayout;

    // 搜索筛选区域
    QGroupBox *m_filterGroup;
    QLineEdit *m_searchLineEdit;
    QPushButton *m_refreshButton;

    // 课程列表
    QGroupBox *m_listGroup;
    QListWidget *m_courseList;
    QLabel *m_courseCountLabel;

    // 统计信息
    QGroupBox *m_statsGroup;
    QLabel *m_totalCoursesLabel;
    QLabel *m_totalStudentsLabel;    // 教师端：教授学生总数
    QLabel *m_totalCreditsLabel;     // 学生端：已选学分总数

    // 操作按钮（教师端）
    QGroupBox *m_actionGroup;
    QPushButton *m_editButton;       // 编辑课程信息

    // 右侧课程详情区域
    CourseDetailWidget *m_courseDetailWidget;

    // 数据缓存
    QList<QVariantMap> m_courses;
    int m_selectedCourseId;
};

// ============================================================================
// CourseDetailWidget - 课程详情组件
// ============================================================================

class CourseDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CourseDetailWidget(QWidget *parent = nullptr);

    void showCourse(const QVariantMap &course, bool isTeacher = false);
    void clearContent();
    void setDatabase(Database *database) { m_database = database; }

private slots:
    void onRefreshMembers();

private:
    void setupUI();
    void setupStyles();
    void showTeacherView(const QVariantMap &course);
    void showStudentView(const QVariantMap &course);
    void loadCourseMembers(int courseId);

    // UI组件
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_mainLayout;

    // 课程基本信息
    QGroupBox *m_basicInfoGroup;
    QLabel *m_courseNameLabel;
    QLabel *m_teacherLabel;
    QLabel *m_collegeLabel;
    QTextEdit *m_descriptionEdit;
    QLabel *m_creditsLabel;
    QLabel *m_hoursLabel;
    QLabel *m_semesterLabel;
    QLabel *m_statusLabel;

    // 班级成员列表
    QGroupBox *m_membersGroup;
    QVBoxLayout *m_membersLayout;
    QHBoxLayout *m_membersHeaderLayout;
    QLabel *m_membersCountLabel;
    QPushButton *m_refreshMembersButton;
    QTableWidget *m_membersTable;

    // 课程统计
    QGroupBox *m_statsGroup;
    QLabel *m_enrollmentStatsLabel;
    QLabel *m_capacityLabel;

    // 欢迎页面
    QWidget *m_welcomeWidget;
    QLabel *m_welcomeLabel;

    // 数据
    QVariantMap m_currentCourse;
    bool m_isTeacherMode;
    Database *m_database;
};

// ============================================================================
// EditCourseDialog - 编辑课程对话框（教师专用）
// ============================================================================

class EditCourseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditCourseDialog(Database *database, const QVariantMap &course, QWidget *parent = nullptr);

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    void setupStyles();
    void loadCourseData();
    bool validateInput();
    bool saveCourseChanges();

    Database *m_database;
    QVariantMap m_originalCourse;
    int m_courseId;

    // UI组件
    QVBoxLayout *m_mainLayout;

    // 基本信息
    QGroupBox *m_basicGroup;
    QLineEdit *m_courseNameEdit;
    QTextEdit *m_descriptionEdit;
    QSpinBox *m_creditsSpinBox;
    QSpinBox *m_hoursSpinBox;
    QLineEdit *m_semesterEdit;
    QSpinBox *m_maxStudentsSpinBox;
    QComboBox *m_statusCombo;

    // 操作按钮
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif // MYCOURSESWIDGET_H
