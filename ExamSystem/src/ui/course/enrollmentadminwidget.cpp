#include "enrollmentadminwidget.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QCheckBox>

EnrollmentAdminWidget::EnrollmentAdminWidget(Database *database, int adminId, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_adminId(adminId)
{
    setupUI();
    setupStyles();
    refreshData();
}

void EnrollmentAdminWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);

    // 筛选区域
    m_filterGroup = new QGroupBox("筛选条件");
    m_filterLayout = new QHBoxLayout(m_filterGroup);

    m_collegeFilterCombo = new QComboBox();
    m_collegeFilterCombo->addItems({"全部学院", "计算机学院", "数学学院", "软件学院"});

    m_statusFilterCombo = new QComboBox();
    m_statusFilterCombo->addItems({"全部状态", "申请中"});

    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("搜索学生姓名或课程名称...");

    m_searchButton = new QPushButton("🔍 搜索");
    m_refreshButton = new QPushButton("🔄 刷新");

    m_filterLayout->addWidget(new QLabel("学院:"));
    m_filterLayout->addWidget(m_collegeFilterCombo);
    m_filterLayout->addWidget(new QLabel("状态:"));
    m_filterLayout->addWidget(m_statusFilterCombo);
    m_filterLayout->addWidget(m_searchLineEdit);
    m_filterLayout->addWidget(m_searchButton);
    m_filterLayout->addWidget(m_refreshButton);

    m_mainLayout->addWidget(m_filterGroup);

    // 统计信息
    m_statsGroup = new QGroupBox("统计信息");
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    m_statsLabel = new QLabel();
    statsLayout->addWidget(m_statsLabel);
    m_mainLayout->addWidget(m_statsGroup);

    // 申请列表表格
    m_tableGroup = new QGroupBox("选课申请列表");
    QVBoxLayout *tableLayout = new QVBoxLayout(m_tableGroup);

    m_enrollmentTable = new QTableWidget();
    m_enrollmentTable->setColumnCount(7);
    QStringList headers = {"选择", "学生姓名", "年级", "学院", "课程名称", "教师", "申请时间"};
    m_enrollmentTable->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    m_enrollmentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_enrollmentTable->setAlternatingRowColors(true);
    m_enrollmentTable->horizontalHeader()->setStretchLastSection(true);
    m_enrollmentTable->verticalHeader()->setVisible(false);

    // 设置列宽
    m_enrollmentTable->setColumnWidth(0, 60);   // 选择
    m_enrollmentTable->setColumnWidth(1, 100);  // 学生姓名
    m_enrollmentTable->setColumnWidth(2, 80);   // 年级
    m_enrollmentTable->setColumnWidth(3, 120);  // 学院
    m_enrollmentTable->setColumnWidth(4, 150);  // 课程名称
    m_enrollmentTable->setColumnWidth(5, 100);  // 教师

    tableLayout->addWidget(m_enrollmentTable);
    m_mainLayout->addWidget(m_tableGroup);

    // 操作区域
    m_actionGroup = new QGroupBox("操作");
    m_actionLayout = new QHBoxLayout(m_actionGroup);

    m_selectAllCheckBox = new QCheckBox("全选");
    m_selectedCountLabel = new QLabel("已选择 0 项");

    m_approveButton = new QPushButton("✅ 通过选中");
    m_rejectButton = new QPushButton("❌ 拒绝选中");
    m_batchApproveButton = new QPushButton("🎯 批量通过");
    m_batchRejectButton = new QPushButton("🚫 批量拒绝");

    m_approveButton->setEnabled(false);
    m_rejectButton->setEnabled(false);

    m_actionLayout->addWidget(m_selectAllCheckBox);
    m_actionLayout->addWidget(m_selectedCountLabel);
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_approveButton);
    m_actionLayout->addWidget(m_rejectButton);
    m_actionLayout->addWidget(m_batchApproveButton);
    m_actionLayout->addWidget(m_batchRejectButton);

    m_mainLayout->addWidget(m_actionGroup);

    // 连接信号槽
    connect(m_searchButton, &QPushButton::clicked, this, &EnrollmentAdminWidget::onFilterChanged);
    connect(m_refreshButton, &QPushButton::clicked, this, &EnrollmentAdminWidget::onRefreshClicked);
    connect(m_collegeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnrollmentAdminWidget::onFilterChanged);
    connect(m_statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EnrollmentAdminWidget::onFilterChanged);

    connect(m_enrollmentTable, &QTableWidget::itemSelectionChanged,
            this, &EnrollmentAdminWidget::onTableSelectionChanged);
    connect(m_enrollmentTable, &QTableWidget::itemChanged,
            this, &EnrollmentAdminWidget::onTableItemChanged);

    connect(m_selectAllCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        selectAllItems(checked);
    });

    connect(m_approveButton, &QPushButton::clicked, this, &EnrollmentAdminWidget::onApproveEnrollment);
    connect(m_rejectButton, &QPushButton::clicked, this, &EnrollmentAdminWidget::onRejectEnrollment);
    connect(m_batchApproveButton, &QPushButton::clicked, this, &EnrollmentAdminWidget::onBatchApprove);
    connect(m_batchRejectButton, &QPushButton::clicked, this, &EnrollmentAdminWidget::onBatchReject);
}

