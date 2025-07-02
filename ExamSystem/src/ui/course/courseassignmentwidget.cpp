#include "courseassignmentwidget.h"
#include "../../models/course.h"
#include "../../models/courseassignment.h"
#include "../../models/assignmentsubmission.h"
#include <QMessageBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QHeaderView>
#include <QApplication>
#include <QInputDialog>

CourseAssignmentWidget::CourseAssignmentWidget(Database *database, int userId, const QString &userType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_userId(userId)
    , m_userType(userType)
    , m_isTeacher(userType == "老师")
    , m_selectedAssignmentId(-1)
    , m_refreshTimer(new QTimer(this))
{
    setupUI();
    setupStyles();

    // 设置自动刷新
    m_refreshTimer->setInterval(30000); // 30秒自动刷新
    connect(m_refreshTimer, &QTimer::timeout, this, &CourseAssignmentWidget::onAutoRefresh);
    m_refreshTimer->start();

    refreshData();
}

void CourseAssignmentWidget::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);

    // 左侧作业列表区域
    m_leftWidget = new QWidget();
    m_leftWidget->setFixedWidth(380); // 增加宽度以容纳更多信息
    m_leftLayout = new QVBoxLayout(m_leftWidget);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);

    // 筛选区域（增强版）
    m_filterGroup = new QGroupBox("筛选与搜索");
    QVBoxLayout *filterLayout = new QVBoxLayout(m_filterGroup);

    m_courseFilterCombo = new QComboBox();
    m_statusFilterCombo = new QComboBox();
    m_statusFilterCombo->addItems({"全部状态", "开放提交", "已截止"});

    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("搜索作业标题...");

    m_refreshButton = new QPushButton("🔄 刷新");
    m_refreshButton->setToolTip("手动刷新作业列表");

    filterLayout->addWidget(new QLabel("课程:"));
    filterLayout->addWidget(m_courseFilterCombo);
    filterLayout->addWidget(new QLabel("状态:"));
    filterLayout->addWidget(m_statusFilterCombo);
    filterLayout->addWidget(new QLabel("搜索:"));
    filterLayout->addWidget(m_searchLineEdit);
    filterLayout->addWidget(m_refreshButton);

    m_leftLayout->addWidget(m_filterGroup);

    // 统计信息区域
    m_statsGroup = new QGroupBox("统计信息");
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);

    m_totalAssignmentsLabel = new QLabel("总作业数: 0");
    m_openAssignmentsLabel = new QLabel("开放中: 0");

    if (m_isTeacher) {
        m_gradedLabel = new QLabel("已批改: 0");
        statsLayout->addWidget(m_totalAssignmentsLabel);
        statsLayout->addWidget(m_openAssignmentsLabel);
        statsLayout->addWidget(m_gradedLabel);
    } else {
        m_submittedLabel = new QLabel("已提交: 0");
        statsLayout->addWidget(m_totalAssignmentsLabel);
        statsLayout->addWidget(m_openAssignmentsLabel);
        statsLayout->addWidget(m_submittedLabel);
    }

    m_leftLayout->addWidget(m_statsGroup);

    // 作业列表
    m_listGroup = new QGroupBox("课程作业");
    QVBoxLayout *listLayout = new QVBoxLayout(m_listGroup);

    m_assignmentList = new QListWidget();
    m_assignmentList->setAlternatingRowColors(true);
    listLayout->addWidget(m_assignmentList);

    m_assignmentCountLabel = new QLabel("共0个作业");
    listLayout->addWidget(m_assignmentCountLabel);

    m_leftLayout->addWidget(m_listGroup);

    // 操作按钮
    if (m_isTeacher) {
        setupTeacherUI();
    } else {
        setupStudentUI();
    }

    m_splitter->addWidget(m_leftWidget);

    // 右侧作业详情区域
    m_assignmentDetailWidget = new AssignmentDetailWidget(this);
    m_assignmentDetailWidget->m_database = m_database; // 传递数据库连接

    // 如果是学生端，设置学生ID
    if (!m_isTeacher) {
        m_assignmentDetailWidget->setStudentId(m_userId);
    }

    m_splitter->addWidget(m_assignmentDetailWidget);

    // 设置分割器比例
    m_splitter->setSizes({380, 700});

    // 连接信号槽
    connect(m_assignmentList, &QListWidget::itemClicked, this, &CourseAssignmentWidget::onAssignmentSelected);
    connect(m_refreshButton, &QPushButton::clicked, this, &CourseAssignmentWidget::onRefreshClicked);
    connect(m_courseFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CourseAssignmentWidget::onCourseFilterChanged);
    connect(m_statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CourseAssignmentWidget::onStatusFilterChanged);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &CourseAssignmentWidget::onSearchTextChanged);
}

void CourseAssignmentWidget::setupTeacherUI()
{
    m_actionGroup = new QGroupBox("操作");
    QVBoxLayout *actionLayout = new QVBoxLayout(m_actionGroup);

    m_publishButton = new QPushButton("📝 发布作业");
    m_editButton = new QPushButton("✏️ 编辑作业");
    m_gradeButton = new QPushButton("📊 批改作业");
    m_deleteButton = new QPushButton("🗑️ 删除作业");

    // 初始状态：只有发布按钮可用
    m_editButton->setEnabled(false);
    m_gradeButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    // 设置按钮样式
    m_publishButton->setObjectName("publishButton");
    m_editButton->setObjectName("editButton");
    m_gradeButton->setObjectName("gradeButton");
    m_deleteButton->setObjectName("deleteButton");

    actionLayout->addWidget(m_publishButton);
    actionLayout->addWidget(m_editButton);
    actionLayout->addWidget(m_gradeButton);
    actionLayout->addWidget(m_deleteButton);

    m_leftLayout->addWidget(m_actionGroup);

    connect(m_publishButton, &QPushButton::clicked, this, &CourseAssignmentWidget::onPublishAssignment);
    connect(m_editButton, &QPushButton::clicked, this, &CourseAssignmentWidget::onEditAssignment);
    connect(m_gradeButton, &QPushButton::clicked, this, &CourseAssignmentWidget::onGradeAssignment);
    connect(m_deleteButton, &QPushButton::clicked, this, &CourseAssignmentWidget::onDeleteAssignment);
}

