#include "paperpreview.h"
#include <QApplication>
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QMessageBox>
#include "course.h"

PaperPreview::PaperPreview(int examId, Database* database, QWidget *parent)
    : QDialog(parent), examId(examId), database(database)
{
    setupUI();
    loadExamData();
    generatePreviewContent();
}

void PaperPreview::setupUI()
{
    setWindowTitle("试卷预览");
    setModal(true);
    resize(800, 900);
    setMinimumSize(700, 800);

    mainLayout = new QVBoxLayout(this);

    // 创建滚动区域
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    contentWidget = new QWidget();
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    // 试卷标题
    titleLabel = new QLabel();
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    contentLayout->addWidget(titleLabel);

    // 考试信息
    infoLabel = new QLabel();
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("font-size: 14px; color: #7f8c8d; margin-bottom: 20px;");
    contentLayout->addWidget(infoLabel);

    // 分割线
    QLabel* separator = new QLabel();
    separator->setFixedHeight(2);
    separator->setStyleSheet("background-color: #bdc3c7; margin: 10px 0;");
    contentLayout->addWidget(separator);

    // 试卷内容
    previewTextEdit = new QTextEdit();
    previewTextEdit->setReadOnly(true);
    previewTextEdit->setStyleSheet("QTextEdit { border: none; background: transparent; font-size: 14px; line-height: 1.6; }");
    contentLayout->addWidget(previewTextEdit);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    // 按钮区域
    buttonLayout = new QHBoxLayout();

    printBtn = new QPushButton("打印试卷");
    closeBtn = new QPushButton("关闭");

    printBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px 20px; font-size: 14px; border: none; border-radius: 4px; } QPushButton:hover { background-color: #2980b9; }");
    closeBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 8px 20px; font-size: 14px; border: none; border-radius: 4px; } QPushButton:hover { background-color: #7f8c8d; }");

    buttonLayout->addStretch();
    buttonLayout->addWidget(printBtn);
    buttonLayout->addWidget(closeBtn);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(printBtn, &QPushButton::clicked, this, &PaperPreview::onPrintClicked);
    connect(closeBtn, &QPushButton::clicked, this, &PaperPreview::onCloseClicked);
}

void PaperPreview::loadExamData()
{
    // 获取试卷题目
    examQuestions = database->getExamQuestions(examId);

    // 直接通过SQL查询获取考试信息
    QSqlQuery query;
    query.prepare("SELECT e.*, c.course_name, t.name as teacher_name "
                  "FROM exams e "
                  "INNER JOIN courses c ON e.course_id = c.course_id "
                  "INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
                  "WHERE e.exam_id = ?");
    query.addBindValue(examId);

    if (query.exec() && query.next()) {
        currentExam.setExamId(query.value("exam_id").toInt());
        currentExam.setExamName(query.value("exam_name").toString());
        currentExam.setCourseId(query.value("course_id").toInt());
        currentExam.setExamDate(query.value("exam_date").toDate());
        currentExam.setStartTime(query.value("start_time").toTime());
        currentExam.setEndTime(query.value("end_time").toTime());
        currentExam.setTotalScore(query.value("total_score").toInt());
        currentExam.setIsPublished(query.value("is_published").toBool());
        currentExam.setIsCompleted(query.value("is_completed").toBool());
        currentExam.setCourseName(query.value("course_name").toString());
        currentExam.setTeacherName(query.value("teacher_name").toString());
    } else {
        qDebug() << "获取考试信息失败:" << query.lastError().text();
        // 设置默认值
        currentExam.setExamId(examId);
        currentExam.setExamName("未知考试");
        currentExam.setExamDate(QDate::currentDate());
        currentExam.setStartTime(QTime(9, 0));
        currentExam.setEndTime(QTime(11, 0));
        currentExam.setTotalScore(100);
    }
}

