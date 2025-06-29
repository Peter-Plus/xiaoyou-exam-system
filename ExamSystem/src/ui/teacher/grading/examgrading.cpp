#include "examgrading.h"

ExamGrading::ExamGrading(Database *database, int teacherId, QWidget *parent)
    : QWidget(parent), database(database), teacherId(teacherId),
    currentExamId(-1), currentAnswerIndex(-1), isGradingChanged(false)
{
    setupUI();
    loadExamList();
}

void ExamGrading::setupUI()
{
    setWindowTitle("考试阅卷");
    setMinimumSize(1200, 800);

    mainLayout = new QVBoxLayout(this);

    // 顶部信息栏
    topLayout = new QHBoxLayout;

    QLabel *examLabel = new QLabel("选择考试:");
    examComboBox = new QComboBox;
    examComboBox->setMinimumWidth(200);

    examInfoLabel = new QLabel;
    examInfoLabel->setStyleSheet("font-weight: bold; color: #2196F3;");

    progressLabel = new QLabel;
    progressLabel->setStyleSheet("font-weight: bold;");

    refreshButton = new QPushButton("刷新");

    topLayout->addWidget(examLabel);
    topLayout->addWidget(examComboBox);
    topLayout->addWidget(examInfoLabel);
    topLayout->addStretch();
    topLayout->addWidget(progressLabel);
    topLayout->addWidget(refreshButton);

    mainLayout->addLayout(topLayout);

    // 主内容区域（分割布局）
    mainSplitter = new QSplitter(Qt::Horizontal);

    // 左侧题目筛选区域（25%）
    filterWidget = new QWidget;
    filterWidget->setMaximumWidth(300);
    filterWidget->setMinimumWidth(250);
    filterLayout = new QVBoxLayout(filterWidget);

    QLabel *filterTitle = new QLabel("题目筛选");
    filterTitle->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    filterTitle->setAlignment(Qt::AlignCenter);

    questionTypeTree = new QTreeWidget;
    questionTypeTree->setHeaderLabel("题目分类");
    questionTypeTree->setMaximumHeight(400);

    filterInfoLabel = new QLabel;
    filterInfoLabel->setWordWrap(true);
    filterInfoLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border: 1px solid #ccc;");

    filterLayout->addWidget(filterTitle);
    filterLayout->addWidget(questionTypeTree);
    filterLayout->addWidget(filterInfoLabel);
    filterLayout->addStretch();

    // 右侧批改区域（75%）
    gradingWidget = new QWidget;
    gradingLayout = new QVBoxLayout(gradingWidget);

    // 学生信息区域
    studentInfoGroup = new QGroupBox("当前批改");
    studentInfoLayout = new QHBoxLayout(studentInfoGroup);

    studentNameLabel = new QLabel("请选择考试和题目");
    studentNameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    questionInfoLabel = new QLabel;
    questionInfoLabel->setStyleSheet("font-weight: bold; color: #666;");

    prevAnswerButton = new QPushButton("上一学生");
    nextAnswerButton = new QPushButton("下一学生");

    studentInfoLayout->addWidget(studentNameLabel);
    studentInfoLayout->addWidget(questionInfoLabel);
    studentInfoLayout->addStretch();
    studentInfoLayout->addWidget(prevAnswerButton);
    studentInfoLayout->addWidget(nextAnswerButton);

    // 题目内容区域
    questionGroup = new QGroupBox("题目内容");
    questionLayout = new QVBoxLayout(questionGroup);

    questionScrollArea = new QScrollArea;
    questionScrollArea->setWidgetResizable(true);
    questionScrollArea->setMaximumHeight(150);

    questionContentLabel = new QLabel;
    questionContentLabel->setWordWrap(true);
    questionContentLabel->setStyleSheet("padding: 10px; background-color: #f9f9f9;");
    questionScrollArea->setWidget(questionContentLabel);

    QLabel *refAnswerTitle = new QLabel("参考答案:");
    refAnswerTitle->setStyleSheet("font-weight: bold; margin-top: 10px;");

    referenceAnswerLabel = new QLabel;
    referenceAnswerLabel->setWordWrap(true);
    referenceAnswerLabel->setStyleSheet("padding: 10px; background-color: #e8f5e8; border: 1px solid #4CAF50;");

    questionLayout->addWidget(questionScrollArea);
    questionLayout->addWidget(refAnswerTitle);
    questionLayout->addWidget(referenceAnswerLabel);

    // 学生答案区域
    answerGroup = new QGroupBox("学生答案");
    answerLayout = new QVBoxLayout(answerGroup);

    answerScrollArea = new QScrollArea;
    answerScrollArea->setWidgetResizable(true);
    answerScrollArea->setMaximumHeight(150);

    studentAnswerLabel = new QLabel;
    studentAnswerLabel->setWordWrap(true);
    studentAnswerLabel->setStyleSheet("padding: 10px; background-color: #fff9c4; border: 1px solid #FFC107;");
    answerScrollArea->setWidget(studentAnswerLabel);

    answerLayout->addWidget(answerScrollArea);

    // 批改区域
    gradingGroup = new QGroupBox("评分");
    gradingControlLayout = new QHBoxLayout(gradingGroup);

    scoreLabel = new QLabel("得分:");
    scoreSpinBox = new QDoubleSpinBox;
    scoreSpinBox->setMinimum(0);
    scoreSpinBox->setMaximum(100);
    scoreSpinBox->setDecimals(1);
    scoreSpinBox->setSuffix(" 分");

    maxScoreLabel = new QLabel("/ 0 分");
    maxScoreLabel->setStyleSheet("font-weight: bold;");

    QLabel *commentLabel = new QLabel("评语:");
    commentEdit = new QLineEdit;
    commentEdit->setPlaceholderText("选填：给学生的评语或建议");

    saveGradingButton = new QPushButton("保存评分");
    saveGradingButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");

    batchButton = new QPushButton("批量给分");
    batchButton->setStyleSheet("background-color: #2196F3; color: white;");

    gradingControlLayout->addWidget(scoreLabel);
    gradingControlLayout->addWidget(scoreSpinBox);
    gradingControlLayout->addWidget(maxScoreLabel);
    gradingControlLayout->addWidget(commentLabel);
    gradingControlLayout->addWidget(commentEdit);
    gradingControlLayout->addWidget(saveGradingButton);
    gradingControlLayout->addWidget(batchButton);

    // 进度条
    gradingProgressBar = new QProgressBar;
    gradingProgressBar->setTextVisible(true);

    // 添加所有组件到右侧布局
    gradingLayout->addWidget(studentInfoGroup);
    gradingLayout->addWidget(questionGroup);
    gradingLayout->addWidget(answerGroup);
    gradingLayout->addWidget(gradingGroup);
    gradingLayout->addWidget(gradingProgressBar);

    // 添加到分割器
    mainSplitter->addWidget(filterWidget);
    mainSplitter->addWidget(gradingWidget);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 3);

    mainLayout->addWidget(mainSplitter);

    // 连接信号
    connect(examComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExamGrading::onExamSelectionChanged);
    connect(questionTypeTree, &QTreeWidget::itemClicked,
            this, &ExamGrading::onQuestionTypeSelected);
    connect(prevAnswerButton, &QPushButton::clicked,
            this, &ExamGrading::onPreviousAnswer);
    connect(nextAnswerButton, &QPushButton::clicked,
            this, &ExamGrading::onNextAnswer);
    connect(saveGradingButton, &QPushButton::clicked,
            this, &ExamGrading::onSaveGrading);
    connect(batchButton, &QPushButton::clicked,
            this, &ExamGrading::onBatchGrading);
    connect(refreshButton, &QPushButton::clicked,
            this, &ExamGrading::refreshGradingData);
    connect(scoreSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](){ isGradingChanged = true; });
    connect(commentEdit, &QLineEdit::textChanged,
            [this](){ isGradingChanged = true; });

    // 初始状态
    gradingWidget->setEnabled(false);
}

