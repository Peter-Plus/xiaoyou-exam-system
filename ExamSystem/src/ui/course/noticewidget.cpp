#include "noticewidget.h"
#include <QDebug>
#include <QHeaderView>
#include <QSplitter>

NoticeWidget::NoticeWidget(Database *database, int userId, UserType userType, int courseId, QWidget *parent)
    : QWidget(parent), m_database(database), m_userId(userId), m_userType(userType),
    m_currentCourseId(courseId), m_totalNotices(0), m_pinnedNotices(0), m_recentNotices(0)
{
    setupUI();

    // 设置定时器 - 每2分钟自动刷新
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &NoticeWidget::autoRefresh);
    m_refreshTimer->start(120000); // 2分钟自动刷新

    // 初始加载数据
    refreshData();
}

NoticeWidget::~NoticeWidget()
{
    if (m_refreshTimer) {
        m_refreshTimer->stop();
    }
}

void NoticeWidget::setupUI()
{
    setObjectName("NoticeWidget");

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);

    // 根据用户类型设置不同UI
    if (m_userType == STUDENT) {
        setupStudentUI();
    } else {
        setupTeacherUI();
    }

    // 应用样式 - 现代化通知系统设计
    setStyleSheet(R"(
        QWidget#NoticeWidget {
            background-color: #f8f9fa;
        }

        QTableWidget {
            gridline-color: #dee2e6;
            background-color: white;
            alternate-background-color: #f1f3f4;
            selection-background-color: #4285f4;
            border: 1px solid #dadce0;
            border-radius: 8px;
            font-size: 14px;
        }

        QTableWidget::item {
            padding: 12px 8px;
            border-bottom: 1px solid #f1f3f4;
        }

        QTableWidget::item:selected {
            background-color: #4285f4;
            color: white;
        }

        QHeaderView::section {
            background-color: #5f6368;
            color: white;
            padding: 12px 8px;
            border: none;
            font-weight: 600;
            font-size: 13px;
        }

        QPushButton {
            background-color: #1a73e8;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-size: 14px;
            font-weight: 500;
        }

        QPushButton:hover {
            background-color: #1557b0;
        }

        QPushButton:pressed {
            background-color: #0d47a1;
        }

        QPushButton:disabled {
            background-color: #9aa0a6;
        }

        QPushButton#PublishButton {
            background-color: #34a853;
        }

        QPushButton#PublishButton:hover {
            background-color: #2d7d32;
        }

        QPushButton#EditButton {
            background-color: #fbbc05;
            color: #202124;
        }

        QPushButton#EditButton:hover {
            background-color: #f9ab00;
        }

        QPushButton#DeleteButton {
            background-color: #ea4335;
        }

        QPushButton#DeleteButton:hover {
            background-color: #d93025;
        }

        QPushButton#PinButton {
            background-color: #9c27b0;
        }

        QPushButton#PinButton:hover {
            background-color: #7b1fa2;
        }

        QLineEdit, QComboBox {
            padding: 10px;
            border: 2px solid #dadce0;
            border-radius: 6px;
            background-color: white;
            font-size: 14px;
        }

        QLineEdit:focus, QComboBox:focus {
            border-color: #4285f4;
            outline: none;
        }

        QGroupBox {
            font-weight: 600;
            font-size: 15px;
            border: 2px solid #e8eaed;
            border-radius: 12px;
            margin-top: 15px;
            padding-top: 15px;
            background-color: white;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px;
            background-color: white;
        }

        QTextEdit {
            border: 2px solid #e8eaed;
            border-radius: 8px;
            background-color: white;
            padding: 12px;
            font-size: 14px;
            line-height: 1.5;
        }

        QLabel#TitleLabel {
            font-size: 18px;
            font-weight: 700;
            color: #202124;
            margin-bottom: 8px;
        }

        QLabel#InfoLabel {
            font-size: 13px;
            color: #5f6368;
            margin-bottom: 16px;
        }
    )");
}

