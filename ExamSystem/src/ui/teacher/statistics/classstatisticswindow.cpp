#include "classstatisticswindow.h"

ClassStatisticsWindow::ClassStatisticsWindow(Database *db, int teacherId, QWidget *parent)
    : QWidget(parent), database(db), currentTeacherId(teacherId)
{
    setWindowTitle("班级成绩统计分析");
    setMinimumSize(1200, 800);
    resize(1400, 900);

    setupUI();
    loadExams();
}

ClassStatisticsWindow::~ClassStatisticsWindow()
{
}

void ClassStatisticsWindow::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 顶部选择栏
    topLayout = new QHBoxLayout();

    QLabel *examLabel = new QLabel("选择考试:");
    examLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    examComboBox = new QComboBox();
    examComboBox->setMinimumWidth(300);
    examComboBox->setStyleSheet("QComboBox { padding: 8px; font-size: 14px; }");

    queryButton = new QPushButton("查询统计");
    queryButton->setStyleSheet("QPushButton { "
                               "background-color: #4CAF50; color: white; "
                               "border: none; padding: 10px 20px; "
                               "border-radius: 5px; font-weight: bold; }"
                               "QPushButton:hover { background-color: #45a049; }");

    exportButton = new QPushButton("导出Excel");
    exportButton->setStyleSheet("QPushButton { "
                                "background-color: #2196F3; color: white; "
                                "border: none; padding: 10px 20px; "
                                "border-radius: 5px; font-weight: bold; }"
                                "QPushButton:hover { background-color: #1976D2; }");
    exportButton->setEnabled(false);

    refreshButton = new QPushButton("刷新");
    refreshButton->setStyleSheet("QPushButton { "
                                 "background-color: #FF9800; color: white; "
                                 "border: none; padding: 10px 20px; "
                                 "border-radius: 5px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #F57C00; }");

    topLayout->addWidget(examLabel);
    topLayout->addWidget(examComboBox);
    topLayout->addWidget(queryButton);
    topLayout->addWidget(exportButton);
    topLayout->addWidget(refreshButton);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // 统计信息卡片区域
    cardLayout = new QHBoxLayout();
    cardLayout->setSpacing(20);

    basicStatsCard = createStatCard("基本统计", "");
    scoreStatsCard = createStatCard("分数统计", "");
    passingStatsCard = createStatCard("及格情况", "");

    cardLayout->addWidget(basicStatsCard);
    cardLayout->addWidget(scoreStatsCard);
    cardLayout->addWidget(passingStatsCard);

    mainLayout->addLayout(cardLayout);

    // 不及格学生列表
    failingGroup = new QGroupBox("不及格学生名单（总分60%以下）");
    failingGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; "
                                "border: 2px solid #FF5722; border-radius: 5px; "
                                "margin-top: 10px; padding-top: 10px; }");

    QVBoxLayout *failingLayout = new QVBoxLayout(failingGroup);

    failingTable = new QTableWidget(0, 4);
    QStringList failingHeaders = {"姓名", "学号", "总分", "得分率"};
    failingTable->setHorizontalHeaderLabels(failingHeaders);
    failingTable->horizontalHeader()->setStretchLastSection(true);
    failingTable->setAlternatingRowColors(true);
    failingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    failingTable->setStyleSheet("QTableWidget { gridline-color: #ddd; }");

    failingLayout->addWidget(failingTable);
    mainLayout->addWidget(failingGroup);

    // 完整成绩列表（可展开）
    allStudentsGroup = new QGroupBox("完整成绩列表");
    allStudentsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; "
                                    "border: 2px solid #4CAF50; border-radius: 5px; "
                                    "margin-top: 10px; padding-top: 10px; }");
    allStudentsGroup->setCheckable(true);
    allStudentsGroup->setChecked(false); // 默认折叠

    QVBoxLayout *allStudentsLayout = new QVBoxLayout(allStudentsGroup);

    allStudentsTable = new QTableWidget(0, 5);
    QStringList allHeaders = {"排名", "姓名", "总分", "得分率", "状态"};
    allStudentsTable->setHorizontalHeaderLabels(allHeaders);
    allStudentsTable->horizontalHeader()->setStretchLastSection(true);
    allStudentsTable->setAlternatingRowColors(true);
    allStudentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    allStudentsTable->setStyleSheet("QTableWidget { gridline-color: #ddd; }");

    allStudentsLayout->addWidget(allStudentsTable);
    mainLayout->addWidget(allStudentsGroup);

    // 连接信号槽
    connect(queryButton, &QPushButton::clicked, this, &ClassStatisticsWindow::onQueryStatistics);
    connect(exportButton, &QPushButton::clicked, this, &ClassStatisticsWindow::onExportExcel);
    connect(refreshButton, &QPushButton::clicked, this, &ClassStatisticsWindow::onRefresh);
    connect(examComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClassStatisticsWindow::onExamSelectionChanged);
}

