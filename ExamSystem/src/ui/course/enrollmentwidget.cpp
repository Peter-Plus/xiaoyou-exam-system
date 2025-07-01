#include "enrollmentwidget.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

EnrollmentWidget::EnrollmentWidget(Database *database, int userId, UserType userType, QWidget *parent)
    : QWidget(parent), m_database(database), m_userId(userId), m_userType(userType),
    m_selectedCourseId(-1), m_totalCourses(0), m_enrolledCourses(0),
    m_pendingApplications(0), m_totalRequests(0), m_approvedRequests(0)
{
    setupUI();

    // 设置定时器
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &EnrollmentWidget::autoRefresh);
    m_refreshTimer->start(60000); // 60秒自动刷新

    // 初始加载数据
    refreshData();
}

EnrollmentWidget::~EnrollmentWidget()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
    }
}

void EnrollmentWidget::setupUI()
{
    setObjectName("EnrollmentWidget");

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);

    // 根据用户类型设置不同UI
    if (m_userType == STUDENT) {
        setupStudentUI();
    } else {
        setupTeacherUI();
    }

    // 应用样式
    setStyleSheet(R"(
        QWidget#EnrollmentWidget {
            background-color: #f8f9fa;
        }

        QTableWidget {
            gridline-color: #dee2e6;
            background-color: white;
            alternate-background-color: #f8f9fa;
            selection-background-color: #3498db;
            border: 1px solid #dee2e6;
            border-radius: 4px;
        }

        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #dee2e6;
        }

        QTableWidget::item:selected {
            background-color: #3498db;
            color: white;
        }

        QHeaderView::section {
            background-color: #6c757d;
            color: white;
            padding: 10px;
            border: none;
            font-weight: bold;
        }

        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-size: 14px;
        }

        QPushButton:hover {
            background-color: #0056b3;
        }

        QPushButton:pressed {
            background-color: #004085;
        }

        QPushButton:disabled {
            background-color: #6c757d;
        }

        QPushButton#ApproveButton {
            background-color: #28a745;
        }

        QPushButton#ApproveButton:hover {
            background-color: #1e7e34;
        }

        QPushButton#RejectButton {
            background-color: #dc3545;
        }

        QPushButton#RejectButton:hover {
            background-color: #bd2130;
        }

        QLineEdit, QComboBox {
            padding: 8px;
            border: 1px solid #ced4da;
            border-radius: 4px;
            background-color: white;
        }

        QLineEdit:focus, QComboBox:focus {
            border-color: #3498db;
            outline: none;
        }

        QGroupBox {
            font-weight: bold;
            border: 2px solid #dee2e6;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }

        QTextEdit {
            border: 1px solid #dee2e6;
            border-radius: 4px;
            background-color: white;
            padding: 8px;
        }
    )");
}