void NoticeWidget::setupStudentUI()
{
    m_splitter = new QSplitter(Qt::Horizontal);

    // 左侧通知列表区域
    m_listWidget = new QWidget();
    m_listLayout = new QVBoxLayout(m_listWidget);

    // 搜索控制栏
    m_controlLayout = new QHBoxLayout();

    QLabel *searchLabel = new QLabel("🔍 搜索通知:");
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("输入关键词搜索通知内容...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &NoticeWidget::onSearchTextChanged);

    m_controlLayout->addWidget(searchLabel);
    m_controlLayout->addWidget(m_searchEdit, 1);

    // 通知表格
    m_noticeTable = new QTableWidget();
    m_noticeTable->setColumnCount(4);
    QStringList headers;
    headers << "标题" << "课程" << "发布时间" << "状态";
    m_noticeTable->setHorizontalHeaderLabels(headers);
    m_noticeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_noticeTable->setAlternatingRowColors(true);
    m_noticeTable->horizontalHeader()->setStretchLastSection(true);
    // 设置列宽比例
    m_noticeTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_noticeTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_noticeTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_noticeTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    connect(m_noticeTable, &QTableWidget::itemSelectionChanged,
            this, &NoticeWidget::onNoticeSelectionChanged);

    m_listLayout->addLayout(m_controlLayout);
    m_listLayout->addWidget(m_noticeTable, 1);

    // 右侧通知内容区域
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);

    m_noticeGroup = new QGroupBox("📢 通知内容");
    QVBoxLayout *noticeLayout = new QVBoxLayout(m_noticeGroup);

    m_titleLabel = new QLabel("请选择通知查看详情");
    m_titleLabel->setObjectName("TitleLabel");
    m_titleLabel->setWordWrap(true);

    m_infoLabel = new QLabel();
    m_infoLabel->setObjectName("InfoLabel");
    m_infoLabel->setWordWrap(true);

    m_contentText = new QTextEdit();
    m_contentText->setReadOnly(true);
    m_contentText->setPlaceholderText("通知内容将在这里显示...");

    noticeLayout->addWidget(m_titleLabel);
    noticeLayout->addWidget(m_infoLabel);
    noticeLayout->addWidget(m_contentText, 1);

    // 统计信息组
    m_statsGroup = new QGroupBox("📊 统计信息");
    m_statsLabel = new QLabel();
    m_refreshButton = new QPushButton("🔄 刷新通知");
    connect(m_refreshButton, &QPushButton::clicked, this, &NoticeWidget::refreshData);

    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    statsLayout->addWidget(m_statsLabel);
    statsLayout->addWidget(m_refreshButton);

    m_contentLayout->addWidget(m_noticeGroup, 3);
    m_contentLayout->addWidget(m_statsGroup, 1);

    // 设置分割器
    m_splitter->addWidget(m_listWidget);
    m_splitter->addWidget(m_contentWidget);
    m_splitter->setSizes({500, 400});

    m_mainLayout->addWidget(m_splitter);
}

