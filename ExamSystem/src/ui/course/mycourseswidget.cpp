#include "mycourseswidget.h"
#include "../../models/course.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QTimer>

MyCoursesWidget::MyCoursesWidget(Database *database, int userId, const QString &userType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_userId(userId)
    , m_userType(userType)
    , m_isTeacher(userType == "老师")
    , m_selectedCourseId(-1)
{
    setupUI();
    setupStyles();
    refreshData();
}

void MyCoursesWidget::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);

    // 左侧课程列表区域
    m_leftWidget = new QWidget();
    m_leftWidget->setFixedWidth(350);
    m_leftLayout = new QVBoxLayout(m_leftWidget);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);

    // 搜索区域
    m_filterGroup = new QGroupBox("搜索课程");
    QVBoxLayout *filterLayout = new QVBoxLayout(m_filterGroup);

    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("搜索课程名称...");
    m_refreshButton = new QPushButton("🔄 刷新");

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(m_searchLineEdit);
    searchLayout->addWidget(m_refreshButton);

    filterLayout->addLayout(searchLayout);
    m_leftLayout->addWidget(m_filterGroup);

    // 课程列表
    m_listGroup = new QGroupBox(m_isTeacher ? "我教授的课程" : "我选修的课程");
    QVBoxLayout *listLayout = new QVBoxLayout(m_listGroup);

    m_courseList = new QListWidget();
    m_courseList->setAlternatingRowColors(true);
    listLayout->addWidget(m_courseList);

    m_courseCountLabel = new QLabel("共0门课程");
    listLayout->addWidget(m_courseCountLabel);

    m_leftLayout->addWidget(m_listGroup);

    // 统计信息
    m_statsGroup = new QGroupBox("统计信息");
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);

    m_totalCoursesLabel = new QLabel("课程数量: 0");
    statsLayout->addWidget(m_totalCoursesLabel);

    if (m_isTeacher) {
        m_totalStudentsLabel = new QLabel("学生总数: 0");
        statsLayout->addWidget(m_totalStudentsLabel);
        setupTeacherUI();
    } else {
        m_totalCreditsLabel = new QLabel("总学分: 0");
        statsLayout->addWidget(m_totalCreditsLabel);
        setupStudentUI();
    }

    m_leftLayout->addWidget(m_statsGroup);

    m_splitter->addWidget(m_leftWidget);

    // 右侧课程详情区域
    m_courseDetailWidget = new CourseDetailWidget(this);
    m_courseDetailWidget->setDatabase(m_database);
    m_splitter->addWidget(m_courseDetailWidget);

    // 设置分割器比例
    m_splitter->setSizes({350, 650});

    // 连接信号槽
    connect(m_courseList, &QListWidget::itemClicked, this, &MyCoursesWidget::onCourseSelected);
    connect(m_refreshButton, &QPushButton::clicked, this, &MyCoursesWidget::onRefreshClicked);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MyCoursesWidget::onSearchTextChanged);
}

void MyCoursesWidget::setupTeacherUI()
{
    // 教师端操作按钮
    m_actionGroup = new QGroupBox("操作");
    QVBoxLayout *actionLayout = new QVBoxLayout(m_actionGroup);

    m_editButton = new QPushButton("✏️ 编辑课程");
    m_editButton->setEnabled(false);
    m_editButton->setObjectName("editButton");

    actionLayout->addWidget(m_editButton);
    m_leftLayout->addWidget(m_actionGroup);

    connect(m_editButton, &QPushButton::clicked, this, &MyCoursesWidget::onEditCourse);
}

void MyCoursesWidget::setupStudentUI()
{
    // 学生端暂时不需要特殊操作按钮
    // 可以在这里添加学生端特有的功能
}

