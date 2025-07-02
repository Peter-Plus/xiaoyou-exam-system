#ifndef COURSEPAGE_H
#define COURSEPAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QGroupBox>

#include "../../core/database.h"

// 前向声明
class EnrollmentWidget;     // 选课申请组件
class EnrollmentAdminWidget; // 选课审核组件
class CourseNoticeWidget;   // 课程通知组件
class CourseAssignmentWidget; // 课程作业组件
class MyCoursesWidget;

class CoursePage : public QWidget
{
    Q_OBJECT

public:
    explicit CoursePage(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);
    ~CoursePage();

    // 公共方法
    void refreshAll();

public slots:
    void showEnrollment();      // 显示选课申请页面
    void showNotices();         // 显示课程通知页面
    void showAssignments();     // 显示课程作业页面
    void showEnrollmentAdmin(); // 显示选课审核页面（仅选课管理员）

signals:
    void enrollmentSubmitted(int courseId);
    void noticePublished(int courseId);
    void assignmentPublished(int courseId);
    void courseUpdated(int courseId);     // 课程信息更新信号

private slots:
    void onNavigationClicked();
    void autoRefresh();
    void updateStatistics();
    void showMyCourses();         // 显示我的课程页面

private:
    enum PageIndex {
        PAGE_MY_COURSES = 0,
        PAGE_ENROLLMENT = 0,    // 仅学生端
        PAGE_NOTICES = 0,       // 教师端从0开始
        PAGE_ASSIGNMENTS = 1,   // 根据用户类型调整
        PAGE_ENROLLMENT_ADMIN = 2  // 仅选课管理员
    };
    void setupUI();
    void setupNavigation();
    void setupContentPages();
    void setupStyles();
    void updateNavigationBadges();
    void checkUserPermissions();

    // 成员变量
    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;
    bool m_isCourseAdmin;       // 是否为选课管理员
    bool m_isTeacher;           // 是否为教师
    MyCoursesWidget *m_myCoursesWidget;
    QPushButton *m_myCoursesBtn;

    // UI组件 - 主布局
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧导航
    QWidget *m_navigationWidget;
    QVBoxLayout *m_navigationLayout;
    QPushButton *m_enrollmentBtn;
    QPushButton *m_noticesBtn;
    QPushButton *m_assignmentsBtn;
    QPushButton *m_enrollmentAdminBtn;  // 选课审核按钮（仅管理员可见）
    QGroupBox *m_statsGroupBox;
    QLabel *m_statsLabel;

    // 右侧内容区域
    QStackedWidget *m_contentStack;

    // 各功能页面
    EnrollmentWidget *m_enrollmentWidget;
    EnrollmentAdminWidget *m_enrollmentAdminWidget;
    CourseNoticeWidget *m_courseNoticeWidget;
    CourseAssignmentWidget *m_courseAssignmentWidget;

    // 定时器
    QTimer *m_refreshTimer;

    // 统计信息
    int m_enrolledCourseCount;      // 已选课程数量
    int m_pendingEnrollmentCount;   // 待审核申请数量（仅管理员）
    int m_unreadNoticeCount;        // 未读通知数量
    int m_assignmentCount;          // 作业数量
    // 添加辅助方法
    int getNoticesPageIndex() const { return m_isTeacher ? 0 : 1; }
    int getAssignmentsPageIndex() const { return m_isTeacher ? 1 : 2; }
    int getEnrollmentAdminPageIndex() const { return 2; }
};

#endif // COURSEPAGE_H
