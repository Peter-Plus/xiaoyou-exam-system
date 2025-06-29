#include "examresult.h"
#include <QDateTime>


ExamResult::ExamResult(Database *database, int examId, int studentId, QWidget *parent)
    : QWidget(parent), database(database), examId(examId), studentId(studentId),
    totalScore(0), maxScore(0), percentage(0), correctCount(0), totalCount(0),
    objectiveScore(0), subjectiveScore(0), objectiveMaxScore(0), subjectiveMaxScore(0)
{
    setupUI();
    loadExamData();
}

void ExamResult::setupUI()
{
    setWindowTitle("考试成绩");
    setMinimumSize(900, 700);

    mainLayout = new QVBoxLayout(this);

    // 基本信息区域
    basicInfoGroup = new QGroupBox("考试基本信息");
    basicInfoLayout = new QGridLayout(basicInfoGroup);

    // 左侧信息
    basicInfoLayout->addWidget(new QLabel("考试名称:"), 0, 0);
    examNameLabel = new QLabel;
    examNameLabel->setStyleSheet("font-weight: bold;");
    basicInfoLayout->addWidget(examNameLabel, 0, 1);

    basicInfoLayout->addWidget(new QLabel("考试时间:"), 1, 0);
    examTimeLabel = new QLabel;
    basicInfoLayout->addWidget(examTimeLabel, 1, 1);

    // 右侧成绩信息
    basicInfoLayout->addWidget(new QLabel("总分:"), 0, 2);
    totalScoreLabel = new QLabel;
    totalScoreLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2196F3;");
    basicInfoLayout->addWidget(totalScoreLabel, 0, 3);

    basicInfoLayout->addWidget(new QLabel("得分率:"), 1, 2);
    percentageLabel = new QLabel;
    percentageLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    basicInfoLayout->addWidget(percentageLabel, 1, 3);

    // 进度条
    basicInfoLayout->addWidget(new QLabel("成绩进度:"), 2, 0);
    scoreProgressBar = new QProgressBar;
    scoreProgressBar->setMinimum(0);
    scoreProgressBar->setMaximum(100);
    scoreProgressBar->setTextVisible(true);
    basicInfoLayout->addWidget(scoreProgressBar, 2, 1, 1, 3);

    mainLayout->addWidget(basicInfoGroup);

    // 操作按钮
    buttonLayout = new QHBoxLayout;
    printButton = new QPushButton("打印成绩单");
    backButton = new QPushButton("返回考试列表");

    buttonLayout->addWidget(printButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);

    mainLayout->addLayout(buttonLayout);

    // 答题详情表格
    detailGroup = new QGroupBox("答题详情");
    QVBoxLayout *detailLayout = new QVBoxLayout(detailGroup);

    detailTable = new QTableWidget;
    detailTable->setColumnCount(7);
    QStringList headers = {"题号", "题型", "分值", "我的答案", "正确答案", "得分", "状态"};
    detailTable->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    detailTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    detailTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    detailTable->setAlternatingRowColors(true);
    detailTable->horizontalHeader()->setStretchLastSection(true);

    detailLayout->addWidget(detailTable);
    mainLayout->addWidget(detailGroup);

    // 统计信息区域
    statisticsGroup = new QGroupBox("统计信息");
    statisticsLayout = new QGridLayout(statisticsGroup);

    statisticsLayout->addWidget(new QLabel("正确题数:"), 0, 0);
    correctCountLabel = new QLabel;
    correctCountLabel->setStyleSheet("font-weight: bold;");
    statisticsLayout->addWidget(correctCountLabel, 0, 1);

    statisticsLayout->addWidget(new QLabel("客观题得分:"), 0, 2);
    objectiveScoreLabel = new QLabel;
    objectiveScoreLabel->setStyleSheet("font-weight: bold; color: #4CAF50;");
    statisticsLayout->addWidget(objectiveScoreLabel, 0, 3);

    statisticsLayout->addWidget(new QLabel("主观题得分:"), 1, 2);
    subjectiveScoreLabel = new QLabel;
    subjectiveScoreLabel->setStyleSheet("font-weight: bold; color: #FF9800;");
    statisticsLayout->addWidget(subjectiveScoreLabel, 1, 3);

    mainLayout->addWidget(statisticsGroup);

    // 连接信号
    connect(printButton, &QPushButton::clicked, this, &ExamResult::onPrintResult);
    connect(backButton, &QPushButton::clicked, this, &ExamResult::onBackToList);
}

void ExamResult::loadExamData()
{
    if (!database) return;

    // 获取考试信息
    QList<Exam> exams = database->getExamsByTeacher(-1);
    for (const Exam &exam : exams) {
        if (exam.getExamId() == examId) {
            currentExam = exam;
            break;
        }
    }

    // 获取考试题目
    examQuestions = database->getExamQuestions(examId);

    // 获取学生答案
    loadStudentAnswers();

    // 计算成绩
    calculateScores();

    // 显示数据
    displayBasicInfo();
    displayAnswerDetails();
    displayStatistics();
}

void ExamResult::loadStudentAnswers()
{
    studentAnswers = database->getStudentAnswers(examId, studentId);

    // 确保每个题目都有对应的答案记录（即使是空答案）
    for (const Question &question : examQuestions) {
        bool found = false;
        for (const StudentAnswer &answer : studentAnswers) {
            if (answer.getQuestionId() == question.getQuestionId()) {
                found = true;
                break;
            }
        }

        if (!found) {
            // 创建空答案记录
            StudentAnswer emptyAnswer;
            emptyAnswer.setExamId(examId);
            emptyAnswer.setQuestionId(question.getQuestionId());
            emptyAnswer.setStudentId(studentId);
            emptyAnswer.setStudentAnswer("");
            emptyAnswer.setScore(0);
            emptyAnswer.setIsGraded(true);
            emptyAnswer.setQuestionContent(question.getContent());
            emptyAnswer.setCorrectAnswer(question.getAnswer());
            emptyAnswer.setQuestionType(question.getQuestionType());
            emptyAnswer.setFullScore(question.getScore());
            studentAnswers.append(emptyAnswer);
        }
    }
}