void EnrollmentAdminWidget::setupStyles()
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
        "QTableWidget {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    gridline-color: #eee;"
        "}"
        "QTableWidget::item {"
        "    padding: 8px;"
        "    border: none;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #e6f7ff;"
        "}"
        "QHeaderView::section {"
        "    background-color: #f5f5f5;"
        "    border: 1px solid #ddd;"
        "    padding: 8px;"
        "    font-weight: bold;"
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
        "}"
        );
}

void EnrollmentAdminWidget::refreshData()
{
    updateEnrollmentTable();
    updateStatistics();
}

void EnrollmentAdminWidget::updateEnrollmentTable()
{
    if (!m_database) return;

    m_pendingEnrollments = m_database->getPendingEnrollments();

    // 应用筛选条件
    QString collegeFilter = m_collegeFilterCombo->currentText();
    QString searchText = m_searchLineEdit->text().trimmed();

    QList<QVariantMap> filteredEnrollments;
    for (const QVariantMap &enrollment : m_pendingEnrollments) {
        bool matches = true;

        // 学院筛选
        if (collegeFilter != "全部学院") {
            if (enrollment["student_college"].toString() != collegeFilter) {
                matches = false;
            }
        }

        // 关键词搜索
        if (!searchText.isEmpty() && matches) {
            QString studentName = enrollment["student_name"].toString();
            QString courseName = enrollment["course_name"].toString();

            if (!studentName.contains(searchText, Qt::CaseInsensitive) &&
                !courseName.contains(searchText, Qt::CaseInsensitive)) {
                matches = false;
            }
        }

        if (matches) {
            filteredEnrollments.append(enrollment);
        }
    }

    // 更新表格
    m_enrollmentTable->setRowCount(filteredEnrollments.size());

    for (int i = 0; i < filteredEnrollments.size(); ++i) {
        createTableRow(filteredEnrollments[i], i);
    }

    // 更新统计
    m_tableGroup->setTitle(QString("选课申请列表 (共%1条)").arg(filteredEnrollments.size()));
}

void EnrollmentAdminWidget::createTableRow(const QVariantMap &enrollment, int row)
{
    // 选择复选框
    QCheckBox *checkBox = new QCheckBox();
    checkBox->setProperty("studentId", enrollment["student_id"].toInt());
    checkBox->setProperty("courseId", enrollment["course_id"].toInt());
    m_enrollmentTable->setCellWidget(row, 0, checkBox);

    connect(checkBox, &QCheckBox::toggled, this, &EnrollmentAdminWidget::onTableSelectionChanged);

    // 学生信息
    m_enrollmentTable->setItem(row, 1, new QTableWidgetItem(enrollment["student_name"].toString()));
    m_enrollmentTable->setItem(row, 2, new QTableWidgetItem(enrollment["grade"].toString()));
    m_enrollmentTable->setItem(row, 3, new QTableWidgetItem(enrollment["student_college"].toString()));

    // 课程信息
    m_enrollmentTable->setItem(row, 4, new QTableWidgetItem(enrollment["course_name"].toString()));
    m_enrollmentTable->setItem(row, 5, new QTableWidgetItem(enrollment["teacher_name"].toString()));

    // 申请时间
    QDateTime enrollmentTime = enrollment["enrollment_time"].toDateTime();
    m_enrollmentTable->setItem(row, 6, new QTableWidgetItem(enrollmentTime.toString("yyyy-MM-dd hh:mm")));

    // 设置行数据
    m_enrollmentTable->item(row, 1)->setData(Qt::UserRole, enrollment["student_id"].toInt());
    m_enrollmentTable->item(row, 4)->setData(Qt::UserRole, enrollment["course_id"].toInt());
}

