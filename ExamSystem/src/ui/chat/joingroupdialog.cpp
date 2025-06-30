#include "joingroupdialog.h"
#include <QDebug>

JoinGroupDialog::JoinGroupDialog(Database *database, int userId, const QString &userType, QWidget *parent)
    : QDialog(parent)
    , m_database(database)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
    , m_selectedGroupId(-1)
{
    setWindowTitle("搜索加入群聊");
    setModal(true);
    resize(500, 600);

    setupUI();
    setupStyles();

    // 设置定时器
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(800); // 800ms延迟搜索
    connect(m_searchTimer, &QTimer::timeout, this, &JoinGroupDialog::onDelayedSearch);
}

void JoinGroupDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 16, 16, 16);
    m_mainLayout->setSpacing(16);

    // 搜索区域
    m_searchGroupBox = new QGroupBox("搜索群聊");
    QVBoxLayout *searchLayout = new QVBoxLayout(m_searchGroupBox);

    // 搜索输入
    QHBoxLayout *searchInputLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("输入群聊名称或群聊ID搜索...");
    m_searchButton = new QPushButton("🔍 搜索");
    m_searchButton->setObjectName("searchButton");

    searchInputLayout->addWidget(m_searchEdit);
    searchInputLayout->addWidget(m_searchButton);

    // 搜索提示
    m_searchHintLabel = new QLabel("💡 您可以通过群聊名称或群聊ID来搜索群聊");
    m_searchHintLabel->setObjectName("hintLabel");
    m_searchHintLabel->setWordWrap(true);

    searchLayout->addLayout(searchInputLayout);
    searchLayout->addWidget(m_searchHintLabel);

    // 搜索结果区域
    m_resultsGroupBox = new QGroupBox("搜索结果");
    QVBoxLayout *resultsLayout = new QVBoxLayout(m_resultsGroupBox);

    m_resultsListWidget = new QListWidget();
    m_resultsListWidget->setMinimumHeight(300);
    m_resultsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    m_statusLabel = new QLabel("请输入关键词开始搜索");
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    resultsLayout->addWidget(m_resultsListWidget);
    resultsLayout->addWidget(m_statusLabel);

    // 底部按钮
    m_buttonLayout = new QHBoxLayout();
    m_joinButton = new QPushButton("申请加入");
    m_cancelButton = new QPushButton("取消");

    m_joinButton->setObjectName("primaryButton");
    m_cancelButton->setObjectName("secondaryButton");
    m_joinButton->setEnabled(false);

    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_joinButton);

    // 组装主布局
    m_mainLayout->addWidget(m_searchGroupBox);
    m_mainLayout->addWidget(m_resultsGroupBox);
    m_mainLayout->addLayout(m_buttonLayout);

    // 连接信号
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &JoinGroupDialog::onSearchTextChanged);
    connect(m_searchButton, &QPushButton::clicked,
            this, &JoinGroupDialog::onSearchClicked);
    connect(m_resultsListWidget, &QListWidget::itemClicked,
            this, &JoinGroupDialog::onGroupItemClicked);
    connect(m_joinButton, &QPushButton::clicked,
            this, &JoinGroupDialog::onJoinClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &JoinGroupDialog::onCancelClicked);
}

void JoinGroupDialog::setupStyles()
{
    QString styles = R"(
        QDialog {
            background-color: #f5f5f5;
        }

        QGroupBox {
            font-weight: bold;
            border: 2px solid #ddd;
            border-radius: 8px;
            margin-top: 8px;
            padding-top: 8px;
            background-color: white;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px 0 8px;
        }

        QLineEdit {
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 14px;
        }

        QLineEdit:focus {
            border-color: #2196F3;
        }

        QListWidget {
            border: 1px solid #ddd;
            border-radius: 4px;
            background-color: white;
        }

        QListWidget::item {
            padding: 12px;
            border-bottom: 1px solid #f0f0f0;
            min-height: 50px;
        }

        QListWidget::item:selected {
            background-color: #e3f2fd;
            border-left: 4px solid #2196F3;
        }

        QListWidget::item:hover {
            background-color: #f5f5f5;
        }

        #searchButton {
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 14px;
            min-width: 80px;
        }

        #searchButton:hover {
            background-color: #1976D2;
        }

        #primaryButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
            min-width: 100px;
        }

        #primaryButton:hover {
            background-color: #45a049;
        }

        #primaryButton:disabled {
            background-color: #cccccc;
        }

        #secondaryButton {
            background-color: #f0f0f0;
            color: #333;
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
            min-width: 80px;
        }

        #secondaryButton:hover {
            background-color: #e0e0e0;
        }

        #hintLabel {
            color: #666;
            font-size: 12px;
            background-color: #e3f2fd;
            border: 1px solid #bbdefb;
            border-radius: 4px;
            padding: 8px;
        }

        #statusLabel {
            color: #999;
            font-size: 14px;
            padding: 20px;
        }
    )";

    setStyleSheet(styles);
}

void JoinGroupDialog::onSearchClicked()
{
    searchGroups();
}

