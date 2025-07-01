#include "assignmentwidget.h"
#include <QDebug>
#include <QHeaderView>

AssignmentWidget::AssignmentWidget(Database *database, int userId, UserType userType, int courseId, QWidget *parent)
    : QWidget(parent), m_database(database), m_userId(userId), m_userType(userType), m_currentCourseId(courseId)
{
    setupUI();
    refreshData();
}

void AssignmentWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    if (m_userType == STUDENT) {
        setupStudentUI();
    } else {
        setupTeacherUI();
    }

    // 简化样式
    setStyleSheet(R"(
        QTableWidget {
            gridline-color: #ddd;
            background-color: white;
            alternate-background-color: #f5f5f5;
            selection-background-color: #3498db;
        }

        QHeaderView::section {
            background-color: #34495e;
            color: white;
            padding: 8px;
            font-weight: bold;
        }

        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
        }

        QPushButton:hover {
            background-color: #2980b9;
        }

        QPushButton:disabled {
            background-color: #bdc3c7;
        }

        QPushButton#PublishButton {
            background-color: #27ae60;
        }

        QPushButton#SubmitButton {
            background-color: #e67e22;
        }

        QPushButton#GradeButton {
            background-color: #8e44ad;
        }

        QGroupBox {
            font-weight: bold;
            border: 2px solid #bdc3c7;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
    )");
}

void AssignmentWidget::setupStudentUI()
{
    m_splitter = new QSplitter(Qt::Horizontal);

    // 左侧作业列表
    m_listWidget = new QWidget();
    QVBoxLayout *listLayout = new QVBoxLayout(m_listWidget);

    QLabel *titleLabel = new QLabel("📝 我的作业");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");

    m_assignmentTable = new QTableWidget();
    m_assignmentTable->setColumnCount(5);
    QStringList headers;
    headers << "作业标题" << "课程" << "截止时间" << "状态" << "得分";
    m_assignmentTable->setHorizontalHeaderLabels(headers);
    m_assignmentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_assignmentTable->setAlternatingRowColors(true);
    m_assignmentTable->horizontalHeader()->setStretchLastSection(true);

    connect(m_assignmentTable, &QTableWidget::itemSelectionChanged,
            this, &AssignmentWidget::onAssignmentSelectionChanged);

    listLayout->addWidget(titleLabel);
    listLayout->addWidget(m_assignmentTable);

    // 右侧详情和提交区域
    m_detailWidget = new QWidget();
    QVBoxLayout *detailLayout = new QVBoxLayout(m_detailWidget);

    m_detailGroup = new QGroupBox("📋 作业详情");
    QVBoxLayout *groupLayout = new QVBoxLayout(m_detailGroup);

    m_titleLabel = new QLabel("请选择作业");
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    m_infoLabel = new QLabel();
    m_infoLabel->setStyleSheet("color: #7f8c8d;");

    m_descriptionText = new QTextEdit();
    m_descriptionText->setReadOnly(true);
    m_descriptionText->setMaximumHeight(150);

    QLabel *submitLabel = new QLabel("📤 提交内容:");
    submitLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");

    m_submissionText = new QTextEdit();
    m_submissionText->setPlaceholderText("在此输入作业内容...");
    m_submissionText->setMaximumHeight(200);

    m_submitButton = new QPushButton("📤 提交作业");
    m_submitButton->setObjectName("SubmitButton");
    m_submitButton->setEnabled(false);
    connect(m_submitButton, &QPushButton::clicked, this, &AssignmentWidget::onSubmitAssignment);

    groupLayout->addWidget(m_titleLabel);
    groupLayout->addWidget(m_infoLabel);
    groupLayout->addWidget(m_descriptionText);
    groupLayout->addWidget(submitLabel);
    groupLayout->addWidget(m_submissionText);
    groupLayout->addWidget(m_submitButton);

    // 统计信息
    m_statsGroup = new QGroupBox("📊 统计信息");
    m_statsLabel = new QLabel();
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    statsLayout->addWidget(m_statsLabel);

    detailLayout->addWidget(m_detailGroup, 3);
    detailLayout->addWidget(m_statsGroup, 1);

    m_splitter->addWidget(m_listWidget);
    m_splitter->addWidget(m_detailWidget);
    m_splitter->setSizes({500, 400});

    m_mainLayout->addWidget(m_splitter);
}