void ExamGrading::loadExamList()
{
    if (!database) return;

    gradingExams = database->getExamsForGrading(teacherId);

    examComboBox->clear();
    examComboBox->addItem("请选择考试", -1);

    for (const Exam &exam : gradingExams) {
        QString itemText = QString("%1 - %2").arg(exam.getExamName()).arg(exam.getCourseName());
        examComboBox->addItem(itemText, exam.getExamId());
    }
}

void ExamGrading::onExamSelectionChanged()
{
    int examId = examComboBox->currentData().toInt();
    if (examId <= 0) {
        gradingWidget->setEnabled(false);
        return;
    }

    currentExamId = examId;

    // 找到选中的考试
    for (const Exam &exam : gradingExams) {
        if (exam.getExamId() == examId) {
            examInfoLabel->setText(QString("参考学生: %1人")
                                       .arg(database->getExamStatistics(examId)["student_count"].toInt()));
            break;
        }
    }

    loadQuestionTypes();
    gradingWidget->setEnabled(true);
}

void ExamGrading::loadQuestionTypes()
{
    questionTypeTree->clear();

    if (currentExamId <= 0) return;

    // 获取阅卷进度
    QMap<QString, int> progress = database->getGradingProgress(currentExamId);

    // 创建根节点
    QTreeWidgetItem *allItem = new QTreeWidgetItem(questionTypeTree);
    int totalAnswers = 0;
    int gradedAnswers = 0;

    // 遍历进度数据，按题型分组
    QMap<QString, QPair<int, int>> typeStats; // 题型 -> (总数, 已批改数)

    for (auto it = progress.begin(); it != progress.end(); ++it) {
        QString key = it.key();
        int value = it.value();

        if (key.endsWith("_total")) {
            QString type = key.left(key.length() - 6);
            typeStats[type].first = value;
            totalAnswers += value;
        } else if (key.endsWith("_graded")) {
            QString type = key.left(key.length() - 7);
            typeStats[type].second = value;
            gradedAnswers += value;
        }
    }

    allItem->setText(0, QString("全部题目 (%1/%2)").arg(gradedAnswers).arg(totalAnswers));
    allItem->setData(0, Qt::UserRole, QString(""));

    // 分类显示
    QTreeWidgetItem *objectiveParent = new QTreeWidgetItem(allItem);
    objectiveParent->setText(0, "客观题");
    objectiveParent->setBackground(0, QColor(144, 238, 144));

    QTreeWidgetItem *subjectiveParent = new QTreeWidgetItem(allItem);
    subjectiveParent->setText(0, "主观题");
    subjectiveParent->setBackground(0, QColor(255, 182, 193));

    for (auto it = typeStats.begin(); it != typeStats.end(); ++it) {
        QString type = it.key();
        int total = it.value().first;
        int graded = it.value().second;

        if (total == 0) continue;

        QTreeWidgetItem *typeItem = new QTreeWidgetItem;
        typeItem->setText(0, QString("%1 (%2/%3)").arg(type).arg(graded).arg(total));
        typeItem->setData(0, Qt::UserRole, type);

        // 根据进度设置颜色
        if (graded == total) {
            typeItem->setBackground(0, QColor(144, 238, 144)); // 已完成-绿色
        } else if (graded > 0) {
            typeItem->setBackground(0, QColor(255, 255, 0)); // 进行中-黄色
        } else {
            typeItem->setBackground(0, QColor(255, 182, 193)); // 未开始-红色
        }

        // 分类到客观题或主观题
        if (type == "单选" || type == "多选" || type == "填空") {
            objectiveParent->addChild(typeItem);
        } else {
            subjectiveParent->addChild(typeItem);
        }
    }

    questionTypeTree->expandAll();

    // 更新总进度
    updateProgressInfo();
}

