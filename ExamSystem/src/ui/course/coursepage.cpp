#include "coursepage.h"
#include "enrollmentwidget.h"
#include "enrollmentadminwidget.h"
#include "coursenoticewidget.h"
#include "courseassignmentwidget.h"
#include "../../models/course.h"  // 添加Course类的完整定义
#include <QMessageBox>
#include "mycourseswidget.h"

CoursePage::CoursePage(Database *database, int userId, const QString &userType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
    , m_isCourseAdmin(false)
    , m_isTeacher(userType == "老师")
    , m_enrollmentWidget(nullptr)
    , m_enrollmentAdminWidget(nullptr)
    , m_courseNoticeWidget(nullptr)
    , m_courseAssignmentWidget(nullptr)
    , m_enrolledCourseCount(0)
    , m_pendingEnrollmentCount(0)
    , m_unreadNoticeCount(0)
    , m_assignmentCount(0)
    , m_enrollmentBtn(nullptr)
    , m_noticesBtn(nullptr)
    , m_assignmentsBtn(nullptr)
    , m_enrollmentAdminBtn(nullptr)
    , m_myCoursesWidget(nullptr)
    , m_myCoursesBtn(nullptr)
{
    checkUserPermissions();
    setupUI();
    setupStyles();

    // 设置定时器
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(30000); // 30秒刷新一次
    connect(m_refreshTimer, &QTimer::timeout, this, &CoursePage::autoRefresh);
    m_refreshTimer->start();

    // 初始加载数据
    refreshAll();
    // 默认显示我的课程页面
    showMyCourses();
}

CoursePage::~CoursePage()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
        m_refreshTimer->deleteLater();  // 使用 deleteLater 而不是直接 delete
        m_refreshTimer = nullptr;
    }
}

void CoursePage::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);

    setupNavigation();
    setupContentPages();

    // 设置分割器比例
    m_splitter->setSizes({220, 800});
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);
}

void CoursePage::setupNavigation()
{
    m_navigationWidget = new QWidget();
    m_navigationWidget->setFixedWidth(220);
    m_navigationLayout = new QVBoxLayout(m_navigationWidget);
    m_navigationLayout->setContentsMargins(10, 10, 10, 10);
    m_navigationLayout->setSpacing(5);

    // 导航按钮 - 添加"我的课程"
    m_myCoursesBtn = new QPushButton("📚 我的课程");  // 新增
    m_navigationLayout->addWidget(m_myCoursesBtn);
    connect(m_myCoursesBtn, &QPushButton::clicked, this, &CoursePage::showMyCourses);

    if (m_isTeacher) {
        // 教师端导航
        m_noticesBtn = new QPushButton("📢 课程通知");
        m_assignmentsBtn = new QPushButton("📝 课程作业");

        m_navigationLayout->addWidget(m_noticesBtn);
        m_navigationLayout->addWidget(m_assignmentsBtn);

        connect(m_noticesBtn, &QPushButton::clicked, this, &CoursePage::showNotices);
        connect(m_assignmentsBtn, &QPushButton::clicked, this, &CoursePage::showAssignments);

        // 选课管理员专用功能
        if (m_isCourseAdmin) {
            m_enrollmentAdminBtn = new QPushButton("🔍 选课审核");
            m_navigationLayout->addWidget(m_enrollmentAdminBtn);
            connect(m_enrollmentAdminBtn, &QPushButton::clicked, this, &CoursePage::showEnrollmentAdmin);
        }
    } else {
        // 学生端导航
        m_enrollmentBtn = new QPushButton("📚 我的选课");
        m_noticesBtn = new QPushButton("📢 课程通知");
        m_assignmentsBtn = new QPushButton("📝 课程作业");

        m_navigationLayout->addWidget(m_enrollmentBtn);
        m_navigationLayout->addWidget(m_noticesBtn);
        m_navigationLayout->addWidget(m_assignmentsBtn);

        connect(m_enrollmentBtn, &QPushButton::clicked, this, &CoursePage::showEnrollment);
        connect(m_noticesBtn, &QPushButton::clicked, this, &CoursePage::showNotices);
        connect(m_assignmentsBtn, &QPushButton::clicked, this, &CoursePage::showAssignments);
    }

    // 统计信息
    m_statsGroupBox = new QGroupBox("统计信息");
    m_statsLabel = new QLabel();
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroupBox);
    statsLayout->addWidget(m_statsLabel);

    m_navigationLayout->addStretch();
    m_navigationLayout->addWidget(m_statsGroupBox);

    m_splitter->addWidget(m_navigationWidget);
}