void ClassStatisticsWindow::loadExams()
{
    examComboBox->clear();
    availableExams = database->getExamsByTeacher(currentTeacherId);

    // 只显示已结束的考试
    for (const Exam &exam : availableExams) {
        QString status = database->getExamStatus(exam.getExamId());
        if (status == "已结束") {
            QString displayText = QString("%1 - %2 (%3)")
            .arg(exam.getExamName())
                .arg(exam.getCourseName())
                .arg(exam.getExamDate().toString("yyyy-MM-dd"));
            examComboBox->addItem(displayText, exam.getExamId());
        }
    }

    if (examComboBox->count() == 0) {
        examComboBox->addItem("暂无已结束的考试", -1);
        queryButton->setEnabled(false);
    } else {
        queryButton->setEnabled(true);
    }
}

void ClassStatisticsWindow::onQueryStatistics()
{
    int examId = examComboBox->currentData().toInt();
    if (examId <= 0) {
        QMessageBox::warning(this, "提示", "请选择一个有效的考试");
        return;
    }

    // 获取班级统计数据
    QMap<QString, QVariant> statsData = database->getClassStatistics(examId);
    if (statsData.isEmpty() || statsData["total_students"].toInt() == 0) {
        QMessageBox::information(this, "提示", "该考试暂无学生成绩或尚未完成批改");
        clearDisplay();
        return;
    }

    // 创建ClassStatistics对象
    ClassStatistics stats;
    stats.setExamId(examId);
    stats.setExamName(statsData["exam_name"].toString());
    stats.setTotalScore(statsData["total_score"].toInt());
    stats.setTotalStudents(statsData["total_students"].toInt());
    stats.setHighestScore(statsData["highest_score"].toDouble());
    stats.setLowestScore(statsData["lowest_score"].toDouble());
    stats.setAverageScore(statsData["average_score"].toDouble());
    stats.setPassingCount(statsData["passing_count"].toInt());
    stats.setPassingRate(statsData["passing_rate"].toDouble());

    // 获取不及格学生列表
    QList<QMap<QString, QVariant>> failingData = database->getFailingStudents(examId);
    QList<StudentScore> failingStudents;
    for (const auto &data : failingData) {
        StudentScore student;
        student.setStudentName(data["student_name"].toString());
        student.setStudentId(data["student_id"].toString());
        student.setTotalScore(data["total_score"].toDouble());
        student.setPercentage(data["percentage"].toDouble());
        student.setStatusText("不及格");
        failingStudents.append(student);
    }
    stats.setFailingStudents(failingStudents);

    currentStats = stats;
    displayStatistics(stats);

    // 获取完整成绩列表
    QList<QMap<QString, QVariant>> allStudentsData = database->getExamStudentScores(examId);
    displayAllStudents(allStudentsData);

    exportButton->setEnabled(true);
}