void MyCoursesWidget::setupStyles()
{
    this->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #ddd;"
        "    border-radius: 8px;"
        "    margin-top: 8px;"
        "    padding-top: 12px;"
        "    background-color: #fafafa;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 8px 0 8px;"
        "    color: #2c3e50;"
        "    font-size: 13px;"
        "}"
        "QListWidget {"
        "    border: 1px solid #ddd;"
        "    border-radius: 6px;"
        "    background-color: white;"
        "    alternate-background-color: #f8f9fa;"
        "}"
        "QListWidget::item {"
        "    padding: 15px;"
        "    border-bottom: 1px solid #eee;"
        "    margin: 1px;"
        "    border-radius: 4px;"
        "    min-height: 60px;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #e6f7ff;"
        "    border: 2px solid #1890ff;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #f0f8ff;"
        "}"

        "#editButton {"
        "    background-color: #1890ff;"
        "    border: none;"
        "    border-radius: 6px;"
        "    color: white;"
        "    font-size: 14px;"
        "    padding: 10px 16px;"
        "}"
        "#editButton:hover {"
        "    background-color: #096dd9;"
        "}"
        "#editButton:disabled {"
        "    background-color: #f5f5f5;"
        "    color: #999;"
        "    border: 1px solid #d9d9d9;"
        "}"

        "QLineEdit {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #1890ff;"
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
        );
}

void MyCoursesWidget::refreshData()
{
    try {
        updateCourseList();
        updateButtonStates();

        // 更新统计信息
        int totalCourses = m_courses.size();
        m_totalCoursesLabel->setText(QString("课程数量: %1").arg(totalCourses));

        if (m_isTeacher) {
            // 教师端：计算教授的学生总数
            int totalStudents = 0;
            for (const QVariantMap &course : m_courses) {
                totalStudents += course["student_count"].toInt();
            }
            m_totalStudentsLabel->setText(QString("学生总数: %1").arg(totalStudents));
        } else {
            // 学生端：计算总学分
            int totalCredits = 0;
            for (const QVariantMap &course : m_courses) {
                if (course["enrollment_status"].toString() == "已通过") {
                    totalCredits += course["credits"].toInt();
                }
            }
            m_totalCreditsLabel->setText(QString("总学分: %1").arg(totalCredits));
        }

        qDebug() << "我的课程数据刷新完成";
    } catch (const std::exception &e) {
        qDebug() << "刷新数据时发生异常:" << e.what();
        showMessage("刷新数据失败，请重试", true);
    } catch (...) {
        qDebug() << "刷新数据时发生未知异常";
        showMessage("刷新数据失败，请重试", true);
    }
}

void MyCoursesWidget::updateCourseList()
{
    if (!m_database) return;

    try {
        m_courses.clear();
        m_courseList->clear();

        QString searchText = m_searchLineEdit->text().trimmed().toLower();

        if (m_isTeacher) {
            // 教师端：获取教授的课程
            QList<Course> teacherCourses = m_database->getTeacherCourses(m_userId);

            for (const Course &course : teacherCourses) {
                QVariantMap courseMap;
                courseMap["course_id"] = course.getCourseId();
                courseMap["course_name"] = course.getCourseName();
                courseMap["college"] = course.getCollege();
                courseMap["teacher_name"] = course.getTeacherName();

                // 使用Course对象的真实数据，而不是硬编码
                courseMap["description"] = course.getDescription();
                courseMap["credits"] = course.getCredits();
                courseMap["course_hours"] = course.getCourseHours();
                courseMap["semester"] = course.getSemester();
                courseMap["max_students"] = course.getMaxStudents();
                courseMap["status"] = course.getStatus();
                courseMap["student_count"] = course.getStudentCount();

                // 应用搜索过滤
                if (!searchText.isEmpty()) {
                    QString courseName = courseMap["course_name"].toString().toLower();
                    if (!courseName.contains(searchText)) {
                        continue;
                    }
                }

                m_courses.append(courseMap);
            }
        } else {
            // 学生端：获取选修的课程
            QList<QVariantMap> studentCourses = m_database->getStudentCourses(m_userId);

            for (const QVariantMap &course : studentCourses) {
                // 应用搜索过滤
                if (!searchText.isEmpty()) {
                    QString courseName = course["course_name"].toString().toLower();
                    if (!courseName.contains(searchText)) {
                        continue;
                    }
                }

                m_courses.append(course);
            }
        }

        // 创建列表项
        for (const QVariantMap &course : m_courses) {
            createCourseListItem(course);
        }

        m_courseCountLabel->setText(QString("共%1门课程").arg(m_courses.size()));

        // 重置选择状态
        m_selectedCourseId = -1;
        m_courseDetailWidget->clearContent();

    } catch (...) {
        qDebug() << "更新课程列表时发生异常";
        showMessage("加载课程列表失败", true);
    }
}

