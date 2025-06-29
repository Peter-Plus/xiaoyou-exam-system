#include "exammanager.h"
#include <QApplication>
#include "examdialog.h"
#include "papercomposer.h"
#include "examstatusmanager.h"

ExamManager::ExamManager(int teacherId, Database* database, QWidget *parent)
    : QWidget(parent), teacherId(teacherId), database(database)
{
    setupUI();
    loadCourses();
    loadExams();

    // 创建并配置状态管理器
    statusManager = new ExamStatusManager(database, this);

    // 连接状态管理器信号
    connect(statusManager, &ExamStatusManager::examStatusChanged,
            this, &ExamManager::onExamStatusChanged);
    connect(statusManager, &ExamStatusManager::examAutoPublished,
            this, &ExamManager::onExamAutoPublished);
    connect(statusManager, &ExamStatusManager::examAutoTerminated,
            this, &ExamManager::onExamAutoTerminated);
    connect(statusManager, &ExamStatusManager::statusCheckCompleted,
            this, &ExamManager::onStatusCheckCompleted);

    // 启动状态自动检查
    statusManager->startStatusCheck();

    // 保留原有的刷新定时器作为备用
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &ExamManager::autoRefresh);
    refreshTimer->start(60000); // 改为60秒，因为状态管理器已经30秒检查一次
}

void ExamManager::setupUI()
{
    setWindowTitle("考试管理");
    setMinimumSize(1200, 700);
    resize(1400, 800);

    mainLayout = new QVBoxLayout(this);

    setupButtons();
    setupFilters();
    setupTable();
    setupStatusBar();
}