void JoinGroupDialog::onSearchTextChanged()
{
    // 重启定时器，实现延迟搜索
    m_searchTimer->stop();

    QString searchText = m_searchEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        m_searchTimer->start();
    } else {
        clearSearchResults();
        m_statusLabel->setText("请输入关键词开始搜索");
    }
}

void JoinGroupDialog::onDelayedSearch()
{
    searchGroups();
}

void JoinGroupDialog::searchGroups()
{
    QString searchText = m_searchEdit->text().trimmed();

    if (searchText.isEmpty()) {
        clearSearchResults();
        m_statusLabel->setText("请输入关键词开始搜索");
        return;
    }

    qDebug() << "搜索群聊:" << searchText;

    clearSearchResults();
    m_statusLabel->setText("正在搜索...");

    // 获取所有群聊进行搜索
    QList<QVariantMap> allGroups = m_database->getAllGroups();
    m_searchResults.clear();

    // 按群聊名称和ID进行匹配
    for (const QVariantMap &group : allGroups) {
        QString groupName = group["group_name"].toString();
        int groupId = group["group_id"].toInt();

        bool nameMatch = groupName.contains(searchText, Qt::CaseInsensitive);
        bool idMatch = QString::number(groupId).contains(searchText);

        if (nameMatch || idMatch) {
            m_searchResults.append(group);
            createGroupSearchItem(group);
        }
    }

    if (m_searchResults.isEmpty()) {
        m_statusLabel->setText("未找到相关群聊");
    } else {
        m_statusLabel->setText(QString("找到 %1 个相关群聊").arg(m_searchResults.size()));
    }

    qDebug() << "搜索完成，找到" << m_searchResults.size() << "个群聊";
}

void JoinGroupDialog::createGroupSearchItem(const QVariantMap &group)
{
    int groupId = group["group_id"].toInt();
    QString groupName = group["group_name"].toString();
    QString creatorName = group["creator_name"].toString();
    QString creatorType = group["creator_type"].toString();
    int memberCount = group["member_count"].toInt();

    QString statusText = getGroupStatusText(groupId);

    QString itemText = QString("群聊ID: %1\n群聊名称: %2\n创建者: %3 %4  成员数: %5人\n状态: %6")
                           .arg(groupId)
                           .arg(groupName)
                           .arg(creatorName)
                           .arg(creatorType)
                           .arg(memberCount)
                           .arg(statusText);

    QListWidgetItem *item = new QListWidgetItem(itemText);
    item->setData(Qt::UserRole, groupId);
    item->setData(Qt::UserRole + 1, groupName);
    item->setData(Qt::UserRole + 2, statusText);

    // 根据状态设置项目的启用状态
    if (statusText == "已是成员" || statusText == "申请中") {
        item->setFlags(Qt::ItemIsEnabled); // 不可选择
        item->setBackground(QBrush(QColor(240, 240, 240))); // 灰色背景
    }

    m_resultsListWidget->addItem(item);
}

QString JoinGroupDialog::getGroupStatusText(int groupId)
{
    // 检查是否已经是群成员
    if (m_database->isGroupMember(groupId, m_currentUserId, m_currentUserType)) {
        return "已是成员";
    }

    // 检查是否有待处理的申请
    QList<QVariantMap> userRequests = m_database->getUserGroupRequests(m_currentUserId, m_currentUserType);
    for (const QVariantMap &request : userRequests) {
        if (request["group_id"].toInt() == groupId && request["status"].toString() == "申请中") {
            return "申请中";
        }
    }

    return "可申请";
}

void JoinGroupDialog::onGroupItemClicked()
{
    QListWidgetItem *currentItem = m_resultsListWidget->currentItem();
    if (!currentItem) {
        m_joinButton->setEnabled(false);
        return;
    }

    m_selectedGroupId = currentItem->data(Qt::UserRole).toInt();
    m_selectedGroupName = currentItem->data(Qt::UserRole + 1).toString();
    QString status = currentItem->data(Qt::UserRole + 2).toString();

    // 只有可申请的群聊才能启用加入按钮
    m_joinButton->setEnabled(status == "可申请");

    qDebug() << "选择群聊:" << m_selectedGroupId << m_selectedGroupName << "状态:" << status;
}

void JoinGroupDialog::onJoinClicked()
{
    if (m_selectedGroupId <= 0) {
        QMessageBox::warning(this, "请选择群聊", "请先选择要加入的群聊。");
        return;
    }

    qDebug() << "申请加入群聊:" << m_selectedGroupId << m_selectedGroupName;

    // 发送加群申请
    bool success = m_database->sendGroupRequest(m_selectedGroupId, m_currentUserId, m_currentUserType);

    if (success) {
        QMessageBox::information(this, "申请已发送",
                                 QString("您的加群申请已发送到群聊「%1」，请等待群主审核。").arg(m_selectedGroupName));
        accept();
    } else {
        QMessageBox::critical(this, "申请失败", "发送加群申请失败，请重试。");
    }
}

void JoinGroupDialog::onCancelClicked()
{
    reject();
}

void JoinGroupDialog::clearSearchResults()
{
    m_resultsListWidget->clear();
    m_searchResults.clear();
    m_selectedGroupId = -1;
    m_selectedGroupName.clear();
    m_joinButton->setEnabled(false);
}