void EnrollmentAdminWidget::updateStatistics()
{
    if (!m_database) return;

    QList<QVariantMap> allPending = m_database->getPendingEnrollments();

    // 按学院统计
    QMap<QString, int> collegeStats;
    for (const QVariantMap &enrollment : allPending) {
        QString college = enrollment["student_college"].toString();
        collegeStats[college]++;
    }

    QString statsText = QString("待审核申请总数: %1条\n").arg(allPending.size());
    for (auto it = collegeStats.begin(); it != collegeStats.end(); ++it) {
        statsText += QString("%1: %2条\n").arg(it.key()).arg(it.value());
    }

    m_statsLabel->setText(statsText.trimmed());
}

void EnrollmentAdminWidget::onApproveEnrollment()
{
    QList<int> selectedRows = getSelectedRows();
    if (selectedRows.isEmpty()) {
        showMessage("请先选择要通过的申请", true);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认操作",
        QString("确定要通过选中的 %1 条申请吗？").arg(selectedRows.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    int successCount = 0;
    for (int row : selectedRows) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_enrollmentTable->cellWidget(row, 0));
        if (checkBox && checkBox->isChecked()) {
            int studentId = checkBox->property("studentId").toInt();
            int courseId = checkBox->property("courseId").toInt();

            if (m_database->approveEnrollment(studentId, courseId)) {
                successCount++;
                emit enrollmentProcessed(studentId, courseId, true);
            }
        }
    }

    showMessage(QString("成功通过 %1 条申请").arg(successCount));
    refreshData();
}

void EnrollmentAdminWidget::onRejectEnrollment()
{
    QList<int> selectedRows = getSelectedRows();
    if (selectedRows.isEmpty()) {
        showMessage("请先选择要拒绝的申请", true);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认操作",
        QString("确定要拒绝选中的 %1 条申请吗？").arg(selectedRows.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    int successCount = 0;
    for (int row : selectedRows) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_enrollmentTable->cellWidget(row, 0));
        if (checkBox && checkBox->isChecked()) {
            int studentId = checkBox->property("studentId").toInt();
            int courseId = checkBox->property("courseId").toInt();

            if (m_database->rejectEnrollment(studentId, courseId)) {
                successCount++;
                emit enrollmentProcessed(studentId, courseId, false);
            }
        }
    }

    showMessage(QString("成功拒绝 %1 条申请").arg(successCount));
    refreshData();
}

void EnrollmentAdminWidget::onBatchApprove()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "批量通过",
        "确定要通过所有待审核的申请吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    selectAllItems(true);
    onApproveEnrollment();
}

void EnrollmentAdminWidget::onBatchReject()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "批量拒绝",
        "确定要拒绝所有待审核的申请吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    selectAllItems(true);
    onRejectEnrollment();
}

void EnrollmentAdminWidget::onFilterChanged()
{
    updateEnrollmentTable();
}

void EnrollmentAdminWidget::onTableSelectionChanged()
{
    QList<int> selectedRows = getSelectedRows();
    int selectedCount = selectedRows.size();

    m_selectedCountLabel->setText(QString("已选择 %1 项").arg(selectedCount));
    m_approveButton->setEnabled(selectedCount > 0);
    m_rejectButton->setEnabled(selectedCount > 0);

    // 更新全选复选框状态
    if (selectedCount == 0) {
        m_selectAllCheckBox->setCheckState(Qt::Unchecked);
    } else if (selectedCount == m_enrollmentTable->rowCount()) {
        m_selectAllCheckBox->setCheckState(Qt::Checked);
    } else {
        m_selectAllCheckBox->setCheckState(Qt::PartiallyChecked);
    }
}

void EnrollmentAdminWidget::onTableItemChanged(QTableWidgetItem *item)
{
    Q_UNUSED(item)
    onTableSelectionChanged();
}

void EnrollmentAdminWidget::onRefreshClicked()
{
    refreshData();
}

QList<int> EnrollmentAdminWidget::getSelectedRows()
{
    QList<int> selectedRows;

    for (int row = 0; row < m_enrollmentTable->rowCount(); ++row) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_enrollmentTable->cellWidget(row, 0));
        if (checkBox && checkBox->isChecked()) {
            selectedRows.append(row);
        }
    }

    return selectedRows;
}

void EnrollmentAdminWidget::selectAllItems(bool select)
{
    for (int row = 0; row < m_enrollmentTable->rowCount(); ++row) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_enrollmentTable->cellWidget(row, 0));
        if (checkBox) {
            checkBox->setChecked(select);
        }
    }
    onTableSelectionChanged();
}

void EnrollmentAdminWidget::showMessage(const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "提示", message);
    } else {
        QMessageBox::information(this, "提示", message);
    }
}