void EnrollmentWidget::setupStudentUI()
{
    m_studentWidget = new QWidget();
    m_studentLayout = new QVBoxLayout(m_studentWidget);

    // 搜索和筛选区域
    m_searchLayout = new QHBoxLayout();

    QLabel *searchLabel = new QLabel("搜索课程:");
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("输入课程名称或教师姓名...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &EnrollmentWidget::onSearchTextChanged);

    QLabel *collegeLabel = new QLabel("学院筛选:");
    m_collegeFilter = new QComboBox();
    m_collegeFilter->addItem("全部学院");
    m_collegeFilter->addItem("计算机学院");
    m_collegeFilter->addItem("数学学院");
    m_collegeFilter->addItem("软件学院");
    connect(m_collegeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnrollmentWidget::onSearchTextChanged);

    m_searchLayout->addWidget(searchLabel);
    m_searchLayout->addWidget(m_searchEdit, 2);
    m_searchLayout->addWidget(collegeLabel);
    m_searchLayout->addWidget(m_collegeFilter, 1);
    m_searchLayout->addStretch();

    // 可选课程表格
    m_courseTable = new QTableWidget();
    m_courseTable->setColumnCount(7);
    QStringList courseHeaders;
    courseHeaders << "课程名称" << "教师" << "学院" << "学分" << "课时" << "已选/最大" << "状态";
    m_courseTable->setHorizontalHeaderLabels(courseHeaders);
    m_courseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_courseTable->setAlternatingRowColors(true);
    m_courseTable->horizontalHeader()->setStretchLastSection(true);
    connect(m_courseTable, &QTableWidget::itemSelectionChanged,
            this, &EnrollmentWidget::onCourseSelectionChanged);

    // 课程详情组
    m_courseDetailGroup = new QGroupBox("📋 课程详情");
    m_courseDetailLabel = new QLabel("请选择课程查看详情");
    m_courseDetailLabel->setWordWrap(true);
    m_courseDetailLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QVBoxLayout *detailLayout = new QVBoxLayout(m_courseDetailGroup);
    detailLayout->addWidget(m_courseDetailLabel);

    m_submitButton = new QPushButton("📝 提交选课申请");
    m_submitButton->setEnabled(false);
    connect(m_submitButton, &QPushButton::clicked, this, &EnrollmentWidget::onSubmitApplication);
    detailLayout->addWidget(m_submitButton);

    // 我的申请状态
    m_applicationGroup = new QGroupBox("📊 我的申请状态");
    m_applicationTable = new QTableWidget();
    m_applicationTable->setColumnCount(5);
    QStringList appHeaders;
    appHeaders << "课程名称" << "教师" << "申请时间" << "状态" << "备注";
    m_applicationTable->setHorizontalHeaderLabels(appHeaders);
    m_applicationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_applicationTable->setAlternatingRowColors(true);
    m_applicationTable->horizontalHeader()->setStretchLastSection(true);

    QVBoxLayout *appLayout = new QVBoxLayout(m_applicationGroup);
    appLayout->addWidget(m_applicationTable);

    // 统计信息和刷新按钮
    m_statsGroup = new QGroupBox("📈 统计信息");
    m_statsLabel = new QLabel();
    m_refreshButton = new QPushButton("🔄 刷新数据");
    connect(m_refreshButton, &QPushButton::clicked, this, &EnrollmentWidget::refreshData);

    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    statsLayout->addWidget(m_statsLabel);
    statsLayout->addWidget(m_refreshButton);

    // 布局组装
    m_splitter = new QSplitter(Qt::Horizontal);

    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->addLayout(m_searchLayout);
    leftLayout->addWidget(m_courseTable, 3);
    leftLayout->addWidget(m_applicationGroup, 2);

    QWidget *rightWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->addWidget(m_courseDetailGroup, 2);
    rightLayout->addWidget(m_statsGroup, 1);

    m_splitter->addWidget(leftWidget);
    m_splitter->addWidget(rightWidget);
    m_splitter->setSizes({600, 300});

    m_studentLayout->addWidget(m_splitter);
    m_mainLayout->addWidget(m_studentWidget);
}

void EnrollmentWidget::setupTeacherUI()
{
    m_teacherWidget = new QWidget();
    m_teacherLayout = new QVBoxLayout(m_teacherWidget);

    // 筛选区域
    m_filterLayout = new QHBoxLayout();

    QLabel *statusLabel = new QLabel("状态筛选:");
    m_statusFilter = new QComboBox();
    m_statusFilter->addItem("全部申请");
    m_statusFilter->addItem("申请中");
    m_statusFilter->addItem("已通过");
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnrollmentWidget::refreshData);

    QLabel *courseLabel = new QLabel("课程筛选:");
    m_courseFilter = new QComboBox();
    m_courseFilter->addItem("全部课程");
    connect(m_courseFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnrollmentWidget::refreshData);

    m_filterLayout->addWidget(statusLabel);
    m_filterLayout->addWidget(m_statusFilter, 1);
    m_filterLayout->addWidget(courseLabel);
    m_filterLayout->addWidget(m_courseFilter, 2);
    m_filterLayout->addStretch();

    // 申请列表表格
    m_requestTable = new QTableWidget();
    m_requestTable->setColumnCount(6);
    QStringList requestHeaders;
    requestHeaders << "学生姓名" << "学生年级" << "课程名称" << "申请时间" << "状态" << "操作";
    m_requestTable->setHorizontalHeaderLabels(requestHeaders);
    m_requestTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_requestTable->setAlternatingRowColors(true);
    m_requestTable->horizontalHeader()->setStretchLastSection(true);
    connect(m_requestTable, &QTableWidget::itemSelectionChanged,
            this, &EnrollmentWidget::onRequestSelectionChanged);

    // 申请详情组
    m_requestDetailGroup = new QGroupBox("📋 申请详情");
    m_requestDetailText = new QTextEdit();
    m_requestDetailText->setMaximumHeight(150);
    m_requestDetailText->setReadOnly(true);

    // 操作按钮
    m_actionLayout = new QHBoxLayout();
    m_approveButton = new QPushButton("✅ 批准申请");
    m_approveButton->setObjectName("ApproveButton");
    m_approveButton->setEnabled(false);
    connect(m_approveButton, &QPushButton::clicked, this, &EnrollmentWidget::onApproveRequest);

    m_rejectButton = new QPushButton("❌ 拒绝申请");
    m_rejectButton->setObjectName("RejectButton");
    m_rejectButton->setEnabled(false);
    connect(m_rejectButton, &QPushButton::clicked, this, &EnrollmentWidget::onRejectRequest);

    m_batchApproveButton = new QPushButton("✅ 批量批准");
    m_batchApproveButton->setObjectName("ApproveButton");
    connect(m_batchApproveButton, &QPushButton::clicked, this, &EnrollmentWidget::onBatchApprove);

    m_batchRejectButton = new QPushButton("❌ 批量拒绝");
    m_batchRejectButton->setObjectName("RejectButton");
    connect(m_batchRejectButton, &QPushButton::clicked, this, &EnrollmentWidget::onBatchReject);

    m_actionLayout->addWidget(m_approveButton);
    m_actionLayout->addWidget(m_rejectButton);
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_batchApproveButton);
    m_actionLayout->addWidget(m_batchRejectButton);

    QVBoxLayout *detailLayout = new QVBoxLayout(m_requestDetailGroup);
    detailLayout->addWidget(m_requestDetailText);
    detailLayout->addLayout(m_actionLayout);

    // 统计信息
    m_statsGroup = new QGroupBox("📈 审核统计");
    m_statsLabel = new QLabel();
    m_refreshButton = new QPushButton("🔄 刷新数据");
    connect(m_refreshButton, &QPushButton::clicked, this, &EnrollmentWidget::refreshData);

    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    statsLayout->addWidget(m_statsLabel);
    statsLayout->addWidget(m_refreshButton);

    // 布局组装
    m_splitter = new QSplitter(Qt::Horizontal);

    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->addLayout(m_filterLayout);
    leftLayout->addWidget(m_requestTable, 3);

    QWidget *rightWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->addWidget(m_requestDetailGroup, 2);
    rightLayout->addWidget(m_statsGroup, 1);

    m_splitter->addWidget(leftWidget);
    m_splitter->addWidget(rightWidget);
    m_splitter->setSizes({700, 300});

    m_teacherLayout->addWidget(m_splitter);
    m_mainLayout->addWidget(m_teacherWidget);
}