void CourseAssignmentWidget::setupStudentUI()
{
    m_actionGroup = new QGroupBox("操作");
    QVBoxLayout *actionLayout = new QVBoxLayout(m_actionGroup);

    m_submitButton = new QPushButton("📤 提交作业");
    m_submitButton->setEnabled(false);
    m_submitButton->setObjectName("submitButton");

    actionLayout->addWidget(m_submitButton);

    m_leftLayout->addWidget(m_actionGroup);

    connect(m_submitButton, &QPushButton::clicked, this, &CourseAssignmentWidget::onSubmitAssignment);
}

void CourseAssignmentWidget::setupStyles()
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

        // 按钮样式
        "#publishButton {"
        "    background-color: #52c41a;"
        "    border: none;"
        "    border-radius: 6px;"
        "    color: white;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    padding: 10px 16px;"
        "}"
        "#publishButton:hover {"
        "    background-color: #389e0d;"
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
        "#gradeButton {"
        "    background-color: #fa8c16;"
        "    border: none;"
        "    border-radius: 6px;"
        "    color: white;"
        "    font-size: 14px;"
        "    padding: 10px 16px;"
        "}"
        "#gradeButton:hover {"
        "    background-color: #d46b08;"
        "}"
        "#deleteButton {"
        "    background-color: #ff4d4f;"
        "    border: none;"
        "    border-radius: 6px;"
        "    color: white;"
        "    font-size: 14px;"
        "    padding: 10px 16px;"
        "}"
        "#deleteButton:hover {"
        "    background-color: #cf1322;"
        "}"
        "#submitButton {"
        "    background-color: #1890ff;"
        "    border: none;"
        "    border-radius: 6px;"
        "    color: white;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    padding: 12px 20px;"
        "}"
        "#submitButton:hover {"
        "    background-color: #096dd9;"
        "}"

        "QPushButton:disabled {"
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

        "QComboBox {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    font-size: 14px;"
        "}"
        );
}

void CourseAssignmentWidget::refreshData()
{
    try {
        updateCourseFilter();
        updateAssignmentList();
        updateStatistics();
        updateButtonStates();

        qDebug() << "课程作业数据刷新完成";
    } catch (const std::exception &e) {
        qDebug() << "刷新数据时发生异常:" << e.what();
        showMessage("刷新数据失败，请重试", true);
    } catch (...) {
        qDebug() << "刷新数据时发生未知异常";
        showMessage("刷新数据失败，请重试", true);
    }
}

void CourseAssignmentWidget::updateCourseFilter()
{
    if (!m_database) {
        qDebug() << "数据库连接无效";
        return;
    }

    try {
        m_courseFilterCombo->clear();
        m_courseFilterCombo->addItem("全部课程");

        if (m_isTeacher) {
            QList<Course> courses = m_database->getTeacherCourses(m_userId);
            m_teacherCourses.clear();

            for (const Course &course : courses) {
                QVariantMap courseMap;
                courseMap["course_id"] = course.getCourseId();
                courseMap["course_name"] = course.getCourseName();
                m_teacherCourses.append(courseMap);

                m_courseFilterCombo->addItem(course.getCourseName(), course.getCourseId());
            }
        } else {
            m_studentCourses = m_database->getStudentCourses(m_userId);

            for (const QVariantMap &course : m_studentCourses) {
                if (course["enrollment_status"].toString() == "已通过") {
                    QString courseName = course["course_name"].toString();
                    int courseId = course["course_id"].toInt();
                    m_courseFilterCombo->addItem(courseName, courseId);
                }
            }
        }
    } catch (...) {
        qDebug() << "更新课程筛选器时发生异常";
    }
}

void CourseAssignmentWidget::updateAssignmentList()
{
    if (!m_database) return;

    try {
        m_assignments.clear();
        m_assignmentList->clear();

        QList<QVariantMap> filteredAssignments = getFilteredAssignments();

        for (const QVariantMap &assignment : filteredAssignments) {
            createAssignmentListItem(assignment);
            m_assignments.append(assignment);
        }

        m_assignmentCountLabel->setText(QString("共%1个作业").arg(m_assignments.size()));

        // 重置选择状态
        m_selectedAssignmentId = -1;
        updateButtonStates();
        m_assignmentDetailWidget->clearContent();

    } catch (...) {
        qDebug() << "更新作业列表时发生异常";
        showMessage("加载作业列表失败", true);
    }
}

void CourseAssignmentWidget::updateStatistics()
{
    if (!m_database || m_assignments.isEmpty()) {
        m_totalAssignmentsLabel->setText("总作业数: 0");
        m_openAssignmentsLabel->setText("开放中: 0");
        if (m_isTeacher) {
            if (m_gradedLabel) m_gradedLabel->setText("已批改: 0");
        } else {
            if (m_submittedLabel) m_submittedLabel->setText("已提交: 0");
        }
        return;
    }

    try {
        int totalCount = m_assignments.size();
        int openCount = 0;
        int submittedOrGradedCount = 0;

        for (const QVariantMap &assignment : m_assignments) {
            if (assignment["status"].toString() == "开放提交") {
                openCount++;
            }

            if (m_isTeacher) {
                // 教师端统计已批改的作业数
                // 这里可以扩展为真正的批改统计
            } else {
                // 学生端统计已提交的作业数
                if (assignment["has_submitted"].toBool()) {
                    submittedOrGradedCount++;
                }
            }
        }

        m_totalAssignmentsLabel->setText(QString("总作业数: %1").arg(totalCount));
        m_openAssignmentsLabel->setText(QString("开放中: %1").arg(openCount));

        if (m_isTeacher) {
            if (m_gradedLabel) m_gradedLabel->setText(QString("待批改: %1").arg(totalCount - submittedOrGradedCount));
        } else {
            if (m_submittedLabel) m_submittedLabel->setText(QString("已提交: %1").arg(submittedOrGradedCount));
        }
    } catch (...) {
        qDebug() << "更新统计信息时发生异常";
    }
}