void ExamManager::setupButtons()
{
    buttonLayout = new QHBoxLayout();

    newExamBtn = new QPushButton("新建考试");
    publishExamBtn = new QPushButton("发布考试");
    terminateExamBtn = new QPushButton("终止考试");
    deleteExamBtn = new QPushButton("删除考试");
    refreshBtn = new QPushButton("刷新");

    // 设置按钮样式
    QString buttonStyle = "QPushButton { padding: 8px 16px; font-size: 14px; }";
    newExamBtn->setStyleSheet(buttonStyle);
    publishExamBtn->setStyleSheet(buttonStyle);
    terminateExamBtn->setStyleSheet(buttonStyle);
    deleteExamBtn->setStyleSheet(buttonStyle);
    refreshBtn->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(newExamBtn);
    buttonLayout->addWidget(publishExamBtn);
    buttonLayout->addWidget(terminateExamBtn);
    buttonLayout->addWidget(deleteExamBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(refreshBtn);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(newExamBtn, &QPushButton::clicked, this, &ExamManager::onNewExamClicked);
    connect(publishExamBtn, &QPushButton::clicked, this, &ExamManager::onPublishExamClicked);
    connect(terminateExamBtn, &QPushButton::clicked, this, &ExamManager::onTerminateExamClicked);
    connect(deleteExamBtn, &QPushButton::clicked, this, &ExamManager::onDeleteExamClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &ExamManager::onRefreshClicked);
}

void ExamManager::setupFilters()
{
    filterLayout = new QHBoxLayout();

    // 课程筛选
    QLabel* courseLabel = new QLabel("课程筛选:");
    courseFilterCombo = new QComboBox();
    courseFilterCombo->setMinimumWidth(150);

    // 状态筛选
    QLabel* statusLabel = new QLabel("状态筛选:");
    statusFilterCombo = new QComboBox();
    statusFilterCombo->addItem("全部状态");
    statusFilterCombo->addItem("未发布");
    statusFilterCombo->addItem("已发布");
    statusFilterCombo->addItem("进行中");
    statusFilterCombo->addItem("已结束");
    statusFilterCombo->setMinimumWidth(120);

    // 搜索框
    QLabel* searchLabel = new QLabel("搜索:");
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("输入考试名称搜索...");
    searchEdit->setMinimumWidth(200);

    filterLayout->addWidget(courseLabel);
    filterLayout->addWidget(courseFilterCombo);
    filterLayout->addSpacing(20);
    filterLayout->addWidget(statusLabel);
    filterLayout->addWidget(statusFilterCombo);
    filterLayout->addSpacing(20);
    filterLayout->addWidget(searchLabel);
    filterLayout->addWidget(searchEdit);
    filterLayout->addStretch();

    mainLayout->addLayout(filterLayout);

    // 连接信号
    connect(courseFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExamManager::onCourseFilterChanged);
    connect(statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExamManager::onStatusFilterChanged);
    connect(searchEdit, &QLineEdit::textChanged, this, &ExamManager::onSearchTextChanged);
}

void ExamManager::setupTable()
{
    examTable = new QTableWidget();
    examTable->setColumnCount(8);

    QStringList headers;
    headers << "考试名称" << "课程" << "考试日期" << "时间段" << "总分" << "题目数" << "状态" << "权限";
    examTable->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    examTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    examTable->setSelectionMode(QAbstractItemView::SingleSelection);
    examTable->setAlternatingRowColors(true);
    examTable->setSortingEnabled(true);
    examTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // 设置列宽
    QHeaderView* header = examTable->horizontalHeader();
    header->setStretchLastSection(false);
    header->resizeSection(0, 200); // 考试名称
    header->resizeSection(1, 150); // 课程
    header->resizeSection(2, 100); // 考试日期
    header->resizeSection(3, 150); // 时间段
    header->resizeSection(4, 80);  // 总分
    header->resizeSection(5, 80);  // 题目数
    header->resizeSection(6, 100); // 状态
    header->resizeSection(7, 100); // 权限

    mainLayout->addWidget(examTable);

    // 连接信号
    connect(examTable, &QTableWidget::cellDoubleClicked, this, &ExamManager::onExamDoubleClicked);
    connect(examTable, &QTableWidget::customContextMenuRequested, this, &ExamManager::onExamRightClicked);
    connect(examTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ExamManager::onExamSelectionChanged);
}

void ExamManager::setupStatusBar()
{
    statusLabel = new QLabel();
    statusLabel->setStyleSheet("QLabel { padding: 5px; font-size: 12px; }");
    mainLayout->addWidget(statusLabel);
}

void ExamManager::loadCourses()
{
    courses = database->getTeacherCourses(teacherId);

    courseFilterCombo->clear();
    courseFilterCombo->addItem("全部课程");

    for (const Course& course : courses) {
        courseFilterCombo->addItem(course.getDisplayName(), course.getCourseId());
    }
}

void ExamManager::loadExams()
{
    exams = database->getExamsByTeacher(teacherId);

    // 更新每个考试的状态和题目数
    for (Exam& exam : exams) {
        QString status = database->getExamStatus(exam.getExamId());
        exam.setStatus(status);

        QList<Question> questions = database->getExamQuestions(exam.getExamId());
        exam.setQuestionCount(questions.size());
    }

    refreshTable();
    updateButtonStates();
    updateStatusBar();
}

void ExamManager::refreshTable()
{
    examTable->setRowCount(0);

    QString courseFilter = courseFilterCombo->currentText();
    QString statusFilter = statusFilterCombo->currentText();
    QString searchText = searchEdit->text().toLower();

    for (const Exam& exam : exams) {
        // 应用筛选条件
        if (courseFilter != "全部课程" && exam.getCourseName() != courseFilter.split(" (")[0]) {
            continue;
        }
        if (statusFilter != "全部状态" && exam.getStatusText() != statusFilter) {
            continue;
        }
        if (!searchText.isEmpty() && !exam.getExamName().toLower().contains(searchText)) {
            continue;
        }

        int row = examTable->rowCount();
        examTable->insertRow(row);

        // 填充数据
        examTable->setItem(row, 0, new QTableWidgetItem(exam.getExamName()));
        examTable->setItem(row, 1, new QTableWidgetItem(exam.getCourseName()));
        examTable->setItem(row, 2, new QTableWidgetItem(exam.getExamDate().toString("yyyy-MM-dd")));
        examTable->setItem(row, 3, new QTableWidgetItem(exam.getTimeRange().split(" ")[1]));
        examTable->setItem(row, 4, new QTableWidgetItem(QString::number(exam.getTotalScore())));
        examTable->setItem(row, 5, new QTableWidgetItem(QString::number(exam.getQuestionCount())));

        // 状态列带颜色
        QTableWidgetItem* statusItem = new QTableWidgetItem(exam.getStatusText());
        statusItem->setForeground(QBrush(getStatusColor(exam.getStatusText())));
        examTable->setItem(row, 6, statusItem);

        // 权限列
        bool canModify = database->canModifyExam(exam.getExamId(), teacherId);
        examTable->setItem(row, 7, new QTableWidgetItem(canModify ? "创建者" : "协作者"));

        // 存储考试ID
        examTable->item(row, 0)->setData(Qt::UserRole, exam.getExamId());
    }
}

void ExamManager::updateButtonStates()
{
    bool hasSelection = examTable->currentRow() >= 0;

    publishExamBtn->setEnabled(false);
    terminateExamBtn->setEnabled(false);
    deleteExamBtn->setEnabled(false);

    if (hasSelection) {
        Exam exam = getSelectedExam();
        bool canModify = database->canModifyExam(exam.getExamId(), teacherId);

        publishExamBtn->setEnabled(canModify && exam.canPublish());
        terminateExamBtn->setEnabled(canModify && exam.getStatusText() == "进行中");
        deleteExamBtn->setEnabled(canModify && exam.canModify());
    }
}

void ExamManager::updateStatusBar()
{
    int total = exams.size();
    int unpublished = 0, published = 0, ongoing = 0, finished = 0;

    for (const Exam& exam : exams) {
        QString status = exam.getStatusText();
        if (status == "未发布") unpublished++;
        else if (status == "已发布") published++;
        else if (status == "进行中") ongoing++;
        else if (status == "已结束") finished++;
    }

    QString statusText = QString("选中: %1 | 总计: %2个考试 | 未发布: %3 | 已发布: %4 | 进行中: %5 | 已结束: %6")
                             .arg(examTable->currentRow() >= 0 ? getSelectedExam().getExamName() : "无")
                             .arg(total).arg(unpublished).arg(published).arg(ongoing).arg(finished);

    statusLabel->setText(statusText);
}

QColor ExamManager::getStatusColor(const QString &status)
{
    if (status == "未发布") return QColor(128, 128, 128); // 灰色
    if (status == "已发布") return QColor(0, 123, 255);   // 蓝色
    if (status == "进行中") return QColor(40, 167, 69);   // 绿色
    if (status == "已结束") return QColor(220, 53, 69);   // 红色
    return QColor(0, 0, 0); // 黑色
}

int ExamManager::getSelectedExamId()
{
    int currentRow = examTable->currentRow();
    if (currentRow >= 0) {
        return examTable->item(currentRow, 0)->data(Qt::UserRole).toInt();
    }
    return -1;
}

Exam ExamManager::getSelectedExam()
{
    int examId = getSelectedExamId();
    for (const Exam& exam : exams) {
        if (exam.getExamId() == examId) {
            return exam;
        }
    }
    return Exam();
}

// 槽函数实现
void ExamManager::onNewExamClicked()
{
    ExamDialog dialog(teacherId, database, this);
    if (dialog.exec() == QDialog::Accepted) {
        Exam newExam = dialog.getExamData();

        // 调用数据库添加考试
        if (database->addExam(newExam.getExamName(), newExam.getCourseId(),
                              newExam.getExamDate(), newExam.getStartTime(),
                              newExam.getEndTime(), newExam.getTotalScore(), teacherId)) {
            QMessageBox::information(this, "成功", "考试创建成功！");
            loadExams(); // 刷新考试列表
        } else {
            QMessageBox::warning(this, "失败", "考试创建失败！请检查数据库连接。");
        }
    }
}

void ExamManager::onPublishExamClicked()
{
    int examId = getSelectedExamId();
    if (examId > 0) {
        if (database->publishExam(examId)) {
            QMessageBox::information(this, "成功", "考试发布成功！");
            loadExams();
        } else {
            QMessageBox::warning(this, "失败", "考试发布失败！");
        }
    }
}

void ExamManager::onTerminateExamClicked()
{
    int examId = getSelectedExamId();
    if (examId > 0) {
        int ret = QMessageBox::question(this, "确认", "确定要终止此考试吗？",
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            if (database->terminateExam(examId)) {
                QMessageBox::information(this, "成功", "考试已终止！");
                loadExams();
            } else {
                QMessageBox::warning(this, "失败", "考试终止失败！");
            }
        }
    }
}

void ExamManager::onDeleteExamClicked()
{
    int examId = getSelectedExamId();
    if (examId > 0) {
        Exam exam = getSelectedExam();
        int ret = QMessageBox::question(this, "确认删除",
                                        QString("确定要删除考试 \"%1\" 吗？\n此操作不可撤销！").arg(exam.getExamName()),
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            if (database->deleteExam(examId, teacherId)) {
                QMessageBox::information(this, "成功", "考试删除成功！");
                loadExams();
            } else {
                QMessageBox::warning(this, "失败", "考试删除失败！");
            }
        }
    }
}

void ExamManager::onCourseFilterChanged()
{
    refreshTable();
    updateStatusBar();
}

void ExamManager::onStatusFilterChanged()
{
    refreshTable();
    updateStatusBar();
}

void ExamManager::onSearchTextChanged()
{
    refreshTable();
    updateStatusBar();
}

void ExamManager::onExamDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    if (row >= 0) {
        int examId = getSelectedExamId();
        if (examId > 0) {
            // 检查权限
            if (database->hasExamPermission(examId, teacherId)) {
                PaperComposer* composer = new PaperComposer(examId, teacherId, database);
                composer->setAttribute(Qt::WA_DeleteOnClose);
                composer->show();
            } else {
                QMessageBox::warning(this, "权限不足", "您没有编辑此试卷的权限！");
            }
        }
    }
}