void EnrollmentWidget::refreshData()
{
    if (!m_database) {
        qDebug() << "数据库未连接";
        return;
    }

    if (m_userType == STUDENT) {
        loadAvailableCourses();
        loadMyApplications();
    } else {
        loadEnrollmentRequests();
        // 加载课程筛选选项
        QList<QVariantMap> courses = m_database->getCoursesByTeacher(m_userId);
        m_courseFilter->clear();
        m_courseFilter->addItem("全部课程");
        for (const auto &course : courses) {
            m_courseFilter->addItem(course["course_name"].toString(), course["course_id"].toInt());
        }
    }

    updateStatistics();
    qDebug() << "选课管理数据刷新完成";
}

void EnrollmentWidget::loadAvailableCourses()
{
    m_availableCourses = m_database->getAvailableCourses(m_userId);
    updateCourseTable();
}

void EnrollmentWidget::loadEnrollmentRequests()
{
    // 检查是否为选课管理员
    if (!m_database->isTeacherCourseAdmin(m_userId)) {
        QMessageBox::warning(this, "权限不足", "只有选课管理员才能审核选课申请！");
        return;
    }

    QList<QVariantMap> requestData = m_database->getEnrollmentRequests(m_userId);
    m_enrollmentRequests.clear();

    for (const auto &data : requestData) {
        EnrollmentRequest request(
            data["student_id"].toInt(),
            data["course_id"].toInt(),
            data["enrollment_time"].toDateTime(),
            EnrollmentRequest::statusFromString(data["enrollment_status"].toString()),
            data["student_name"].toString(),
            data["student_grade"].toString(),
            data["student_college"].toString(),
            data["course_name"].toString(),
            data["course_college"].toString(),
            data["teacher_name"].toString()
            );
        m_enrollmentRequests.append(request);
    }

    updateRequestTable();
}