void CourseAssignmentWidget::updateButtonStates()
{
    bool hasSelection = (m_selectedAssignmentId > 0);

    if (m_isTeacher) {
        m_editButton->setEnabled(hasSelection);
        m_gradeButton->setEnabled(hasSelection);
        m_deleteButton->setEnabled(hasSelection);
    } else {
        // 学生端：检查是否可以提交
        bool canSubmit = false;
        if (hasSelection) {
            for (const QVariantMap &assignment : m_assignments) {
                if (assignment["assignment_id"].toInt() == m_selectedAssignmentId) {
                    QString status = assignment["status"].toString();
                    bool hasSubmitted = assignment["has_submitted"].toBool();
                    QDateTime deadline = assignment["deadline"].toDateTime();
                    canSubmit = (status == "开放提交" && !hasSubmitted && QDateTime::currentDateTime() < deadline);
                    break;
                }
            }
        }
        m_submitButton->setEnabled(canSubmit);
    }
}

QList<QVariantMap> CourseAssignmentWidget::getFilteredAssignments()
{
    QList<QVariantMap> allAssignments;

    if (!m_database) return allAssignments;

    try {
        int selectedCourseId = m_courseFilterCombo->currentData().toInt();
        QString statusFilter = m_statusFilterCombo->currentText();
        QString searchText = m_searchLineEdit->text().trimmed().toLower();

        if (m_isTeacher) {
            // 教师端：获取自己课程的作业
            for (const QVariantMap &course : m_teacherCourses) {
                int courseId = course["course_id"].toInt();

                if (selectedCourseId > 0 && courseId != selectedCourseId) {
                    continue;
                }

                QList<QVariantMap> courseAssignments = m_database->getCourseAssignments(courseId);
                for (QVariantMap assignment : courseAssignments) {
                    assignment["course_name"] = course["course_name"].toString();
                    allAssignments.append(assignment);
                }
            }
        } else {
            // 学生端：获取已选课程的作业
            allAssignments = m_database->getStudentAssignments(m_userId);
        }

        // 应用状态过滤
        if (statusFilter != "全部状态") {
            QList<QVariantMap> filteredAssignments;
            for (const QVariantMap &assignment : allAssignments) {
                QString status = assignment["status"].toString();
                if (statusFilter == status) {
                    filteredAssignments.append(assignment);
                }
            }
            allAssignments = filteredAssignments;
        }

        // 应用搜索过滤
        if (!searchText.isEmpty()) {
            QList<QVariantMap> searchedAssignments;
            for (const QVariantMap &assignment : allAssignments) {
                QString title = assignment["title"].toString().toLower();
                QString courseName = assignment["course_name"].toString().toLower();
                if (title.contains(searchText) || courseName.contains(searchText)) {
                    searchedAssignments.append(assignment);
                }
            }
            allAssignments = searchedAssignments;
        }
    } catch (...) {
        qDebug() << "筛选作业时发生异常";
    }

    return allAssignments;
}

void CourseAssignmentWidget::createAssignmentListItem(const QVariantMap &assignment)
{
    try {
        QListWidgetItem *item = new QListWidgetItem();

        QString title = assignment["title"].toString();
        QString courseName = assignment["course_name"].toString();
        QDateTime deadline = assignment["deadline"].toDateTime();
        QString status = assignment["status"].toString();
        int assignmentId = assignment["assignment_id"].toInt();
        int maxScore = assignment["max_score"].toInt();

        // 计算剩余时间
        QString timeText;
        QDateTime now = QDateTime::currentDateTime();
        qint64 secsToDeadline = now.secsTo(deadline);

        if (secsToDeadline < 0) {
            timeText = "已过期";
        } else if (secsToDeadline < 3600) {
            timeText = QString("剩余%1分钟").arg(secsToDeadline / 60);
        } else if (secsToDeadline < 86400) {
            timeText = QString("剩余%1小时").arg(secsToDeadline / 3600);
        } else {
            timeText = QString("剩余%1天").arg(secsToDeadline / 86400);
        }

        QString displayText;
        if (m_isTeacher) {
            // 教师端显示
            displayText = QString("📋 %1\n📚 课程: %2\n⏰ %3 | 📊 满分: %4分\n📈 状态: %5")
                              .arg(title)
                              .arg(courseName)
                              .arg(deadline.toString("MM-dd hh:mm截止"))
                              .arg(maxScore)
                              .arg(status);
        } else {
            // 学生端显示提交状态
            bool hasSubmitted = assignment["has_submitted"].toBool();
            QString submissionStatus = hasSubmitted ? "✅ 已提交" : "❌ 未提交";

            displayText = QString("📋 %1\n📚 课程: %2\n⏰ %3 |  满分: %4分\n %5 |  %6")
                              .arg(title)
                              .arg(courseName)
                              .arg(timeText)
                              .arg(maxScore)
                              .arg(submissionStatus)
                              .arg(status);
        }

        item->setText(displayText);
        item->setData(Qt::UserRole, assignmentId);

        // 根据状态设置颜色
        if (status == "已截止" || secsToDeadline < 0) {
            item->setBackground(QBrush(QColor(255, 245, 245))); // 浅红色
        } else if (!m_isTeacher && assignment["has_submitted"].toBool()) {
            item->setBackground(QBrush(QColor(245, 255, 245))); // 浅绿色
        } else if (secsToDeadline < 86400 && secsToDeadline > 0) {
            item->setBackground(QBrush(QColor(255, 250, 205))); // 浅黄色（即将截止）
        }

        m_assignmentList->addItem(item);

    } catch (...) {
        qDebug() << "创建作业列表项时发生异常";
    }
}

void CourseAssignmentWidget::onAssignmentSelected(QListWidgetItem *item)
{
    if (!item) return;

    try {
        int assignmentId = item->data(Qt::UserRole).toInt();
        m_selectedAssignmentId = assignmentId;

        // 找到对应的作业数据
        QVariantMap selectedAssignment;
        for (const QVariantMap &assignment : m_assignments) {
            if (assignment["assignment_id"].toInt() == assignmentId) {
                selectedAssignment = assignment;
                break;
            }
        }

        if (!selectedAssignment.isEmpty()) {
            showAssignmentDetail(selectedAssignment);
            updateButtonStates();
        }
    } catch (...) {
        qDebug() << "选择作业时发生异常";
    }
}

void CourseAssignmentWidget::showAssignmentDetail(const QVariantMap &assignment)
{
    try {
        m_assignmentDetailWidget->showAssignment(assignment, m_isTeacher);
    } catch (...) {
        qDebug() << "显示作业详情时发生异常";
    }
}

