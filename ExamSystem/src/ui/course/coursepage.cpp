#include "coursepage.h"
#include <QDebug>
#include <QMessageBox>
#include <QListWidgetItem>

CoursePage::CoursePage(Database *database, int userId, UserType userType, QWidget *parent)
    : QWidget(parent), m_database(database), m_userId(userId), m_userType(userType),
    m_currentCourseId(-1), m_currentPage(PAGE_MY_COURSES),
    m_totalCourses(0), m_pendingRequests(0), m_unreadNotices(0), m_pendingAssignments(0)
{
    setupUI();

    // 设置定时器
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &CoursePage::autoRefresh);
    m_refreshTimer->start(30000); // 30秒自动刷新

    // 初始加载数据
    refreshData();
}

CoursePage::~CoursePage()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
    }
}

void CoursePage::setupUI()
{
    setObjectName("CoursePage");

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setObjectName("CoursePageSplitter");

    createNavigationPanel();
    createContentArea();

    // 设置分割器比例
    m_splitter->addWidget(m_navigationWidget);
    m_splitter->addWidget(m_contentWidget);
    m_splitter->addWidget(m_detailWidget);
    m_splitter->setStretchFactor(0, 0); // 导航栏固定宽度
    m_splitter->setStretchFactor(1, 1); // 内容区域可伸缩
    m_splitter->setStretchFactor(2, 1); // 详情区域可伸缩
    m_splitter->setSizes({220, 400, 400});

    m_mainLayout->addWidget(m_splitter);

    // 根据用户类型设置UI
    if (m_userType == STUDENT) {
        setupStudentUI();
    } else {
        setupTeacherUI();
    }

    // 应用样式
    setStyleSheet(R"(
        QWidget#CoursePage {
            background-color: #f5f5f5;
        }

        QSplitter#CoursePageSplitter::handle {
            background-color: #ddd;
            width: 1px;
        }

        QSplitter#CoursePageSplitter::handle:hover {
            background-color: #3498db;
        }

        QWidget#NavigationWidget {
            background-color: #34495e;
            border-right: 1px solid #2c3e50;
        }

        QListWidget#NavigationList {
            background-color: transparent;
            border: none;
            outline: none;
            font-size: 14px;
            color: #ecf0f1;
        }

        QListWidget#NavigationList::item {
            padding: 12px 15px;
            border-bottom: 1px solid #2c3e50;
        }

        QListWidget#NavigationList::item:hover {
            background-color: #3498db;
        }

        QListWidget#NavigationList::item:selected {
            background-color: #2980b9;
            font-weight: bold;
        }

        QGroupBox#StatisticsGroup {
            color: #ecf0f1;
            font-size: 12px;
            border: 1px solid #2c3e50;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }

        QGroupBox#StatisticsGroup::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }

        QPushButton#RefreshButton {
            background-color: #3498db;
            color: white;
            border: none;
            padding: 8px 15px;
            border-radius: 4px;
            font-size: 12px;
        }

        QPushButton#RefreshButton:hover {
            background-color: #2980b9;
        }

        QPushButton#RefreshButton:pressed {
            background-color: #21618c;
        }

        QWidget#ContentWidget, QWidget#DetailWidget {
            background-color: white;
            border: 1px solid #ddd;
        }

        QLabel#PlaceholderLabel {
            color: #7f8c8d;
            font-size: 16px;
            text-align: center;
        }
    )");
}