void ClassStatisticsWindow::displayStatistics(const ClassStatistics &stats)
{
    // 更新基本统计卡片
    QString basicText = QString("参考人数: %1\n批改完成: %1")
                            .arg(stats.getTotalStudents());
    basicStatsCard->findChild<QLabel*>("valueLabel")->setText(basicText);

    // 更新分数统计卡片
    QString scoreText = QString("最高分: %1\n最低分: %2\n平均分: %3")
                            .arg(QString::number(stats.getHighestScore(), 'f', 1))
                            .arg(QString::number(stats.getLowestScore(), 'f', 1))
                            .arg(QString::number(stats.getAverageScore(), 'f', 1));
    scoreStatsCard->findChild<QLabel*>("valueLabel")->setText(scoreText);

    // 更新及格情况卡片
    QString passingText = QString("及格人数: %1\n及格率: %2\n不及格: %3人")
                              .arg(stats.getPassingCount())
                              .arg(stats.getPassingRateText())
                              .arg(stats.getFailingCount());
    passingStatsCard->findChild<QLabel*>("valueLabel")->setText(passingText);

    // 显示不及格学生
    displayFailingStudents(stats.getFailingStudents());
}

void ClassStatisticsWindow::displayFailingStudents(const QList<StudentScore> &students)
{
    failingTable->setRowCount(students.size());

    for (int i = 0; i < students.size(); ++i) {
        const StudentScore &student = students[i];

        failingTable->setItem(i, 0, new QTableWidgetItem(student.getStudentName()));
        failingTable->setItem(i, 1, new QTableWidgetItem(student.getStudentId()));
        failingTable->setItem(i, 2, new QTableWidgetItem(student.getScoreText()));
        failingTable->setItem(i, 3, new QTableWidgetItem(student.getPercentageText()));

        // 不及格行标红
        for (int j = 0; j < 4; ++j) {
            failingTable->item(i, j)->setBackground(QColor(255, 235, 235));
        }
    }

    failingTable->resizeColumnsToContents();
}

void ClassStatisticsWindow::displayAllStudents(const QList<QMap<QString, QVariant>> &students)
{
    allStudentsTable->setRowCount(students.size());

    for (int i = 0; i < students.size(); ++i) {
        const auto &student = students[i];

        allStudentsTable->setItem(i, 0, new QTableWidgetItem(student["rank"].toString()));
        allStudentsTable->setItem(i, 1, new QTableWidgetItem(student["student_name"].toString()));
        allStudentsTable->setItem(i, 2, new QTableWidgetItem(QString::number(student["total_score"].toDouble(), 'f', 1)));
        allStudentsTable->setItem(i, 3, new QTableWidgetItem(QString::number(student["percentage"].toDouble(), 'f', 1) + "%"));
        allStudentsTable->setItem(i, 4, new QTableWidgetItem(student["status"].toString()));

        // 根据状态设置颜色
        QColor backgroundColor;
        if (student["status"].toString() == "及格") {
            backgroundColor = QColor(235, 255, 235); // 浅绿色
        } else {
            backgroundColor = QColor(255, 235, 235); // 浅红色
        }

        for (int j = 0; j < 5; ++j) {
            allStudentsTable->item(i, j)->setBackground(backgroundColor);
        }
    }

    allStudentsTable->resizeColumnsToContents();
}

void ClassStatisticsWindow::clearDisplay()
{
    basicStatsCard->findChild<QLabel*>("valueLabel")->setText("暂无数据");
    scoreStatsCard->findChild<QLabel*>("valueLabel")->setText("暂无数据");
    passingStatsCard->findChild<QLabel*>("valueLabel")->setText("暂无数据");

    failingTable->setRowCount(0);
    allStudentsTable->setRowCount(0);

    exportButton->setEnabled(false);
}