void NoticeWidget::setupTeacherUI()
{
    m_splitter = new QSplitter(Qt::Horizontal);

    // 左侧通知列表区域
    m_listWidget = new QWidget();
    m_listLayout = new QVBoxLayout(m_listWidget);

    // 控制栏
    m_controlLayout = new QHBoxLayout();

    QLabel *courseLabel = new QLabel("📚 选择课程:");
    m_courseCombo = new QComboBox();
    m_courseCombo->setMinimumWidth(200);
    connect(m_courseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NoticeWidget::onCourseSelectionChanged);

    m_publishButton = new QPushButton("✏️ 发布通知");
    m_publishButton->setObjectName("PublishButton");
    m_publishButton->setEnabled(false);
    connect(m_publishButton, &QPushButton::clicked, this, &NoticeWidget::onPublishNotice);

    m_controlLayout->addWidget(courseLabel);
    m_controlLayout->addWidget(m_courseCombo, 1);
    m_controlLayout->addWidget(m_publishButton);

    // 搜索栏
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel("🔍 搜索:");
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索通知标题或内容...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &NoticeWidget::onSearchTextChanged);

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchEdit, 1);

    // 通知表格
    m_noticeTable = new QTableWidget();
    m_noticeTable->setColumnCount(5);
    QStringList headers;
    headers << "标题" << "发布时间" << "状态" << "阅读" << "操作";
    m_noticeTable->setHorizontalHeaderLabels(headers);
    m_noticeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_noticeTable->setAlternatingRowColors(true);
    m_noticeTable->horizontalHeader()->setStretchLastSection(true);

    connect(m_noticeTable, &QTableWidget::itemSelectionChanged,
            this, &NoticeWidget::onNoticeSelectionChanged);

    m_listLayout->addLayout(m_controlLayout);
    m_listLayout->addLayout(searchLayout);
    m_listLayout->addWidget(m_noticeTable, 1);

    // 右侧通知内容和操作区域
    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);

    m_noticeGroup = new QGroupBox("📢 通知详情");
    QVBoxLayout *noticeLayout = new QVBoxLayout(m_noticeGroup);

    m_titleLabel = new QLabel("请选择课程和通知");
    m_titleLabel->setObjectName("TitleLabel");
    m_titleLabel->setWordWrap(true);

    m_infoLabel = new QLabel();
    m_infoLabel->setObjectName("InfoLabel");
    m_infoLabel->setWordWrap(true);

    m_contentText = new QTextEdit();
    m_contentText->setReadOnly(true);

    // 操作按钮
    m_actionLayout = new QHBoxLayout();

    m_editButton = new QPushButton("✏️ 编辑");
    m_editButton->setObjectName("EditButton");
    m_editButton->setEnabled(false);
    connect(m_editButton, &QPushButton::clicked, this, &NoticeWidget::onEditNotice);

    m_pinButton = new QPushButton("📌 置顶");
    m_pinButton->setObjectName("PinButton");
    m_pinButton->setEnabled(false);
    connect(m_pinButton, &QPushButton::clicked, this, &NoticeWidget::onTogglePin);

    m_deleteButton = new QPushButton("🗑️ 删除");
    m_deleteButton->setObjectName("DeleteButton");
    m_deleteButton->setEnabled(false);
    connect(m_deleteButton, &QPushButton::clicked, this, &NoticeWidget::onDeleteNotice);

    m_actionLayout->addWidget(m_editButton);
    m_actionLayout->addWidget(m_pinButton);
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_deleteButton);

    noticeLayout->addWidget(m_titleLabel);
    noticeLayout->addWidget(m_infoLabel);
    noticeLayout->addWidget(m_contentText, 1);
    noticeLayout->addLayout(m_actionLayout);

    // 统计信息组
    m_statsGroup = new QGroupBox("📊 通知统计");
    m_statsLabel = new QLabel();
    m_refreshButton = new QPushButton("🔄 刷新数据");
    connect(m_refreshButton, &QPushButton::clicked, this, &NoticeWidget::refreshData);

    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    statsLayout->addWidget(m_statsLabel);
    statsLayout->addWidget(m_refreshButton);

    m_contentLayout->addWidget(m_noticeGroup, 3);
    m_contentLayout->addWidget(m_statsGroup, 1);

    // 设置分割器
    m_splitter->addWidget(m_listWidget);
    m_splitter->addWidget(m_contentWidget);
    m_splitter->setSizes({600, 400});

    m_mainLayout->addWidget(m_splitter);
}

void NoticeWidget::refreshData()
{
    if (!m_database) {
        qDebug() << "数据库未连接";
        return;
    }

    if (m_userType == TEACHER) {
        loadCourseList();
    }

    loadNotices();
    updateStatistics();

    qDebug() << "通知数据刷新完成";
}

void NoticeWidget::loadCourseList()
{
    m_courseList = m_database->getCoursesByTeacher(m_userId);

    m_courseCombo->clear();
    m_courseCombo->addItem("请选择课程", -1);

    for (const auto &course : m_courseList) {
        QString courseText = QString("%1 (%2)")
        .arg(course["course_name"].toString())
            .arg(course["enrolled_count"].toString() + "人");
        m_courseCombo->addItem(courseText, course["course_id"].toInt());
    }

    qDebug() << "加载了" << m_courseList.size() << "门课程";
}

