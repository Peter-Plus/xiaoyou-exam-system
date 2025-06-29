#include "studentexamlist.h"
#include <QDateTime>
#include "Question.h"

StudentExamList::StudentExamList(Database *database, int studentId, QWidget *parent)
    : QWidget(parent), database(database), studentId(studentId)
{
    setupUI();
    loadExamData();

    // 设置定时器，每分钟更新一次倒计时
    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &StudentExamList::updateCountdown);
    countdownTimer->start(60000); // 每分钟更新
}

void StudentExamList::setupUI()
{
    setWindowTitle("参与考试");
    setMinimumSize(1000, 600);

    mainLayout = new QVBoxLayout(this);

    // 筛选区域
    filterLayout = new QHBoxLayout;

    QLabel *statusLabel = new QLabel("状态筛选:");
    statusComboBox = new QComboBox;
    statusComboBox->addItems({"全部", "可参与", "已完成", "批改中", "已过期"});

    QLabel *searchLabel = new QLabel("搜索:");
    searchLineEdit = new QLineEdit;
    searchLineEdit->setPlaceholderText("考试名称/课程");

    refreshButton = new QPushButton("刷新");

    filterLayout->addWidget(statusLabel);
    filterLayout->addWidget(statusComboBox);
    filterLayout->addWidget(searchLabel);
    filterLayout->addWidget(searchLineEdit);
    filterLayout->addStretch();
    filterLayout->addWidget(refreshButton);

    mainLayout->addLayout(filterLayout);

    // 考试列表表格
    examTable = new QTableWidget;
    examTable->setColumnCount(8);
    QStringList headers = {"考试名称", "课程", "考试时间", "时长", "总分", "题目数", "状态", "操作"};
    examTable->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    examTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    examTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    examTable->horizontalHeader()->setStretchLastSection(true);
    examTable->setAlternatingRowColors(true);

    mainLayout->addWidget(examTable);

    // 底部状态栏
    statisticsLabel = new QLabel;
    mainLayout->addWidget(statisticsLabel);

    // 连接信号
    connect(statusComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &StudentExamList::onStatusFilterChanged);
    connect(searchLineEdit, &QLineEdit::textChanged,
            this, &StudentExamList::onSearchTextChanged);
    connect(refreshButton, &QPushButton::clicked,
            this, &StudentExamList::refreshExamList);
}

void StudentExamList::loadExamData()
{
    if (!database) return;

    allExams = database->getAvailableExamsForStudent(studentId);
    applyFilters();
    updateStatistics();
}

void StudentExamList::applyFilters()
{
    filteredExams.clear();
    QString statusFilter = statusComboBox->currentText();
    QString searchText = searchLineEdit->text().trimmed().toLower();

    for (const Exam &exam : allExams) {
        // 状态筛选
        QString examStatus = getExamStatusForStudent(exam);
        if (statusFilter != "全部") {
            if (statusFilter == "可参与" && examStatus != "可参与" && examStatus != "进行中") continue;
            if (statusFilter == "已完成" && examStatus != "已完成") continue;
            if (statusFilter == "已过期" && examStatus != "已过期") continue;
            if (statusFilter == "批改中" && examStatus != "批改中") continue;
        }

        // 搜索筛选
        if (!searchText.isEmpty()) {
            QString examName = exam.getExamName().toLower();
            QString courseName = exam.getCourseName().toLower();
            if (!examName.contains(searchText) && !courseName.contains(searchText)) {
                continue;
            }
        }

        filteredExams.append(exam);
    }

    populateTable(filteredExams);
}

void StudentExamList::populateTable(const QList<Exam> &exams)
{
    examTable->setRowCount(exams.size());

    for (int i = 0; i < exams.size(); ++i) {
        const Exam &exam = exams[i];

        // 考试名称
        examTable->setItem(i, 0, new QTableWidgetItem(exam.getExamName()));

        // 课程
        examTable->setItem(i, 1, new QTableWidgetItem(exam.getCourseName()));

        // 考试时间
        examTable->setItem(i, 2, new QTableWidgetItem(exam.getTimeRange()));

        // 时长
        QTime duration = QTime(0, 0).addSecs(exam.getStartTime().secsTo(exam.getEndTime()));
        examTable->setItem(i, 3, new QTableWidgetItem(duration.toString("hh:mm")));

        // 总分
        examTable->setItem(i, 4, new QTableWidgetItem(QString::number(exam.getTotalScore())));

        // 题目数
        QList<Question> questions = database->getExamQuestions(exam.getExamId());
        examTable->setItem(i, 5, new QTableWidgetItem(QString::number(questions.size())));

        // 状态
        QString status = getExamStatusForStudent(exam);
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);

        // 状态颜色设置
        if (status == "可参与") {
            statusItem->setBackground(QColor(144, 238, 144)); // 浅绿色
        } else if (status == "进行中") {
            statusItem->setBackground(QColor(255, 215, 0)); // 金色
        } else if (status == "批改中") {
            statusItem->setBackground(QColor(255, 165, 0)); // 橙色
        } else if (status == "已完成") {
            statusItem->setBackground(QColor(173, 216, 230)); // 浅蓝色
        } else if (status == "已过期") {
            statusItem->setBackground(QColor(255, 182, 193)); // 浅红色
        }

        examTable->setItem(i, 6, statusItem);

        // 操作按钮
        QWidget *buttonWidget = new QWidget;
        QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
        buttonLayout->setContentsMargins(2, 2, 2, 2);

        if (status == "可参与") {
            QPushButton *startButton = new QPushButton("开始考试");
            startButton->setProperty("examId", exam.getExamId());
            connect(startButton, &QPushButton::clicked, this, &StudentExamList::onStartExam);
            buttonLayout->addWidget(startButton);
        } else if (status == "批改中") {
            QPushButton *gradingButton = new QPushButton("批改中");
            gradingButton->setEnabled(false);  // 设为不可点击
            gradingButton->setStyleSheet("color: #888; background-color: #f0f0f0; border: 1px solid #ccc;");
            buttonLayout->addWidget(gradingButton);
        } else if (status == "已完成") {
            QPushButton *viewButton = new QPushButton("查看成绩");
            viewButton->setProperty("examId", exam.getExamId());
            connect(viewButton, &QPushButton::clicked, this, &StudentExamList::onViewResult);
            buttonLayout->addWidget(viewButton);
        } else if (status == "已过期") {
            QPushButton *expiredButton = new QPushButton("已过期");
            expiredButton->setEnabled(false);
            expiredButton->setStyleSheet("color: #888; background-color: #f5f5f5; border: 1px solid #ccc;");
            buttonLayout->addWidget(expiredButton);
        } else {
            // 其他状态（如"未开始"）
            QPushButton *disabledButton = new QPushButton("未开始");
            disabledButton->setEnabled(false);
            disabledButton->setStyleSheet("color: #888; background-color: #f5f5f5; border: 1px solid #ccc;");
            buttonLayout->addWidget(disabledButton);
        }

        examTable->setCellWidget(i, 7, buttonWidget);
    }

    // 调整列宽
    examTable->resizeColumnsToContents();
    examTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