void CourseAssignmentWidget::onPublishAssignment()
{
    try {
        PublishAssignmentDialog dialog(m_database, m_userId, this);
        connect(&dialog, &QDialog::accepted, [this]() {
            refreshData();
            emit assignmentPublished(0);
        });

        dialog.exec();
    } catch (...) {
        qDebug() << "发布作业时发生异常";
        showMessage("发布作业失败", true);
    }
}

void CourseAssignmentWidget::onEditAssignment()
{
    if (m_selectedAssignmentId <= 0) {
        showMessage("请先选择要编辑的作业", true);
        return;
    }

    showMessage("编辑作业功能开发中...", false);
    // TODO: 实现编辑作业功能
}

void CourseAssignmentWidget::onDeleteAssignment()
{
    if (m_selectedAssignmentId <= 0) {
        showMessage("请先选择要删除的作业", true);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        "确定要删除这个作业吗？此操作不可撤销！",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            if (m_database->deleteAssignment(m_selectedAssignmentId)) {
                showMessage("作业删除成功", false);
                refreshData();
            } else {
                showMessage("作业删除失败", true);
            }
        } catch (...) {
            qDebug() << "删除作业时发生异常";
            showMessage("删除作业失败", true);
        }
    }
}

void CourseAssignmentWidget::onSubmitAssignment()
{
    if (m_selectedAssignmentId <= 0) {
        showMessage("请先选择要提交的作业", true);
        return;
    }

    try {
        // 找到选中的作业
        QVariantMap selectedAssignment;
        for (const QVariantMap &assignment : m_assignments) {
            if (assignment["assignment_id"].toInt() == m_selectedAssignmentId) {
                selectedAssignment = assignment;
                break;
            }
        }

        if (selectedAssignment.isEmpty()) {
            showMessage("找不到选中的作业", true);
            return;
        }

        QString title = selectedAssignment["title"].toString();
        SubmitAssignmentDialog dialog(m_selectedAssignmentId, title, m_database, m_userId, this);
        connect(&dialog, &QDialog::accepted, [this]() {
            refreshData();
            emit assignmentSubmitted(m_selectedAssignmentId);
        });

        dialog.exec();

    } catch (...) {
        qDebug() << "提交作业时发生异常";
        showMessage("提交作业失败", true);
    }
}

void CourseAssignmentWidget::onGradeAssignment()
{
    if (m_selectedAssignmentId <= 0) {
        showMessage("请先选择要批改的作业", true);
        return;
    }

    try {
        // 创建批改作业对话框
        GradeAssignmentDialog *dialog = new GradeAssignmentDialog(m_database, m_selectedAssignmentId, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(dialog, &QDialog::accepted, [this]() {
            refreshData();
            emit assignmentGraded(m_selectedAssignmentId);
        });

        dialog->show();

    } catch (...) {
        qDebug() << "打开批改界面时发生异常";
        showMessage("打开批改界面失败", true);
    }
}

void CourseAssignmentWidget::onRefreshClicked()
{
    refreshData();
}

void CourseAssignmentWidget::onAutoRefresh()
{
    // 自动刷新时不显示提示
    try {
        updateAssignmentList();
        updateStatistics();
    } catch (...) {
        qDebug() << "自动刷新失败";
    }
}

void CourseAssignmentWidget::onCourseFilterChanged()
{
    updateAssignmentList();
    updateStatistics();
}

void CourseAssignmentWidget::onStatusFilterChanged()
{
    updateAssignmentList();
    updateStatistics();
}

void CourseAssignmentWidget::onSearchTextChanged()
{
    // 延迟搜索，避免频繁查询
    static QTimer *searchTimer = nullptr;
    if (!searchTimer) {
        searchTimer = new QTimer(this);
        searchTimer->setSingleShot(true);
        searchTimer->setInterval(500); // 500ms延迟
        connect(searchTimer, &QTimer::timeout, [this]() {
            updateAssignmentList();
            updateStatistics();
        });
    }

    searchTimer->start();
}

void CourseAssignmentWidget::showMessage(const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "提示", message);
    } else {
        QMessageBox::information(this, "提示", message);
    }
}

// ============================================================================
// AssignmentDetailWidget - 作业详情组件实现（增强版）
// ============================================================================

AssignmentDetailWidget::AssignmentDetailWidget(QWidget *parent)
    : QWidget(parent), m_database(nullptr), m_isTeacherMode(false), m_currentStudentId(-1)
{
    setupUI();
    setupStyles();
    clearContent();
}

void AssignmentDetailWidget::setupUI()
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

    // 基本信息
    m_titleLabel = new QLabel();
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setWordWrap(true);

    m_infoLabel = new QLabel();
    m_infoLabel->setObjectName("infoLabel");

    contentLayout->addWidget(m_titleLabel);
    contentLayout->addWidget(m_infoLabel);

    // 作业描述
    QLabel *descLabel = new QLabel("📋 作业描述:");
    descLabel->setObjectName("sectionLabel");
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setReadOnly(true);
    m_descriptionEdit->setMaximumHeight(150);

    contentLayout->addWidget(descLabel);
    contentLayout->addWidget(m_descriptionEdit);

    // 提交相关（学生端）
    m_submissionGroup = new QGroupBox("📤 我的提交");
    QVBoxLayout *submissionLayout = new QVBoxLayout(m_submissionGroup);

    m_submissionEdit = new QTextEdit();
    m_submissionEdit->setReadOnly(true);
    m_submissionEdit->setMaximumHeight(120);

    m_gradeLabel = new QLabel();
    m_gradeLabel->setObjectName("gradeLabel");

    submissionLayout->addWidget(m_submissionEdit);
    submissionLayout->addWidget(m_gradeLabel);

    contentLayout->addWidget(m_submissionGroup);

    // 统计相关（教师端）
    m_statsGroup = new QGroupBox("📊 提交统计");
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);

    m_submissionStatsLabel = new QLabel();
    m_gradeProgressBar = new QProgressBar();
    m_viewSubmissionsButton = new QPushButton("👥 查看所有提交");

    statsLayout->addWidget(m_submissionStatsLabel);
    statsLayout->addWidget(m_gradeProgressBar);
    statsLayout->addWidget(m_viewSubmissionsButton);

    contentLayout->addWidget(m_statsGroup);

    // 欢迎页面
    m_welcomeWidget = new QWidget();
    QVBoxLayout *welcomeLayout = new QVBoxLayout(m_welcomeWidget);
    welcomeLayout->setAlignment(Qt::AlignCenter);
    m_welcomeLabel = new QLabel("📝\n\n选择左侧作业查看详细内容");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setObjectName("welcomeLabel");
    welcomeLayout->addWidget(m_welcomeLabel);

    contentLayout->addWidget(m_welcomeWidget);
    contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);

    // 连接信号槽
    connect(m_viewSubmissionsButton, &QPushButton::clicked, this, &AssignmentDetailWidget::onViewSubmissionsClicked);
}

