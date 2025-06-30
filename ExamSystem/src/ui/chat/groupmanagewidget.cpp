#include "groupmanagewidget.h"
#include <QDebug>

GroupManageWidget::GroupManageWidget(Database *database, int userId, const QString &userType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
    , m_selectedGroupId(-1)
    , m_selectedRequestId(-1)
{
    setupUI();
    setupStyles();
    refreshData();
}

void GroupManageWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(12);

    // 我创建的群聊 - 移除管理按钮
    m_createdGroupsBox = new QGroupBox("📋 我创建的群聊");
    QVBoxLayout *createdLayout = new QVBoxLayout(m_createdGroupsBox);

    m_createdGroupsList = new QListWidget();
    m_createdGroupsList->setObjectName("createdGroupsList");
    // 增加高度，因为不需要为按钮预留空间
    m_createdGroupsList->setMinimumHeight(180);

    createdLayout->addWidget(m_createdGroupsList);
    // 移除：createdLayout->addWidget(m_manageCreatedButton);

    // 我加入的群聊 - 移除查看按钮
    m_joinedGroupsBox = new QGroupBox("👥 我加入的群聊");
    QVBoxLayout *joinedLayout = new QVBoxLayout(m_joinedGroupsBox);

    m_joinedGroupsList = new QListWidget();
    m_joinedGroupsList->setObjectName("joinedGroupsList");
    // 增加高度，因为不需要为按钮预留空间
    m_joinedGroupsList->setMinimumHeight(180);

    joinedLayout->addWidget(m_joinedGroupsList);
    // 移除：joinedLayout->addWidget(m_manageJoinedButton);

    // 待处理申请
    m_requestsBox = new QGroupBox("📮 待处理申请");
    QVBoxLayout *requestsLayout = new QVBoxLayout(m_requestsBox);

    m_requestsList = new QListWidget();
    m_requestsList->setObjectName("requestsList");
    m_requestsList->setMinimumHeight(200);

    m_requestButtonsLayout = new QHBoxLayout();
    m_acceptButton = new QPushButton("✅ 同意");
    m_rejectButton = new QPushButton("❌ 拒绝");
    m_acceptButton->setObjectName("acceptButton");
    m_rejectButton->setObjectName("rejectButton");
    m_acceptButton->setEnabled(false);
    m_rejectButton->setEnabled(false);

    m_requestButtonsLayout->addWidget(m_acceptButton);
    m_requestButtonsLayout->addWidget(m_rejectButton);
    m_requestButtonsLayout->addStretch();

    requestsLayout->addWidget(m_requestsList);
    requestsLayout->addLayout(m_requestButtonsLayout);

    // 统计信息
    m_statisticsLabel = new QLabel("正在加载统计信息...");
    m_statisticsLabel->setObjectName("statisticsLabel");
    m_statisticsLabel->setAlignment(Qt::AlignCenter);

    // 组装主布局 - 移除分割器
    m_mainLayout->addWidget(m_createdGroupsBox);
    m_mainLayout->addWidget(m_joinedGroupsBox);
    m_mainLayout->addWidget(m_requestsBox);
    m_mainLayout->addWidget(m_statisticsLabel);
    m_mainLayout->addStretch();

    // 连接信号 - 移除按钮相关的连接
    connect(m_createdGroupsList, &QListWidget::itemClicked,
            this, &GroupManageWidget::onCreatedGroupClicked);
    connect(m_joinedGroupsList, &QListWidget::itemClicked,
            this, &GroupManageWidget::onJoinedGroupClicked);
    connect(m_requestsList, &QListWidget::itemClicked,
            this, &GroupManageWidget::onRequestsClicked);

    connect(m_acceptButton, &QPushButton::clicked,
            this, &GroupManageWidget::onAcceptRequestClicked);
    connect(m_rejectButton, &QPushButton::clicked,
            this, &GroupManageWidget::onRejectRequestClicked);
}