void EnrollmentWidget::loadMyApplications()
{
    m_myApplications = m_database->getCoursesByStudent(m_userId, true);
    updateApplicationTable();
}

void EnrollmentWidget::updateCourseTable()
{
    m_courseTable->setRowCount(m_availableCourses.size());

    for (int i = 0; i < m_availableCourses.size(); ++i) {
        const QVariantMap &course = m_availableCourses[i];

        m_courseTable->setItem(i, 0, new QTableWidgetItem(course["course_name"].toString()));
        m_courseTable->setItem(i, 1, new QTableWidgetItem(course["teacher_name"].toString()));
        m_courseTable->setItem(i, 2, new QTableWidgetItem(course["college"].toString()));
        m_courseTable->setItem(i, 3, new QTableWidgetItem(course["credits"].toString()));
        m_courseTable->setItem(i, 4, new QTableWidgetItem(course["course_hours"].toString()));

        QString capacityText = QString("%1/%2")
                                   .arg(course["enrolled_count"].toInt())
                                   .arg(course["max_students"].toInt());
        m_courseTable->setItem(i, 5, new QTableWidgetItem(capacityText));
        m_courseTable->setItem(i, 6, new QTableWidgetItem("可申请"));

        // 存储课程ID
        m_courseTable->item(i, 0)->setData(Qt::UserRole, course["course_id"]);
    }

    m_courseTable->resizeColumnsToContents();
}

void EnrollmentWidget::updateRequestTable()
{
    m_requestTable->setRowCount(m_enrollmentRequests.size());

    for (int i = 0; i < m_enrollmentRequests.size(); ++i) {
        const EnrollmentRequest &request = m_enrollmentRequests[i];

        m_requestTable->setItem(i, 0, new QTableWidgetItem(request.getStudentName()));
        m_requestTable->setItem(i, 1, new QTableWidgetItem(request.getStudentGrade()));
        m_requestTable->setItem(i, 2, new QTableWidgetItem(request.getCourseName()));
        m_requestTable->setItem(i, 3, new QTableWidgetItem(request.getEnrollmentTimeText()));

        QTableWidgetItem *statusItem = new QTableWidgetItem(request.getStatusText());
        statusItem->setForeground(QColor(request.getStatusColor()));
        m_requestTable->setItem(i, 4, statusItem);

        QString actionText = request.isPending() ? "待处理" : "已处理";
        m_requestTable->setItem(i, 5, new QTableWidgetItem(actionText));
    }

    m_requestTable->resizeColumnsToContents();
}