QString StudentExamList::getExamStatusForStudent(const Exam &exam)
{
    // 首先检查学生是否已经提交了考试（有答题记录）
    bool hasSubmitted = database->hasStudentStartedExam(studentId, exam.getExamId());

    if (hasSubmitted) {
        // 学生已提交考试，检查是否已完全批改完成
        bool isFullyGraded = database->isExamFullyGraded(exam.getExamId());
        if (isFullyGraded) {
            return "已完成";  // 已批改完成，可以查看成绩
        } else {
            return "批改中";  // 已提交但未完全批改，显示批改中
        }
    }

    // 学生未提交考试，根据考试整体状态判断
    QString baseStatus = database->getExamStatus(exam.getExamId());
    if (baseStatus == "已结束") {
        return "已过期";  // 考试已结束但学生未参与
    } else if (baseStatus == "进行中" || baseStatus == "已发布") {
        return "可参与";  // 考试进行中或已发布，学生可以参与
    }

    return "未开始";  // 考试未发布
}

QString StudentExamList::getStudentScore(int examId)
{
    double score = database->getStudentExamTotalScore(examId, studentId);
    if (score > 0) {
        return QString::number(score, 'f', 1);
    }
    return "-";
}

void StudentExamList::updateStatistics()
{
    int total = allExams.size();
    int available = 0;
    int completed = 0;
    int grading = 0;
    double totalScore = 0;
    int scoredExams = 0;

    for (const Exam &exam : allExams) {
        QString status = getExamStatusForStudent(exam);
        if (status == "可参与" || status == "进行中") {
            available++;
        } else if (status == "已完成") {
            completed++;
            double score = database->getStudentExamTotalScore(exam.getExamId(), studentId);
            if (score > 0) {
                totalScore += score;
                scoredExams++;
            }
        } else if (status == "批改中") {
            grading++;
        }
    }

    QString statsText = QString("总计: %1个考试 | 可参与: %2 | 批改中: %3 | 已完成: %4")
                            .arg(total).arg(available).arg(grading).arg(completed);

    if (scoredExams > 0) {
        double avgScore = totalScore / scoredExams;
        statsText += QString(" | 平均分: %1").arg(QString::number(avgScore, 'f', 1));
    }

    statisticsLabel->setText(statsText);
}

void StudentExamList::refreshExamList()
{
    loadExamData();
}

void StudentExamList::onStatusFilterChanged()
{
    applyFilters();
}

void StudentExamList::onSearchTextChanged()
{
    applyFilters();
}

void StudentExamList::onStartExam()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int examId = button->property("examId").toInt();

    // 检查是否可以参与考试
    if (!database->canStudentTakeExam(studentId, examId)) {
        QMessageBox::warning(this, "提示", "您无法参与此考试！");
        return;
    }

    // 显示考试确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "开始考试",
        "确定要开始考试吗？\n考试开始后将无法返回此页面。",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 创建并打开考试界面
        ExamTaking *examWindow = new ExamTaking(database, examId, studentId);
        examWindow->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
        examWindow->showMaximized(); // 全屏显示考试界面

        // 隐藏当前窗口
        this->hide();

        // 考试窗口关闭后重新显示考试列表并刷新
        connect(examWindow, &QObject::destroyed, [this]() {
            this->show();
            this->refreshExamList(); // 刷新考试状态
        });
    }
}

void StudentExamList::onViewResult()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int examId = button->property("examId").toInt();

    // 创建并显示成绩查看界面
    ExamResult *resultWindow = new ExamResult(database, examId, studentId);
    resultWindow->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
    resultWindow->show();
}

void StudentExamList::updateCountdown()
{
    // 更新表格中的状态信息
    applyFilters();
}

void StudentExamList::onViewSubmittedAnswers()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int examId = button->property("examId").toInt();

    // 创建一个只读的考试查看界面
    ExamResult *resultWindow = new ExamResult(database, examId, studentId);
    resultWindow->setWindowTitle("查看已提交答案 - 等待批改中");
    resultWindow->setAttribute(Qt::WA_DeleteOnClose);
    resultWindow->show();
}