void NoticeWidget::loadNotices()
{
    m_notices.clear();

    if (!m_database) {
        return;
    }

    QList<QVariantMap> noticeData;

    if (m_userType == STUDENT) {
        // 学生查看所有已选课程的通知
        noticeData = m_database->getStudentAllNotices(m_userId, 100);
    } else {
        // 教师查看指定课程的通知
        if (m_currentCourseId > 0) {
            noticeData = m_database->getCourseNotices(m_currentCourseId, 0);
        }
    }

    // 转换为CourseNotice对象
    for (const auto &data : noticeData) {
        CourseNotice notice(
            data["notice_id"].toInt(),
            data["course_id"].toInt(),
            data["title"].toString(),
            data["content"].toString(),
            data["publish_time"].toDateTime(),
            data["is_pinned"].toBool(),
            data.contains("course_name") ? data["course_name"].toString() : ""
            );
        m_notices.append(notice);
    }

    // 按置顶和时间排序
    std::sort(m_notices.begin(), m_notices.end());

    updateNoticeTable();
    clearNoticeContent();

    qDebug() << "加载了" << m_notices.size() << "条通知";
}

void NoticeWidget::updateNoticeTable()
{
    m_noticeTable->setRowCount(m_notices.size());

    for (int i = 0; i < m_notices.size(); ++i) {
        const CourseNotice &notice = m_notices[i];

        // 标题（带置顶标识）
        QTableWidgetItem *titleItem = new QTableWidgetItem(notice.getDisplayTitle());
        if (notice.isPinned()) {
            titleItem->setForeground(QColor("#e91e63"));
            titleItem->setFont(QFont(titleItem->font().family(), titleItem->font().pointSize(), QFont::Bold));
        }
        if (notice.isNew()) {
            titleItem->setBackground(QColor("#fff3e0"));
        }
        m_noticeTable->setItem(i, 0, titleItem);

        if (m_userType == STUDENT) {
            // 学生端显示课程名称
            m_noticeTable->setItem(i, 1, new QTableWidgetItem(notice.getCourseName()));
            m_noticeTable->setItem(i, 2, new QTableWidgetItem(notice.getRelativeTimeText()));

            QString statusText = notice.isPinned() ? "📌 置顶" : "📢 普通";
            if (notice.isNew()) statusText += " 🆕";
            m_noticeTable->setItem(i, 3, new QTableWidgetItem(statusText));
        } else {
            // 教师端不显示课程名称
            m_noticeTable->setItem(i, 1, new QTableWidgetItem(notice.getRelativeTimeText()));

            QString statusText = notice.isPinned() ? "📌 置顶" : "📢 普通";
            m_noticeTable->setItem(i, 2, new QTableWidgetItem(statusText));
            m_noticeTable->setItem(i, 3, new QTableWidgetItem("--"));
            m_noticeTable->setItem(i, 4, new QTableWidgetItem("📝 管理"));
        }

        // 存储通知对象用于后续操作
        m_noticeTable->item(i, 0)->setData(Qt::UserRole, i);
    }

    m_noticeTable->resizeColumnsToContents();
}

void NoticeWidget::updateStatistics()
{
    m_totalNotices = m_notices.size();
    m_pinnedNotices = 0;
    m_recentNotices = 0;

    QDateTime weekAgo = QDateTime::currentDateTime().addDays(-7);

    for (const CourseNotice &notice : m_notices) {
        if (notice.isPinned()) {
            m_pinnedNotices++;
        }
        if (notice.getPublishTime() >= weekAgo) {
            m_recentNotices++;
        }
    }

    QString statsText;
    if (m_userType == STUDENT) {
        statsText = QString(
                        "📊 通知统计\n\n"
                        "总通知数: %1 条\n"
                        "置顶通知: %2 条\n"
                        "本周新增: %3 条\n"
                        "未读通知: -- 条"
                        ).arg(m_totalNotices).arg(m_pinnedNotices).arg(m_recentNotices);
    } else {
        statsText = QString(
                        "📊 通知统计\n\n"
                        "已发布: %1 条\n"
                        "置顶通知: %2 条\n"
                        "本周发布: %3 条\n"
                        "课程数: %4 门"
                        ).arg(m_totalNotices).arg(m_pinnedNotices).arg(m_recentNotices).arg(m_courseList.size());
    }

    m_statsLabel->setText(statsText);
}