void CoursePage::setupContentPages()
{
    m_contentStack = new QStackedWidget();

    // 我的课程页面 - 索引0（第一个页面）
    m_myCoursesWidget = new MyCoursesWidget(m_database, m_currentUserId, m_currentUserType, this);
    m_contentStack->addWidget(m_myCoursesWidget);

    connect(m_myCoursesWidget, &MyCoursesWidget::courseUpdated,
            this, &CoursePage::courseUpdated);

    if (!m_isTeacher) {
        // 学生端：选课申请页面 - 索引1
        m_enrollmentWidget = new EnrollmentWidget(m_database, m_currentUserId, this);
        m_contentStack->addWidget(m_enrollmentWidget);

        connect(m_enrollmentWidget, &EnrollmentWidget::enrollmentSubmitted,
                this, &CoursePage::enrollmentSubmitted);
    }

    // 课程通知页面
    m_courseNoticeWidget = new CourseNoticeWidget(m_database, m_currentUserId, m_currentUserType, this);
    m_contentStack->addWidget(m_courseNoticeWidget);

    connect(m_courseNoticeWidget, &CourseNoticeWidget::noticePublished,
            this, &CoursePage::noticePublished);

    // 课程作业页面
    m_courseAssignmentWidget = new CourseAssignmentWidget(m_database, m_currentUserId, m_currentUserType, this);
    m_contentStack->addWidget(m_courseAssignmentWidget);

    connect(m_courseAssignmentWidget, &CourseAssignmentWidget::assignmentPublished,
            this, &CoursePage::assignmentPublished);

    // 选课审核页面（仅选课管理员）
    if (m_isCourseAdmin) {
        m_enrollmentAdminWidget = new EnrollmentAdminWidget(m_database, m_currentUserId, this);
        m_contentStack->addWidget(m_enrollmentAdminWidget);
    }

    m_splitter->addWidget(m_contentStack);
}

void CoursePage::setupStyles()
{
    // 设置导航区域样式
    m_navigationWidget->setStyleSheet(
        "QWidget {"
        "    background-color: #f5f5f5;"
        "    border-right: 1px solid #ddd;"
        "}"
        "QPushButton {"
        "    text-align: left;"
        "    padding: 12px 16px;"
        "    border: none;"
        "    background-color: transparent;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "    border-radius: 6px;"
        "    margin: 2px 0px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e8f4ff;"
        "    color: #1890ff;"
        "}"
        "QPushButton:checked {"
        "    background-color: #1890ff;"
        "    color: white;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #ddd;"
        "    border-radius: 6px;"
        "    margin-top: 6px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "}"
        );

    // 设置内容区域样式
    m_contentStack->setStyleSheet(
        "QStackedWidget {"
        "    background-color: white;"
        "    border: none;"
        "}"
        );

    // 设置分割器样式
    m_splitter->setStyleSheet(
        "QSplitter::handle {"
        "    background-color: #ddd;"
        "    width: 1px;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #1890ff;"
        "}"
        );
}

void CoursePage::checkUserPermissions()
{
    if (m_isTeacher && m_database) {
        m_isCourseAdmin = m_database->isTeacherCourseAdmin(m_currentUserId);
        qDebug() << "用户权限检查 - 教师ID:" << m_currentUserId << "是否为选课管理员:" << m_isCourseAdmin;
    }
}

void CoursePage::refreshAll()
{
    updateStatistics();
    updateNavigationBadges();

    // 刷新当前显示的页面
    if (m_myCoursesWidget && m_myCoursesWidget->isVisible()) {
        m_myCoursesWidget->refreshData();
    }
    if (m_enrollmentWidget && m_enrollmentWidget->isVisible()) {
        m_enrollmentWidget->refreshData();
    }
    if (m_enrollmentAdminWidget && m_enrollmentAdminWidget->isVisible()) {
        m_enrollmentAdminWidget->refreshData();
    }
    if (m_courseNoticeWidget && m_courseNoticeWidget->isVisible()) {
        m_courseNoticeWidget->refreshData();
    }
    if (m_courseAssignmentWidget && m_courseAssignmentWidget->isVisible()) {
        m_courseAssignmentWidget->refreshData();
    }
}

