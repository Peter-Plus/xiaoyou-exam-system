#include "questiondialog.h"

// 新建模式构造函数
QuestionDialog::QuestionDialog(Mode mode, int teacherId, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_teacherId(teacherId)
    , m_database(new Database())
{
    m_database->connectToDatabase();
    setupUI();
    updateWindowTitle();
}

// 编辑/查看模式构造函数
QuestionDialog::QuestionDialog(Mode mode, int teacherId, const Question &question, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_teacherId(teacherId)
    , m_question(question)
    , m_database(new Database())
{
    m_database->connectToDatabase();
    setupUI();
    loadQuestionData();
    updateWindowTitle();
}

void QuestionDialog::setupUI()
{
    setModal(true); // 设置为模态对话框
    setMinimumSize(500, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    setupFormLayout();
    setupButtonLayout();

    mainLayout->addLayout(mainLayout);

    // 根据模式设置控件状态
    if (m_mode == MODE_VIEW) {
        // 查看模式：所有控件只读
        m_typeCombo->setEnabled(false);
        m_courseCombo->setEnabled(false);
        m_scoreSpinBox->setEnabled(false);
        m_contentEdit->setReadOnly(true);
        m_answerEdit->setReadOnly(true);
        m_publicCheckBox->setEnabled(false);
        m_okButton->setVisible(false); // 隐藏确定按钮
    }
}

void QuestionDialog::setupFormLayout()
{
    QFormLayout *formLayout = new QFormLayout();

    // 题型选择
    m_typeCombo = new QComboBox();
    QStringList types = m_database->getQuestionTypes();
    m_typeCombo->addItems(types);
    formLayout->addRow("题型:", m_typeCombo);

    // 课程选择
    m_courseCombo = new QComboBox();
    m_courseCombo->setEditable(true); // 允许输入新课程名
    QStringList courses = m_database->getCourseNames();
    m_courseCombo->addItems(courses);
    formLayout->addRow("课程:", m_courseCombo);

    // 分值设置
    m_scoreSpinBox = new QSpinBox();
    m_scoreSpinBox->setRange(1, 100);
    m_scoreSpinBox->setValue(5); // 默认5分
    formLayout->addRow("分值:", m_scoreSpinBox);

    // 题目内容
    m_contentEdit = new QTextEdit();
    m_contentEdit->setMinimumHeight(150);
    m_contentEdit->setPlaceholderText("请输入题目内容...\n\n选择题格式示例：\n题干描述\nA. 选项1\nB. 选项2\nC. 选项3\nD. 选项4");
    formLayout->addRow("题目内容:", m_contentEdit);

    // 答案
    m_answerEdit = new QTextEdit();
    m_answerEdit->setMinimumHeight(100);
    m_answerEdit->setPlaceholderText("请输入答案...\n\n选择题答案格式：\n单选题: A 或 B 或 C 等\n多选题: ABC 或 BD 等");
    formLayout->addRow("答案:", m_answerEdit);

    // 是否公开
    m_publicCheckBox = new QCheckBox("公开题目（其他教师可以看到和收藏）");
    m_publicCheckBox->setChecked(true); // 默认公开
    formLayout->addRow("", m_publicCheckBox);

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (!mainLayout) {
        mainLayout = new QVBoxLayout(this);
    }
    mainLayout->addLayout(formLayout);
}

void QuestionDialog::setupButtonLayout()
{
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_okButton = new QPushButton();
    m_cancelButton = new QPushButton("取消");

    // 根据模式设置按钮文字
    switch (m_mode) {
    case MODE_ADD:
        m_okButton->setText("添加");
        break;
    case MODE_EDIT:
        m_okButton->setText("保存");
        break;
    case MODE_VIEW:
        m_cancelButton->setText("关闭");
        break;
    }

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    // 连接信号
    connect(m_okButton, &QPushButton::clicked, this, &QuestionDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QuestionDialog::onCancelClicked);

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    mainLayout->addLayout(buttonLayout);
}

void QuestionDialog::loadQuestionData()
{
    // 设置题型
    int typeIndex = m_typeCombo->findText(m_question.getQuestionType());
    if (typeIndex >= 0) {
        m_typeCombo->setCurrentIndex(typeIndex);
    }

    // 设置课程
    m_courseCombo->setCurrentText(m_question.getCourseName());

    // 设置分值
    m_scoreSpinBox->setValue(m_question.getScore());

    // 设置题目内容
    m_contentEdit->setPlainText(m_question.getContent());

    // 设置答案
    m_answerEdit->setPlainText(m_question.getAnswer());

    // 设置公开状态
    m_publicCheckBox->setChecked(m_question.getIsPublic());
}

void QuestionDialog::updateWindowTitle()
{
    QString title;
    switch (m_mode) {
    case MODE_ADD:
        title = "新建题目";
        break;
    case MODE_EDIT:
        title = "修改题目";
        break;
    case MODE_VIEW:
        title = "查看题目";
        break;
    }
    setWindowTitle(title);
}

void QuestionDialog::onOkClicked()
{
    if (!validateInput()) {
        return;
    }

    bool success = false;

    if (m_mode == MODE_ADD) {
        // 新建题目
        success = m_database->addQuestion(
            m_typeCombo->currentText(),
            m_courseCombo->currentText(),
            m_scoreSpinBox->value(),
            m_contentEdit->toPlainText(),
            m_answerEdit->toPlainText(),
            m_teacherId,
            m_publicCheckBox->isChecked()
            );

        if (success) {
            QMessageBox::information(this, "成功", "题目添加成功！");
            accept();
        } else {
            showError("题目添加失败，请检查数据库连接");
        }
    } else if (m_mode == MODE_EDIT) {
        // 修改题目
        success = m_database->updateQuestion(
            m_question.getQuestionId(),
            m_typeCombo->currentText(),
            m_courseCombo->currentText(),
            m_scoreSpinBox->value(),
            m_contentEdit->toPlainText(),
            m_answerEdit->toPlainText(),
            m_teacherId,
            m_publicCheckBox->isChecked()
            );

        if (success) {
            QMessageBox::information(this, "成功", "题目修改成功！");
            accept();
        } else {
            showError("题目修改失败，请检查权限或数据库连接");
        }
    }
}

void QuestionDialog::onCancelClicked()
{
    reject(); // 关闭对话框，返回取消状态
}

bool QuestionDialog::validateInput()
{
    // 检查题型
    if (m_typeCombo->currentText().isEmpty()) {
        showError("请选择题型");
        m_typeCombo->setFocus();
        return false;
    }

    // 检查课程
    QString courseName = m_courseCombo->currentText().trimmed();
    if (courseName.isEmpty()) {
        showError("请输入课程名称");
        m_courseCombo->setFocus();
        return false;
    }

    // 检查分值
    if (m_scoreSpinBox->value() <= 0) {
        showError("分值必须大于0");
        m_scoreSpinBox->setFocus();
        return false;
    }

    // 检查题目内容
    QString content = m_contentEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        showError("请输入题目内容");
        m_contentEdit->setFocus();
        return false;
    }

    // 检查答案
    QString answer = m_answerEdit->toPlainText().trimmed();
    if (answer.isEmpty()) {
        showError("请输入答案");
        m_answerEdit->setFocus();
        return false;
    }

    // 选择题特殊验证
    QString questionType = m_typeCombo->currentText();
    if (questionType == "单选" || questionType == "多选") {
        // 简单验证答案格式
        if (questionType == "单选") {
            if (answer.length() != 1 || (answer < "A" || answer > "Z")) {
                showError("单选题答案应为单个字母（如：A）");
                m_answerEdit->setFocus();
                return false;
            }
        } else if (questionType == "多选") {
            // 多选题答案应为字母组合
            for (QChar c : answer) {
                if (c < 'A' || c > 'Z') {
                    showError("多选题答案应为字母组合（如：ABC）");
                    m_answerEdit->setFocus();
                    return false;
                }
            }
        }
    }

    return true;
}

Question QuestionDialog::getQuestion() const
{
    Question question;
    question.setQuestionType(m_typeCombo->currentText());
    question.setCourseName(m_courseCombo->currentText().trimmed());
    question.setScore(m_scoreSpinBox->value());
    question.setContent(m_contentEdit->toPlainText().trimmed());
    question.setAnswer(m_answerEdit->toPlainText().trimmed());
    question.setIsPublic(m_publicCheckBox->isChecked());
    return question;
}

void QuestionDialog::showError(const QString &message)
{
    QMessageBox::warning(this, "输入错误", message);
}