void CoursePage::createNavigationPanel()
{
    m_navigationWidget = new QWidget();
    m_navigationWidget->setObjectName("NavigationWidget");
    m_navigationWidget->setFixedWidth(220);

    m_navigationLayout = new QVBoxLayout(m_navigationWidget);
    m_navigationLayout->setContentsMargins(0, 0, 0, 0);
    m_navigationLayout->setSpacing(0);

    // 导航列表
    m_navigationList = new QListWidget();
    m_navigationList->setObjectName("NavigationList");
    connect(m_navigationList, &QListWidget::itemClicked, this, &CoursePage::onNavigationItemClicked);

    // 统计信息组
    m_statisticsGroup = new QGroupBox("📊 统计信息");
    m_statisticsGroup->setObjectName("StatisticsGroup");

    m_statisticsLabel = new QLabel();
    m_statisticsLabel->setWordWrap(true);
    m_statisticsLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QVBoxLayout *statsLayout = new QVBoxLayout(m_statisticsGroup);
    statsLayout->addWidget(m_statisticsLabel);

    // 刷新按钮
    m_refreshButton = new QPushButton("🔄 刷新数据");
    m_refreshButton->setObjectName("RefreshButton");
    connect(m_refreshButton, &QPushButton::clicked, this, &CoursePage::refreshData);

    m_navigationLayout->addWidget(m_navigationList);
    m_navigationLayout->addWidget(m_statisticsGroup);
    m_navigationLayout->addWidget(m_refreshButton);
    m_navigationLayout->addStretch();
}

void CoursePage::createContentArea()
{
    // 中间内容区域
    m_contentWidget = new QWidget();
    m_contentWidget->setObjectName("ContentWidget");

    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(10, 10, 10, 10);

    m_contentStack = new QStackedWidget();
    m_contentLayout->addWidget(m_contentStack);

    // 右侧详情区域
    m_detailWidget = new QWidget();
    m_detailWidget->setObjectName("DetailWidget");

    m_detailLayout = new QVBoxLayout(m_detailWidget);
    m_detailLayout->setContentsMargins(10, 10, 10, 10);

    m_detailStack = new QStackedWidget();
    m_detailLayout->addWidget(m_detailStack);

    // 默认显示占位页面
    showPlaceholderPage("请选择左侧功能");
}

void CoursePage::setupStudentUI()
{
    // 学生端导航项
    QStringList navigationItems;
    navigationItems << "📚 我的课程" << "➕ 选课申请" << "📋 课程详情";

    for (int i = 0; i < navigationItems.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(navigationItems[i]);
        item->setData(Qt::UserRole, i);
        m_navigationList->addItem(item);
    }

    // 默认选中第一项
    m_navigationList->setCurrentRow(0);
}

void CoursePage::setupTeacherUI()
{
    // 教师端导航项
    QStringList navigationItems;

    if (m_userType == TEACHER) {
        // 检查是否为选课管理员
        if (m_database) {
            QSqlQuery query(m_database->getDatabase());
            query.prepare("SELECT is_course_admin FROM teachers WHERE teacher_id = ?");
            query.addBindValue(m_userId);

            bool isCourseAdmin = false;
            if (query.exec() && query.next()) {
                isCourseAdmin = query.value(0).toBool();
            }

            if (isCourseAdmin) {
                navigationItems << "📚 我的课程" << "✅ 选课审核" << "📋 课程管理";
            } else {
                navigationItems << "📚 我的课程" << "📋 课程管理";
            }
        } else {
            navigationItems << "📚 我的课程" << "📋 课程管理";
        }
    }

    for (int i = 0; i < navigationItems.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(navigationItems[i]);
        item->setData(Qt::UserRole, i);
        m_navigationList->addItem(item);
    }

    // 默认选中第一项
    m_navigationList->setCurrentRow(0);
}

void CoursePage::refreshData()
{
    if (!m_database) {
        qDebug() << "数据库未连接";
        return;
    }

    // 加载课程列表
    loadCourseList();

    // 更新统计信息
    updateStatistics();

    qDebug() << "课程页面数据刷新完成";
}

void CoursePage::loadCourseList()
{
    m_courseList.clear();

    if (!m_database) {
        return;
    }

    if (m_userType == STUDENT) {
        // 加载学生的课程
        m_courseList = m_database->getCoursesByStudent(m_userId, true); // 包含申请中的课程
    } else {
        // 加载教师的课程
        m_courseList = m_database->getCoursesByTeacher(m_userId);
    }

    qDebug() << "加载了" << m_courseList.size() << "门课程";
}

