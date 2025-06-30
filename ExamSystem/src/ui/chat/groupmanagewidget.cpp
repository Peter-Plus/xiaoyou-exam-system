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
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(8);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);

    // 左侧管理区域
    m_leftWidget = new QWidget();
    m_leftWidget->setMinimumWidth(350);
    m_leftWidget->setMaximumWidth(500);

    m_leftLayout = new QVBoxLayout(m_leftWidget);
    m_leftLayout->setContentsMargins(8, 8, 8, 8);
    m_leftLayout->setSpacing(12);

    // 我创建的群聊
    m_createdGroupsBox = new QGroupBox("📋 我创建的群聊");
    QVBoxLayout *createdLayout = new QVBoxLayout(m_createdGroupsBox);

    m_createdGroupsList = new QListWidget();
    m_createdGroupsList->setMaximumHeight(150);
    m_createdGroupsList->setObjectName("createdGroupsList");

    m_manageCreatedButton = new QPushButton("管理选中群聊");
    m_manageCreatedButton->setObjectName("manageButton");
    m_manageCreatedButton->setEnabled(false);

    createdLayout->addWidget(m_createdGroupsList);
    createdLayout->addWidget(m_manageCreatedButton);

    // 我加入的群聊
    m_joinedGroupsBox = new QGroupBox("👥 我加入的群聊");
    QVBoxLayout *joinedLayout = new QVBoxLayout(m_joinedGroupsBox);

    m_joinedGroupsList = new QListWidget();
    m_joinedGroupsList->setMaximumHeight(150);
    m_joinedGroupsList->setObjectName("joinedGroupsList");

    m_manageJoinedButton = new QPushButton("查看群聊详情");
    m_manageJoinedButton->setObjectName("manageButton");
    m_manageJoinedButton->setEnabled(false);

    joinedLayout->addWidget(m_joinedGroupsList);
    joinedLayout->addWidget(m_manageJoinedButton);

    // 待处理申请
    m_requestsBox = new QGroupBox("📮 待处理申请");
    QVBoxLayout *requestsLayout = new QVBoxLayout(m_requestsBox);

    m_requestsList = new QListWidget();
    m_requestsList->setMaximumHeight(200);
    m_requestsList->setObjectName("requestsList");

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

    // 组装左侧布局
    m_leftLayout->addWidget(m_createdGroupsBox);
    m_leftLayout->addWidget(m_joinedGroupsBox);
    m_leftLayout->addWidget(m_requestsBox);
    m_leftLayout->addWidget(m_statisticsLabel);
    m_leftLayout->addStretch();

    // 右侧详情区域
    m_rightWidget = new QWidget();
    m_rightLayout = new QVBoxLayout(m_rightWidget);
    m_rightLayout->setContentsMargins(8, 8, 8, 8);

    m_detailsArea = new QScrollArea();
    m_detailsArea->setWidgetResizable(true);
    m_detailsArea->setObjectName("detailsArea");

    m_detailsContent = new QWidget();
    m_detailsLayout = new QVBoxLayout(m_detailsContent);
    m_detailsLayout->setContentsMargins(16, 16, 16, 16);

    m_detailsLabel = new QLabel("选择左侧的群聊或申请查看详细信息");
    m_detailsLabel->setAlignment(Qt::AlignCenter);
    m_detailsLabel->setObjectName("detailsLabel");

    m_detailsLayout->addWidget(m_detailsLabel);
    m_detailsLayout->addStretch();

    m_detailsArea->setWidget(m_detailsContent);
    m_rightLayout->addWidget(m_detailsArea);

    // 添加到分割器
    m_splitter->addWidget(m_leftWidget);
    m_splitter->addWidget(m_rightWidget);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes({400, 600});

    // 连接信号
    connect(m_createdGroupsList, &QListWidget::itemClicked,
            this, &GroupManageWidget::onCreatedGroupClicked);
    connect(m_joinedGroupsList, &QListWidget::itemClicked,
            this, &GroupManageWidget::onJoinedGroupClicked);
    connect(m_requestsList, &QListWidget::itemClicked,
            this, &GroupManageWidget::onRequestsClicked);

    connect(m_manageCreatedButton, &QPushButton::clicked,
            this, &GroupManageWidget::onManageGroupClicked);
    connect(m_manageJoinedButton, &QPushButton::clicked,
            this, &GroupManageWidget::onManageGroupClicked);

    connect(m_acceptButton, &QPushButton::clicked,
            this, &GroupManageWidget::onAcceptRequestClicked);
    connect(m_rejectButton, &QPushButton::clicked,
            this, &GroupManageWidget::onRejectRequestClicked);
}