void CoursePage::showEnrollment()
{
    if (m_enrollmentWidget) {
        m_contentStack->setCurrentWidget(m_enrollmentWidget);
        m_enrollmentWidget->refreshData();

        // 更新按钮状态
        m_myCoursesBtn->setChecked(false);
        m_enrollmentBtn->setChecked(true);
        m_noticesBtn->setChecked(false);
        m_assignmentsBtn->setChecked(false);
    }
}

void CoursePage::showNotices()
{
    if (m_courseNoticeWidget) {
        m_contentStack->setCurrentWidget(m_courseNoticeWidget);
        m_courseNoticeWidget->refreshData();

        // 更新按钮状态
        m_myCoursesBtn->setChecked(false);
        if (m_enrollmentBtn) m_enrollmentBtn->setChecked(false);
        m_noticesBtn->setChecked(true);
        m_assignmentsBtn->setChecked(false);
        if (m_enrollmentAdminBtn) m_enrollmentAdminBtn->setChecked(false);
    }
}

void CoursePage::showAssignments()
{
    if (m_courseAssignmentWidget) {
        m_contentStack->setCurrentWidget(m_courseAssignmentWidget);
        m_courseAssignmentWidget->refreshData();

        // 更新按钮状态
        m_myCoursesBtn->setChecked(false);
        if (m_enrollmentBtn) m_enrollmentBtn->setChecked(false);
        m_noticesBtn->setChecked(false);
        m_assignmentsBtn->setChecked(true);
        if (m_enrollmentAdminBtn) m_enrollmentAdminBtn->setChecked(false);
    }
}

void CoursePage::showEnrollmentAdmin()
{
    if (m_enrollmentAdminWidget) {
        m_contentStack->setCurrentWidget(m_enrollmentAdminWidget);
        m_enrollmentAdminWidget->refreshData();

        // 更新按钮状态
        m_myCoursesBtn->setChecked(false);
        m_noticesBtn->setChecked(false);
        m_assignmentsBtn->setChecked(false);
        m_enrollmentAdminBtn->setChecked(true);
    }
}

void CoursePage::onNavigationClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    // 重置所有按钮状态
    m_myCoursesBtn->setChecked(false);
    if (m_enrollmentBtn) m_enrollmentBtn->setChecked(false);
    m_noticesBtn->setChecked(false);
    m_assignmentsBtn->setChecked(false);
    if (m_enrollmentAdminBtn) m_enrollmentAdminBtn->setChecked(false);

    // 设置当前按钮为选中状态
    button->setChecked(true);
}

void CoursePage::autoRefresh()
{
    updateStatistics();
    updateNavigationBadges();
}

void CoursePage::updateStatistics()
{
    if (!m_database) return;

    QString statsText;

    if (m_isTeacher) {
        // 教师端统计
        if (m_isCourseAdmin) {
            // 选课管理员统计
            QList<QVariantMap> pendingEnrollments = m_database->getPendingEnrollments();
            m_pendingEnrollmentCount = pendingEnrollments.size();

            statsText = QString("待审核申请: %1条").arg(m_pendingEnrollmentCount);
        } else {
            // 普通教师统计
            QList<Course> courses = m_database->getTeacherCourses(m_currentUserId);
            statsText = QString("授课课程: %1门").arg(courses.size());
        }
    } else {
        // 学生端统计
        QList<QVariantMap> studentCourses = m_database->getStudentCourses(m_currentUserId);
        m_enrolledCourseCount = 0;
        int pendingCount = 0;

        for (const auto &course : studentCourses) {
            QString status = course["enrollment_status"].toString();
            if (status == "已通过") {
                m_enrolledCourseCount++;
            } else if (status == "申请中") {
                pendingCount++;
            }
        }

        statsText = QString("已选课程: %1门\n申请中: %2门")
                        .arg(m_enrolledCourseCount)
                        .arg(pendingCount);
    }

    m_statsLabel->setText(statsText);
}

void CoursePage::updateNavigationBadges()
{
    // 这里可以添加未读消息数量等徽章显示
    // 暂时简化实现
}

void CoursePage::showMyCourses()
{
    if (m_myCoursesWidget) {
        m_contentStack->setCurrentWidget(m_myCoursesWidget);
        m_myCoursesWidget->refreshData();

        // 更新按钮状态
        m_myCoursesBtn->setChecked(true);
        if (m_enrollmentBtn) m_enrollmentBtn->setChecked(false);
        m_noticesBtn->setChecked(false);
        m_assignmentsBtn->setChecked(false);
        if (m_enrollmentAdminBtn) m_enrollmentAdminBtn->setChecked(false);
    }
}
