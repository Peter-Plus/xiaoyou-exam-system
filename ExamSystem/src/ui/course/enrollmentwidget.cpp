#include "enrollmentwidget.h"
#include "../../models/course.h"
#include "../../models/enrollmentrequest.h"
#include <QMessageBox>
#include <QTimer>

EnrollmentWidget::EnrollmentWidget(Database *database, int studentId, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_studentId(studentId)
    , m_selectedCourseId(-1)
{
    setupUI();
    setupStyles();
    refreshData();
}

void EnrollmentWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(10);

    // 创建标签页
    m_tabWidget = new QTabWidget();
    setupAvailableCoursesTab();
    setupMyCoursesTab();

    m_mainLayout->addWidget(m_tabWidget);

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &EnrollmentWidget::onTabChanged);
}

void EnrollmentWidget::setupAvailableCoursesTab()
{
    m_availableCoursesTab = new QWidget();
    m_availableLayout = new QVBoxLayout(m_availableCoursesTab);
    m_availableLayout->setContentsMargins(10, 10, 10, 10);

    // 搜索区域
    QGroupBox *searchGroup = new QGroupBox("课程搜索");
    m_searchLayout = new QHBoxLayout(searchGroup);

    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("搜索课程名称或教师姓名...");
    m_searchButton = new QPushButton("🔍 搜索");
    m_collegeFilterCombo = new QComboBox();
    m_collegeFilterCombo->addItem("全部学院");
    m_collegeFilterCombo->addItem("计算机学院");
    m_collegeFilterCombo->addItem("数学学院");
    m_collegeFilterCombo->addItem("软件学院");

    m_searchLayout->addWidget(new QLabel("学院:"));
    m_searchLayout->addWidget(m_collegeFilterCombo);
    m_searchLayout->addWidget(m_searchLineEdit);
    m_searchLayout->addWidget(m_searchButton);

    m_availableLayout->addWidget(searchGroup);

    // 课程列表
    QGroupBox *courseGroup = new QGroupBox("可选课程");
    QVBoxLayout *courseLayout = new QVBoxLayout(courseGroup);

    m_availableCoursesList = new QListWidget();
    m_availableCoursesList->setSelectionMode(QAbstractItemView::SingleSelection);
    courseLayout->addWidget(m_availableCoursesList);

    // 操作区域
    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_availableCountLabel = new QLabel("共0门课程");
    m_applyButton = new QPushButton("📝 申请选课");
    m_applyButton->setEnabled(false);

    actionLayout->addWidget(m_availableCountLabel);
    actionLayout->addStretch();
    actionLayout->addWidget(m_applyButton);

    courseLayout->addLayout(actionLayout);
    m_availableLayout->addWidget(courseGroup);

    m_tabWidget->addTab(m_availableCoursesTab, "📚 可选课程");

    // 连接信号槽
    connect(m_searchButton, &QPushButton::clicked, this, &EnrollmentWidget::onCourseSearch);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &EnrollmentWidget::onSearchTextChanged);
    connect(m_collegeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnrollmentWidget::onCourseSearch);
    connect(m_availableCoursesList, &QListWidget::itemSelectionChanged, [this]() {
        QListWidgetItem *item = m_availableCoursesList->currentItem();
        if (item) {
            m_selectedCourseId = item->data(Qt::UserRole).toInt();
            m_applyButton->setEnabled(true);
        } else {
            m_selectedCourseId = -1;
            m_applyButton->setEnabled(false);
        }
    });
    connect(m_availableCoursesList, &QListWidget::itemDoubleClicked,
            this, &EnrollmentWidget::onCourseDoubleClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &EnrollmentWidget::onApplyForCourse);
}