void AssignmentDetailWidget::setupStyles()
{
    this->setStyleSheet(
        "#titleLabel {"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    padding: 10px 0px;"
        "    border-bottom: 3px solid #3498db;"
        "    margin-bottom: 10px;"
        "}"
        "#infoLabel {"
        "    font-size: 14px;"
        "    color: #7f8c8d;"
        "    padding: 8px 0px 15px 0px;"
        "    line-height: 1.5;"
        "}"
        "#sectionLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #34495e;"
        "    margin-top: 15px;"
        "    margin-bottom: 8px;"
        "}"
        "#gradeLabel {"
        "    font-size: 14px;"
        "    color: #2c3e50;"
        "    padding: 12px;"
        "    background-color: #ecf0f1;"
        "    border-radius: 6px;"
        "    border-left: 4px solid #3498db;"
        "}"
        "#welcomeLabel {"
        "    font-size: 18px;"
        "    color: #95a5a6;"
        "    font-style: italic;"
        "}"
        "QTextEdit {"
        "    border: 1px solid #ddd;"
        "    border-radius: 6px;"
        "    background-color: #fafafa;"
        "    font-size: 14px;"
        "    padding: 8px;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #ddd;"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 15px;"
        "    background-color: #f8f9fa;"
        "}"
        "QProgressBar {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    text-align: center;"
        "    height: 20px;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #52c41a;"
        "    border-radius: 3px;"
        "}"
        );
}

void AssignmentDetailWidget::showAssignment(const QVariantMap &assignment, bool isTeacher)
{
    try {
        m_currentAssignment = assignment;
        m_isTeacherMode = isTeacher;

        m_welcomeWidget->hide();

        if (isTeacher) {
            showTeacherView(assignment);
        } else {
            showStudentView(assignment);
        }
    } catch (...) {
        qDebug() << "显示作业详情时发生异常";
        clearContent();
    }
}

void AssignmentDetailWidget::showTeacherView(const QVariantMap &assignment)
{
    // 显示基本信息
    m_titleLabel->setText("📋 " + assignment["title"].toString());
    m_titleLabel->show();

    QString courseName = assignment["course_name"].toString();
    QDateTime deadline = assignment["deadline"].toDateTime();
    QString status = assignment["status"].toString();
    int maxScore = assignment["max_score"].toInt();

    QString infoText = QString("📚 课程: %1\n⏰ 截止时间: %2\n📊 满分: %3分\n📈 状态: %4")
                           .arg(courseName)
                           .arg(deadline.toString("yyyy年MM月dd日 hh:mm"))
                           .arg(maxScore)
                           .arg(status);

    m_infoLabel->setText(infoText);
    m_infoLabel->show();

    // 显示作业描述
    m_descriptionEdit->setPlainText(assignment["description"].toString());
    m_descriptionEdit->show();

    // 隐藏学生相关组件
    m_submissionGroup->hide();

    // 显示教师统计信息
    if (m_database) {
        try {
            int assignmentId = assignment["assignment_id"].toInt();
            QList<QVariantMap> submissions = m_database->getAssignmentSubmissions(assignmentId);

            int totalSubmissions = submissions.size();
            int gradedSubmissions = 0;

            for (const QVariantMap &submission : submissions) {
                if (submission["status"].toString() == "已批改") {
                    gradedSubmissions++;
                }
            }

            m_submissionStatsLabel->setText(QString("📤 提交人数: %1\n✅ 已批改: %2\n⏳ 待批改: %3")
                                                .arg(totalSubmissions)
                                                .arg(gradedSubmissions)
                                                .arg(totalSubmissions - gradedSubmissions));

            int progress = totalSubmissions > 0 ? (gradedSubmissions * 100 / totalSubmissions) : 0;
            m_gradeProgressBar->setValue(progress);
            m_gradeProgressBar->setFormat(QString("批改进度: %1%").arg(progress));

            m_statsGroup->show();
        } catch (...) {
            qDebug() << "加载教师统计信息失败";
            m_statsGroup->hide();
        }
    } else {
        m_statsGroup->hide();
    }
}

void AssignmentDetailWidget::showStudentView(const QVariantMap &assignment)
{
    // 显示基本信息
    m_titleLabel->setText("📋 " + assignment["title"].toString());
    m_titleLabel->show();

    QString courseName = assignment["course_name"].toString();
    QDateTime deadline = assignment["deadline"].toDateTime();
    QString status = assignment["status"].toString();
    int maxScore = assignment["max_score"].toInt();

    // 计算时间状态
    QString timeStatus;
    QDateTime now = QDateTime::currentDateTime();
    if (now > deadline) {
        timeStatus = "⏰ 已过期";
    } else {
        qint64 secsRemaining = now.secsTo(deadline);
        if (secsRemaining < 86400) {
            timeStatus = QString("⚠️ 剩余 %1 小时").arg(secsRemaining / 3600);
        } else {
            timeStatus = QString("⏰ 剩余 %1 天").arg(secsRemaining / 86400);
        }
    }

    QString infoText = QString("📚 课程: %1\n%2\n📊 满分: %3分\n📈 状态: %4")
                           .arg(courseName)
                           .arg(timeStatus)
                           .arg(maxScore)
                           .arg(status);

    m_infoLabel->setText(infoText);
    m_infoLabel->show();

    // 显示作业描述
    m_descriptionEdit->setPlainText(assignment["description"].toString());
    m_descriptionEdit->show();

    // 隐藏教师相关组件
    m_statsGroup->hide();

    // 显示学生提交信息
    if (assignment["has_submitted"].toBool()) {
        // 显示真实的提交内容（直接从assignment数据中获取）
        QString submissionContent = assignment["submission_content"].toString();
        if (!submissionContent.isEmpty()) {
            m_submissionEdit->setPlainText(submissionContent);
        } else {
            m_submissionEdit->setPlainText("提交内容为空");
        }
        m_submissionEdit->show();

        // 修复批改结果显示问题
        if (assignment["submission_status"].toString() == "已批改") {
            // 修复格式化字符串问题
            double score = assignment["score"].toDouble();
            QString feedback = assignment["feedback"].toString();

            QString gradeText = QString("🎯 得分: %1/%2分\n💬 教师评语: %3")
                                    .arg(QString::number(score, 'f', 1))  // 正确的分数
                                    .arg(maxScore)                        // 正确的满分
                                    .arg(feedback.isEmpty() ? "无评语" : feedback);  // 正确的评语
            m_gradeLabel->setText(gradeText);
        } else {
            m_gradeLabel->setText("⏳ 等待教师批改中...");
        }
        m_gradeLabel->show();
        m_submissionGroup->show();
    } else {
        m_submissionEdit->hide();
        m_gradeLabel->setText("❌ 尚未提交作业");
        m_gradeLabel->show();
        m_submissionGroup->show();
    }
}