void EnrollmentWidget::updateApplicationTable()
{
    // 只显示申请记录
    QList<QVariantMap> applications;
    for (const auto &course : m_myApplications) {
        if (course["enrollment_status"].toString() != "已通过") {
            applications.append(course);
        }
    }

    m_applicationTable->setRowCount(applications.size());

    for (int i = 0; i < applications.size(); ++i) {
        const QVariantMap &app = applications[i];

        m_applicationTable->setItem(i, 0, new QTableWidgetItem(app["course_name"].toString()));
        m_applicationTable->setItem(i, 1, new QTableWidgetItem(app["teacher_name"].toString()));
        m_applicationTable->setItem(i, 2, new QTableWidgetItem(
                                              app["enrollment_time"].toDateTime().toString("yyyy-MM-dd hh:mm")));

        QString status = app["enrollment_status"].toString();
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        if (status == "申请中") {
            statusItem->setForeground(QColor("#f39c12"));
        } else {
            statusItem->setForeground(QColor("#27ae60"));
        }
        m_applicationTable->setItem(i, 3, statusItem);
        m_applicationTable->setItem(i, 4, new QTableWidgetItem("等待审核"));
    }

    m_applicationTable->resizeColumnsToContents();
}

void EnrollmentWidget::updateStatistics()
{
    QString statsText;

    if (m_userType == STUDENT) {
        m_totalCourses = m_availableCourses.size();
        m_enrolledCourses = 0;
        m_pendingApplications = 0;

        for (const auto &course : m_myApplications) {
            if (course["enrollment_status"].toString() == "已通过") {
                m_enrolledCourses++;
            } else if (course["enrollment_status"].toString() == "申请中") {
                m_pendingApplications++;
            }
        }

        statsText = QString("可选课程: %1门\n已选课程: %2门\n申请中: %3门")
                        .arg(m_totalCourses)
                        .arg(m_enrolledCourses)
                        .arg(m_pendingApplications);
    } else {
        m_totalRequests = m_enrollmentRequests.size();
        m_approvedRequests = 0;

        for (const auto &request : m_enrollmentRequests) {
            if (request.isApproved()) {
                m_approvedRequests++;
            }
        }

        int pendingRequests = m_totalRequests - m_approvedRequests;

        statsText = QString("总申请数: %1条\n待处理: %2条\n已处理: %3条")
                        .arg(m_totalRequests)
                        .arg(pendingRequests)
                        .arg(m_approvedRequests);
    }

    m_statsLabel->setText(statsText);
}

void EnrollmentWidget::onCourseSelectionChanged()
{
    int row = m_courseTable->currentRow();
    if (row < 0 || row >= m_availableCourses.size()) {
        m_selectedCourseId = -1;
        m_submitButton->setEnabled(false);
        m_courseDetailLabel->setText("请选择课程查看详情");
        return;
    }

    m_selectedCourseId = m_courseTable->item(row, 0)->data(Qt::UserRole).toInt();
    showCourseDetails(m_selectedCourseId);
    m_submitButton->setEnabled(true);
}

void EnrollmentWidget::onRequestSelectionChanged()
{
    int row = m_requestTable->currentRow();
    if (row < 0 || row >= m_enrollmentRequests.size()) {
        m_approveButton->setEnabled(false);
        m_rejectButton->setEnabled(false);
        m_requestDetailText->clear();
        return;
    }

    m_selectedRequest = m_enrollmentRequests[row];
    showRequestDetails(m_selectedRequest);

    bool canProcess = m_selectedRequest.isPending();
    m_approveButton->setEnabled(canProcess);
    m_rejectButton->setEnabled(canProcess);
}