QFrame* ClassStatisticsWindow::createStatCard(const QString &title, const QString &value, const QString &subtitle)
{
    QFrame *card = new QFrame();
    card->setFrameStyle(QFrame::Box | QFrame::Raised);
    card->setLineWidth(2);
    card->setStyleSheet("QFrame { "
                        "background-color: white; "
                        "border: 2px solid #ddd; "
                        "border-radius: 10px; "
                        "padding: 15px; }");
    card->setMinimumHeight(120);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *valueLabel = new QLabel(value.isEmpty() ? "暂无数据" : value);
    valueLabel->setObjectName("valueLabel"); // 设置对象名便于查找
    valueLabel->setStyleSheet("font-size: 14px; color: #666; margin-top: 10px;");
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setWordWrap(true);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(valueLabel);

    if (!subtitle.isEmpty()) {
        QLabel *subtitleLabel = new QLabel(subtitle);
        subtitleLabel->setStyleSheet("font-size: 12px; color: #999;");
        subtitleLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(subtitleLabel);
    }

    return card;
}

void ClassStatisticsWindow::onExportExcel()
{
    if (!currentStats.hasStatistics()) {
        QMessageBox::warning(this, "提示", "请先查询统计数据");
        return;
    }

    QString defaultFileName = QString("班级成绩统计_%1_%2")
                                  .arg(currentStats.getExamName())
                                  .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "导出成绩统计",
                                                    QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + defaultFileName,
                                                    "Excel文件 (*.csv);;文本文件 (*.txt)");

    if (!fileName.isEmpty()) {
        if (fileName.endsWith(".csv") || fileName.endsWith(".txt")) {
            exportToCSV(fileName);
        } else {
            // 默认添加.csv扩展名
            exportToCSV(fileName + ".csv");
        }
    }
}

// 修复 exportToCSV 方法中的编码设置问题
void ClassStatisticsWindow::exportToCSV(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件: " + filename);
        return;
    }

    QTextStream out(&file);

    // Qt 6兼容性修复：使用setEncoding替代setCodec
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8"); // Qt 5兼容
#endif

    // 写入BOM以确保Excel正确识别UTF-8
    out << "\xEF\xBB\xBF";

    // 标题和时间戳
    out << "班级成绩统计报告\n";
    out << "生成时间," << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << "\n";

    // 考试基本信息
    out << "=== 考试基本信息 ===\n";
    out << "考试名称," << currentStats.getExamName() << "\n";
    out << "考试总分," << currentStats.getTotalScore() << "\n";
    out << "参考人数," << currentStats.getTotalStudents() << "\n";
    out << "\n";

    // 班级统计数据
    out << "=== 班级统计数据 ===\n";
    out << "最高分," << QString::number(currentStats.getHighestScore(), 'f', 1) << "\n";
    out << "最低分," << QString::number(currentStats.getLowestScore(), 'f', 1) << "\n";
    out << "平均分," << QString::number(currentStats.getAverageScore(), 'f', 1) << "\n";
    out << "分数范围," << currentStats.getScoreRangeText() << "\n";
    out << "\n";

    // 及格情况统计
    out << "=== 及格情况统计 ===\n";
    out << "及格人数," << currentStats.getPassingCount() << "\n";
    out << "不及格人数," << currentStats.getFailingCount() << "\n";
    out << "及格率," << currentStats.getPassingRateText() << "\n";
    out << "不及格率," << QString::number(100.0 - currentStats.getPassingRate(), 'f', 1) << "%\n";
    out << "\n";

    // 不及格学生明细
    if (currentStats.getFailingCount() > 0) {
        out << "=== 不及格学生名单（总分60%以下）===\n";
        out << "序号,姓名,学号,总分,得分率,状态\n";
        int index = 1;
        for (const StudentScore &student : currentStats.getFailingStudents()) {
            out << index++ << ","
                << student.getStudentName() << ","
                << student.getStudentId() << ","
                << student.getScoreText() << ","
                << student.getPercentageText() << ","
                << "不及格\n";
        }
        out << "\n";
    } else {
        out << "=== 不及格学生名单 ===\n";
        out << "恭喜！本次考试无不及格学生\n";
        out << "\n";
    }

    // 完整成绩排名
    out << "=== 完整成绩排名 ===\n";
    out << "排名,姓名,总分,得分率,状态\n";

    // 获取完整成绩数据并导出
    QList<QMap<QString, QVariant>> allStudentsData = database->getExamStudentScores(currentStats.getExamId());
    for (const auto &student : allStudentsData) {
        out << student["rank"].toString() << ","
            << student["student_name"].toString() << ","
            << student["student_id"].toString() << ","
            << QString::number(student["total_score"].toDouble(), 'f', 1) << ","
            << QString::number(student["percentage"].toDouble(), 'f', 1) << "%,"
            << student["status"].toString() << "\n";
    }

    // 添加统计说明
    out << "\n";
    out << "=== 说明 ===\n";
    out << "1. 及格标准：总分的60%及以上\n";
    out << "2. 统计范围：已完成批改的学生成绩\n";
    out << "3. 排名依据：按总分从高到低排序\n";
    out << "4. 数据来源：在线考试系统班级成绩统计模块\n";

    file.close();

    // 成功提示，并询问是否打开文件
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "导出成功",
                                                              QString("成绩统计报告已导出到:\n%1\n\n是否要打开该文件？").arg(filename),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 使用系统默认程序打开文件
        QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
    }
}