void NoticeWidget::showNoticeContent(const CourseNotice &notice)
{
    m_titleLabel->setText(notice.getDisplayTitle());

    QString infoText;
    if (m_userType == STUDENT) {
        infoText = QString("📚 课程: %1  |  ⏰ 发布时间: %2  |  👁️ 状态: %3")
                       .arg(notice.getCourseName())
                       .arg(notice.getPublishTimeText())
                       .arg(notice.getPinnedStatusText());
    } else {
        infoText = QString("⏰ 发布时间: %1  |  👁️ 状态: %2  |  🔄 更新: %3")
                       .arg(notice.getPublishTimeText())
                       .arg(notice.getPinnedStatusText())
                       .arg(notice.getRelativeTimeText());
    }
    m_infoLabel->setText(infoText);

    m_contentText->setPlainText(notice.getContent());

    // 更新教师端操作按钮状态
    if (m_userType == TEACHER) {
        bool canManage = validateNoticePermissions(notice.getCourseId());
        m_editButton->setEnabled(canManage);
        m_deleteButton->setEnabled(canManage);
        m_pinButton->setEnabled(canManage);
        m_pinButton->setText(notice.isPinned() ? "📌 取消置顶" : "📌 置顶");
    }
}

void NoticeWidget::clearNoticeContent()
{
    if (m_userType == STUDENT) {
        m_titleLabel->setText("请选择通知查看详情");
    } else {
        m_titleLabel->setText("请选择课程和通知");
    }
    m_infoLabel->clear();
    m_contentText->clear();

    if (m_userType == TEACHER) {
        m_editButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
        m_pinButton->setEnabled(false);
    }
}

bool NoticeWidget::validateNoticePermissions(int courseId)
{
    return m_database && m_database->canManageCourseNotices(m_userId, courseId);
}