void MyCoursesWidget::createCourseListItem(const QVariantMap &course)
{
    try {
        QListWidgetItem *item = new QListWidgetItem();

        QString courseName = course["course_name"].toString();
        QString college = course["college"].toString();
        int courseId = course["course_id"].toInt();

        QString displayText;

        if (m_isTeacher) {
            int studentCount = course["student_count"].toInt();
            int credits = course["credits"].toInt();

            displayText = QString("📚 %1\n🏫 学院: %2\n👥 学生: %3人 | 📊 学分: %4分")
                              .arg(courseName)
                              .arg(college)
                              .arg(studentCount)
                              .arg(credits);
        } else {
            QString teacherName = course["teacher_name"].toString();
            QString enrollmentStatus = course["enrollment_status"].toString();
            int credits = course["credits"].toInt();

            // 状态图标
            QString statusIcon = "✅";
            if (enrollmentStatus == "申请中") {
                statusIcon = "⏳";
            }

            displayText = QString("📚 %1\n👨‍🏫 教师: %2 | 🏫 %3\n%4 %5 | 📊 学分: %6分")
                              .arg(courseName)
                              .arg(teacherName)
                              .arg(college)
                              .arg(statusIcon)
                              .arg(enrollmentStatus)
                              .arg(credits);
        }

        item->setText(displayText);
        item->setData(Qt::UserRole, courseId);

        // 根据状态设置颜色（学生端）
        if (!m_isTeacher) {
            QString enrollmentStatus = course["enrollment_status"].toString();
            if (enrollmentStatus == "已通过") {
                item->setBackground(QBrush(QColor(240, 255, 240))); // 浅绿色
            } else if (enrollmentStatus == "申请中") {
                item->setBackground(QBrush(QColor(255, 248, 220))); // 浅黄色
            }
        }

        m_courseList->addItem(item);

    } catch (...) {
        qDebug() << "创建课程列表项时发生异常";
    }
}

void MyCoursesWidget::onCourseSelected(QListWidgetItem *item)
{
    if (!item) return;

    try {
        int courseId = item->data(Qt::UserRole).toInt();
        m_selectedCourseId = courseId;

        // 找到对应的课程数据
        QVariantMap selectedCourse;
        for (const QVariantMap &course : m_courses) {
            if (course["course_id"].toInt() == courseId) {
                selectedCourse = course;
                break;
            }
        }

        if (!selectedCourse.isEmpty()) {
            showCourseDetail(selectedCourse);
            updateButtonStates();
        }
    } catch (...) {
        qDebug() << "选择课程时发生异常";
    }
}

void MyCoursesWidget::showCourseDetail(const QVariantMap &course)
{
    try {
        m_courseDetailWidget->showCourse(course, m_isTeacher);
    } catch (...) {
        qDebug() << "显示课程详情时发生异常";
    }
}

void MyCoursesWidget::updateButtonStates()
{
    bool hasSelection = (m_selectedCourseId > 0);

    if (m_isTeacher && m_editButton) {
        m_editButton->setEnabled(hasSelection);
    }
}