void ExamGrading::onQuestionTypeSelected()
{
    QTreeWidgetItem *item = questionTypeTree->currentItem();
    if (!item) return;

    currentQuestionType = item->data(0, Qt::UserRole).toString();
    loadAnswersForGrading();
}

void ExamGrading::loadAnswersForGrading()
{
    if (currentExamId <= 0) return;

    currentAnswers = database->getAnswersForGrading(currentExamId, currentQuestionType);
    currentAnswerIndex = -1;

    // 更新筛选信息
    QString filterInfo;
    if (currentQuestionType.isEmpty()) {
        filterInfo = QString("显示全部题目的答案\n共 %1 个答案").arg(currentAnswers.size());
    } else {
        filterInfo = QString("显示 %1 题的答案\n共 %2 个答案").arg(currentQuestionType).arg(currentAnswers.size());

        // 统计批改状态
        int graded = 0;
        for (const StudentAnswer &answer : currentAnswers) {
            if (answer.getIsGraded()) graded++;
        }
        filterInfo += QString("\n已批改: %1\n待批改: %2").arg(graded).arg(currentAnswers.size() - graded);
    }
    filterInfoLabel->setText(filterInfo);

    if (!currentAnswers.isEmpty()) {
        // 找到第一个未批改的答案
        for (int i = 0; i < currentAnswers.size(); ++i) {
            if (!currentAnswers[i].getIsGraded()) {
                currentAnswerIndex = i;
                break;
            }
        }

        // 如果没有未批改的，显示第一个
        if (currentAnswerIndex == -1) {
            currentAnswerIndex = 0;
        }

        displayCurrentAnswer();
    } else {
        studentNameLabel->setText("暂无答案需要批改");
        questionContentLabel->clear();
        referenceAnswerLabel->clear();
        studentAnswerLabel->clear();
    }

    updateNavigationButtons();
}