void ExamManager::onExamRightClicked(const QPoint &pos)
{
    if (examTable->itemAt(pos)) {
        QMenu menu(this);
        menu.addAction("编辑考试", [this]() {
            Exam exam = getSelectedExam();
            if (exam.canModify()) {
                ExamDialog dialog(exam, teacherId, database, this);
                if (dialog.exec() == QDialog::Accepted) {
                    Exam updatedExam = dialog.getExamData();

                    if (database->updateExam(updatedExam.getExamId(), updatedExam.getExamName(),
                                             updatedExam.getExamDate(), updatedExam.getStartTime(),
                                             updatedExam.getEndTime(), updatedExam.getTotalScore())) {
                        QMessageBox::information(this, "成功", "考试修改成功！");
                        loadExams();
                    } else {
                        QMessageBox::warning(this, "失败", "考试修改失败！");
                    }
                }
            } else {
                QMessageBox::information(this, "提示", "已发布的考试不能修改基本信息！");
            }
        });
        menu.addSeparator();
        menu.addAction("组卷", [this]() {
            int examId = getSelectedExamId();
            if (examId > 0) {
                // 检查权限
                if (database->hasExamPermission(examId, teacherId)) {
                    PaperComposer* composer = new PaperComposer(examId, teacherId, database);
                    composer->setAttribute(Qt::WA_DeleteOnClose);
                    composer->show();
                } else {
                    QMessageBox::warning(this, "权限不足", "您没有编辑此试卷的权限！");
                }
            }
        });
        menu.addAction("分享权限", [this]() {
            QMessageBox::information(this, "提示", "分享权限功能待实现");
        });
        menu.addAction("查看详情", [this]() {
            Exam exam = getSelectedExam();
            QString info = QString("考试名称: %1\n课程: %2\n日期: %3\n时间: %4\n总分: %5\n题目数: %6\n状态: %7")
                               .arg(exam.getExamName())
                               .arg(exam.getCourseName())
                               .arg(exam.getExamDate().toString("yyyy-MM-dd"))
                               .arg(exam.getTimeRange().split(" ")[1])
                               .arg(exam.getTotalScore())
                               .arg(exam.getQuestionCount())
                               .arg(exam.getStatusText());
            QMessageBox::information(this, "考试详情", info);
        });
        menu.exec(examTable->mapToGlobal(pos));
    }
}