void AssignmentWidget::setupTeacherUI()
{
    m_splitter = new QSplitter(Qt::Horizontal);

    // 左侧作业列表
    m_listWidget = new QWidget();
    QVBoxLayout *listLayout = new QVBoxLayout(m_listWidget);

    // 课程选择和发布按钮
    QHBoxLayout *controlLayout = new QHBoxLayout();
    QLabel *courseLabel = new QLabel("选择课程:");
    m_courseCombo = new QComboBox();
    m_courseCombo->setMinimumWidth(200);
    connect(m_courseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AssignmentWidget::onCourseSelectionChanged);

    m_publishButton = new QPushButton("➕ 发布作业");
    m_publishButton->setObjectName("PublishButton");
    m_publishButton->setEnabled(false);
    connect(m_publishButton, &QPushButton::clicked, this, &AssignmentWidget::onPublishAssignment);

    controlLayout->addWidget(courseLabel);
    controlLayout->addWidget(m_courseCombo, 1);
    controlLayout->addWidget(m_publishButton);

    m_assignmentTable = new QTableWidget();
    m_assignmentTable->setColumnCount(5);
    QStringList headers;
    headers << "作业标题" << "发布时间" << "截止时间" << "状态" << "提交情况";
    m_assignmentTable->setHorizontalHeaderLabels(headers);
    m_assignmentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_assignmentTable->setAlternatingRowColors(true);
    m_assignmentTable->horizontalHeader()->setStretchLastSection(true);

    connect(m_assignmentTable, &QTableWidget::itemSelectionChanged,
            this, &AssignmentWidget::onAssignmentSelectionChanged);

    listLayout->addLayout(controlLayout);
    listLayout->addWidget(m_assignmentTable);

    // 右侧详情和管理区域
    m_detailWidget = new QWidget();
    QVBoxLayout *detailLayout = new QVBoxLayout(m_detailWidget);

    m_detailGroup = new QGroupBox("📋 作业管理");
    QVBoxLayout *groupLayout = new QVBoxLayout(m_detailGroup);

    m_titleLabel = new QLabel("请选择课程和作业");
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    m_infoLabel = new QLabel();
    m_infoLabel->setStyleSheet("color: #7f8c8d;");

    m_descriptionText = new QTextEdit();
    m_descriptionText->setReadOnly(true);
    m_descriptionText->setMaximumHeight(150);

    // 操作按钮
    QHBoxLayout *actionLayout = new QHBoxLayout();

    m_viewButton = new QPushButton("👀 查看提交");
    m_viewButton->setEnabled(false);
    connect(m_viewButton, &QPushButton::clicked, this, &AssignmentWidget::onViewSubmissions);

    m_gradeButton = new QPushButton("📝 批改作业");
    m_gradeButton->setObjectName("GradeButton");
    m_gradeButton->setEnabled(false);
    connect(m_gradeButton, &QPushButton::clicked, this, &AssignmentWidget::onGradeAssignment);

    actionLayout->addWidget(m_viewButton);
    actionLayout->addWidget(m_gradeButton);
    actionLayout->addStretch();

    groupLayout->addWidget(m_titleLabel);
    groupLayout->addWidget(m_infoLabel);
    groupLayout->addWidget(m_descriptionText);
    groupLayout->addLayout(actionLayout);

    // 统计信息
    m_statsGroup = new QGroupBox("📊 作业统计");
    m_statsLabel = new QLabel();
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    statsLayout->addWidget(m_statsLabel);

    detailLayout->addWidget(m_detailGroup, 3);
    detailLayout->addWidget(m_statsGroup, 1);

    m_splitter->addWidget(m_listWidget);
    m_splitter->addWidget(m_detailWidget);
    m_splitter->setSizes({600, 400});

    m_mainLayout->addWidget(m_splitter);
}