void PaperPreview::generatePreviewContent()
{
    // 设置标题和基本信息
    titleLabel->setText(currentExam.getExamName());

    // 计算实际组卷总分
    int actualTotalScore = 0;
    for (const Question& question : examQuestions) {
        actualTotalScore += question.getScore();
    }

    // 计算考试时长（分钟）
    int durationMinutes = currentExam.getStartTime().secsTo(currentExam.getEndTime()) / 60;

    QString examInfo = QString("考试时间：%1  考试时长：%2分钟  总分：%3分  题目数：%4题")
                           .arg(currentExam.getExamDate().toString("yyyy年MM月dd日"))
                           .arg(durationMinutes)
                           .arg(actualTotalScore)
                           .arg(examQuestions.size());

    infoLabel->setText(examInfo);

    // 生成试卷内容
    QString content = generatePaperContent();
    previewTextEdit->setHtml(content);
}

QString PaperPreview::generatePaperContent()
{
    QString html = "<div style='font-family: SimSun, serif; font-size: 14px; line-height: 1.8;'>";

    // 考生信息栏
    html += "<div style='margin-bottom: 30px; padding: 15px; border: 1px solid #bdc3c7; background-color: #f8f9fa;'>";
    html += "<table width='100%' style='border-collapse: collapse;'>";
    html += "<tr>";
    html += "<td width='50%'>姓名：________________</td>";
    html += "<td width='50%'>学号：________________</td>";
    html += "</tr>";
    html += "<tr>";
    html += "<td>班级：________________</td>";
    html += "<td>座位号：________________</td>";
    html += "</tr>";
    html += "</table>";
    html += "</div>";

    // 考试说明
    html += "<div style='margin-bottom: 25px; padding: 10px; background-color: #fff3cd; border-left: 4px solid #ffc107;'>";
    html += "<p style='margin:5px 0;'>&nbsp;</p>";
    html += "<p style='margin: 0; font-weight: bold;'>考试说明：</p>";
    // 计算实际组卷总分
    int actualTotalScore = 0;
    for (const Question& question : examQuestions) {
        actualTotalScore += question.getScore();
    }

    // 计算考试时长
    int durationMinutes = currentExam.getStartTime().secsTo(currentExam.getEndTime()) / 60;

    html += "<p style='margin: 5px 0 0 0;'>1. 本试卷共" + QString::number(examQuestions.size()) + "题，总分" + QString::number(actualTotalScore) + "分。</p>";
    html += "<p style='margin: 5px 0 0 0;'>2. 考试时间" + QString::number(durationMinutes) + "分钟。</p>";
    html += "<p style='margin: 5px 0 0 0;'>3. 请在答题纸上作答，保持卷面整洁。</p>";
    html += "</div>";

    // 按题型分组
    QMap<QString, QList<QPair<int, Question>>> questionsByType;
    for (int i = 0; i < examQuestions.size(); ++i) {
        const Question& q = examQuestions[i];
        questionsByType[q.getQuestionType()].append(qMakePair(i + 1, q));
    }

    // 输出题目
    QStringList typeOrder = {"单选", "多选", "填空", "简答", "论述", "计算"};

    for (const QString& type : typeOrder) {
        if (!questionsByType.contains(type)) continue;

        const QList<QPair<int, Question>>& questions = questionsByType[type];

        // 题型标题
        html += QString("<h3 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 5px; margin-top: 30px;'>%1题（共%2题）</h3>")
                    .arg(type).arg(questions.size());

        // 题目列表
        for (const QPair<int, Question>& pair : questions) {
            html += formatQuestion(pair.second, pair.first);
        }
    }

    html += "</div>";
    return html;
}