void EnrollmentWidget::setupMyCoursesTab()
{
    m_myCoursesTab = new QWidget();
    m_myCoursesLayout = new QVBoxLayout(m_myCoursesTab);
    m_myCoursesLayout->setContentsMargins(10, 10, 10, 10);

    // 我的选课列表
    QGroupBox *myCoursesGroup = new QGroupBox("我的选课");
    QVBoxLayout *myCoursesGroupLayout = new QVBoxLayout(myCoursesGroup);

    m_myCoursesList = new QListWidget();
    myCoursesGroupLayout->addWidget(m_myCoursesList);

    // 操作区域
    QHBoxLayout *myActionLayout = new QHBoxLayout();
    m_myCoursesCountLabel = new QLabel("共0门课程");
    m_refreshButton = new QPushButton("🔄 刷新");

    myActionLayout->addWidget(m_myCoursesCountLabel);
    myActionLayout->addStretch();
    myActionLayout->addWidget(m_refreshButton);

    myCoursesGroupLayout->addLayout(myActionLayout);
    m_myCoursesLayout->addWidget(myCoursesGroup);

    m_tabWidget->addTab(m_myCoursesTab, "📋 我的选课");

    // 连接信号槽
    connect(m_refreshButton, &QPushButton::clicked, this, &EnrollmentWidget::refreshData);
}

void EnrollmentWidget::setupStyles()
{
    this->setStyleSheet(
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
        "QListWidget {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    alternate-background-color: #f8f9fa;"
        "}"
        "QListWidget::item {"
        "    padding: 8px;"
        "    border-bottom: 1px solid #eee;"
        "    margin: 2px;"
        "    border-radius: 4px;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #e6f7ff;"
        "    border: 2px solid #1890ff;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #f0f0f0;"
        "}"
        "QPushButton {"
        "    padding: 8px 16px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: #f8f9fa;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e8f4ff;"
        "    border-color: #1890ff;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1890ff;"
        "    color: white;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #f5f5f5;"
        "    color: #999;"
        "    border-color: #ddd;"
        "}"
        "QLineEdit {"
        "    padding: 8px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #1890ff;"
        "}"
        "QComboBox {"
        "    padding: 8px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    font-size: 14px;"
        "}"
        );
}

void EnrollmentWidget::refreshData()
{
    updateAvailableCourses();
    updateMyCourses();
}

void EnrollmentWidget::updateAvailableCourses()
{
    if (!m_database) return;

    // 获取所有课程
    QList<Course> allCourses = m_database->getAllCourses();
    m_availableCourses.clear();

    for (const Course &course : allCourses) {
        // 检查学生是否已选课
        if (!m_database->isStudentEnrolled(m_studentId, course.getCourseId())) {
            QVariantMap courseMap;
            courseMap["course_id"] = course.getCourseId();
            courseMap["course_name"] = course.getCourseName();
            courseMap["teacher_name"] = course.getTeacherName();
            courseMap["college"] = course.getCollege();
            m_availableCourses.append(courseMap);
        }
    }

    // 更新列表显示
    m_availableCoursesList->clear();
    for (const QVariantMap &course : m_availableCourses) {
        createCourseListItem(course, m_availableCoursesList, false);
    }

    m_availableCountLabel->setText(QString("共%1门课程").arg(m_availableCourses.size()));
}

void EnrollmentWidget::updateMyCourses()
{
    if (!m_database) return;

    m_myCourses = m_database->getStudentCourses(m_studentId);

    // 更新列表显示
    m_myCoursesList->clear();
    for (const QVariantMap &course : m_myCourses) {
        createCourseListItem(course, m_myCoursesList, true);
    }

    m_myCoursesCountLabel->setText(QString("共%1门课程").arg(m_myCourses.size()));
}

void EnrollmentWidget::createCourseListItem(const QVariantMap &course, QListWidget *listWidget, bool isEnrolled)
{
    QListWidgetItem *item = new QListWidgetItem();

    QString courseName = course["course_name"].toString();
    QString teacherName = course["teacher_name"].toString();
    QString college = course["college"].toString();
    int courseId = course["course_id"].toInt();

    QString displayText;
    QString statusText;

    if (isEnrolled) {
        QString enrollmentStatus = course["enrollment_status"].toString();
        QDateTime enrollmentTime = course["enrollment_time"].toDateTime();

        if (enrollmentStatus == "已通过") {
            statusText = "✅ 已选课";
        } else if (enrollmentStatus == "申请中") {
            statusText = "⏳ 申请中";
        }

        displayText = QString("%1\n教师: %2 | 学院: %3\n状态: %4 | 选课时间: %5")
                          .arg(courseName)
                          .arg(teacherName)
                          .arg(college)
                          .arg(statusText)
                          .arg(enrollmentTime.toString("yyyy-MM-dd hh:mm"));
    } else {
        // 检查是否有待处理申请
        // 这里简化处理，实际应该检查数据库
        displayText = QString("%1\n教师: %2 | 学院: %3\n状态: 可申请")
                          .arg(courseName)
                          .arg(teacherName)
                          .arg(college);
    }

    item->setText(displayText);
    item->setData(Qt::UserRole, courseId);

    // 设置不同状态的颜色
    if (isEnrolled) {
        QString enrollmentStatus = course["enrollment_status"].toString();
        if (enrollmentStatus == "已通过") {
            item->setBackground(QBrush(QColor(240, 255, 240))); // 浅绿色
        } else if (enrollmentStatus == "申请中") {
            item->setBackground(QBrush(QColor(255, 248, 220))); // 浅黄色
        }
    }

    listWidget->addItem(item);
}