void EnrollmentWidget::showCourseDetails(int courseId)
{
    for (const auto &course : m_availableCourses) {
        if (course["course_id"].toInt() == courseId) {
            QString details = QString(
                                  "课程名称: %1\n"
                                  "任课教师: %2\n"
                                  "开课学院: %3\n"
                                  "学分: %4\n"
                                  "课时: %5\n"
                                  "开课学期: %6\n"
                                  "课程描述: %7\n"
                                  "选课情况: %8/%9人\n"
                                  "课程状态: %10"
                                  ).arg(course["course_name"].toString())
                                  .arg(course["teacher_name"].toString())
                                  .arg(course["college"].toString())
                                  .arg(course["credits"].toInt())
                                  .arg(course["course_hours"].toInt())
                                  .arg(course["semester"].toString())
                                  .arg(course["description"].toString())
                                  .arg(course["enrolled_count"].toInt())
                                  .arg(course["max_students"].toInt())
                                  .arg("开放选课");

            m_courseDetailLabel->setText(details);
            break;
        }
    }
}

void EnrollmentWidget::showRequestDetails(const EnrollmentRequest &request)
{
    QString details = QString(
                          "申请详情:\n\n"
                          "学生信息:\n"
                          "姓名: %1\n"
                          "年级: %2\n"
                          "学院: %3\n\n"
                          "课程信息:\n"
                          "课程名称: %4\n"
                          "开课学院: %5\n"
                          "任课教师: %6\n\n"
                          "申请信息:\n"
                          "申请时间: %7\n"
                          "当前状态: %8\n"
                          "相对时间: %9"
                          ).arg(request.getStudentName())
                          .arg(request.getStudentGrade())
                          .arg(request.getStudentCollege())
                          .arg(request.getCourseName())
                          .arg(request.getCourseCollege())
                          .arg(request.getTeacherName())
                          .arg(request.getEnrollmentTimeText())
                          .arg(request.getStatusText())
                          .arg(request.getRelativeTimeText());

    m_requestDetailText->setText(details);
}

void EnrollmentWidget::onSubmitApplication()
{
    if (m_selectedCourseId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要申请的课程！");
        return;
    }

    if (!validateEnrollment(m_selectedCourseId)) {
        return;
    }

    // 提交选课申请
    if (m_database->submitEnrollmentRequest(m_userId, m_selectedCourseId)) {
        QMessageBox::information(this, "成功", "选课申请提交成功！请等待审核。");
        emit enrollmentSubmitted(m_selectedCourseId);
        refreshData();
    } else {
        QMessageBox::critical(this, "失败", "选课申请提交失败！请检查是否已申请该课程或课程已满员。");
    }
}

void EnrollmentWidget::onApproveRequest()
{
    if (!m_selectedRequest.isValid() || !m_selectedRequest.isPending()) {
        QMessageBox::warning(this, "提示", "请选择有效的待处理申请！");
        return;
    }

    int ret = QMessageBox::question(this, "确认操作",
                                    QString("确定要批准 %1 的选课申请吗？\n课程：%2")
                                        .arg(m_selectedRequest.getStudentName())
                                        .arg(m_selectedRequest.getCourseName()),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->processEnrollmentRequest(m_selectedRequest.getStudentId(),
                                                 m_selectedRequest.getCourseId(),
                                                 true, m_userId)) {
            QMessageBox::information(this, "成功", "申请已批准！");
            emit requestProcessed(m_selectedRequest.getStudentId(), m_selectedRequest.getCourseId(), true);
            refreshData();
        } else {
            QMessageBox::critical(this, "失败", "批准申请失败！请检查课程是否已满员。");
        }
    }
}

void EnrollmentWidget::onRejectRequest()
{
    if (!m_selectedRequest.isValid() || !m_selectedRequest.isPending()) {
        QMessageBox::warning(this, "提示", "请选择有效的待处理申请！");
        return;
    }

    int ret = QMessageBox::question(this, "确认操作",
                                    QString("确定要拒绝 %1 的选课申请吗？\n课程：%2")
                                        .arg(m_selectedRequest.getStudentName())
                                        .arg(m_selectedRequest.getCourseName()),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->processEnrollmentRequest(m_selectedRequest.getStudentId(),
                                                 m_selectedRequest.getCourseId(),
                                                 false, m_userId)) {
            QMessageBox::information(this, "成功", "申请已拒绝！");
            emit requestProcessed(m_selectedRequest.getStudentId(), m_selectedRequest.getCourseId(), false);
            refreshData();
        } else {
            QMessageBox::critical(this, "失败", "拒绝申请失败！");
        }
    }
}

