#ifndef COURSEPAGE_H
#define COURSEPAGE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QTimer>
#include <QSqlQuery>
#include "../../core/database.h"
#include "enrollmentwidget.h"
#include "noticewidget.h"
#include "assignmentwidget.h"

class CoursePage : public QWidget
{
    Q_OBJECT

public:
    enum UserType {
        STUDENT = 0,
        TEACHER = 1
    };

    enum PageType {
        PAGE_MY_COURSES = 0,     // 我的课程
        PAGE_ENROLLMENT = 1,     // 选课申请/选课审核
        PAGE_COURSE_DETAIL = 2   // 课程详情（通知、作业）
    };

    enum SubPageType {
        SUBPAGE_NOTICE = 0,
        SUBPAGE_ASSIGNMENT = 1
    };

    CoursePage(Database *database, int userId, UserType userType, QWidget *parent = nullptr);
    ~CoursePage();

    // 页面切换方法
    void switchToSubPage(SubPageType subPage);
    void showNotices();
    void showAssignments();
    void showEnrollment();

public slots:
    void refreshData();
    void onCourseSelected(int courseId);
    void onPageChanged(int pageIndex);

signals:
    void courseEnrolled(int courseId);
    void enrollmentProcessed(int studentId, int courseId, bool approved);
    void noticePublished(int courseId, const QString &title);
    void assignmentPublished(int courseId, const QString &title);

private slots:
    void autoRefresh();
    void onCourseListItemClicked();
    void onNavigationItemClicked();

private:
    void setupUI();
    void setupStudentUI();
    void setupTeacherUI();
    void createNavigationPanel();
    void createContentArea();
    void createEnrollmentPage();
    void createNoticePage();
    void createAssignmentPage();
    void loadCourseList();
    void updateStatistics();
    void showPlaceholderPage(const QString &message);

    // UI组件
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧导航栏
    QWidget *m_navigationWidget;
    QVBoxLayout *m_navigationLayout;
    QListWidget *m_navigationList;
    QGroupBox *m_statisticsGroup;
    QLabel *m_statisticsLabel;
    QPushButton *m_refreshButton;

    // 中间内容区域
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    QStackedWidget *m_contentStack;

    // 右侧详情区域
    QWidget *m_detailWidget;
    QVBoxLayout *m_detailLayout;
    QStackedWidget *m_detailStack;

    // 数据
    Database *m_database;
    int m_userId;
    UserType m_userType;
    int m_currentCourseId;
    PageType m_currentPage;
    SubPageType m_currentSubPage;

    // 定时器
    QTimer *m_refreshTimer;

    // 课程列表数据
    QList<QVariantMap> m_courseList;
    QList<QVariantMap> m_enrollmentRequests;

    // 统计数据
    int m_totalCourses;
    int m_pendingRequests;
    int m_unreadNotices;
    int m_pendingAssignments;

    // 功能组件
    EnrollmentWidget *m_enrollmentWidget;
    NoticeWidget *m_noticeWidget;
    AssignmentWidget *m_assignmentWidget;
};

#endif // COURSEPAGE_H