void MyCoursesWidget::onEditCourse()
{
    if (m_selectedCourseId <= 0) {
        showMessage("请先选择要编辑的课程", true);
        return;
    }

    try {
        // 找到选中的课程数据
        QVariantMap selectedCourse;
        for (const QVariantMap &course : m_courses) {
            if (course["course_id"].toInt() == m_selectedCourseId) {
                selectedCourse = course;
                break;
            }
        }

        if (selectedCourse.isEmpty()) {
            showMessage("找不到选中的课程", true);
            return;
        }

        EditCourseDialog dialog(m_database, selectedCourse, this);
        connect(&dialog, &QDialog::accepted, [this]() {
            refreshData();
            emit courseUpdated(m_selectedCourseId);
        });

        dialog.exec();

    } catch (...) {
        qDebug() << "编辑课程时发生异常";
        showMessage("编辑课程失败", true);
    }
}

void MyCoursesWidget::onRefreshClicked()
{
    refreshData();
}

void MyCoursesWidget::onSearchTextChanged()
{
    // 延迟搜索，避免频繁查询
    static QTimer *searchTimer = nullptr;
    if (!searchTimer) {
        searchTimer = new QTimer(this);
        searchTimer->setSingleShot(true);
        searchTimer->setInterval(500); // 500ms延迟
        connect(searchTimer, &QTimer::timeout, [this]() {
            updateCourseList();
        });
    }

    searchTimer->start();
}

void MyCoursesWidget::showMessage(const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "提示", message);
    } else {
        QMessageBox::information(this, "提示", message);
    }
}

// ============================================================================
// CourseDetailWidget - 课程详情组件实现
// ============================================================================

CourseDetailWidget::CourseDetailWidget(QWidget *parent)
    : QWidget(parent), m_database(nullptr), m_isTeacherMode(false)
{
    setupUI();
    setupStyles();
    clearContent();
}

void CourseDetailWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // 创建滚动区域
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);

    // 课程基本信息
    m_basicInfoGroup = new QGroupBox("📚 课程基本信息");
    QFormLayout *basicLayout = new QFormLayout(m_basicInfoGroup);

    m_courseNameLabel = new QLabel();
    m_courseNameLabel->setObjectName("courseNameLabel");
    m_teacherLabel = new QLabel();
    m_collegeLabel = new QLabel();
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setReadOnly(true);
    m_descriptionEdit->setMaximumHeight(100);
    m_creditsLabel = new QLabel();
    m_hoursLabel = new QLabel();
    m_semesterLabel = new QLabel();
    m_statusLabel = new QLabel();

    basicLayout->addRow("课程名称:", m_courseNameLabel);
    basicLayout->addRow("任课教师:", m_teacherLabel);
    basicLayout->addRow("开课学院:", m_collegeLabel);
    basicLayout->addRow("课程描述:", m_descriptionEdit);
    basicLayout->addRow("学分:", m_creditsLabel);
    basicLayout->addRow("课时:", m_hoursLabel);
    basicLayout->addRow("开课学期:", m_semesterLabel);
    basicLayout->addRow("课程状态:", m_statusLabel);

    contentLayout->addWidget(m_basicInfoGroup);

    // 班级成员列表
    m_membersGroup = new QGroupBox("👥 班级成员");
    m_membersLayout = new QVBoxLayout(m_membersGroup);

    // 成员列表头部
    m_membersHeaderLayout = new QHBoxLayout();
    m_membersCountLabel = new QLabel("成员数量: 0");
    m_refreshMembersButton = new QPushButton("🔄 刷新成员");
    m_membersHeaderLayout->addWidget(m_membersCountLabel);
    m_membersHeaderLayout->addStretch();
    m_membersHeaderLayout->addWidget(m_refreshMembersButton);
    m_membersLayout->addLayout(m_membersHeaderLayout);

    // 成员表格
    m_membersTable = new QTableWidget();
    m_membersTable->setColumnCount(4);
    m_membersTable->setHorizontalHeaderLabels({"学号", "姓名", "年级", "状态"});
    m_membersTable->horizontalHeader()->setStretchLastSection(true);
    m_membersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_membersTable->setAlternatingRowColors(true);
    m_membersTable->setMaximumHeight(300);
    m_membersLayout->addWidget(m_membersTable);

    contentLayout->addWidget(m_membersGroup);

    // 课程统计
    m_statsGroup = new QGroupBox("📊 课程统计");
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);

    m_enrollmentStatsLabel = new QLabel();
    m_capacityLabel = new QLabel();

    statsLayout->addWidget(m_enrollmentStatsLabel);
    statsLayout->addWidget(m_capacityLabel);

    contentLayout->addWidget(m_statsGroup);

    // 欢迎页面
    m_welcomeWidget = new QWidget();
    QVBoxLayout *welcomeLayout = new QVBoxLayout(m_welcomeWidget);
    welcomeLayout->setAlignment(Qt::AlignCenter);
    m_welcomeLabel = new QLabel("📚\n\n选择左侧课程查看详细信息");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setObjectName("welcomeLabel");
    welcomeLayout->addWidget(m_welcomeLabel);

    contentLayout->addWidget(m_welcomeWidget);
    contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);

    // 连接信号槽
    connect(m_refreshMembersButton, &QPushButton::clicked, this, &CourseDetailWidget::onRefreshMembers);
}