void ExamGrading::displayCurrentAnswer()
{
    if (currentAnswerIndex < 0 || currentAnswerIndex >= currentAnswers.size()) return;

    const StudentAnswer &answer = currentAnswers[currentAnswerIndex];

    // 学生信息
    studentNameLabel->setText(QString("学生: %1").arg(answer.getStudentName()));
    questionInfoLabel->setText(QString("第%1题 (%2 %3分)")
                                   .arg(currentAnswerIndex + 1)
                                   .arg(answer.getQuestionType())
                                   .arg(answer.getFullScore()));

    // 题目内容
    questionContentLabel->setText(answer.getQuestionContent());
    referenceAnswerLabel->setText(answer.getCorrectAnswer());

    // 学生答案
    QString studentAnswer = answer.getStudentAnswer();
    if (studentAnswer.isEmpty()) {
        studentAnswerLabel->setText("学生未作答");
        studentAnswerLabel->setStyleSheet("padding: 10px; background-color: #ffebee; border: 1px solid #f44336; font-style: italic;");
    } else {
        studentAnswerLabel->setText(studentAnswer);
        studentAnswerLabel->setStyleSheet("padding: 10px; background-color: #fff9c4; border: 1px solid #FFC107;");
    }

    // 设置评分信息
    scoreSpinBox->setMaximum(answer.getFullScore());
    scoreSpinBox->setValue(answer.getScore());
    maxScoreLabel->setText(QString("/ %1 分").arg(answer.getFullScore()));

    // 清空评语（如果需要保留评语，可以从数据库获取）
    commentEdit->clear();

    // 如果已批改，显示状态
    if (answer.getIsGraded()) {
        saveGradingButton->setText("更新评分");
    } else {
        saveGradingButton->setText("保存评分");
    }

    isGradingChanged = false;
    updateNavigationButtons();
}

void ExamGrading::updateNavigationButtons()
{
    prevAnswerButton->setEnabled(currentAnswerIndex > 0);
    nextAnswerButton->setEnabled(currentAnswerIndex < currentAnswers.size() - 1);

    if (currentAnswers.size() > 0) {
        QString navText = QString("(%1/%2)").arg(currentAnswerIndex + 1).arg(currentAnswers.size());
        prevAnswerButton->setText("上一学生 " + navText);
        nextAnswerButton->setText("下一学生 " + navText);
    }
}