void AssignmentDetailWidget::clearContent()
{
    m_titleLabel->hide();
    m_infoLabel->hide();
    m_descriptionEdit->hide();
    m_submissionGroup->hide();
    m_statsGroup->hide();
    m_welcomeWidget->show();
}

void AssignmentDetailWidget::onViewSubmissionsClicked()
{
    if (m_currentAssignment.isEmpty()) return;

    int assignmentId = m_currentAssignment["assignment_id"].toInt();

    // 创建并显示批改对话框
    if (m_database) {
        GradeAssignmentDialog *dialog = new GradeAssignmentDialog(m_database, assignmentId, this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
}

// ============================================================================
// GradeAssignmentDialog - 批改作业对话框实现
// ============================================================================

GradeAssignmentDialog::GradeAssignmentDialog(Database *database, int assignmentId, QWidget *parent)
    : QDialog(parent), m_database(database), m_assignmentId(assignmentId), m_currentStudentId(-1)
{
    // 获取作业信息
    if (m_database) {
        m_assignmentTitle = m_database->getAssignmentTitle(assignmentId);
        m_maxScore = m_database->getAssignmentMaxScore(assignmentId);
    } else {
        m_assignmentTitle = "未知作业";
        m_maxScore = 100;
    }

    setupUI();
    setWindowTitle(QString("批改作业 - %1").arg(m_assignmentTitle));
    setModal(false);
    resize(1000, 700);

    loadSubmissions();
}

void GradeAssignmentDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 顶部信息
    QLabel *titleLabel = new QLabel("📊 作业批改管理");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(m_splitter);

    // 左侧：学生列表
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    QLabel *listLabel = new QLabel("📋 提交列表");
    listLabel->setStyleSheet("font-weight: bold; margin-bottom: 5px;");
    leftLayout->addWidget(listLabel);

    m_studentTable = new QTableWidget();
    m_studentTable->setColumnCount(4);
    m_studentTable->setHorizontalHeaderLabels({"学生", "提交时间", "状态", "得分"});
    m_studentTable->horizontalHeader()->setStretchLastSection(true);
    m_studentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_studentTable->setAlternatingRowColors(true);
    leftLayout->addWidget(m_studentTable);

    // 进度信息
    m_progressLabel = new QLabel("批改进度: 0/0");
    m_progressBar = new QProgressBar();
    leftLayout->addWidget(m_progressLabel);
    leftLayout->addWidget(m_progressBar);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton("🔄 刷新");
    m_batchGradeButton = new QPushButton("📊 批量评分");
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addWidget(m_batchGradeButton);
    buttonLayout->addStretch();
    leftLayout->addLayout(buttonLayout);

    m_splitter->addWidget(leftWidget);

    // 右侧：批改界面
    m_gradeGroup = new QGroupBox("📝 批改详情");
    QVBoxLayout *gradeLayout = new QVBoxLayout(m_gradeGroup);

    m_studentInfoLabel = new QLabel("请选择学生进行批改");
    m_studentInfoLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");
    gradeLayout->addWidget(m_studentInfoLabel);

    gradeLayout->addWidget(new QLabel("📄 提交内容:"));
    m_submissionContentEdit = new QTextEdit();
    m_submissionContentEdit->setReadOnly(true);
    m_submissionContentEdit->setMaximumHeight(200);
    gradeLayout->addWidget(m_submissionContentEdit);

    QHBoxLayout *scoreLayout = new QHBoxLayout();
    scoreLayout->addWidget(new QLabel("📊 得分:"));
    m_scoreSpinBox = new QSpinBox();
    m_scoreSpinBox->setMinimum(0);
    m_scoreSpinBox->setMaximum(100);
    scoreLayout->addWidget(m_scoreSpinBox);
    scoreLayout->addWidget(new QLabel("分"));
    scoreLayout->addStretch();
    gradeLayout->addLayout(scoreLayout);

    gradeLayout->addWidget(new QLabel("💬 评语:"));
    m_feedbackEdit = new QTextEdit();
    m_feedbackEdit->setMaximumHeight(100);
    m_feedbackEdit->setPlaceholderText("请输入评语...");
    gradeLayout->addWidget(m_feedbackEdit);

    m_submitGradeButton = new QPushButton("✅ 提交评分");
    m_submitGradeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #52c41a;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 10px 20px;"
        "    border: none;"
        "    border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #389e0d;"
        "}"
        );
    gradeLayout->addWidget(m_submitGradeButton);

    m_splitter->addWidget(m_gradeGroup);

    // 设置分割器比例
    m_splitter->setSizes({400, 600});

    // 连接信号槽
    connect(m_studentTable, &QTableWidget::itemSelectionChanged, this, &GradeAssignmentDialog::onStudentSelected);
    connect(m_submitGradeButton, &QPushButton::clicked, this, &GradeAssignmentDialog::onGradeSubmitted);
    connect(m_refreshButton, &QPushButton::clicked, this, &GradeAssignmentDialog::onRefreshSubmissions);
    connect(m_batchGradeButton, &QPushButton::clicked, this, &GradeAssignmentDialog::onBatchGrade);
}