void CourseDetailWidget::setupStyles()
{
    this->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #ddd;"
        "    border-radius: 8px;"
        "    margin-top: 8px;"
        "    padding-top: 12px;"
        "    background-color: #fafafa;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 8px 0 8px;"
        "    color: #2c3e50;"
        "    font-size: 13px;"
        "}"
        "#courseNameLabel {"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: #1890ff;"
        "}"
        "#welcomeLabel {"
        "    font-size: 24px;"
        "    color: #999;"
        "}"
        "QTextEdit {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: #f8f9fa;"
        "    padding: 8px;"
        "}"
        "QTableWidget {"
        "    border: 1px solid #ddd;"
        "    border-radius: 6px;"
        "    background-color: white;"
        "    alternate-background-color: #f8f9fa;"
        "}"
        "QTableWidget::item {"
        "    padding: 8px;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #e6f7ff;"
        "}"
        "QPushButton {"
        "    padding: 6px 12px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: #f8f9fa;"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e8f4ff;"
        "    border-color: #1890ff;"
        "}"
        );
}

void CourseDetailWidget::clearContent()
{
    // 隐藏详情区域，显示欢迎页面
    m_basicInfoGroup->hide();
    m_membersGroup->hide();
    m_statsGroup->hide();
    m_welcomeWidget->show();

    // 清空数据
    m_courseNameLabel->clear();
    m_teacherLabel->clear();
    m_collegeLabel->clear();
    m_descriptionEdit->clear();
    m_creditsLabel->clear();
    m_hoursLabel->clear();
    m_semesterLabel->clear();
    m_statusLabel->clear();
    m_membersTable->setRowCount(0);
    m_membersCountLabel->setText("成员数量: 0");
    m_enrollmentStatsLabel->clear();
    m_capacityLabel->clear();
    m_currentCourse.clear();
}