QString PaperPreview::formatQuestion(const Question& question, int questionNumber)
{
    QString html = QString("<div style='margin-bottom: 25px; padding: 15px; border-left: 3px solid #e74c3c;'>");

    // 题目序号和分值
    html += QString("<p style='margin: 0 0 10px 0; font-weight: bold; color: #2c3e50;'>%1. （%2分）</p>")
                .arg(questionNumber).arg(question.getScore());

    // 题目内容
    QString content = getQuestionContent(question);
    html += QString("<div style='margin-left: 20px; line-height: 1.8;'>%1</div>").arg(content);

    // 答题区域
    if (question.getQuestionType() == "单选" || question.getQuestionType() == "多选") {
        html += "<p style='margin:5px 0;'>&nbsp;</p>";
    } else if (question.getQuestionType() == "填空") {
        html += "<div style='margin: 15px 0 0 20px; height: 40px; border-bottom: 1px solid #bdc3c7;'></div>";
    } else {
        // 简答、论述、计算题
        int lines = question.getScore() / 5 + 2; // 根据分值估算答题行数
        html += "<div style='margin: 15px 0 0 20px;'>";
        for (int i = 0; i < lines; ++i) {
            html += "<div style='height: 25px; border-bottom: 1px solid #ecf0f1; margin-bottom: 5px;'></div>";
        }
        html += "</div>";

        // 为主观题添加5行额外空行
        html += "<div style='margin: 10px 0 0 20px;'>";
        for (int i = 0; i < 5; ++i) {
            html += "<p style='margin:5px 0;'>&nbsp;</p>";
        }
        html += "</div>";
    }


    html += "</div>";
    return html;
}

QString PaperPreview::getQuestionContent(const Question& question)
{
    QString content = question.getContent();

    // 处理换行符
    content = content.replace("\n", "<br>");

    // 如果是选择题，确保选项格式正确
    if (question.getQuestionType() == "单选" || question.getQuestionType() == "多选") {
        // 题目内容已经包含了选项，直接返回
        return content;
    }

    return content;
}

void PaperPreview::onPrintClicked()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    QPrintDialog printDialog(&printer, this);
    printDialog.setWindowTitle("打印试卷");

    if (printDialog.exec() == QDialog::Accepted) {
        // 创建用于打印的文档
        QTextDocument printDoc;

        // 生成完整的打印内容
        QString printContent = QString(
            "<html><head><style>"
            "body { font-family: SimSun, serif; font-size: 12pt; line-height: 1.6; }"
            "h1 { text-align: center; color: #2c3e50; margin-bottom: 10px; }"
            "h3 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 5px; }"
            ".exam-info { text-align: center; color: #7f8c8d; margin-bottom: 20px; }"
            ".student-info { border: 1px solid #bdc3c7; padding: 15px; margin-bottom: 20px; }"
            ".instructions { background-color: #fff3cd; border-left: 4px solid #ffc107; padding: 10px; margin-bottom: 20px; }"
            ".question { margin-bottom: 20px; padding: 10px; border-left: 3px solid #e74c3c; }"
            "</style></head><body>"
            );

        printContent += QString("<h1>%1</h1>").arg(currentExam.getExamName());
        printContent += QString("<div class='exam-info'>考试时间：%1  考试时长：%2-%3  总分：%4分</div>")
                            .arg(currentExam.getExamDate().toString("yyyy年MM月dd日"))
                            .arg(currentExam.getStartTime().toString("hh:mm"))
                            .arg(currentExam.getEndTime().toString("hh:mm"))
                            .arg(currentExam.getTotalScore());

        printContent += "<div class='student-info'>";
        printContent += "<table width='100%'>";
        printContent += "<tr><td width='50%'>姓名：________________</td><td>学号：________________</td></tr>";
        printContent += "<tr><td>班级：________________</td><td>座位号：________________</td></tr>";
        printContent += "</table></div>";

        printContent += generatePaperContent();
        printContent += "</body></html>";

        printDoc.setHtml(printContent);
        printDoc.print(&printer);

        QMessageBox::information(this, "打印完成", "试卷打印完成！");
    }
}

void PaperPreview::onCloseClicked()
{
    accept();
}