void AssignmentWidget::refreshData()
{
    if (!m_database) return;

    if (m_userType == TEACHER) {
        loadCourseList();
    }
    loadAssignments();
    updateStatistics();
}

void AssignmentWidget::loadCourseList()
{
    m_courseList = m_database->getCoursesByTeacher(m_userId);

    m_courseCombo->clear();
    m_courseCombo->addItem("请选择课程", -1);

    for (const auto &course : m_courseList) {
        QString courseText = QString("%1 (%2人)")
                                 .arg(course["course_name"].toString())
                                 .arg(course["enrolled_count"].toString());
        m_courseCombo->addItem(courseText, course["course_id"].toInt());
    }
}

void AssignmentWidget::loadAssignments()
{
    m_assignments.clear();

    if (m_userType == STUDENT) {
        m_assignments = m_database->getStudentAllAssignments(m_userId, true);
    } else if (m_currentCourseId > 0) {
        m_assignments = m_database->getAssignments(m_currentCourseId, 0);
    }

    updateAssignmentTable();
}

void AssignmentWidget::updateAssignmentTable()
{
    m_assignmentTable->setRowCount(m_assignments.size());

    for (int i = 0; i < m_assignments.size(); ++i) {
        const QVariantMap &assignment = m_assignments[i];

        if (m_userType == STUDENT) {
            m_assignmentTable->setItem(i, 0, new QTableWidgetItem(assignment["title"].toString()));
            m_assignmentTable->setItem(i, 1, new QTableWidgetItem(assignment["course_name"].toString()));
            m_assignmentTable->setItem(i, 2, new QTableWidgetItem(
                                                 assignment["deadline"].toDateTime().toString("MM-dd hh:mm")));

            QString status = "未提交";
            QString score = "--";
            if (!assignment["submission_status"].toString().isEmpty()) {
                if (assignment["submission_status"].toString() == "已批改") {
                    status = "已批改";
                    score = QString("%1/%2").arg(assignment["score"].toInt())
                                .arg(assignment["max_score"].toInt());
                } else {
                    status = "已提交";
                    score = "待批改";
                }
            }

            m_assignmentTable->setItem(i, 3, new QTableWidgetItem(status));
            m_assignmentTable->setItem(i, 4, new QTableWidgetItem(score));
        } else {
            m_assignmentTable->setItem(i, 0, new QTableWidgetItem(assignment["title"].toString()));
            m_assignmentTable->setItem(i, 1, new QTableWidgetItem(
                                                 assignment["publish_time"].toDateTime().toString("MM-dd hh:mm")));
            m_assignmentTable->setItem(i, 2, new QTableWidgetItem(
                                                 assignment["deadline"].toDateTime().toString("MM-dd hh:mm")));
            m_assignmentTable->setItem(i, 3, new QTableWidgetItem(assignment["status"].toString()));
            m_assignmentTable->setItem(i, 4, new QTableWidgetItem("--/--"));
        }

        // 存储作业ID
        m_assignmentTable->item(i, 0)->setData(Qt::UserRole, i);
    }

    m_assignmentTable->resizeColumnsToContents();
}

void AssignmentWidget::updateStatistics()
{
    QString statsText;

    if (m_userType == STUDENT) {
        int total = 0, submitted = 0, graded = 0;
        for (const auto &assignment : m_assignments) {
            total++;
            if (!assignment["submission_status"].toString().isEmpty()) {
                submitted++;
                if (assignment["submission_status"].toString() == "已批改") {
                    graded++;
                }
            }
        }
        statsText = QString("总作业: %1个\n已提交: %2个\n已批改: %3个")
                        .arg(total).arg(submitted).arg(graded);
    } else {
        int total = m_assignments.size();
        statsText = QString("已发布: %1个\n本课程作业\n管理课程: %2门")
                        .arg(total).arg(m_courseList.size());
    }

    m_statsLabel->setText(statsText);
}