void CourseDetailWidget::showCourse(const QVariantMap &course, bool isTeacher)
{
    m_currentCourse = course;
    m_isTeacherMode = isTeacher;

    // 隐藏欢迎页面，显示详情区域
    m_welcomeWidget->hide();
    m_basicInfoGroup->show();
    m_statsGroup->show();

    // 填充基本信息
    m_courseNameLabel->setText(course["course_name"].toString());
    m_collegeLabel->setText(course["college"].toString());
    m_descriptionEdit->setText(course["description"].toString());
    m_creditsLabel->setText(QString::number(course["credits"].toInt()));
    m_hoursLabel->setText(QString::number(course["course_hours"].toInt()));
    m_semesterLabel->setText(course["semester"].toString());

    QString status = course["status"].toString();
    m_statusLabel->setText(status);

    // 根据状态设置颜色
    if (status == "开放选课") {
        m_statusLabel->setStyleSheet("color: #52c41a; font-weight: bold;");
    } else if (status == "关闭选课") {
        m_statusLabel->setStyleSheet("color: #ff4d4f; font-weight: bold;");
    } else {
        m_statusLabel->setStyleSheet("color: #999; font-weight: bold;");
    }

    if (isTeacher) {
        showTeacherView(course);
    } else {
        showStudentView(course);
    }
}

void CourseDetailWidget::showTeacherView(const QVariantMap &course)
{
    // 教师视图：显示班级成员列表
    m_teacherLabel->setText("任课教师: 我");
    m_membersGroup->show();

    // 加载班级成员
    int courseId = course["course_id"].toInt();
    loadCourseMembers(courseId);

    // 使用真实的课程统计数据
    if (m_database) {
        QVariantMap stats = m_database->getCourseStats(courseId);

        int enrolledCount = stats["enrolled_count"].toInt();
        int maxStudents = stats["max_students"].toInt();
        int assignmentCount = stats["assignment_count"].toInt();
        int examCount = stats["exam_count"].toInt();
        int noticeCount = stats["notice_count"].toInt();

        m_enrollmentStatsLabel->setText(QString("选课人数: %1/%2").arg(enrolledCount).arg(maxStudents));

        if (enrolledCount >= maxStudents) {
            m_capacityLabel->setText("⚠️ 选课人数已满");
            m_capacityLabel->setStyleSheet("color: #ff4d4f;");
        } else {
            m_capacityLabel->setText(QString("剩余名额: %1").arg(maxStudents - enrolledCount));
            m_capacityLabel->setStyleSheet("color: #52c41a;");
        }

        // 可以添加更多统计信息显示
        qDebug() << "课程统计 - 作业:" << assignmentCount << "考试:" << examCount << "通知:" << noticeCount;
    } else {
        // 降级处理：使用课程对象中的数据
        int studentCount = course["student_count"].toInt();
        int maxStudents = course["max_students"].toInt();
        m_enrollmentStatsLabel->setText(QString("选课人数: %1/%2").arg(studentCount).arg(maxStudents));

        if (studentCount >= maxStudents) {
            m_capacityLabel->setText("⚠️ 选课人数已满");
            m_capacityLabel->setStyleSheet("color: #ff4d4f;");
        } else {
            m_capacityLabel->setText(QString("剩余名额: %1").arg(maxStudents - studentCount));
            m_capacityLabel->setStyleSheet("color: #52c41a;");
        }
    }
}

void CourseDetailWidget::showStudentView(const QVariantMap &course)
{
    // 学生视图：不显示班级成员列表
    m_teacherLabel->setText(QString("任课教师: %1").arg(course["teacher_name"].toString()));
    m_membersGroup->hide();

    // 更新统计信息
    QString enrollmentStatus = course["enrollment_status"].toString();
    m_enrollmentStatsLabel->setText(QString("选课状态: %1").arg(enrollmentStatus));

    if (enrollmentStatus == "已通过") {
        m_enrollmentStatsLabel->setStyleSheet("color: #52c41a; font-weight: bold;");
    } else if (enrollmentStatus == "申请中") {
        m_enrollmentStatsLabel->setStyleSheet("color: #faad14; font-weight: bold;");
    }

    // 显示考试成绩（如果有）
    if (course.contains("exam_score") && !course["exam_score"].isNull()) {
        double score = course["exam_score"].toDouble();
        m_capacityLabel->setText(QString("考试成绩: %1分").arg(score));
        m_capacityLabel->setStyleSheet("color: #1890ff; font-weight: bold;");
    } else {
        m_capacityLabel->setText("暂无考试成绩");
        m_capacityLabel->setStyleSheet("color: #999;");
    }
}