void ExamResult::calculateScores()
{
    totalScore = 0;
    maxScore = 0;
    correctCount = 0;
    totalCount = studentAnswers.size();
    objectiveScore = 0;
    subjectiveScore = 0;
    objectiveMaxScore = 0;
    subjectiveMaxScore = 0;

    for (const StudentAnswer &answer : studentAnswers) {
        totalScore += answer.getScore();
        maxScore += answer.getFullScore();

        if (answer.isCorrect()) {
            correctCount++;
        }

        // 分类统计客观题和主观题
        QString questionType = answer.getQuestionType();
        if (questionType == "单选" || questionType == "多选" || questionType == "填空") {
            // 客观题
            objectiveScore += answer.getScore();
            objectiveMaxScore += answer.getFullScore();
        } else {
            // 主观题
            subjectiveScore += answer.getScore();
            subjectiveMaxScore += answer.getFullScore();
        }
    }

    if (maxScore > 0) {
        percentage = (totalScore / maxScore) * 100.0;
    }
}

void ExamResult::displayBasicInfo()
{
    examNameLabel->setText(currentExam.getExamName());
    examTimeLabel->setText(currentExam.getTimeRange());
    totalScoreLabel->setText(QString("%1/%2分").arg(QString::number(totalScore, 'f', 1)).arg(QString::number(maxScore, 'f', 0)));

    QString percentageText = QString("%1%").arg(QString::number(percentage, 'f', 1));
    percentageLabel->setText(percentageText);

    // 根据得分率设置颜色
    if (percentage >= 90) {
        percentageLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #4CAF50;"); // 绿色
    } else if (percentage >= 70) {
        percentageLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FF9800;"); // 橙色
    } else if (percentage >= 60) {
        percentageLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2196F3;"); // 蓝色
    } else {
        percentageLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #F44336;"); // 红色
    }

    // 设置进度条
    scoreProgressBar->setValue(static_cast<int>(percentage));
    if (percentage >= 80) {
        scoreProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #4CAF50; }");
    } else if (percentage >= 60) {
        scoreProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #FF9800; }");
    } else {
        scoreProgressBar->setStyleSheet("QProgressBar::chunk { background-color: #F44336; }");
    }
}

void ExamResult::displayAnswerDetails()
{
    detailTable->setRowCount(studentAnswers.size());

    for (int i = 0; i < studentAnswers.size(); ++i) {
        const StudentAnswer &answer = studentAnswers[i];

        // 题号
        detailTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));

        // 题型
        detailTable->setItem(i, 1, new QTableWidgetItem(answer.getQuestionType()));

        // 分值
        detailTable->setItem(i, 2, new QTableWidgetItem(QString::number(answer.getFullScore())));

        // 我的答案
        QString myAnswer = answer.getStudentAnswer();
        if (myAnswer.isEmpty()) {
            myAnswer = "未作答";
        }
        // 如果答案太长，截断显示
        if (myAnswer.length() > 50) {
            myAnswer = myAnswer.left(47) + "...";
        }
        detailTable->setItem(i, 3, new QTableWidgetItem(myAnswer));

        // 正确答案
        QString correctAnswer = answer.getCorrectAnswer();
        if (correctAnswer.length() > 50) {
            correctAnswer = correctAnswer.left(47) + "...";
        }
        detailTable->setItem(i, 4, new QTableWidgetItem(correctAnswer));

        // 得分
        detailTable->setItem(i, 5, new QTableWidgetItem(answer.getScoreText()));

        // 状态
        QString status = answer.getGradingStatus();
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);

        // 根据状态设置颜色
        if (status == "正确") {
            statusItem->setBackground(QColor(144, 238, 144)); // 浅绿色
            statusItem->setIcon(QIcon(":/icons/correct.png")); // 可选：添加图标
        } else if (status == "部分正确") {
            statusItem->setBackground(QColor(255, 255, 0)); // 黄色
        } else if (status == "错误" || status == "未作答") {
            statusItem->setBackground(QColor(255, 182, 193)); // 浅红色
        } else {
            statusItem->setBackground(QColor(220, 220, 220)); // 灰色
        }

        detailTable->setItem(i, 6, statusItem);
    }

    // 调整列宽
    detailTable->resizeColumnsToContents();
    detailTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    detailTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
}

void ExamResult::displayStatistics()
{
    correctCountLabel->setText(QString("%1/%2").arg(correctCount).arg(totalCount));

    if (objectiveMaxScore > 0) {
        objectiveScoreLabel->setText(QString("%1/%2")
                                         .arg(QString::number(objectiveScore, 'f', 1))
                                         .arg(QString::number(objectiveMaxScore, 'f', 0)));
    } else {
        objectiveScoreLabel->setText("无客观题");
    }

    if (subjectiveMaxScore > 0) {
        subjectiveScoreLabel->setText(QString("%1/%2")
                                          .arg(QString::number(subjectiveScore, 'f', 1))
                                          .arg(QString::number(subjectiveMaxScore, 'f', 0)));
    } else {
        subjectiveScoreLabel->setText("无主观题");
    }
}

void ExamResult::onPrintResult()
{
    QMessageBox::information(this, "提示", "打印功能开发中...\n您可以使用浏览器的打印功能打印此页面。");
    // TODO: 实现打印功能
}

void ExamResult::onBackToList()
{
    this->close();
}