void ExamManager::onExamSelectionChanged()
{
    updateButtonStates();
    updateStatusBar();
}

void ExamManager::autoRefresh()
{
    // 静默刷新，只更新状态
    for (Exam& exam : exams) {
        QString newStatus = database->getExamStatus(exam.getExamId());
        if (newStatus != exam.getStatus()) {
            exam.setStatus(newStatus);
            refreshTable();
            updateButtonStates();
            updateStatusBar();
            break;
        }
    }
}

void ExamManager::onExamStatusChanged(int examId, const QString &oldStatus, const QString &newStatus)
{
    Q_UNUSED(examId)
    qDebug() << "收到考试状态变化通知：" << oldStatus << "→" << newStatus;

    // 刷新考试列表
    loadExams();

    // 可选：显示状态变化通知
    QString message = QString("考试状态已更新：%1 → %2").arg(oldStatus, newStatus);
    statusLabel->setText(message);

    // 3秒后恢复正常状态栏显示
    QTimer::singleShot(3000, this, [this]() {
        updateStatusBar();
    });
}

void ExamManager::onExamAutoPublished(int examId, const QString &examName)
{
    Q_UNUSED(examId)
    qDebug() << "考试自动发布：" << examName;

    // 刷新考试列表
    loadExams();

    // 显示自动发布通知
    QString message = QString("考试 \"%1\" 已自动发布").arg(examName);
    statusLabel->setText(message);
    statusLabel->setStyleSheet("QLabel { padding: 5px; font-size: 12px; color: #28a745; font-weight: bold; }");

    // 5秒后恢复正常状态栏显示
    QTimer::singleShot(5000, this, [this]() {
        statusLabel->setStyleSheet("QLabel { padding: 5px; font-size: 12px; }");
        updateStatusBar();
    });
}