void CourseDetailWidget::onRefreshMembers()
{
    if (m_currentCourse.isEmpty() || !m_isTeacherMode) return;

    int courseId = m_currentCourse["course_id"].toInt();
    loadCourseMembers(courseId);
}

void CourseDetailWidget::loadCourseMembers(int courseId)
{
    if (!m_database) return;

    try {
        // 清空表格
        m_membersTable->setRowCount(0);

        // 获取课程成员列表 - 使用真实的数据库方法
        QList<QVariantMap> students = m_database->getCourseMembers(courseId);

        // 填充表格
        m_membersTable->setRowCount(students.size());
        for (int i = 0; i < students.size(); ++i) {
            const QVariantMap &student = students[i];

            m_membersTable->setItem(i, 0, new QTableWidgetItem(student["student_id"].toString()));
            m_membersTable->setItem(i, 1, new QTableWidgetItem(student["name"].toString()));
            m_membersTable->setItem(i, 2, new QTableWidgetItem(student["grade"].toString()));

            QTableWidgetItem *statusItem = new QTableWidgetItem(student["enrollment_status"].toString());
            if (student["enrollment_status"].toString() == "已通过") {
                statusItem->setForeground(QColor("#52c41a"));
            } else if (student["enrollment_status"].toString() == "申请中") {
                statusItem->setForeground(QColor("#faad14"));
            }
            m_membersTable->setItem(i, 3, statusItem);
        }

        m_membersCountLabel->setText(QString("成员数量: %1").arg(students.size()));

    } catch (...) {
        qDebug() << "加载班级成员时发生异常";
        QMessageBox::warning(this, "错误", "加载班级成员失败，请重试");
    }
}
// ============================================================================
// EditCourseDialog - 编辑课程对话框实现
// ============================================================================

EditCourseDialog::EditCourseDialog(Database *database, const QVariantMap &course, QWidget *parent)
    : QDialog(parent)
    , m_database(database)
    , m_originalCourse(course)
    , m_courseId(course["course_id"].toInt())
{
    setWindowTitle("编辑课程信息");
    setFixedSize(500, 600);

    setupUI();
    setupStyles();
    loadCourseData();
}

void EditCourseDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // 基本信息组
    m_basicGroup = new QGroupBox("课程基本信息");
    QFormLayout *formLayout = new QFormLayout(m_basicGroup);

    m_courseNameEdit = new QLineEdit();
    m_courseNameEdit->setPlaceholderText("请输入课程名称");
    formLayout->addRow("课程名称:", m_courseNameEdit);

    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText("请输入课程描述");
    m_descriptionEdit->setMaximumHeight(100);
    formLayout->addRow("课程描述:", m_descriptionEdit);

    m_creditsSpinBox = new QSpinBox();
    m_creditsSpinBox->setRange(1, 10);
    m_creditsSpinBox->setSuffix(" 学分");
    formLayout->addRow("学分:", m_creditsSpinBox);

    m_hoursSpinBox = new QSpinBox();
    m_hoursSpinBox->setRange(16, 128);
    m_hoursSpinBox->setSingleStep(16);
    m_hoursSpinBox->setSuffix(" 课时");
    formLayout->addRow("课时:", m_hoursSpinBox);

    m_semesterEdit = new QLineEdit();
    m_semesterEdit->setPlaceholderText("如：2024-2025学年第一学期");
    formLayout->addRow("开课学期:", m_semesterEdit);

    m_maxStudentsSpinBox = new QSpinBox();
    m_maxStudentsSpinBox->setRange(10, 200);
    m_maxStudentsSpinBox->setSuffix(" 人");
    formLayout->addRow("最大人数:", m_maxStudentsSpinBox);

    m_statusCombo = new QComboBox();
    m_statusCombo->addItems({"开放选课", "关闭选课", "已结束"});
    formLayout->addRow("课程状态:", m_statusCombo);

    m_mainLayout->addWidget(m_basicGroup);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_saveButton = new QPushButton("保存");
    m_saveButton->setObjectName("saveButton");
    m_cancelButton = new QPushButton("取消");
    m_cancelButton->setObjectName("cancelButton");

    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);

    m_mainLayout->addStretch();
    m_mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(m_saveButton, &QPushButton::clicked, this, &EditCourseDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &EditCourseDialog::onCancelClicked);
}

