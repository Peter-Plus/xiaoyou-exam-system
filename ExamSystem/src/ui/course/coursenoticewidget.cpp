#include "coursenoticewidget.h"
#include "../../models/course.h"
#include "../../models/coursenotice.h"
#include <QMessageBox>
#include <QTimer>
#include <QCheckBox>
#include <QDialog>
#include <QFormLayout>

CourseNoticeWidget::CourseNoticeWidget(Database *database, int userId, const QString &userType, QWidget *parent)
    : QWidget(parent)
{
    qDebug() << "CourseNoticeWidget 构造函数开始";
    qDebug() << "database:" << database;
    qDebug() << "userId:" << userId;
    qDebug() << "userType:" << userType;
    qDebug() << "parent:" << parent;

    m_database = database;
    m_userId = userId;
    m_userType = userType;
    m_isTeacher = (userType == "老师");
    m_selectedNoticeId = -1;
    m_publishDialog = nullptr;
    m_searchTimer = nullptr;

    qDebug() << "成员变量初始化完成";

    // 创建搜索定时器
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(500);
    connect(m_searchTimer, &QTimer::timeout, this, &CourseNoticeWidget::updateNoticeList);

    qDebug() << "定时器创建完成";

    setupUI();
    qDebug() << "setupUI 完成";

    setupStyles();
    qDebug() << "setupStyles 完成";

    refreshData();
    qDebug() << "refreshData 完成";

    qDebug() << "CourseNoticeWidget 构造函数结束";
}

void CourseNoticeWidget::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);

    // 左侧通知列表区域
    m_leftWidget = new QWidget();
    m_leftWidget->setFixedWidth(400);
    m_leftLayout = new QVBoxLayout(m_leftWidget);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);
    m_leftLayout->setSpacing(10);

    // 筛选区域
    m_filterGroup = new QGroupBox("筛选条件");
    m_filterLayout = new QHBoxLayout(m_filterGroup);

    m_courseFilterCombo = new QComboBox();
    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("搜索通知标题或内容...");
    m_searchButton = new QPushButton("🔍 搜索");
    m_refreshButton = new QPushButton("🔄 刷新");

    m_filterLayout->addWidget(new QLabel("课程:"));
    m_filterLayout->addWidget(m_courseFilterCombo);
    m_filterLayout->addWidget(m_searchLineEdit);
    m_filterLayout->addWidget(m_searchButton);
    m_filterLayout->addWidget(m_refreshButton);

    m_leftLayout->addWidget(m_filterGroup);

    // 通知列表
    m_listGroup = new QGroupBox("课程通知");
    QVBoxLayout *listLayout = new QVBoxLayout(m_listGroup);

    m_noticeList = new QListWidget();
    listLayout->addWidget(m_noticeList);

    QHBoxLayout *countLayout = new QHBoxLayout();
    m_noticeCountLabel = new QLabel("共0条通知");
    countLayout->addWidget(m_noticeCountLabel);
    countLayout->addStretch();
    listLayout->addLayout(countLayout);

    m_leftLayout->addWidget(m_listGroup);

    // 操作按钮（仅教师端）
    if (m_isTeacher) {
        setupTeacherUI();
    } else {
        setupStudentUI();
    }

    m_splitter->addWidget(m_leftWidget);

    // 右侧通知详情区域
    m_noticeDetailWidget = new NoticeDetailWidget(this);
    m_splitter->addWidget(m_noticeDetailWidget);

    // 设置分割器比例
    m_splitter->setSizes({400, 600});

    // 连接信号槽
    connect(m_noticeList, &QListWidget::itemClicked, this, &CourseNoticeWidget::onNoticeSelected);
    connect(m_noticeList, &QListWidget::itemDoubleClicked, this, &CourseNoticeWidget::onNoticeDoubleClicked);
    connect(m_searchButton, &QPushButton::clicked, this, &CourseNoticeWidget::updateNoticeList);
    connect(m_refreshButton, &QPushButton::clicked, this, &CourseNoticeWidget::onRefreshClicked);
    connect(m_courseFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CourseNoticeWidget::onCourseFilterChanged);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &CourseNoticeWidget::onSearchTextChanged);
}

void CourseNoticeWidget::setupTeacherUI()
{
    m_actionGroup = new QGroupBox("操作");
    m_actionLayout = new QHBoxLayout(m_actionGroup);

    m_publishButton = new QPushButton("📝 发布通知");
    m_editButton = new QPushButton("✏️ 编辑");
    m_deleteButton = new QPushButton("🗑️ 删除");

    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    m_actionLayout->addWidget(m_publishButton);
    m_actionLayout->addWidget(m_editButton);
    m_actionLayout->addWidget(m_deleteButton);
    m_actionLayout->addStretch();

    m_leftLayout->addWidget(m_actionGroup);

    // 连接教师操作信号槽
    connect(m_publishButton, &QPushButton::clicked, this, &CourseNoticeWidget::onPublishNotice);
    connect(m_editButton, &QPushButton::clicked, this, &CourseNoticeWidget::onEditNotice);
    connect(m_deleteButton, &QPushButton::clicked, this, &CourseNoticeWidget::onDeleteNotice);
}