void AssignmentWidget::setCourseId(int courseId)
{
    m_currentCourseId = courseId;
    if (m_userType == TEACHER && m_courseCombo) {
        for (int i = 0; i < m_courseCombo->count(); ++i) {
            if (m_courseCombo->itemData(i).toInt() == courseId) {
                m_courseCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    loadAssignments();
}

void AssignmentWidget::onAssignmentSelectionChanged()
{
    int row = m_assignmentTable->currentRow();
    if (row < 0 || row >= m_assignments.size()) {
        m_selectedAssignment.clear();
        if (m_userType == STUDENT) {
            m_submitButton->setEnabled(false);
        } else {
            m_viewButton->setEnabled(false);
            m_gradeButton->setEnabled(false);
        }
        return;
    }

    m_selectedAssignment = m_assignments[row];
    showAssignmentDetails();

    if (m_userType == STUDENT) {
        bool canSubmit = m_selectedAssignment["status"].toString() == "开放提交" &&
                         m_selectedAssignment["submission_status"].toString().isEmpty();
        m_submitButton->setEnabled(canSubmit);

        // 如果已提交，显示提交内容
        if (!m_selectedAssignment["submission_status"].toString().isEmpty()) {
            QVariantMap submission = m_database->getSubmissionDetail(
                m_selectedAssignment["assignment_id"].toInt(), m_userId);
            m_submissionText->setPlainText(submission["content"].toString());
            m_submissionText->setReadOnly(true);
        } else {
            m_submissionText->clear();
            m_submissionText->setReadOnly(false);
        }
    } else {
        m_viewButton->setEnabled(true);
        m_gradeButton->setEnabled(true);
    }
}

void AssignmentWidget::onCourseSelectionChanged()
{
    int courseId = m_courseCombo->currentData().toInt();
    m_currentCourseId = courseId;
    m_publishButton->setEnabled(courseId > 0);
    loadAssignments();
}

void AssignmentWidget::showAssignmentDetails()
{
    if (m_selectedAssignment.isEmpty()) return;

    m_titleLabel->setText(m_selectedAssignment["title"].toString());

    QString infoText;
    if (m_userType == STUDENT) {
        infoText = QString("课程: %1 | 截止: %2 | 满分: %3分")
                       .arg(m_selectedAssignment["course_name"].toString())
                       .arg(m_selectedAssignment["deadline"].toDateTime().toString("yyyy-MM-dd hh:mm"))
                       .arg(m_selectedAssignment["max_score"].toInt());
    } else {
        infoText = QString("发布: %1 | 截止: %2 | 满分: %3分")
                       .arg(m_selectedAssignment["publish_time"].toDateTime().toString("yyyy-MM-dd hh:mm"))
                       .arg(m_selectedAssignment["deadline"].toDateTime().toString("yyyy-MM-dd hh:mm"))
                       .arg(m_selectedAssignment["max_score"].toInt());
    }
    m_infoLabel->setText(infoText);

    m_descriptionText->setPlainText(m_selectedAssignment["description"].toString());
}

void AssignmentWidget::onPublishAssignment()
{
    if (m_currentCourseId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择课程！");
        return;
    }

    AssignmentEditDialog dialog(m_currentCourseId, this);
    if (dialog.exec() == QDialog::Accepted) {
        int assignmentId = m_database->addAssignment(
            m_currentCourseId,
            dialog.getTitle(),
            dialog.getDescription(),
            dialog.getDeadline(),
            dialog.getMaxScore(),
            m_userId
            );

        if (assignmentId > 0) {
            QMessageBox::information(this, "成功", "作业发布成功！");
            emit assignmentPublished(m_currentCourseId, dialog.getTitle());
            refreshData();
        } else {
            QMessageBox::critical(this, "失败", "作业发布失败！");
        }
    }
}

void AssignmentWidget::onSubmitAssignment()
{
    if (m_selectedAssignment.isEmpty()) return;

    QString content = m_submissionText->toPlainText().trimmed();
    if (content.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入作业内容！");
        return;
    }

    int assignmentId = m_selectedAssignment["assignment_id"].toInt();

    if (m_database->submitAssignment(assignmentId, m_userId, content)) {
        QMessageBox::information(this, "成功", "作业提交成功！");
        emit assignmentSubmitted(assignmentId, m_userId);
        refreshData();
    } else {
        QMessageBox::critical(this, "失败", "作业提交失败！");
    }
}

void AssignmentWidget::onGradeAssignment()
{
    if (m_selectedAssignment.isEmpty()) return;

    int assignmentId = m_selectedAssignment["assignment_id"].toInt();

    // 简化版批改界面 - 显示提交列表
    QList<QVariantMap> submissions = m_database->getSubmissions(assignmentId, m_userId);

    if (submissions.isEmpty()) {
        QMessageBox::information(this, "提示", "暂无学生提交作业！");
        return;
    }

    QString info = QString("共有 %1 名学生提交了作业").arg(submissions.size());
    QMessageBox::information(this, "批改信息", info);
}

void AssignmentWidget::onViewSubmissions()
{
    if (m_selectedAssignment.isEmpty()) return;

    int assignmentId = m_selectedAssignment["assignment_id"].toInt();
    QVariantMap stats = m_database->getAssignmentStats(assignmentId);

    QString info = QString(
                       "提交统计:\n\n"
                       "应提交人数: %1人\n"
                       "已提交人数: %2人\n"
                       "已批改人数: %3人\n"
                       "平均分: %4分"
                       ).arg(stats["total_students"].toInt())
                       .arg(stats["submitted_count"].toInt())
                       .arg(stats["graded_count"].toInt())
                       .arg(stats["average_score"].toDouble(), 0, 'f', 1);

    QMessageBox::information(this, "提交统计", info);
}

// ============================================================================
// AssignmentEditDialog 实现
// ============================================================================

AssignmentEditDialog::AssignmentEditDialog(int courseId, QWidget *parent)
    : QDialog(parent), m_courseId(courseId), m_maxScore(100)
{
    setWindowTitle("发布作业");
    setModal(true);
    resize(500, 400);

    setupUI();
}

void AssignmentEditDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // 标题
    QLabel *titleLabel = new QLabel("作业标题:");
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("请输入作业标题");

    // 描述
    QLabel *descLabel = new QLabel("作业描述:");
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText("请输入作业要求和描述");
    m_descriptionEdit->setMaximumHeight(200);

    // 截止时间
    QLabel *deadlineLabel = new QLabel("截止时间:");
    m_deadlineEdit = new QDateTimeEdit();
    m_deadlineEdit->setDateTime(QDateTime::currentDateTime().addDays(7));
    m_deadlineEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

    // 分数
    QLabel *scoreLabel = new QLabel("满分:");
    m_scoreSpinBox = new QSpinBox();
    m_scoreSpinBox->setRange(1, 1000);
    m_scoreSpinBox->setValue(100);
    m_scoreSpinBox->setSuffix(" 分");

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("发布作业");
    QPushButton *cancelButton = new QPushButton("取消");

    connect(okButton, &QPushButton::clicked, this, &AssignmentEditDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);

    layout->addWidget(titleLabel);
    layout->addWidget(m_titleEdit);
    layout->addWidget(descLabel);
    layout->addWidget(m_descriptionEdit);
    layout->addWidget(deadlineLabel);
    layout->addWidget(m_deadlineEdit);
    layout->addWidget(scoreLabel);
    layout->addWidget(m_scoreSpinBox);
    layout->addLayout(buttonLayout);
}

void AssignmentEditDialog::onAccept()
{
    m_title = m_titleEdit->text().trimmed();
    m_description = m_descriptionEdit->toPlainText().trimmed();
    m_deadline = m_deadlineEdit->dateTime();
    m_maxScore = m_scoreSpinBox->value();

    if (m_title.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入作业标题！");
        return;
    }

    if (m_description.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入作业描述！");
        return;
    }

    if (m_deadline <= QDateTime::currentDateTime()) {
        QMessageBox::warning(this, "错误", "截止时间不能早于当前时间！");
        return;
    }

    accept();
}