void ClassStatisticsWindow::onRefresh()
{
    loadExams();
    clearDisplay();
}

void ClassStatisticsWindow::onExamSelectionChanged()
{
    clearDisplay();
}

void ClassStatisticsWindow::exportMultipleExams()
{
    // 这是一个扩展功能，允许教师一次性导出多个考试的统计报告
    QStringList examNames;
    QList<int> examIds;

    // 获取所有已结束的考试
    for (int i = 0; i < examComboBox->count(); ++i) {
        int examId = examComboBox->itemData(i).toInt();
        if (examId > 0) {
            examIds.append(examId);
            examNames.append(examComboBox->itemText(i));
        }
    }

    if (examIds.isEmpty()) {
        QMessageBox::information(this, "提示", "没有可导出的考试");
        return;
    }

    QString dirPath = QFileDialog::getExistingDirectory(this, "选择导出目录",
                                                        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    if (dirPath.isEmpty()) {
        return;
    }

    int successCount = 0;
    for (int i = 0; i < examIds.size(); ++i) {
        int examId = examIds[i];

        // 获取该考试的统计数据
        QMap<QString, QVariant> statsData = database->getClassStatistics(examId);
        if (statsData["total_students"].toInt() > 0) {
            // 创建文件名
            QString fileName = QString("%1/班级统计_%2_%3.csv")
                                   .arg(dirPath)
                                   .arg(examNames[i].split(" - ")[0]) // 提取考试名称
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));

            // 临时设置当前统计数据并导出
            ClassStatistics tempStats;
            tempStats.setExamId(examId);
            tempStats.setExamName(statsData["exam_name"].toString());
            tempStats.setTotalScore(statsData["total_score"].toInt());
            tempStats.setTotalStudents(statsData["total_students"].toInt());
            tempStats.setHighestScore(statsData["highest_score"].toDouble());
            tempStats.setLowestScore(statsData["lowest_score"].toDouble());
            tempStats.setAverageScore(statsData["average_score"].toDouble());
            tempStats.setPassingCount(statsData["passing_count"].toInt());
            tempStats.setPassingRate(statsData["passing_rate"].toDouble());

            ClassStatistics originalStats = currentStats;
            currentStats = tempStats;

            exportToCSV(fileName);
            successCount++;

            currentStats = originalStats;
        }
    }

    QMessageBox::information(this, "批量导出完成",
                             QString("成功导出 %1 个考试的统计报告到目录：\n%2").arg(successCount).arg(dirPath));
}