void GradeAssignmentDialog::loadSubmissions()
{
    if (!m_database) return;

    try {
        m_submissions = m_database->getAssignmentSubmissions(m_assignmentId);

        m_studentTable->setRowCount(m_submissions.size());

        for (int i = 0; i < m_submissions.size(); ++i) {
            const QVariantMap &submission = m_submissions[i];

            QString studentName = submission["student_name"].toString();
            QDateTime submitTime = submission["submit_time"].toDateTime();
            QString status = submission["status"].toString();
            QString scoreText = submission["score"].isNull() ? "未评分" :
                                    QString::number(submission["score"].toDouble(), 'f', 1);

            m_studentTable->setItem(i, 0, new QTableWidgetItem(studentName));
            m_studentTable->setItem(i, 1, new QTableWidgetItem(submitTime.toString("MM-dd hh:mm")));
            m_studentTable->setItem(i, 2, new QTableWidgetItem(status));
            m_studentTable->setItem(i, 3, new QTableWidgetItem(scoreText));

            // 存储学生ID
            m_studentTable->item(i, 0)->setData(Qt::UserRole, submission["student_id"].toInt());

            // 根据状态设置行颜色
            if (status == "已批改") {
                for (int j = 0; j < 4; ++j) {
                    m_studentTable->item(i, j)->setBackground(QColor(240, 255, 240));
                }
            }
        }

        updateGradeProgress();

    } catch (...) {
        qDebug() << "加载提交列表失败";
    }
}

void GradeAssignmentDialog::onStudentSelected()
{
    QList<QTableWidgetItem*> selectedItems = m_studentTable->selectedItems();
    if (selectedItems.isEmpty()) return;

    int row = selectedItems[0]->row();
    if (row < 0 || row >= m_submissions.size()) return;

    m_currentStudentId = m_studentTable->item(row, 0)->data(Qt::UserRole).toInt();
    loadStudentSubmission(m_currentStudentId);
}

void GradeAssignmentDialog::loadStudentSubmission(int studentId)
{
    if (!m_database) return;

    try {
        // 找到对应的提交记录
        QVariantMap submission;
        for (const QVariantMap &sub : m_submissions) {
            if (sub["student_id"].toInt() == studentId) {
                submission = sub;
                break;
            }
        }

        if (submission.isEmpty()) return;

        QString studentName = submission["student_name"].toString();
        QString grade = submission["grade"].toString();
        m_studentInfoLabel->setText(QString("👤 学生: %1 (%2)").arg(studentName, grade));

        m_submissionContentEdit->setPlainText(submission["content"].toString());

        if (!submission["score"].isNull()) {
            m_scoreSpinBox->setValue(submission["score"].toInt());
        } else {
            m_scoreSpinBox->setValue(0);
        }

        m_feedbackEdit->setPlainText(submission["feedback"].toString());

        // 更新满分设置
        if (m_maxScore > 0) {
            m_scoreSpinBox->setMaximum(m_maxScore);
        }

    } catch (...) {
        qDebug() << "加载学生提交详情失败";
    }
}

void GradeAssignmentDialog::onGradeSubmitted()
{
    if (m_currentStudentId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择学生");
        return;
    }

    try {
        double score = m_scoreSpinBox->value();
        QString feedback = m_feedbackEdit->toPlainText().trimmed();

        if (m_database->gradeAssignment(m_assignmentId, m_currentStudentId, score, feedback)) {
            QMessageBox::information(this, "成功", "评分提交成功！");
            onRefreshSubmissions();
        } else {
            QMessageBox::warning(this, "失败", "评分提交失败，请重试");
        }

    } catch (...) {
        qDebug() << "提交评分时发生异常";
        QMessageBox::critical(this, "错误", "提交评分失败");
    }
}

void GradeAssignmentDialog::onBatchGrade()
{
    bool ok;
    double batchScore = QInputDialog::getDouble(this, "批量评分",
                                                "请输入统一分数:", 85.0, 0.0, 100.0, 1, &ok);

    if (!ok) return;

    QString batchFeedback = QInputDialog::getText(this, "批量评分",
                                                  "请输入统一评语:", QLineEdit::Normal, "完成良好", &ok);

    if (!ok) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认批量评分",
        QString("确定要将所有未评分作业都评为 %.1f 分吗？").arg(batchScore),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    try {
        int count = 0;
        for (const QVariantMap &submission : m_submissions) {
            if (submission["status"].toString() != "已批改") {
                int studentId = submission["student_id"].toInt();
                if (m_database->gradeAssignment(m_assignmentId, studentId, batchScore, batchFeedback)) {
                    count++;
                }
            }
        }

        QMessageBox::information(this, "批量评分完成", QString("成功评分 %1 份作业").arg(count));
        onRefreshSubmissions();

    } catch (...) {
        qDebug() << "批量评分时发生异常";
        QMessageBox::critical(this, "错误", "批量评分失败");
    }
}

void GradeAssignmentDialog::onRefreshSubmissions()
{
    loadSubmissions();
}

void GradeAssignmentDialog::updateGradeProgress()
{
    int total = m_submissions.size();
    int graded = 0;

    for (const QVariantMap &submission : m_submissions) {
        if (submission["status"].toString() == "已批改") {
            graded++;
        }
    }

    m_progressLabel->setText(QString("批改进度: %1/%2").arg(graded).arg(total));

    int progress = total > 0 ? (graded * 100 / total) : 0;
    m_progressBar->setValue(progress);
}

// ============================================================================
// PublishAssignmentDialog - 发布作业对话框实现（保持原有功能）
// ============================================================================

PublishAssignmentDialog::PublishAssignmentDialog(Database *database, int teacherId, QWidget *parent)
    : QDialog(parent)
    , m_database(database)
    , m_teacherId(teacherId)
{
    setupUI();
    setWindowTitle("发布课程作业");
    setModal(true);
    resize(500, 400);
}

void PublishAssignmentDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 基本信息
    QGroupBox *basicGroup = new QGroupBox("基本信息");
    QFormLayout *basicLayout = new QFormLayout(basicGroup);

    m_courseCombo = new QComboBox();
    QList<Course> courses = m_database->getTeacherCourses(m_teacherId);
    for (const Course &course : courses) {
        m_courseCombo->addItem(course.getCourseName(), course.getCourseId());
    }

    m_titleLineEdit = new QLineEdit();
    m_titleLineEdit->setPlaceholderText("请输入作业标题...");

    m_deadlineEdit = new QDateTimeEdit();
    m_deadlineEdit->setDateTime(QDateTime::currentDateTime().addDays(7)); // 默认一周后
    m_deadlineEdit->setMinimumDateTime(QDateTime::currentDateTime());

    m_maxScoreEdit = new QLineEdit("100");

    basicLayout->addRow("选择课程:", m_courseCombo);
    basicLayout->addRow("作业标题:", m_titleLineEdit);
    basicLayout->addRow("截止时间:", m_deadlineEdit);
    basicLayout->addRow("满分:", m_maxScoreEdit);

    mainLayout->addWidget(basicGroup);

    // 作业描述
    QGroupBox *contentGroup = new QGroupBox("作业描述");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentGroup);

    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText("请输入作业要求和描述...");
    m_descriptionEdit->setMinimumHeight(150);

    contentLayout->addWidget(m_descriptionEdit);
    mainLayout->addWidget(contentGroup);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *publishButton = new QPushButton("📝 发布作业");
    QPushButton *cancelButton = new QPushButton("❌ 取消");

    publishButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #52c41a;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 10px 20px;"
        "    border: none;"
        "    border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #389e0d;"
        "}"
        );

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(publishButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(publishButton, &QPushButton::clicked, this, &PublishAssignmentDialog::onPublishClicked);
    connect(cancelButton, &QPushButton::clicked, this, &PublishAssignmentDialog::onCancelClicked);
}

void PublishAssignmentDialog::onPublishClicked()
{
    if (!validateInput()) {
        return;
    }

    int courseId = m_courseCombo->currentData().toInt();
    QString title = m_titleLineEdit->text().trimmed();
    QString description = m_descriptionEdit->toPlainText().trimmed();
    QDateTime deadline = m_deadlineEdit->dateTime();
    int maxScore = m_maxScoreEdit->text().toInt();

    int assignmentId = m_database->publishAssignment(courseId, title, description, deadline, maxScore);

    if (assignmentId > 0) {
        QMessageBox::information(this, "成功", "作业发布成功！");
        accept();
    } else {
        QMessageBox::warning(this, "失败", "作业发布失败，请重试");
    }
}

void PublishAssignmentDialog::onCancelClicked()
{
    reject();
}

bool PublishAssignmentDialog::validateInput()
{
    if (m_courseCombo->currentData().toInt() <= 0) {
        QMessageBox::warning(this, "提示", "请选择课程");
        return false;
    }

    if (m_titleLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入作业标题");
        m_titleLineEdit->setFocus();
        return false;
    }

    if (m_descriptionEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入作业描述");
        m_descriptionEdit->setFocus();
        return false;
    }

    if (m_deadlineEdit->dateTime() <= QDateTime::currentDateTime()) {
        QMessageBox::warning(this, "提示", "截止时间必须晚于当前时间");
        m_deadlineEdit->setFocus();
        return false;
    }

    bool ok;
    int maxScore = m_maxScoreEdit->text().toInt(&ok);
    if (!ok || maxScore <= 0) {
        QMessageBox::warning(this, "提示", "请输入有效的满分数值");
        m_maxScoreEdit->setFocus();
        return false;
    }

    return true;
}

// ============================================================================
// SubmitAssignmentDialog - 提交作业对话框实现（保持原有功能）
// ============================================================================

SubmitAssignmentDialog::SubmitAssignmentDialog(int assignmentId, const QString &assignmentTitle,
                                               Database *database, int studentId, QWidget *parent)
    : QDialog(parent)
    , m_assignmentId(assignmentId)
    , m_database(database)
    , m_studentId(studentId)
    , m_assignmentTitle(assignmentTitle)
{
    setupUI();
    setWindowTitle("提交作业");
    setModal(true);
    resize(500, 400);
}

void SubmitAssignmentDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 作业信息
    QLabel *titleLabel = new QLabel(QString("📋 作业: %1").arg(m_assignmentTitle));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // 提交内容
    QGroupBox *contentGroup = new QGroupBox("📝 提交内容");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentGroup);

    m_contentEdit = new QTextEdit();
    m_contentEdit->setPlaceholderText("请在此输入作业内容...");
    m_contentEdit->setMinimumHeight(250);

    contentLayout->addWidget(m_contentEdit);
    mainLayout->addWidget(contentGroup);

    // 提示信息
    QLabel *tipLabel = new QLabel("💡 提示: 提交后无法修改，请确认内容无误后再提交");
    tipLabel->setStyleSheet("color: #fa8c16; font-size: 12px; margin: 10px 0;");
    mainLayout->addWidget(tipLabel);

    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *submitButton = new QPushButton("📤 提交作业");
    QPushButton *cancelButton = new QPushButton("❌ 取消");

    submitButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #1890ff;"
        "    color: white;"
        "    font-weight: bold;"
        "    padding: 10px 20px;"
        "    border: none;"
        "    border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #096dd9;"
        "}"
        );

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(submitButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(submitButton, &QPushButton::clicked, this, &SubmitAssignmentDialog::onSubmitClicked);
    connect(cancelButton, &QPushButton::clicked, this, &SubmitAssignmentDialog::onCancelClicked);
}

void SubmitAssignmentDialog::onSubmitClicked()
{
    if (!validateInput()) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认提交",
        "确定要提交这份作业吗？提交后无法修改。",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    QString content = m_contentEdit->toPlainText().trimmed();

    if (m_database->submitAssignment(m_assignmentId, m_studentId, content)) {
        QMessageBox::information(this, "成功", "作业提交成功！");
        accept();
    } else {
        QMessageBox::warning(this, "失败", "作业提交失败，请重试");
    }
}

void SubmitAssignmentDialog::onCancelClicked()
{
    reject();
}

bool SubmitAssignmentDialog::validateInput()
{
    if (m_contentEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入作业内容");
        m_contentEdit->setFocus();
        return false;
    }

    return true;
}