void GroupManageWidget::setupStyles()
{
    QString styles = R"(
        QWidget {
            background-color: #f5f5f5;
            font-family: 'Microsoft YaHei', Arial, sans-serif;
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

        #createdGroupsList {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 4px;
        }

        #createdGroupsList::item {
            padding: 8px;
            border-bottom: 1px solid #f0f0f0;
        }

        #createdGroupsList::item:selected {
            background-color: #e8f5e8;
            border-left: 3px solid #4CAF50;
        }

        #joinedGroupsList {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 4px;
        }

        #joinedGroupsList::item {
            padding: 8px;
            border-bottom: 1px solid #f0f0f0;
        }

        #joinedGroupsList::item:selected {
            background-color: #e3f2fd;
            border-left: 3px solid #2196F3;
        }

        #requestsList {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 4px;
        }

        #requestsList::item {
            padding: 8px;
            border-bottom: 1px solid #f0f0f0;
        }

        #requestsList::item:selected {
            background-color: #fff3e0;
            border-left: 3px solid #FF9800;
        }

        #manageButton {
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 12px;
        }

        #manageButton:hover {
            background-color: #1976D2;
        }

        #manageButton:disabled {
            background-color: #cccccc;
        }

        #acceptButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 12px;
        }

        #acceptButton:hover {
            background-color: #45a049;
        }

        #acceptButton:disabled {
            background-color: #cccccc;
        }

        #rejectButton {
            background-color: #f44336;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-size: 12px;
        }

        #rejectButton:hover {
            background-color: #d32f2f;
        }

        #rejectButton:disabled {
            background-color: #cccccc;
        }

        #statisticsLabel {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 12px;
            font-size: 12px;
            color: #666;
        }

        #detailsArea {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 8px;
        }

        #detailsLabel {
            color: #999;
            font-size: 16px;
        }

        QSplitter::handle {
            background-color: #ddd;
            width: 2px;
        }

        QSplitter::handle:hover {
            background-color: #bbb;
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
    clearGroupDetails();
}