void CourseNoticeWidget::setupStudentUI()
{
    // 学生端只需要显示统计信息
    QGroupBox *infoGroup = new QGroupBox("统计信息");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    QLabel *infoLabel = new QLabel("查看课程通知和重要信息");
    infoLayout->addWidget(infoLabel);
    m_leftLayout->addWidget(infoGroup);
}

void CourseNoticeWidget::setupStyles()
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
        "QListWidget {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    alternate-background-color: #f8f9fa;"
        "}"
        "QListWidget::item {"
        "    padding: 12px;"
        "    border-bottom: 1px solid #eee;"
        "    margin: 2px;"
        "    border-radius: 4px;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #e6f7ff;"
        "    border: 2px solid #1890ff;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #f0f0f0;"
        "}"
        "QPushButton {"
        "    padding: 8px 16px;"
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
        "QPushButton:disabled {"
        "    background-color: #f5f5f5;"
        "    color: #999;"
        "}"
        );

    // 设置分割器样式
    m_splitter->setStyleSheet(
        "QSplitter::handle {"
        "    background-color: #ddd;"
        "    width: 2px;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #1890ff;"
        "}"
        );
}

void CourseNoticeWidget::refreshData()
{
    if (!m_database) {
        qDebug() << "错误：数据库连接为空";
        return;
    }
    updateCourseFilter();
    updateNoticeList();
}

void CourseNoticeWidget::updateCourseFilter()
{
    m_courseFilterCombo->clear();
    m_courseFilterCombo->addItem("全部课程");

    if (m_isTeacher) {
        // 教师端：获取自己教授的课程
        QList<Course> courses = m_database->getTeacherCourses(m_userId);
        m_teacherCourses.clear();

        for (const Course &course : courses) {
            QVariantMap courseMap;
            courseMap["course_id"] = course.getCourseId();
            courseMap["course_name"] = course.getCourseName();
            m_teacherCourses.append(courseMap);

            m_courseFilterCombo->addItem(course.getCourseName(), course.getCourseId());
        }
    } else {
        // 学生端：获取已选课程
        m_studentCourses = m_database->getStudentCourses(m_userId);

        for (const QVariantMap &course : m_studentCourses) {
            if (course["enrollment_status"].toString() == "已通过") {
                QString courseName = course["course_name"].toString();
                int courseId = course["course_id"].toInt();
                m_courseFilterCombo->addItem(courseName, courseId);
            }
        }
    }
}

void CourseNoticeWidget::updateNoticeList()
{
    if (!m_database) return;

    m_notices.clear();
    m_noticeList->clear();

    // 获取过滤后的通知
    QList<QVariantMap> filteredNotices = getFilteredNotices();

    for (const QVariantMap &notice : filteredNotices) {
        createNoticeListItem(notice);
        m_notices.append(notice);
    }

    m_noticeCountLabel->setText(QString("共%1条通知").arg(m_notices.size()));

    // 重置选择状态
    m_selectedNoticeId = -1;
    if (m_isTeacher) {
        m_editButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
    }
    m_noticeDetailWidget->clearContent();
}

QList<QVariantMap> CourseNoticeWidget::getFilteredNotices()
{
    QList<QVariantMap> allNotices;
    QString searchText = m_searchLineEdit->text().trimmed();
    int selectedCourseId = m_courseFilterCombo->currentData().toInt();

    if (m_isTeacher) {
        // 教师端：获取自己课程的通知
        for (const QVariantMap &course : m_teacherCourses) {
            int courseId = course["course_id"].toInt();

            // 如果选择了特定课程，只获取该课程的通知
            if (selectedCourseId > 0 && courseId != selectedCourseId) {
                continue;
            }

            QList<QVariantMap> courseNotices = m_database->getCourseNotices(courseId);
            for (QVariantMap notice : courseNotices) {
                notice["course_name"] = course["course_name"].toString();
                allNotices.append(notice);
            }
        }
    } else {
        // 学生端：获取已选课程的通知
        for (const QVariantMap &course : m_studentCourses) {
            if (course["enrollment_status"].toString() != "已通过") {
                continue;
            }

            int courseId = course["course_id"].toInt();

            // 如果选择了特定课程，只获取该课程的通知
            if (selectedCourseId > 0 && courseId != selectedCourseId) {
                continue;
            }

            QList<QVariantMap> courseNotices = m_database->getCourseNotices(courseId);
            for (QVariantMap notice : courseNotices) {
                notice["course_name"] = course["course_name"].toString();
                notice["teacher_name"] = course["teacher_name"].toString();
                allNotices.append(notice);
            }
        }
    }

    // 应用搜索过滤
    if (!searchText.isEmpty()) {
        QList<QVariantMap> filteredNotices;
        for (const QVariantMap &notice : allNotices) {
            QString title = notice["title"].toString();
            QString content = notice["content"].toString();

            if (title.contains(searchText, Qt::CaseInsensitive) ||
                content.contains(searchText, Qt::CaseInsensitive)) {
                filteredNotices.append(notice);
            }
        }
        return filteredNotices;
    }

    return allNotices;
}

