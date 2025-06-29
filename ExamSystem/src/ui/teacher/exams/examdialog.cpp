#include "examdialog.h"
#include <QDateTime>

ExamDialog::ExamDialog(int teacherId, Database* database, QWidget *parent)
    : QDialog(parent), teacherId(teacherId), database(database), isEditMode(false), examId(0)
{
    setWindowTitle("新建考试");
    setupUI();
    loadCourses();

    // 设置默认值
    examDateEdit->setDate(QDate::currentDate().addDays(1));
    startTimeEdit->setTime(QTime(9, 0));
    endTimeEdit->setTime(QTime(11, 0));
    totalScoreSpinBox->setValue(100);
}

ExamDialog::ExamDialog(const Exam &exam, int teacherId, Database* database, QWidget *parent)
    : QDialog(parent), teacherId(teacherId), database(database), isEditMode(true), examId(exam.getExamId())
{
    setWindowTitle("编辑考试");
    setupUI();
    loadCourses();

    // 填充现有数据
    examNameEdit->setText(exam.getExamName());
    examDateEdit->setDate(exam.getExamDate());
    startTimeEdit->setTime(exam.getStartTime());
    endTimeEdit->setTime(exam.getEndTime());
    totalScoreSpinBox->setValue(exam.getTotalScore());

    // 设置课程
    for (int i = 0; i < courseComboBox->count(); ++i) {
        if (courseComboBox->itemData(i).toInt() == exam.getCourseId()) {
            courseComboBox->setCurrentIndex(i);
            break;
        }
    }

    // 如果考试已发布，禁用某些字段
    if (exam.getIsPublished()) {
        examDateEdit->setEnabled(false);
        startTimeEdit->setEnabled(false);
        endTimeEdit->setEnabled(false);
        courseComboBox->setEnabled(false);
    }
}

void ExamDialog::setupUI()
{
    setModal(true);
    setFixedSize(400, 300);

    mainLayout = new QVBoxLayout(this);

    // 表单布局
    formLayout = new QFormLayout();

    // 考试名称
    examNameEdit = new QLineEdit();
    examNameEdit->setPlaceholderText("请输入考试名称");
    formLayout->addRow("考试名称*:", examNameEdit);

    // 课程选择
    courseComboBox = new QComboBox();
    courseComboBox->setMinimumWidth(200);
    formLayout->addRow("选择课程*:", courseComboBox);

    // 考试日期
    examDateEdit = new QDateEdit();
    examDateEdit->setCalendarPopup(true);
    examDateEdit->setMinimumDate(QDate::currentDate());
    examDateEdit->setDisplayFormat("yyyy-MM-dd");
    formLayout->addRow("考试日期*:", examDateEdit);

    // 开始时间
    startTimeEdit = new QTimeEdit();
    startTimeEdit->setDisplayFormat("hh:mm");
    formLayout->addRow("开始时间*:", startTimeEdit);

    // 结束时间
    endTimeEdit = new QTimeEdit();
    endTimeEdit->setDisplayFormat("hh:mm");
    formLayout->addRow("结束时间*:", endTimeEdit);

    // 总分
    totalScoreSpinBox = new QSpinBox();
    totalScoreSpinBox->setRange(1, 1000);
    totalScoreSpinBox->setSuffix(" 分");
    formLayout->addRow("总分*:", totalScoreSpinBox);

    mainLayout->addLayout(formLayout);

    // 按钮布局
    buttonLayout = new QHBoxLayout();

    okButton = new QPushButton(isEditMode ? "保存" : "创建");
    cancelButton = new QPushButton("取消");

    okButton->setDefault(true);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(okButton, &QPushButton::clicked, this, &ExamDialog::onAcceptClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ExamDialog::onRejectClicked);

    // 添加说明标签
    QLabel* noteLabel = new QLabel("注意：标有*的字段为必填项");
    noteLabel->setStyleSheet("color: gray; font-size: 12px;");
    mainLayout->addWidget(noteLabel);
}

void ExamDialog::loadCourses()
{
    courses = database->getAllCourses();

    courseComboBox->clear();
    courseComboBox->addItem("请选择课程", -1);

    for (const Course& course : courses) {
        courseComboBox->addItem(course.getDisplayName(), course.getCourseId());
    }
}

bool ExamDialog::validateInput()
{
    // 考试名称验证
    if (examNameEdit->text().trimmed().isEmpty()) {
        showValidationError("请输入考试名称", examNameEdit);
        return false;
    }

    // 课程选择验证
    if (courseComboBox->currentData().toInt() <= 0) {
        showValidationError("请选择课程", courseComboBox);
        return false;
    }

    // 日期验证
    if (!examDateEdit->date().isValid()) {
        showValidationError("请选择有效的考试日期", examDateEdit);
        return false;
    }

    if (examDateEdit->date() < QDate::currentDate()) {
        showValidationError("考试日期不能是过去的日期", examDateEdit);
        return false;
    }

    // 时间验证
    if (!startTimeEdit->time().isValid() || !endTimeEdit->time().isValid()) {
        showValidationError("请设置有效的考试时间", startTimeEdit);
        return false;
    }

    if (startTimeEdit->time() >= endTimeEdit->time()) {
        showValidationError("结束时间必须晚于开始时间", endTimeEdit);
        return false;
    }

    // 检查考试时长（至少30分钟）
    int duration = startTimeEdit->time().secsTo(endTimeEdit->time()) / 60;
    if (duration < 30) {
        showValidationError("考试时长不能少于30分钟", endTimeEdit);
        return false;
    }

    // 总分验证
    if (totalScoreSpinBox->value() <= 0) {
        showValidationError("总分必须大于0", totalScoreSpinBox);
        return false;
    }

    return true;
}

void ExamDialog::showValidationError(const QString &message, QWidget *focusWidget)
{
    QMessageBox::warning(this, "输入错误", message);
    if (focusWidget) {
        focusWidget->setFocus();
    }
}

Exam ExamDialog::getExamData() const
{
    Exam exam;

    if (isEditMode) {
        exam.setExamId(examId);
    }

    exam.setExamName(examNameEdit->text().trimmed());
    exam.setCourseId(courseComboBox->currentData().toInt());
    exam.setExamDate(examDateEdit->date());
    exam.setStartTime(startTimeEdit->time());
    exam.setEndTime(endTimeEdit->time());
    exam.setTotalScore(totalScoreSpinBox->value());

    // 设置课程名称（用于显示）
    QString courseName = courseComboBox->currentText();
    if (courseName.contains(" (")) {
        courseName = courseName.split(" (")[0];
    }
    exam.setCourseName(courseName);

    return exam;
}

void ExamDialog::onAcceptClicked()
{
    if (validateInput()) {
        accept();
    }
}

void ExamDialog::onRejectClicked()
{
    reject();
}