void CoursePage::updateStatistics()
{
    if (!m_database) {
        return;
    }

    QString statsText;

    if (m_userType == STUDENT) {
        m_totalCourses = 0;
        m_pendingRequests = 0;

        for (const auto &course : m_courseList) {
            if (course["enrollment_status"].toString() == "已通过") {
                m_totalCourses++;
            } else if (course["enrollment_status"].toString() == "申请中") {
                m_pendingRequests++;
            }
        }

        statsText = QString("已选课程: %1门\n申请中课程: %2门\n待完成作业: %3个")
                        .arg(m_totalCourses)
                        .arg(m_pendingRequests)
                        .arg(m_pendingAssignments);
    } else {
        m_totalCourses = m_courseList.size();

        // 获取选课申请统计
        QVariantMap enrollmentStats = m_database->getEnrollmentStats();
        m_pendingRequests = enrollmentStats["pending_count"].toInt();

        statsText = QString("教授课程: %1门\n待处理申请: %2条\n发布通知: %3条")
                        .arg(m_totalCourses)
                        .arg(m_pendingRequests)
                        .arg(m_unreadNotices);
    }

    m_statisticsLabel->setText(statsText);
}

void CoursePage::showPlaceholderPage(const QString &message)
{
    // 清空当前内容
    while (m_contentStack->count() > 0) {
        QWidget *widget = m_contentStack->widget(0);
        m_contentStack->removeWidget(widget);
        widget->deleteLater();
    }

    while (m_detailStack->count() > 0) {
        QWidget *widget = m_detailStack->widget(0);
        m_detailStack->removeWidget(widget);
        widget->deleteLater();
    }

    // 创建占位页面
    QWidget *placeholderWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(placeholderWidget);

    QLabel *placeholderLabel = new QLabel(message);
    placeholderLabel->setObjectName("PlaceholderLabel");
    placeholderLabel->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(placeholderLabel);
    layout->addStretch();

    m_contentStack->addWidget(placeholderWidget);

    // 右侧也显示占位内容
    QWidget *detailPlaceholder = new QWidget();
    QVBoxLayout *detailLayout = new QVBoxLayout(detailPlaceholder);

    QLabel *detailLabel = new QLabel("选择课程查看详情");
    detailLabel->setObjectName("PlaceholderLabel");
    detailLabel->setAlignment(Qt::AlignCenter);

    detailLayout->addStretch();
    detailLayout->addWidget(detailLabel);
    detailLayout->addStretch();

    m_detailStack->addWidget(detailPlaceholder);
}

void CoursePage::autoRefresh()
{
    // 静默刷新，不影响用户当前操作
    if (m_database) {
        updateStatistics();
    }
}

void CoursePage::onCourseListItemClicked()
{
    // 课程列表项点击处理（将在后续步骤中实现）
    qDebug() << "课程列表项被点击";
}

void CoursePage::onNavigationItemClicked()
{
    QListWidgetItem *item = m_navigationList->currentItem();
    if (!item) {
        return;
    }

    int pageIndex = item->data(Qt::UserRole).toInt();
    onPageChanged(pageIndex);
}

void CoursePage::onCourseSelected(int courseId)
{
    m_currentCourseId = courseId;
    qDebug() << "选中课程ID:" << courseId;

    // 后续步骤中实现课程详情显示
}

void CoursePage::onPageChanged(int pageIndex)
{
    m_currentPage = static_cast<PageType>(pageIndex);

    QString pageName;
    switch (m_currentPage) {
    case PAGE_MY_COURSES:
        pageName = "我的课程";
        break;
    case PAGE_ENROLLMENT:
        pageName = (m_userType == STUDENT) ? "选课申请" : "选课审核";
        break;
    case PAGE_COURSE_DETAIL:
        pageName = (m_userType == STUDENT) ? "课程详情" : "课程管理";
        break;
    }

    // 暂时显示占位页面，后续步骤中实现具体功能页面
    showPlaceholderPage(QString("即将推出：%1").arg(pageName));

    qDebug() << "切换到页面:" << pageName;
}