void ExamManager::onExamAutoTerminated(int examId, const QString &examName)
{
    Q_UNUSED(examId)
    qDebug() << "考试自动终止：" << examName;

    // 刷新考试列表
    loadExams();

    // 显示自动终止通知
    QString message = QString("考试 \"%1\" 已自动结束").arg(examName);
    statusLabel->setText(message);
    statusLabel->setStyleSheet("QLabel { padding: 5px; font-size: 12px; color: #dc3545; font-weight: bold; }");

    // 5秒后恢复正常状态栏显示
    QTimer::singleShot(5000, this, [this]() {
        statusLabel->setStyleSheet("QLabel { padding: 5px; font-size: 12px; }");
        updateStatusBar();
    });
}

void ExamManager::onStatusCheckCompleted(int totalExams, int changedExams)
{
    if (changedExams > 0) {
        qDebug() << QString("状态检查完成：检查了%1个考试，%2个状态发生变化").arg(totalExams).arg(changedExams);
    }
}

void ExamManager::onRefreshClicked()
{
    // 手动触发状态检查
    if (statusManager) {
        statusManager->checkAllExams();
    }

    loadExams();
}

ExamManager::~ExamManager()
{
    if (statusManager) {
        statusManager->stopStatusCheck();
    }
}

void ExamManager::closeEvent(QCloseEvent *event)
{
    if (statusManager) {
        statusManager->stopStatusCheck();
    }
    QWidget::closeEvent(event);
}