void NoticeWidget::setCourseId(int courseId)
{
    m_currentCourseId = courseId;

    if (m_userType == TEACHER && m_courseCombo) {
        // 在下拉框中选中对应课程
        for (int i = 0; i < m_courseCombo->count(); ++i) {
            if (m_courseCombo->itemData(i).toInt() == courseId) {
                m_courseCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    refreshData();
}

void NoticeWidget::onNoticeSelectionChanged()
{
    int row = m_noticeTable->currentRow();
    if (row < 0 || row >= m_notices.size()) {
        m_selectedNotice = CourseNotice();
        clearNoticeContent();
        return;
    }

    int noticeIndex = m_noticeTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (noticeIndex < 0 || noticeIndex >= m_notices.size()) {
        return;
    }

    m_selectedNotice = m_notices[noticeIndex];
    showNoticeContent(m_selectedNotice);
}

void NoticeWidget::onCourseSelectionChanged()
{
    if (!m_courseCombo) return;

    int courseId = m_courseCombo->currentData().toInt();
    m_currentCourseId = courseId;

    m_publishButton->setEnabled(courseId > 0);

    loadNotices();
    updateStatistics();
}

void NoticeWidget::onPublishNotice()
{
    if (m_currentCourseId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要发布通知的课程！");
        return;
    }

    if (!validateNoticePermissions(m_currentCourseId)) {
        QMessageBox::warning(this, "权限不足", "您没有权限在该课程发布通知！");
        return;
    }

    // 创建新通知
    CourseNotice newNotice;
    newNotice.setCourseId(m_currentCourseId);

    NoticeEditDialog dialog(newNotice, false, this);
    if (dialog.exec() == QDialog::Accepted) {
        CourseNotice notice = dialog.getNotice();

        if (m_database->addCourseNotice(
                notice.getCourseId(),
                notice.getTitle(),
                notice.getContent(),
                notice.isPinned(),
                m_userId)) {

            QMessageBox::information(this, "成功", "通知发布成功！");
            emit noticePublished(notice.getCourseId(), notice.getTitle());
            refreshData();
        } else {
            QMessageBox::critical(this, "失败", "通知发布失败！请检查网络连接和权限。");
        }
    }
}

void NoticeWidget::onEditNotice()
{
    if (!m_selectedNotice.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的通知！");
        return;
    }

    if (!validateNoticePermissions(m_selectedNotice.getCourseId())) {
        QMessageBox::warning(this, "权限不足", "您没有权限编辑该通知！");
        return;
    }

    NoticeEditDialog dialog(m_selectedNotice, true, this);
    if (dialog.exec() == QDialog::Accepted) {
        CourseNotice notice = dialog.getNotice();

        if (m_database->updateCourseNotice(
                notice.getNoticeId(),
                notice.getTitle(),
                notice.getContent(),
                notice.isPinned(),
                m_userId)) {

            QMessageBox::information(this, "成功", "通知更新成功！");
            emit noticeUpdated(notice.getNoticeId(), notice.getTitle());
            refreshData();
        } else {
            QMessageBox::critical(this, "失败", "通知更新失败！请检查网络连接和权限。");
        }
    }
}

void NoticeWidget::onDeleteNotice()
{
    if (!m_selectedNotice.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要删除的通知！");
        return;
    }

    if (!validateNoticePermissions(m_selectedNotice.getCourseId())) {
        QMessageBox::warning(this, "权限不足", "您没有权限删除该通知！");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除通知《%1》吗？\n此操作不可恢复！")
                                        .arg(m_selectedNotice.getTitle()),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->deleteCourseNotice(m_selectedNotice.getNoticeId(), m_userId)) {
            QMessageBox::information(this, "成功", "通知删除成功！");
            emit noticeDeleted(m_selectedNotice.getNoticeId());
            refreshData();
        } else {
            QMessageBox::critical(this, "失败", "通知删除失败！请检查网络连接和权限。");
        }
    }
}

void NoticeWidget::onTogglePin()
{
    if (!m_selectedNotice.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要置顶的通知！");
        return;
    }

    if (!validateNoticePermissions(m_selectedNotice.getCourseId())) {
        QMessageBox::warning(this, "权限不足", "您没有权限修改该通知的置顶状态！");
        return;
    }

    bool newPinStatus = !m_selectedNotice.isPinned();
    QString action = newPinStatus ? "置顶" : "取消置顶";

    if (m_database->updateCourseNotice(
            m_selectedNotice.getNoticeId(),
            m_selectedNotice.getTitle(),
            m_selectedNotice.getContent(),
            newPinStatus,
            m_userId)) {

        QMessageBox::information(this, "成功", QString("通知%1成功！").arg(action));
        emit noticeUpdated(m_selectedNotice.getNoticeId(), m_selectedNotice.getTitle());
        refreshData();
    } else {
        QMessageBox::critical(this, "失败", QString("通知%1失败！").arg(action));
    }
}

void NoticeWidget::onSearchTextChanged()
{
    QString searchText = m_searchEdit->text().toLower();

    for (int i = 0; i < m_noticeTable->rowCount(); ++i) {
        bool visible = true;

        if (!searchText.isEmpty()) {
            QString title = m_noticeTable->item(i, 0)->text().toLower();
            int noticeIndex = m_noticeTable->item(i, 0)->data(Qt::UserRole).toInt();

            if (noticeIndex >= 0 && noticeIndex < m_notices.size()) {
                QString content = m_notices[noticeIndex].getContent().toLower();
                visible = title.contains(searchText) || content.contains(searchText);
            } else {
                visible = title.contains(searchText);
            }
        }

        m_noticeTable->setRowHidden(i, !visible);
    }
}

void NoticeWidget::autoRefresh()
{
    // 静默刷新统计信息，不影响用户当前操作
    if (m_database) {
        updateStatistics();
    }
}

// ============================================================================
// NoticeEditDialog 实现
// ============================================================================

NoticeEditDialog::NoticeEditDialog(const CourseNotice &notice, bool isEdit, QWidget *parent)
    : QDialog(parent), m_notice(notice), m_isEdit(isEdit)
{
    setupUI();
    if (isEdit) {
        loadNoticeData();
    }

    setModal(true);
    resize(600, 500);
}

CourseNotice NoticeEditDialog::getNotice() const
{
    return m_notice;
}

void NoticeEditDialog::setupUI()
{
    setWindowTitle(m_isEdit ? "编辑通知" : "发布新通知");
    setWindowIcon(QIcon(":/icons/notice.png"));

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题输入
    QLabel *titleLabel = new QLabel("📝 通知标题:");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("请输入通知标题（必填）");
    m_titleEdit->setMaxLength(200);

    // 内容输入
    QLabel *contentLabel = new QLabel("📄 通知内容:");
    contentLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_contentEdit = new QTextEdit();
    m_contentEdit->setPlaceholderText("请输入通知的详细内容...");

    // 置顶选项
    m_pinnedCheck = new QCheckBox("📌 设为置顶通知");
    m_pinnedCheck->setStyleSheet("font-size: 14px;");

    // 按钮
    m_buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton(m_isEdit ? "💾 保存修改" : "📤 发布通知");
    m_cancelButton = new QPushButton("❌ 取消");

    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &NoticeEditDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &NoticeEditDialog::onCancel);

    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_okButton);

    // 布局组装
    m_mainLayout->addWidget(titleLabel);
    m_mainLayout->addWidget(m_titleEdit);
    m_mainLayout->addWidget(contentLabel);
    m_mainLayout->addWidget(m_contentEdit, 1);
    m_mainLayout->addWidget(m_pinnedCheck);
    m_mainLayout->addLayout(m_buttonLayout);

    // 应用样式
    setStyleSheet(R"(
        QDialog {
            background-color: #f8f9fa;
        }

        QLabel {
            color: #202124;
        }

        QLineEdit, QTextEdit {
            border: 2px solid #dadce0;
            border-radius: 6px;
            padding: 10px;
            background-color: white;
            font-size: 14px;
        }

        QLineEdit:focus, QTextEdit:focus {
            border-color: #4285f4;
        }

        QCheckBox {
            color: #202124;
        }

        QCheckBox::indicator {
            width: 18px;
            height: 18px;
        }

        QCheckBox::indicator:unchecked {
            border: 2px solid #dadce0;
            background-color: white;
            border-radius: 3px;
        }

        QCheckBox::indicator:checked {
            border: 2px solid #4285f4;
            background-color: #4285f4;
            border-radius: 3px;
        }

        QPushButton {
            background-color: #1a73e8;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 6px;
            font-size: 14px;
            font-weight: 500;
            min-width: 100px;
        }

        QPushButton:hover {
            background-color: #1557b0;
        }

        QPushButton:pressed {
            background-color: #0d47a1;
        }

        QPushButton:default {
            background-color: #34a853;
        }

        QPushButton:default:hover {
            background-color: #2d7d32;
        }
    )");
}

void NoticeEditDialog::loadNoticeData()
{
    m_titleEdit->setText(m_notice.getTitle());
    m_contentEdit->setPlainText(m_notice.getContent());
    m_pinnedCheck->setChecked(m_notice.isPinned());
}

bool NoticeEditDialog::validateInput()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入通知标题！");
        m_titleEdit->setFocus();
        return false;
    }

    if (m_contentEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入通知内容！");
        m_contentEdit->setFocus();
        return false;
    }

    return true;
}

void NoticeEditDialog::onAccept()
{
    if (!validateInput()) {
        return;
    }

    // 更新通知对象
    m_notice.setTitle(m_titleEdit->text().trimmed());
    m_notice.setContent(m_contentEdit->toPlainText().trimmed());
    m_notice.setPinned(m_pinnedCheck->isChecked());

    if (!m_isEdit) {
        m_notice.setPublishTime(QDateTime::currentDateTime());
    }

    accept();
}

void NoticeEditDialog::onCancel()
{
    reject();
}