void EnrollmentWidget::onApplyForCourse()
{
    if (m_selectedCourseId <= 0 || !m_database) {
        showMessage("请先选择一门课程", true);
        return;
    }

    // 确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认申请",
        "确定要申请选修这门课程吗？\n提交后需要等待选课管理员审核。",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 提交选课申请
    if (m_database->applyForCourse(m_studentId, m_selectedCourseId)) {
        showMessage("选课申请提交成功！请等待审核。");
        emit enrollmentSubmitted(m_selectedCourseId);

        // 刷新数据
        QTimer::singleShot(500, this, &EnrollmentWidget::refreshData);

        // 切换到我的选课标签页
        m_tabWidget->setCurrentIndex(1);
    } else {
        showMessage("选课申请失败，请重试", true);
    }
}

void EnrollmentWidget::onCourseSearch()
{
    QString searchText = m_searchLineEdit->text().trimmed();
    QString selectedCollege = m_collegeFilterCombo->currentText();

    // 过滤课程列表
    m_availableCoursesList->clear();

    for (const QVariantMap &course : m_availableCourses) {
        bool matches = true;

        // 学院过滤
        if (selectedCollege != "全部学院") {
            if (course["college"].toString() != selectedCollege) {
                matches = false;
            }
        }

        // 关键词搜索
        if (!searchText.isEmpty() && matches) {
            QString courseName = course["course_name"].toString();
            QString teacherName = course["teacher_name"].toString();

            if (!courseName.contains(searchText, Qt::CaseInsensitive) &&
                !teacherName.contains(searchText, Qt::CaseInsensitive)) {
                matches = false;
            }
        }

        if (matches) {
            createCourseListItem(course, m_availableCoursesList, false);
        }
    }

    int filteredCount = m_availableCoursesList->count();
    m_availableCountLabel->setText(QString("共%1门课程").arg(filteredCount));
}

void EnrollmentWidget::onTabChanged(int index)
{
    // 切换标签页时刷新数据
    if (index == 0) {
        updateAvailableCourses();
    } else if (index == 1) {
        updateMyCourses();
    }
}

void EnrollmentWidget::onCourseDoubleClicked(QListWidgetItem *item)
{
    if (item && m_tabWidget->currentIndex() == 0) {
        // 在可选课程标签页双击时自动申请
        m_selectedCourseId = item->data(Qt::UserRole).toInt();
        onApplyForCourse();
    }
}

void EnrollmentWidget::onSearchTextChanged()
{
    // 延迟搜索，避免频繁查询
    static QTimer *searchTimer = nullptr;
    if (!searchTimer) {
        searchTimer = new QTimer(this);
        searchTimer->setSingleShot(true);
        searchTimer->setInterval(500);
        connect(searchTimer, &QTimer::timeout, this, &EnrollmentWidget::onCourseSearch);
    }

    searchTimer->start();
}

void EnrollmentWidget::showMessage(const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "提示", message);
    } else {
        QMessageBox::information(this, "提示", message);
    }
}

void EnrollmentWidget::showAvailableCourses()
{
    // 切换到可选课程标签页
    m_tabWidget->setCurrentIndex(0);
    updateAvailableCourses();
}

void EnrollmentWidget::showMyCourses()
{
    // 切换到我的选课标签页
    m_tabWidget->setCurrentIndex(1);
    updateMyCourses();
}