void GroupManageWidget::setupStyles()
{
    QString styles = R"(
        QWidget {
            background-color: #f8f9fa;
            font-family: 'Microsoft YaHei', Arial, sans-serif;
        }

        QGroupBox {
            font-weight: bold;
            border: 2px solid #e9ecef;
            border-radius: 8px;
            margin-top: 8px;
            padding-top: 8px;
            background-color: white;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 8px 0 8px;
            color: #495057;
        }

        #createdGroupsList {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 4px;
        }

        #createdGroupsList::item {
            padding: 10px;
            border-bottom: 1px solid #e9ecef;
            min-height: 40px;
        }

        #createdGroupsList::item:selected {
            background-color: #d4edda;
            border-left: 4px solid #28a745;
            font-weight: bold;
        }

        #createdGroupsList::item:hover {
            background-color: #e9ecef;
        }

        #joinedGroupsList {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 4px;
        }

        #joinedGroupsList::item {
            padding: 10px;
            border-bottom: 1px solid #e9ecef;
            min-height: 40px;
        }

        #joinedGroupsList::item:selected {
            background-color: #cce5ff;
            border-left: 4px solid #007bff;
            font-weight: bold;
        }

        #joinedGroupsList::item:hover {
            background-color: #e9ecef;
        }

        #requestsList {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 4px;
        }

        #requestsList::item {
            padding: 10px;
            border-bottom: 1px solid #e9ecef;
            min-height: 50px;
        }

        #requestsList::item:selected {
            background-color: #fff3cd;
            border-left: 4px solid #ffc107;
            font-weight: bold;
        }

        #requestsList::item:hover {
            background-color: #e9ecef;
        }

        #acceptButton {
            background-color: #28a745;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 16px;
            font-size: 14px;
            font-weight: bold;
        }

        #acceptButton:hover {
            background-color: #218838;
        }

        #acceptButton:disabled {
            background-color: #6c757d;
        }

        #rejectButton {
            background-color: #dc3545;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 16px;
            font-size: 14px;
            font-weight: bold;
        }

        #rejectButton:hover {
            background-color: #c82333;
        }

        #rejectButton:disabled {
            background-color: #6c757d;
        }

        #statisticsLabel {
            background-color: white;
            border: 1px solid #dee2e6;
            border-radius: 8px;
            padding: 16px;
            font-size: 14px;
            color: #495057;
        }
    )";

    setStyleSheet(styles);
}

void GroupManageWidget::refreshData()
{
    qDebug() << "刷新群聊管理数据";

    loadCreatedGroups();
    loadJoinedGroups();
    loadPendingRequests();
    updateStatistics();

    // 清除选择状态
    clearSelection();
}

void GroupManageWidget::clearSelection()
{
    m_createdGroupsList->clearSelection();
    m_joinedGroupsList->clearSelection();
    m_requestsList->clearSelection();

    m_selectedGroupId = -1;
    m_selectedRequestId = -1;

    m_acceptButton->setEnabled(false);
    m_rejectButton->setEnabled(false);

    emit noGroupSelected();
}

void GroupManageWidget::onCreatedGroupClicked()
{
    QListWidgetItem *item = m_createdGroupsList->currentItem();
    if (!item) return;

    m_selectedGroupId = item->data(Qt::UserRole).toInt();

    // 清除其他选择
    m_joinedGroupsList->clearSelection();
    m_requestsList->clearSelection();
    m_acceptButton->setEnabled(false);
    m_rejectButton->setEnabled(false);

    qDebug() << "选择我创建的群聊:" << m_selectedGroupId;
    emit groupSelected(m_selectedGroupId, true);  // true表示是创建者
}

void GroupManageWidget::onJoinedGroupClicked()
{
    QListWidgetItem *item = m_joinedGroupsList->currentItem();
    if (!item) return;

    m_selectedGroupId = item->data(Qt::UserRole).toInt();

    // 清除其他选择
    m_createdGroupsList->clearSelection();
    m_requestsList->clearSelection();
    m_acceptButton->setEnabled(false);
    m_rejectButton->setEnabled(false);

    qDebug() << "选择我加入的群聊:" << m_selectedGroupId;
    emit groupSelected(m_selectedGroupId, false);  // false表示是普通成员
}

void GroupManageWidget::onRequestsClicked()
{
    QListWidgetItem *item = m_requestsList->currentItem();
    if (!item) return;

    m_selectedRequestId = item->data(Qt::UserRole).toInt();
    m_selectedGroupId = item->data(Qt::UserRole + 1).toInt();
    m_acceptButton->setEnabled(true);
    m_rejectButton->setEnabled(true);

    // 清除其他选择
    m_createdGroupsList->clearSelection();
    m_joinedGroupsList->clearSelection();

    qDebug() << "选择申请:" << m_selectedRequestId;
    emit noGroupSelected();  // 申请详情暂时不在第三栏显示，可以后续扩展
}