void GroupManageWidget::loadCreatedGroups()
{
    m_createdGroupsList->clear();
    m_createdGroups.clear();

    QList<QVariantMap> userGroups = m_database->getUserGroups(m_currentUserId, m_currentUserType);

    for (const QVariantMap &group : userGroups) {
        if (group["user_role"].toString() == "创建者") {
            m_createdGroups.append(group);

            QString itemText = QString("%1 (%2人)")
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

            QString itemText = QString("%1 (%2人)\n创建者: %3")
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

            QString itemText = QString("%1 申请加入\n群聊: %2\n时间: %3")
                                   .arg(request["requester_name"].toString())
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
    QString statsText = QString("我创建的群聊: %1个\n我加入的群聊: %2个\n待处理申请: %3条")
                            .arg(m_createdGroups.size())
                            .arg(m_joinedGroups.size())
                            .arg(m_pendingRequests.size());

    m_statisticsLabel->setText(statsText);
}

void GroupManageWidget::onCreatedGroupClicked()
{
    QListWidgetItem *item = m_createdGroupsList->currentItem();
    if (!item) return;

    m_selectedGroupId = item->data(Qt::UserRole).toInt();
    m_manageCreatedButton->setEnabled(true);

    // 清除其他选择
    m_joinedGroupsList->clearSelection();
    m_requestsList->clearSelection();
    m_manageJoinedButton->setEnabled(false);
    m_acceptButton->setEnabled(false);
    m_rejectButton->setEnabled(false);

    showGroupDetails(m_selectedGroupId);
}

void GroupManageWidget::onJoinedGroupClicked()
{
    QListWidgetItem *item = m_joinedGroupsList->currentItem();
    if (!item) return;

    m_selectedGroupId = item->data(Qt::UserRole).toInt();
    m_manageJoinedButton->setEnabled(true);

    // 清除其他选择
    m_createdGroupsList->clearSelection();
    m_requestsList->clearSelection();
    m_manageCreatedButton->setEnabled(false);
    m_acceptButton->setEnabled(false);
    m_rejectButton->setEnabled(false);

    showGroupDetails(m_selectedGroupId);
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
    m_manageCreatedButton->setEnabled(false);
    m_manageJoinedButton->setEnabled(false);

    // 显示申请详情
    for (const QVariantMap &request : m_pendingRequests) {
        if (request["request_id"].toInt() == m_selectedRequestId) {
            QString detailsText = QString(
                                      "<h3>加群申请详情</h3>"
                                      "<p><b>申请者:</b> %1 %2</p>"
                                      "<p><b>申请者学院:</b> %3</p>"
                                      "<p><b>申请时间:</b> %4</p>"
                                      "<p><b>申请群聊:</b> %5</p>"
                                      "<hr>"
                                      "<p>请审核此申请，选择同意或拒绝。</p>"
                                      ).arg(request["requester_name"].toString())
                                      .arg(request["requester_type"].toString())
                                      .arg(request["requester_college"].toString())
                                      .arg(request["request_time"].toDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                                      .arg(findGroupName(m_selectedGroupId));

            m_detailsLabel->setText(detailsText);
            break;
        }
    }
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

void GroupManageWidget::onManageGroupClicked()
{
    if (m_selectedGroupId <= 0) return;

    // 这里可以打开群聊设置对话框
    // 暂时显示消息框
    QMessageBox::information(this, "群聊管理",
                             QString("群聊管理功能开发中...\n群聊ID: %1").arg(m_selectedGroupId));
}

void GroupManageWidget::showGroupDetails(int groupId)
{
    QVariantMap groupInfo = m_database->getGroupInfo(groupId);
    if (groupInfo.isEmpty()) return;

    QList<QVariantMap> members = m_database->getGroupMembers(groupId);

    QString detailsText = QString(
                              "<h3>群聊详情</h3>"
                              "<p><b>群聊名称:</b> %1</p>"
                              "<p><b>群聊ID:</b> %2</p>"
                              "<p><b>创建者:</b> %3 %4</p>"
                              "<p><b>成员数量:</b> %5人</p>"
                              "<p><b>创建时间:</b> %6</p>"
                              "<hr>"
                              "<h4>群聊成员</h4>"
                              ).arg(groupInfo["group_name"].toString())
                              .arg(groupInfo["group_id"].toInt())
                              .arg(groupInfo["creator_name"].toString())
                              .arg(groupInfo["creator_type"].toString())
                              .arg(groupInfo["member_count"].toInt())
                              .arg(groupInfo["created_time"].toDateTime().toString("yyyy-MM-dd hh:mm"));

    for (const QVariantMap &member : members) {
        QString memberText = QString("<p>• %1 %2 (%3)</p>")
                                 .arg(member["user_name"].toString())
                                 .arg(member["user_type"].toString())
                                 .arg(member["role"].toString());
        detailsText += memberText;
    }

    m_detailsLabel->setText(detailsText);
}

void GroupManageWidget::clearGroupDetails()
{
    m_detailsLabel->setText("选择左侧的群聊或申请查看详细信息");
    m_selectedGroupId = -1;
    m_selectedRequestId = -1;
}

void GroupManageWidget::onRequestItemClicked()
{
    // 这个方法的功能已经在onRequestsClicked中实现
    // 保持空实现以满足链接器要求
    qDebug() << "onRequestItemClicked called";
}

void GroupManageWidget::onGroupItemClicked()
{
    // 这个方法的功能已经在onCreatedGroupClicked和onJoinedGroupClicked中实现
    // 保持空实现以满足链接器要求
    qDebug() << "onGroupItemClicked called";
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
