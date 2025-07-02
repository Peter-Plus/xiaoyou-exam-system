// ============================================================================
// NoticeDetailWidget - 通知详情组件实现
// ============================================================================

NoticeDetailWidget::NoticeDetailWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
    clearContent(); // 初始显示欢迎页面
}

void NoticeDetailWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);

    // 头部信息区域
    m_headerWidget = new QWidget();
    m_headerLayout = new QVBoxLayout(m_headerWidget);
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(5);

    m_titleLabel = new QLabel();
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setObjectName("titleLabel");

    m_infoLabel = new QLabel();
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setObjectName("infoLabel");

    m_headerLayout->addWidget(m_titleLabel);
    m_headerLayout->addWidget(m_infoLabel);

    // 内容区域
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_contentTextEdit = new QTextEdit();
    m_contentTextEdit->setReadOnly(true);
    m_contentTextEdit->setObjectName("contentTextEdit");

    m_contentLayout->addWidget(m_contentTextEdit);

    // 欢迎页面
    m_welcomeWidget = new QWidget();
    QVBoxLayout *welcomeLayout = new QVBoxLayout(m_welcomeWidget);
    welcomeLayout->setAlignment(Qt::AlignCenter);

    m_welcomeLabel = new QLabel("📢\n\n选择左侧通知查看详细内容");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setObjectName("welcomeLabel");

    welcomeLayout->addWidget(m_welcomeLabel);

    // 添加到主布局（初始隐藏内容区域）
    m_mainLayout->addWidget(m_headerWidget);
    m_mainLayout->addWidget(m_contentWidget);
    m_mainLayout->addWidget(m_welcomeWidget);

    m_headerWidget->hide();
    m_contentWidget->hide();
}

void NoticeDetailWidget::setupStyles()
{
    this->setStyleSheet(
        "#titleLabel {"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: #333;"
        "    padding: 10px 0px;"
        "    border-bottom: 2px solid #1890ff;"
        "}"
        "#infoLabel {"
        "    font-size: 14px;"
        "    color: #666;"
        "    padding: 5px 0px 15px 0px;"
        "}"
        "#contentTextEdit {"
        "    border: 1px solid #ddd;"
        "    border-radius: 6px;"
        "    background-color: #fafafa;"
        "    font-size: 14px;"
        "    line-height: 1.5;"
        "    padding: 15px;"
        "}"
        "#welcomeLabel {"
        "    font-size: 16px;"
        "    color: #999;"
        "    line-height: 2;"
        "}"
        );
}

void NoticeDetailWidget::showNotice(const QVariantMap &notice)
{
    // 显示内容区域，隐藏欢迎页面
    m_headerWidget->show();
    m_contentWidget->show();
    m_welcomeWidget->hide();

    // 设置标题
    QString title = notice["title"].toString();
    bool isPinned = notice["is_pinned"].toBool();
    QString pinnedText = isPinned ? "📌 " : "";
    m_titleLabel->setText(pinnedText + title);

    // 设置信息行
    QString courseName = notice["course_name"].toString();
    QDateTime publishTime = notice["publish_time"].toDateTime();
    QString teacherName = notice["teacher_name"].toString();

    QString infoText;
    if (!teacherName.isEmpty()) {
        infoText = QString("课程: %1 | 发布者: %2 | 时间: %3")
                       .arg(courseName)
                       .arg(teacherName)
                       .arg(publishTime.toString("yyyy-MM-dd hh:mm"));
    } else {
        infoText = QString("课程: %1 | 时间: %2")
                       .arg(courseName)
                       .arg(publishTime.toString("yyyy-MM-dd hh:mm"));
    }

    if (isPinned) {
        infoText += " | 🔝 置顶通知";
    }

    m_infoLabel->setText(infoText);

    // 设置内容
    QString content = notice["content"].toString();
    m_contentTextEdit->setPlainText(content);
}

void NoticeDetailWidget::clearContent()
{
    // 隐藏内容区域，显示欢迎页面
    m_headerWidget->hide();
    m_contentWidget->hide();
    m_welcomeWidget->show();
}

// ============================================================================
// PublishNoticeDialog - 发布通知对话框实现
// ============================================================================

PublishNoticeDialog::PublishNoticeDialog(Database *database, int teacherId, Mode mode, QWidget *parent)
    : QDialog(parent)
    , m_database(database)
    , m_teacherId(teacherId)
    , m_mode(mode)
    , m_noticeId(-1)
{
    setupUI();
    setupStyles();
    updateCourseCombo();

    QString title = (mode == PUBLISH) ? "发布课程通知" : "编辑课程通知";
    setWindowTitle(title);
    setModal(true);
    resize(600, 500);
}

void PublishNoticeDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);

    // 基本信息区域
    m_basicGroup = new QGroupBox("基本信息");
    QFormLayout *basicLayout = new QFormLayout(m_basicGroup);

    m_courseCombo = new QComboBox();
    m_titleLineEdit = new QLineEdit();
    m_titleLineEdit->setPlaceholderText("请输入通知标题...");
    m_pinnedCheckBox = new QCheckBox("置顶通知");

    basicLayout->addRow("选择课程:", m_courseCombo);
    basicLayout->addRow("通知标题:", m_titleLineEdit);
    basicLayout->addRow("", m_pinnedCheckBox);

    m_mainLayout->addWidget(m_basicGroup);

    // 内容区域
    m_contentGroup = new QGroupBox("通知内容");
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentGroup);

    m_contentTextEdit = new QTextEdit();
    m_contentTextEdit->setPlaceholderText("请输入通知内容...");
    m_contentTextEdit->setMinimumHeight(200);

    contentLayout->addWidget(m_contentTextEdit);
    m_mainLayout->addWidget(m_contentGroup);

    // 操作按钮
    m_buttonLayout = new QHBoxLayout();

    QString publishButtonText = (m_mode == PUBLISH) ? "📝 发布通知" : "💾 保存修改";
    m_publishButton = new QPushButton(publishButtonText);
    m_previewButton = new QPushButton("👁️ 预览");
    m_cancelButton = new QPushButton("❌ 取消");

    m_buttonLayout->addWidget(m_previewButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_publishButton);

    m_mainLayout->addLayout(m_buttonLayout);

    // 连接信号槽
    connect(m_publishButton, &QPushButton::clicked, this, &PublishNoticeDialog::onPublishClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &PublishNoticeDialog::onCancelClicked);
    connect(m_previewButton, &QPushButton::clicked, this, &PublishNoticeDialog::onPreviewClicked);
}

void PublishNoticeDialog::setupStyles()
{
    this->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #ddd;"
        "    border-radius: 6px;"
        "    margin-top: 6px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "}"
        "QLineEdit {"
        "    padding: 8px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #1890ff;"
        "}"
        "QTextEdit {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "    line-height: 1.5;"
        "}"
        "QTextEdit:focus {"
        "    border-color: #1890ff;"
        "}"
        "QPushButton {"
        "    padding: 10px 20px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: #f8f9fa;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e8f4ff;"
        "    border-color: #1890ff;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1890ff;"
        "    color: white;"
        "}"
        "QComboBox {"
        "    padding: 8px;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    font-size: 14px;"
        "}"
        );
}

void PublishNoticeDialog::updateCourseCombo()
{
    if (!m_database) return;

    m_courseCombo->clear();
    m_teacherCourses.clear();

    QList<Course> courses = m_database->getTeacherCourses(m_teacherId);

    for (const Course &course : courses) {
        QVariantMap courseMap;
        courseMap["course_id"] = course.getCourseId();
        courseMap["course_name"] = course.getCourseName();
        m_teacherCourses.append(courseMap);

        m_courseCombo->addItem(course.getCourseName(), course.getCourseId());
    }

    if (m_courseCombo->count() == 0) {
        m_courseCombo->addItem("暂无可选课程", -1);
        m_publishButton->setEnabled(false);
    }
}

void PublishNoticeDialog::setNoticeData(const QVariantMap &notice)
{
    m_noticeId = notice["notice_id"].toInt();

    // 设置课程
    int courseId = notice["course_id"].toInt();
    int courseIndex = m_courseCombo->findData(courseId);
    if (courseIndex >= 0) {
        m_courseCombo->setCurrentIndex(courseIndex);
    }

    // 设置标题和内容
    m_titleLineEdit->setText(notice["title"].toString());
    m_contentTextEdit->setPlainText(notice["content"].toString());
    m_pinnedCheckBox->setChecked(notice["is_pinned"].toBool());
}

QVariantMap PublishNoticeDialog::getNoticeData() const
{
    QVariantMap data;
    data["course_id"] = m_courseCombo->currentData().toInt();
    data["title"] = m_titleLineEdit->text().trimmed();
    data["content"] = m_contentTextEdit->toPlainText().trimmed();
    data["is_pinned"] = m_pinnedCheckBox->isChecked();

    if (m_mode == EDIT) {
        data["notice_id"] = m_noticeId;
    }

    return data;
}

void PublishNoticeDialog::onPublishClicked()
{
    if (!validateInput()) {
        return;
    }

    QVariantMap data = getNoticeData();
    int courseId = data["course_id"].toInt();
    QString title = data["title"].toString();
    QString content = data["content"].toString();
    bool isPinned = data["is_pinned"].toBool();

    bool success = false;
    QString operation = (m_mode == PUBLISH) ? "发布" : "更新";

    if (m_mode == PUBLISH) {
        int noticeId = m_database->publishCourseNotice(courseId, title, content, isPinned);
        success = (noticeId > 0);
    } else {
        success = m_database->updateCourseNotice(m_noticeId, title, content, isPinned);
    }

    if (success) {
        QMessageBox::information(this, "成功", QString("通知%1成功！").arg(operation));
        accept();
    } else {
        QMessageBox::warning(this, "失败", QString("通知%1失败，请重试").arg(operation));
    }
}

void PublishNoticeDialog::onCancelClicked()
{
    reject();
}

void PublishNoticeDialog::onPreviewClicked()
{
    if (!validateInput()) {
        return;
    }

    QVariantMap data = getNoticeData();
    QString title = data["title"].toString();
    QString content = data["content"].toString();
    bool isPinned = data["is_pinned"].toBool();

    QString previewText = QString("标题: %1%2\n\n内容:\n%3")
                              .arg(isPinned ? "📌 " : "")
                              .arg(title)
                              .arg(content);

    QMessageBox preview(this);
    preview.setWindowTitle("通知预览");
    preview.setText(previewText);
    preview.setStandardButtons(QMessageBox::Ok);
    preview.exec();
}

bool PublishNoticeDialog::validateInput()
{
    if (m_courseCombo->currentData().toInt() <= 0) {
        QMessageBox::warning(this, "提示", "请选择课程");
        return false;
    }

    if (m_titleLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入通知标题");
        m_titleLineEdit->setFocus();
        return false;
    }

    if (m_contentTextEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入通知内容");
        m_contentTextEdit->setFocus();
        return false;
    }

    return true;
}