void CourseNoticeWidget::createNoticeListItem(const QVariantMap &notice)
{
    QListWidgetItem *item = new QListWidgetItem();

    QString title = notice["title"].toString();
    QString courseName = notice["course_name"].toString();
    QDateTime publishTime = notice["publish_time"].toDateTime();
    bool isPinned = notice["is_pinned"].toBool();
    int noticeId = notice["notice_id"].toInt();

    // 创建显示文本
    QString pinnedText = isPinned ? "📌 " : "";
    QString timeText = CourseNotice().getTimeString(); // 使用CourseNotice的时间格式化方法

    // 重新计算相对时间
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = publishTime.secsTo(now);

    if (secondsAgo < 60) {
        timeText = "刚刚";
    } else if (secondsAgo < 3600) {
        timeText = QString("%1分钟前").arg(secondsAgo / 60);
    } else if (secondsAgo < 86400) {
        timeText = QString("%1小时前").arg(secondsAgo / 3600);
    } else if (secondsAgo < 604800) {
        timeText = QString("%1天前").arg(secondsAgo / 86400);
    } else {
        timeText = publishTime.toString("MM-dd hh:mm");
    }

    QString displayText = QString("%1%2\n课程: %3 | %4")
                              .arg(pinnedText)
                              .arg(title)
                              .arg(courseName)
                              .arg(timeText);

    item->setText(displayText);
    item->setData(Qt::UserRole, noticeId);

    // 置顶通知使用不同颜色
    if (isPinned) {
        item->setBackground(QBrush(QColor(255, 248, 220))); // 浅黄色
        item->setIcon(QIcon("📌"));
    }

    m_noticeList->addItem(item);
}

void CourseNoticeWidget::onNoticeSelected(QListWidgetItem *item)
{
    if (!item) return;

    int noticeId = item->data(Qt::UserRole).toInt();
    m_selectedNoticeId = noticeId;

    // 找到对应的通知数据
    QVariantMap selectedNotice;
    for (const QVariantMap &notice : m_notices) {
        if (notice["notice_id"].toInt() == noticeId) {
            selectedNotice = notice;
            break;
        }
    }

    if (!selectedNotice.isEmpty()) {
        showNoticeDetail(selectedNotice);

        // 更新教师端操作按钮状态
        if (m_isTeacher) {
            m_editButton->setEnabled(true);
            m_deleteButton->setEnabled(true);
        }
    }
}

void CourseNoticeWidget::showNoticeDetail(const QVariantMap &notice)
{
    m_noticeDetailWidget->showNotice(notice);
}

void CourseNoticeWidget::onPublishNotice()
{
    if (!m_publishDialog) {
        m_publishDialog = new PublishNoticeDialog(m_database, m_userId, PublishNoticeDialog::PUBLISH, this);
        connect(m_publishDialog, &QDialog::accepted, [this]() {
            refreshData();
            emit noticePublished(0); // 0表示不特定课程
        });
    }

    m_publishDialog->exec();
}

void CourseNoticeWidget::onEditNotice()
{
    if (m_selectedNoticeId <= 0) {
        showMessage("请先选择要编辑的通知", true);
        return;
    }

    // 找到选中的通知数据
    QVariantMap selectedNotice;
    for (const QVariantMap &notice : m_notices) {
        if (notice["notice_id"].toInt() == m_selectedNoticeId) {
            selectedNotice = notice;
            break;
        }
    }

    if (selectedNotice.isEmpty()) {
        showMessage("找不到选中的通知", true);
        return;
    }

    PublishNoticeDialog editDialog(m_database, m_userId, PublishNoticeDialog::EDIT, this);
    editDialog.setNoticeData(selectedNotice);

    connect(&editDialog, &QDialog::accepted, [this]() {
        refreshData();
        emit noticeUpdated(m_selectedNoticeId);
    });

    editDialog.exec();
}

void CourseNoticeWidget::onDeleteNotice()
{
    if (m_selectedNoticeId <= 0) {
        showMessage("请先选择要删除的通知", true);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        "确定要删除这条通知吗？删除后无法恢复。",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_database->deleteCourseNotice(m_selectedNoticeId)) {
            showMessage("通知删除成功");
            refreshData();
            emit noticeDeleted(m_selectedNoticeId);
        } else {
            showMessage("删除通知失败，请重试", true);
        }
    }
}

void CourseNoticeWidget::onRefreshClicked()
{
    refreshData();
}

void CourseNoticeWidget::onCourseFilterChanged()
{
    updateNoticeList();
}

void CourseNoticeWidget::onSearchTextChanged()
{
    // 使用成员变量而不是静态变量
    if (m_searchTimer) {
        m_searchTimer->start();
    }
}

void CourseNoticeWidget::onNoticeDoubleClicked(QListWidgetItem *item)
{
    // 双击时选中并显示详情
    onNoticeSelected(item);
}

void CourseNoticeWidget::showMessage(const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::warning(this, "提示", message);
    } else {
        QMessageBox::information(this, "提示", message);
    }
}

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