void GroupManageWidget::loadCreatedGroups()
{
    m_createdGroupsList->clear();
    m_createdGroups.clear();

    QList<QVariantMap> userGroups = m_database->getUserGroups(m_currentUserId, m_currentUserType);

    for (const QVariantMap &group : userGroups) {
        if (group["user_role"].toString() == "创建者") {
            m_createdGroups.append(group);

            QString itemText = QString("%1\n成员: %2人")
                                   .arg(group["group_name"].toString())
                                   .arg(group["member_count"].toInt());

            QListWidgetItem *item = new QListWidgetItem(itemText);
            item->setData(Qt::UserRole, group["group_id"]);
            m_createdGroupsList->addItem(item);
        }
    }

    qDebug() << "加载我创建的群聊:" << m_createdGroups.size() << "个";
}

void GroupManageWidget::loadJoinedGroups()
{
    m_joinedGroupsList->clear();
    m_joinedGroups.clear();

    QList<QVariantMap> userGroups = m_database->getUserGroups(m_currentUserId, m_currentUserType);

    for (const QVariantMap &group : userGroups) {
        if (group["user_role"].toString() == "成员") {
            m_joinedGroups.append(group);

            QString itemText = QString("%1\n成员: %2人 · 创建者: %3")
                                   .arg(group["group_name"].toString())
                                   .arg(group["member_count"].toInt())
                                   .arg(group["creator_name"].toString());

            QListWidgetItem *item = new QListWidgetItem(itemText);
            item->setData(Qt::UserRole, group["group_id"]);
            m_joinedGroupsList->addItem(item);
        }
    }

    qDebug() << "加载我加入的群聊:" << m_joinedGroups.size() << "个";
}

void GroupManageWidget::loadPendingRequests()
{
    m_requestsList->clear();
    m_pendingRequests.clear();

    // 获取我创建的群聊的待处理申请
    for (const QVariantMap &group : m_createdGroups) {
        int groupId = group["group_id"].toInt();
        QList<QVariantMap> requests = m_database->getGroupRequests(groupId);

        for (const QVariantMap &request : requests) {
            m_pendingRequests.append(request);

            QString itemText = QString("%1 (%2) 申请加入\n群聊: %3\n时间: %4")
                                   .arg(request["requester_name"].toString())
                                   .arg(request["requester_type"].toString())
                                   .arg(group["group_name"].toString())
                                   .arg(request["request_time"].toDateTime().toString("MM-dd hh:mm"));

            QListWidgetItem *item = new QListWidgetItem(itemText);
            item->setData(Qt::UserRole, request["request_id"]);
            item->setData(Qt::UserRole + 1, groupId);
            m_requestsList->addItem(item);
        }
    }

    qDebug() << "加载待处理申请:" << m_pendingRequests.size() << "个";
}

void GroupManageWidget::updateStatistics()
{
    QString statsText = QString("📋 我创建的群聊: %1个\n👥 我加入的群聊: %2个\n📮 待处理申请: %3条")
                            .arg(m_createdGroups.size())
                            .arg(m_joinedGroups.size())
                            .arg(m_pendingRequests.size());

    m_statisticsLabel->setText(statsText);
}

void GroupManageWidget::onAcceptRequestClicked()
{
    if (m_selectedRequestId <= 0) return;

    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "确认操作", "确定要同意这个加群申请吗？",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        bool success = m_database->processGroupRequest(m_selectedRequestId, true);
        if (success) {
            QMessageBox::information(this, "操作成功", "已同意加群申请");
            refreshData();
            emit groupRequestProcessed();
        } else {
            QMessageBox::critical(this, "操作失败", "处理申请失败，请重试");
        }
    }
}

void GroupManageWidget::onRejectRequestClicked()
{
    if (m_selectedRequestId <= 0) return;

    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "确认操作", "确定要拒绝这个加群申请吗？",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        bool success = m_database->processGroupRequest(m_selectedRequestId, false);
        if (success) {
            QMessageBox::information(this, "操作成功", "已拒绝加群申请");
            refreshData();
            emit groupRequestProcessed();
        } else {
            QMessageBox::critical(this, "操作失败", "处理申请失败，请重试");
        }
    }
}

QString GroupManageWidget::findGroupName(int groupId)
{
    for (const QVariantMap &group : m_createdGroups) {
        if (group["group_id"].toInt() == groupId) {
            return group["group_name"].toString();
        }
    }
    return QString("群聊 #%1").arg(groupId);
}