void ExamGrading::updateProgressInfo()
{
    if (currentExamId <= 0) return;

    QMap<QString, int> progress = database->getGradingProgress(currentExamId);
    int totalAnswers = 0;
    int gradedAnswers = 0;

    for (auto it = progress.begin(); it != progress.end(); ++it) {
        if (it.key().endsWith("_total")) {
            totalAnswers += it.value();
        } else if (it.key().endsWith("_graded")) {
            gradedAnswers += it.value();
        }
    }

    if (totalAnswers > 0) {
        int percentage = (gradedAnswers * 100) / totalAnswers;
        progressLabel->setText(QString("阅卷进度: %1/%2 (%3%)")
                                   .arg(gradedAnswers).arg(totalAnswers).arg(percentage));
        gradingProgressBar->setMaximum(totalAnswers);
        gradingProgressBar->setValue(gradedAnswers);
    } else {
        progressLabel->setText("暂无答案");
        gradingProgressBar->setValue(0);
    }
}

void ExamGrading::onPreviousAnswer()
{
    if (currentAnswerIndex > 0) {
        saveCurrentGrading();
        currentAnswerIndex--;
        displayCurrentAnswer();
    }
}

void ExamGrading::onNextAnswer()
{
    if (currentAnswerIndex < currentAnswers.size() - 1) {
        saveCurrentGrading();
        currentAnswerIndex++;
        displayCurrentAnswer();
    }
}

void ExamGrading::saveCurrentGrading()
{
    if (!isGradingChanged || currentAnswerIndex < 0 || currentAnswerIndex >= currentAnswers.size()) {
        return;
    }

    const StudentAnswer &answer = currentAnswers[currentAnswerIndex];
    double score = scoreSpinBox->value();

    database->updateAnswerScore(answer.getExamId(), answer.getQuestionId(),
                                answer.getStudentId(), score, true);

    isGradingChanged = false;
}

void ExamGrading::onSaveGrading()
{
    if (currentAnswerIndex < 0 || currentAnswerIndex >= currentAnswers.size()) return;

    const StudentAnswer &answer = currentAnswers[currentAnswerIndex];
    double score = scoreSpinBox->value();

    bool success = database->updateAnswerScore(answer.getExamId(), answer.getQuestionId(),
                                               answer.getStudentId(), score, true);

    if (success) {
        QMessageBox::information(this, "提示", "评分保存成功！");

        // 更新当前答案状态
        currentAnswers[currentAnswerIndex].setScore(score);
        currentAnswers[currentAnswerIndex].setIsGraded(true);

        // 自动跳转到下一个未批改的答案
        for (int i = currentAnswerIndex + 1; i < currentAnswers.size(); ++i) {
            if (!currentAnswers[i].getIsGraded()) {
                currentAnswerIndex = i;
                displayCurrentAnswer();
                updateProgressInfo();
                loadQuestionTypes(); // 刷新进度显示
                return;
            }
        }

        QMessageBox::information(this, "提示", "当前筛选的答案已全部批改完成！");
        updateProgressInfo();
        loadQuestionTypes();
    } else {
        QMessageBox::warning(this, "错误", "评分保存失败！");
    }

    isGradingChanged = false;
}

void ExamGrading::onBatchGrading()
{
    if (currentAnswers.isEmpty()) return;

    bool ok;
    double batchScore = QInputDialog::getDouble(this, "批量给分",
                                                QString("请输入分数 (0-%1):").arg(scoreSpinBox->maximum()),
                                                0, 0, scoreSpinBox->maximum(), 1, &ok);

    if (!ok) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认批量给分",
        QString("确定要将当前筛选的所有未批改答案都给 %1 分吗？").arg(batchScore),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int count = 0;
        for (StudentAnswer &answer : currentAnswers) {
            if (!answer.getIsGraded()) {
                database->updateAnswerScore(answer.getExamId(), answer.getQuestionId(),
                                            answer.getStudentId(), batchScore, true);
                answer.setScore(batchScore);
                answer.setIsGraded(true);
                count++;
            }
        }

        QMessageBox::information(this, "完成", QString("批量给分完成，共处理 %1 个答案。").arg(count));
        displayCurrentAnswer();
        updateProgressInfo();
        loadQuestionTypes();
    }
}

void ExamGrading::refreshGradingData()
{
    loadExamList();
    if (currentExamId > 0) {
        loadQuestionTypes();
        loadAnswersForGrading();
    }
}
