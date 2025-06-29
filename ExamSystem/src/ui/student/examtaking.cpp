#include "examtaking.h"
#include <QDateTime>
#include <QApplication>
#include <QMessageBox>
#include <QRegularExpression>
#include "studentanswer.h"

ExamTaking::ExamTaking(Database *database, int examId, int studentId, QWidget *parent)
    : QWidget(parent), database(database), examId(examId), studentId(studentId),
    currentQuestionIndex(0), isAnswerChanged(false), isExamSubmitted(false)
{
    // 检查考试是否已提交
    if (database && database->hasStudentSubmittedExam(studentId, examId)) {
        QMessageBox::warning(this, "提示", "您已经提交过此考试，无法再次参与！");
        this->close();
        return;
    }

    setupUI();
    loadExamData();

    // 设置定时器
    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &ExamTaking::updateCountdown);
    countdownTimer->start(1000);

    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &ExamTaking::autoSaveAnswers);
    autoSaveTimer->start(30000);
}

void ExamTaking::setupUI()
{
    setWindowTitle("在线考试");
    setMinimumSize(1200, 800);

    mainLayout = new QVBoxLayout(this);

    // 头部信息区域
    headerLayout = new QHBoxLayout;

    examInfoLabel = new QLabel;
    examInfoLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    countdownLabel = new QLabel;
    countdownLabel->setStyleSheet("color: red; font-weight: bold; font-size: 16px;");

    progressLabel = new QLabel;
    progressLabel->setStyleSheet("font-weight: bold;");

    submitButton = new QPushButton("提交考试");
    submitButton->setStyleSheet("background-color: #ff6b6b; color: white; font-weight: bold;");

    headerLayout->addWidget(examInfoLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(progressLabel);
    headerLayout->addWidget(countdownLabel);
    headerLayout->addWidget(submitButton);

    mainLayout->addLayout(headerLayout);

    // 主内容区域（分割布局）
    contentSplitter = new QSplitter(Qt::Horizontal);

    // 左侧导航区域（20%）
    navigationWidget = new QWidget;
    navigationWidget->setMaximumWidth(250);
    navigationWidget->setMinimumWidth(200);
    navigationLayout = new QVBoxLayout(navigationWidget);

    QLabel *navTitle = new QLabel("题目导航");
    navTitle->setStyleSheet("font-weight: bold; padding: 5px;");
    navTitle->setAlignment(Qt::AlignCenter);

    questionNavList = new QListWidget;
    questionNavList->setFixedHeight(400);

    QHBoxLayout *navButtonLayout = new QHBoxLayout;
    prevButton = new QPushButton("上一题");
    nextButton = new QPushButton("下一题");
    navButtonLayout->addWidget(prevButton);
    navButtonLayout->addWidget(nextButton);

    navigationLayout->addWidget(navTitle);
    navigationLayout->addWidget(questionNavList);
    navigationLayout->addLayout(navButtonLayout);
    navigationLayout->addStretch();

    // 右侧答题区域（80%）
    answerWidget = new QWidget;
    answerLayout = new QVBoxLayout(answerWidget);

    questionInfoLabel = new QLabel;
    questionInfoLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 10px;");

    questionContentLabel = new QLabel;
    questionContentLabel->setWordWrap(true);
    questionContentLabel->setStyleSheet("font-size: 13px; padding: 10px; border: 1px solid #ccc; background-color: #f9f9f9;");
    questionContentLabel->setMinimumHeight(100);

    // 答题区域使用滚动区域
    answerScrollArea = new QScrollArea;
    answerScrollArea->setWidgetResizable(true);
    answerScrollArea->setMinimumHeight(200);

    QHBoxLayout *answerButtonLayout = new QHBoxLayout;
    saveAnswerButton = new QPushButton("保存答案");
    markButton = new QPushButton("标记");
    answerButtonLayout->addWidget(saveAnswerButton);
    answerButtonLayout->addWidget(markButton);
    answerButtonLayout->addStretch();

    answerLayout->addWidget(questionInfoLabel);
    answerLayout->addWidget(questionContentLabel);
    answerLayout->addWidget(answerScrollArea);
    answerLayout->addLayout(answerButtonLayout);

    // 添加到分割器
    contentSplitter->addWidget(navigationWidget);
    contentSplitter->addWidget(answerWidget);
    contentSplitter->setStretchFactor(0, 1);
    contentSplitter->setStretchFactor(1, 4);

    mainLayout->addWidget(contentSplitter);

    // 连接信号
    connect(questionNavList, &QListWidget::itemClicked, this, &ExamTaking::onQuestionNavigationClicked);
    connect(prevButton, &QPushButton::clicked, this, &ExamTaking::onPreviousQuestion);
    connect(nextButton, &QPushButton::clicked, this, &ExamTaking::onNextQuestion);
    connect(saveAnswerButton, &QPushButton::clicked, this, &ExamTaking::onSaveAnswer);
    connect(submitButton, &QPushButton::clicked, this, &ExamTaking::onSubmitExam);
}

void ExamTaking::loadExamData()
{
    if (!database) {
        qDebug() << "数据库连接为空";
        return;
    }

    // 获取考试信息
    currentExam = database->getExamById(examId);

    if (currentExam.getExamId() == 0) {
        qDebug() << "获取考试信息失败";
        QMessageBox::critical(this, "错误", "无法获取考试信息！");
        return;
    }

    qDebug() << "成功获取考试信息:" << currentExam.getExamName();

    // 设置考试时间
    examStartTime = QDateTime(currentExam.getExamDate(), currentExam.getStartTime());
    examEndTime = QDateTime(currentExam.getExamDate(), currentExam.getEndTime());

    // 更新头部信息
    examInfoLabel->setText(QString("考试: %1 | 课程: %2")
                               .arg(currentExam.getExamName())
                               .arg(currentExam.getCourseName()));

    loadExamQuestions();
}

void ExamTaking::loadExamQuestions()
{
    qDebug() << "开始加载考试题目...";
    examQuestions = database->getExamQuestions(examId);
    qDebug() << "获取到题目数量:" << examQuestions.size();

    // 加载已有答案
    qDebug() << "开始加载学生答案...";
    QList<StudentAnswer> existingAnswers = database->getStudentAnswers(examId, studentId);
    qDebug() << "获取到答案数量:" << existingAnswers.size();

    for (const StudentAnswer &answer : existingAnswers) {
        studentAnswers[answer.getQuestionId()] = answer.getStudentAnswer();
    }
    qDebug() << "答案加载完成";

    // 填充导航列表
    qDebug() << "开始填充导航列表...";
    questionNavList->clear();
    for (int i = 0; i < examQuestions.size(); ++i) {
        const Question &question = examQuestions[i];
        qDebug() << "处理题目" << i << "ID:" << question.getQuestionId() << "类型:" << question.getQuestionType();

        QString navText = QString("%1. %2").arg(i + 1).arg(question.getQuestionType());
        QListWidgetItem *item = new QListWidgetItem(navText);

        // 根据答题状态设置颜色
        if (studentAnswers.contains(question.getQuestionId()) &&
            !studentAnswers[question.getQuestionId()].isEmpty()) {
            item->setBackground(QColor(144, 238, 144)); // 已答题(绿色)
        } else {
            item->setBackground(QColor(220, 220, 220)); // 未答题(灰色)
        }

        questionNavList->addItem(item);
    }
    qDebug() << "导航列表填充完成";

    if (!examQuestions.isEmpty()) {
        qDebug() << "开始显示第一题...";
        displayQuestion(0);
        qDebug() << "第一题显示完成";
    }

    qDebug() << "开始更新进度...";
    updateQuestionProgress();
    qDebug() << "进度更新完成";
    qDebug() << "loadExamQuestions 完成";
}

void ExamTaking::displayQuestion(int index)
{
    if (index < 0 || index >= examQuestions.size()) return;

    // 保存当前答案
    if (currentQuestionIndex != index && currentQuestionIndex < examQuestions.size()) {
        saveCurrentAnswer();
    }

    currentQuestionIndex = index;
    const Question &question = examQuestions[index];

    // 更新题目信息
    questionInfoLabel->setText(QString("第%1题 (%2 %3分)")
                                   .arg(index + 1)
                                   .arg(question.getQuestionType())
                                   .arg(question.getScore()));

    questionContentLabel->setText(question.getContent());

    // 创建答题控件
    createAnswerWidget(question);

    // 设置已有答案
    if (studentAnswers.contains(question.getQuestionId())) {
        setCurrentAnswer(studentAnswers[question.getQuestionId()]);
    }

    // 更新导航按钮状态
    updateNavigationButtons();

    // 高亮当前题目
    questionNavList->setCurrentRow(index);

    isAnswerChanged = false;
}

void ExamTaking::createAnswerWidget(const Question &question)
{
    // 清理之前的控件
    if (answerInputWidget) {
        answerScrollArea->takeWidget();
        delete answerInputWidget;
    }

    answerInputWidget = new QWidget;
    QVBoxLayout *inputLayout = new QVBoxLayout(answerInputWidget);

    QString questionType = question.getQuestionType();

    if (questionType == "单选") {
        // 单选题
        radioGroup = new QButtonGroup(this);
        QStringList options = question.getContent().split('\n');

        for (const QString &option : options) {
            QString trimmedOption = option.trimmed();
            if (trimmedOption.length() >= 2 && trimmedOption[0].isLetter() && trimmedOption[1] == '.') {
                QRadioButton *radio = new QRadioButton(trimmedOption);
                connect(radio, &QRadioButton::toggled, this, &ExamTaking::onAnswerChanged);
                radioGroup->addButton(radio);
                inputLayout->addWidget(radio);
            }
        }

    } else if (questionType == "多选") {
        // 多选题
        checkBoxes.clear();
        QStringList options = question.getContent().split('\n');

        for (const QString &option : options) {
            QString trimmedOption = option.trimmed();
            if (trimmedOption.length() >= 2 && trimmedOption[0].isLetter() && trimmedOption[1] == '.') {
                QCheckBox *checkBox = new QCheckBox(trimmedOption);
                connect(checkBox, &QCheckBox::toggled, this, &ExamTaking::onAnswerChanged);
                checkBoxes.append(checkBox);
                inputLayout->addWidget(checkBox);
            }
        }

    } else if (questionType == "填空") {
        // 填空题
        QLabel *answerLabel = new QLabel("答案:");
        lineEdit = new QLineEdit;
        lineEdit->setPlaceholderText("请输入答案");
        connect(lineEdit, &QLineEdit::textChanged, this, &ExamTaking::onAnswerChanged);

        inputLayout->addWidget(answerLabel);
        inputLayout->addWidget(lineEdit);

    } else {
        // 主观题（简答、论述、计算）
        QLabel *answerLabel = new QLabel("答案:");
        textEdit = new QTextEdit;
        textEdit->setPlaceholderText("请输入答案");
        textEdit->setMinimumHeight(150);
        connect(textEdit, &QTextEdit::textChanged, this, &ExamTaking::onAnswerChanged);

        inputLayout->addWidget(answerLabel);
        inputLayout->addWidget(textEdit);
    }

    inputLayout->addStretch();
    answerScrollArea->setWidget(answerInputWidget);
}

QString ExamTaking::getCurrentAnswer()
{
    if (currentQuestionIndex >= examQuestions.size()) return QString();

    const Question &question = examQuestions[currentQuestionIndex];
    QString questionType = question.getQuestionType();

    if (questionType == "单选") {
        if (radioGroup) {
            QAbstractButton *checked = radioGroup->checkedButton();
            if (checked) {
                QString text = checked->text();
                if (text.length() > 0) {
                    return QString(text[0]); // 返回选项字母
                }
            }
        }
    } else if (questionType == "多选") {
        QString answer;
        for (QCheckBox *checkBox : checkBoxes) {
            if (checkBox->isChecked()) {
                QString text = checkBox->text();
                if (text.length() > 0) {
                    answer += text[0]; // 累加选项字母
                }
            }
        }
        return answer;
    } else if (questionType == "填空") {
        return lineEdit ? lineEdit->text() : QString();
    } else {
        return textEdit ? textEdit->toPlainText() : QString();
    }

    return QString();
}

void ExamTaking::setCurrentAnswer(const QString &answer)
{
    if (currentQuestionIndex >= examQuestions.size()) return;

    const Question &question = examQuestions[currentQuestionIndex];
    QString questionType = question.getQuestionType();

    if (questionType == "单选") {
        if (radioGroup && answer.length() == 1) {
            QList<QAbstractButton*> buttons = radioGroup->buttons();
            for (QAbstractButton *button : buttons) {
                if (button->text().startsWith(answer[0])) {
                    button->setChecked(true);
                    break;
                }
            }
        }
    } else if (questionType == "多选") {
        for (QCheckBox *checkBox : checkBoxes) {
            QString text = checkBox->text();
            if (text.length() > 0 && answer.contains(text[0])) {
                checkBox->setChecked(true);
            }
        }
    } else if (questionType == "填空") {
        if (lineEdit) {
            lineEdit->setText(answer);
        }
    } else {
        if (textEdit) {
            textEdit->setPlainText(answer);
        }
    }
}

void ExamTaking::saveCurrentAnswer()
{
    if (currentQuestionIndex >= examQuestions.size()) return;

    const Question &question = examQuestions[currentQuestionIndex];
    QString answer = getCurrentAnswer();

    studentAnswers[question.getQuestionId()] = answer;

    // 保存到数据库
    database->saveStudentAnswer(examId, question.getQuestionId(), studentId, answer);

    // 更新导航列表颜色
    QListWidgetItem *item = questionNavList->item(currentQuestionIndex);
    if (item) {
        if (!answer.isEmpty()) {
            item->setBackground(QColor(144, 238, 144)); // 已答题(绿色)
        } else {
            item->setBackground(QColor(220, 220, 220)); // 未答题(灰色)
        }
    }

    updateQuestionProgress();
    isAnswerChanged = false;
}

void ExamTaking::updateNavigationButtons()
{
    prevButton->setEnabled(currentQuestionIndex > 0);
    nextButton->setEnabled(currentQuestionIndex < examQuestions.size() - 1);
}

void ExamTaking::updateQuestionProgress()
{
    int answered = 0;
    for (const QString &answer : studentAnswers.values()) {
        if (!answer.isEmpty()) {
            answered++;
        }
    }

    progressLabel->setText(QString("已答题: %1/%2").arg(answered).arg(examQuestions.size()));
}

void ExamTaking::updateCountdown()
{
    QDateTime now = QDateTime::currentDateTime();

    if (now >= examEndTime) {
        // 时间到，自动提交
        autoSubmitExam();
        return;
    }

    qint64 remainingSeconds = now.secsTo(examEndTime);
    QTime remainingTime = QTime(0, 0).addSecs(remainingSeconds);

    QString timeText = QString("剩余时间: %1").arg(remainingTime.toString("hh:mm:ss"));

    // 最后5分钟变红色警告
    if (remainingSeconds <= 300) {
        countdownLabel->setStyleSheet("color: red; font-weight: bold; font-size: 18px;");
    }

    countdownLabel->setText(timeText);
}

void ExamTaking::onQuestionNavigationClicked()
{
    int newIndex = questionNavList->currentRow();
    if (newIndex >= 0 && newIndex < examQuestions.size()) {
        displayQuestion(newIndex);
    }
}

void ExamTaking::onAnswerChanged()
{
    isAnswerChanged = true;
}

void ExamTaking::onSaveAnswer()
{
    saveCurrentAnswer();
    QMessageBox::information(this, "提示", "答案已保存");
}

void ExamTaking::onPreviousQuestion()
{
    if (currentQuestionIndex > 0) {
        displayQuestion(currentQuestionIndex - 1);
    }
}

void ExamTaking::onNextQuestion()
{
    if (currentQuestionIndex < examQuestions.size() - 1) {
        displayQuestion(currentQuestionIndex + 1);
    }
}

void ExamTaking::onSubmitExam()
{
    submitExamConfirm();
}

void ExamTaking::autoSaveAnswers()
{
    if (isAnswerChanged) {
        saveCurrentAnswer();
    }
}

void ExamTaking::submitExamConfirm()
{
    // 先保存当前答案
    saveCurrentAnswer();

    int answered = 0;
    for (const QString &answer : studentAnswers.values()) {
        if (!answer.isEmpty()) {
            answered++;
        }
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "提交考试",
        QString("确定要提交考试吗？\n已答题: %1/%2\n提交后将无法修改答案。")
            .arg(answered).arg(examQuestions.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 提交考试
        database->submitExam(examId, studentId);
        isExamSubmitted = true;

        QMessageBox::information(this, "提示", "考试提交成功！客观题已自动批改。");
        closeExam();
    }
}

void ExamTaking::autoSubmitExam()
{
    // 时间到自动提交
    saveCurrentAnswer();
    database->submitExam(examId, studentId);
    isExamSubmitted = true;

    QMessageBox::warning(this, "时间到", "考试时间已到，系统已自动提交您的答案。");
    closeExam();
}

void ExamTaking::closeExam()
{
    countdownTimer->stop();
    autoSaveTimer->stop();
    this->close();
}