void EnrollmentWidget::onBatchApprove()
{
    processSelectedRequests(true);
}

void EnrollmentWidget::onBatchReject()
{
    processSelectedRequests(false);
}

void EnrollmentWidget::processSelectedRequests(bool approve)
{
    QList<QTableWidgetItem*> selectedItems = m_requestTable->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择要处理的申请！");
        return;
    }

    // 获取选中的行
    QSet<int> selectedRows;
    for (QTableWidgetItem* item : selectedItems) {
        selectedRows.insert(item->row());
    }

    // 检查是否有待处理的申请
    QList<EnrollmentRequest> pendingRequests;
    for (int row : selectedRows) {
        if (row < m_enrollmentRequests.size() && m_enrollmentRequests[row].isPending()) {
            pendingRequests.append(m_enrollmentRequests[row]);
        }
    }

    if (pendingRequests.isEmpty()) {
        QMessageBox::warning(this, "提示", "所选申请中没有待处理的记录！");
        return;
    }

    QString action = approve ? "批准" : "拒绝";
    int ret = QMessageBox::question(this, "确认操作",
                                    QString("确定要%1选中的 %2 条申请吗？")
                                        .arg(action).arg(pendingRequests.size()),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        int successCount = 0;
        for (const EnrollmentRequest &request : pendingRequests) {
            if (m_database->processEnrollmentRequest(request.getStudentId(),
                                                     request.getCourseId(),
                                                     approve, m_userId)) {
                successCount++;
                emit requestProcessed(request.getStudentId(), request.getCourseId(), approve);
            }
        }

        QMessageBox::information(this, "完成",
                                 QString("成功%1了 %2/%3 条申请！")
                                     .arg(action).arg(successCount).arg(pendingRequests.size()));
        refreshData();
    }
}

bool EnrollmentWidget::validateEnrollment(int courseId)
{
    // 检查是否已选择或申请该课程
    int status = m_database->getEnrollmentStatus(m_userId, courseId);
    if (status == 1) {
        QMessageBox::warning(this, "提示", "您已经选择了该课程！");
        return false;
    } else if (status == 2) {
        QMessageBox::warning(this, "提示", "您已经申请了该课程，请等待审核！");
        return false;
    }

    // 检查课程是否还有名额
    for (const auto &course : m_availableCourses) {
        if (course["course_id"].toInt() == courseId) {
            int enrolled = course["enrolled_count"].toInt();
            int maxStudents = course["max_students"].toInt();
            if (enrolled >= maxStudents) {
                QMessageBox::warning(this, "提示", "该课程已满员，无法申请！");
                return false;
            }
            break;
        }
    }

    return true;
}

void EnrollmentWidget::onSearchTextChanged()
{
    // 简单的筛选实现
    QString searchText = m_searchEdit->text().toLower();
    QString selectedCollege = m_collegeFilter->currentText();

    for (int i = 0; i < m_courseTable->rowCount(); ++i) {
        bool visible = true;

        // 搜索文本筛选
        if (!searchText.isEmpty()) {
            QString courseName = m_courseTable->item(i, 0)->text().toLower();
            QString teacherName = m_courseTable->item(i, 1)->text().toLower();
            visible = courseName.contains(searchText) || teacherName.contains(searchText);
        }

        // 学院筛选
        if (visible && selectedCollege != "全部学院") {
            QString courseCollege = m_courseTable->item(i, 2)->text();
            visible = (courseCollege == selectedCollege);
        }

        m_courseTable->setRowHidden(i, !visible);
    }
}

void EnrollmentWidget::autoRefresh()
{
    // 静默刷新统计信息
    updateStatistics();
}