void EditCourseDialog::setupStyles()
{
    this->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #ddd;"
        "    border-radius: 8px;"
        "    margin-top: 8px;"
        "    padding-top: 12px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 8px 0 8px;"
        "}"
        "QLineEdit, QTextEdit, QSpinBox, QComboBox {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 6px;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QComboBox:focus {"
        "    border-color: #1890ff;"
        "}"
        "#saveButton {"
        "    background-color: #1890ff;"
        "    border: none;"
        "    border-radius: 4px;"
        "    color: white;"
        "    padding: 8px 20px;"
        "    font-size: 14px;"
        "}"
        "#saveButton:hover {"
        "    background-color: #096dd9;"
        "}"
        "#cancelButton {"
        "    background-color: #f5f5f5;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 8px 20px;"
        "    font-size: 14px;"
        "}"
        "#cancelButton:hover {"
        "    background-color: #e8e8e8;"
        "}"
        );
}

void EditCourseDialog::loadCourseData()
{
    m_courseNameEdit->setText(m_originalCourse["course_name"].toString());
    m_descriptionEdit->setText(m_originalCourse["description"].toString());
    m_creditsSpinBox->setValue(m_originalCourse["credits"].toInt());
    m_hoursSpinBox->setValue(m_originalCourse["course_hours"].toInt());
    m_semesterEdit->setText(m_originalCourse["semester"].toString());
    m_maxStudentsSpinBox->setValue(m_originalCourse["max_students"].toInt());

    QString status = m_originalCourse["status"].toString();
    int index = m_statusCombo->findText(status);
    if (index >= 0) {
        m_statusCombo->setCurrentIndex(index);
    }
}

void EditCourseDialog::onSaveClicked()
{
    if (!validateInput()) {
        return;
    }

    if (saveCourseChanges()) {
        accept();
    }
}

void EditCourseDialog::onCancelClicked()
{
    reject();
}

bool EditCourseDialog::validateInput()
{
    if (m_courseNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入验证", "课程名称不能为空！");
        m_courseNameEdit->setFocus();
        return false;
    }

    if (m_descriptionEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入验证", "课程描述不能为空！");
        m_descriptionEdit->setFocus();
        return false;
    }

    if (m_semesterEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入验证", "开课学期不能为空！");
        m_semesterEdit->setFocus();
        return false;
    }

    return true;
}

bool EditCourseDialog::saveCourseChanges()
{
    if (!m_database) {
        QMessageBox::critical(this, "错误", "数据库连接失败！");
        return false;
    }

    try {
        // 使用真实的数据库更新方法
        bool success = m_database->updateCourseInfo(
            m_courseId,
            m_courseNameEdit->text().trimmed(),
            m_descriptionEdit->toPlainText().trimmed(),
            m_creditsSpinBox->value(),
            m_hoursSpinBox->value(),
            m_semesterEdit->text().trimmed(),
            m_maxStudentsSpinBox->value(),
            m_statusCombo->currentText()
            );

        if (success) {
            QMessageBox::information(this, "成功", "课程信息更新成功！");
            return true;
        } else {
            QMessageBox::critical(this, "错误", "更新课程信息失败，请重试！");
            return false;
        }

    } catch (...) {
        QMessageBox::critical(this, "错误", "保存课程信息时发生错误！");
        return false;
    }
}
