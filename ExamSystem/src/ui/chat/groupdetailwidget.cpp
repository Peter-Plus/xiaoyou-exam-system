#include "groupdetailwidget.h"
#include <QDebug>

GroupDetailWidget::GroupDetailWidget(Database *database, int userId,
                                     const QString &userType, QWidget *parent)
    : QWidget(parent)
    , m_database(database)
    , m_userId(userId)
    , m_userType(userType)
    , m_currentGroupId(-1)
    , m_isCreator(false)
{
    setupUI();
    setupStyles();
    showEmptyState();
}

void GroupDetailWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // 滚动区域
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setObjectName("groupDetailScrollArea");

    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(16, 16, 16, 16);
    m_contentLayout->setSpacing(16);

    // 群聊基本信息
    m_basicInfoBox = new QGroupBox("📋 群聊信息");
    QVBoxLayout *basicLayout = new QVBoxLayout(m_basicInfoBox);
    basicLayout->setSpacing(8);

    m_groupNameLabel = new QLabel();
    m_groupNameLabel->setObjectName("groupNameLabel");
    m_creatorLabel = new QLabel();
    m_creatorLabel->setObjectName("groupInfoLabel");
    m_memberCountLabel = new QLabel();
    m_memberCountLabel->setObjectName("groupInfoLabel");
    m_createTimeLabel = new QLabel();
    m_createTimeLabel->setObjectName("groupInfoLabel");

    basicLayout->addWidget(m_groupNameLabel);
    basicLayout->addWidget(m_creatorLabel);
    basicLayout->addWidget(m_memberCountLabel);
    basicLayout->addWidget(m_createTimeLabel);

    // 成员列表
    m_membersBox = new QGroupBox("👥 群聊成员");
    QVBoxLayout *membersLayout = new QVBoxLayout(m_membersBox);

    m_memberList = new QListWidget();
    m_memberList->setObjectName("groupMemberList");
    m_memberList->setMaximumHeight(200);
    membersLayout->addWidget(m_memberList);

    // 操作按钮
    m_operationsBox = new QGroupBox("⚙️ 群聊操作");
    m_buttonLayout = new QHBoxLayout(m_operationsBox);

    m_inviteButton = new QPushButton("➕ 邀请成员");
    m_inviteButton->setObjectName("inviteButton");
    m_disbandButton = new QPushButton("💥 解散群聊");
    m_disbandButton->setObjectName("disbandButton");
    m_leaveButton = new QPushButton("🚪 退出群聊");
    m_leaveButton->setObjectName("leaveButton");

    m_buttonLayout->addWidget(m_inviteButton);
    m_buttonLayout->addWidget(m_disbandButton);
    m_buttonLayout->addWidget(m_leaveButton);
    m_buttonLayout->addStretch();

    // 空状态标签
    m_emptyLabel = new QLabel("请从左侧选择一个群聊查看详细信息");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setObjectName("emptyStateLabel");

    // 组装布局
    m_contentLayout->addWidget(m_basicInfoBox);
    m_contentLayout->addWidget(m_membersBox);
    m_contentLayout->addWidget(m_operationsBox);
    m_contentLayout->addWidget(m_emptyLabel);
    m_contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);

    // 连接信号
    connect(m_inviteButton, &QPushButton::clicked,
            this, &GroupDetailWidget::onInviteMember);
    connect(m_disbandButton, &QPushButton::clicked,
            this, &GroupDetailWidget::onDisbandGroup);
    connect(m_leaveButton, &QPushButton::clicked,
            this, &GroupDetailWidget::onLeaveGroup);
}

void GroupDetailWidget::setupStyles()
{
    QString styles = R"(
        QWidget {
            background-color: #f8f9fa;
            font-family: 'Microsoft YaHei', Arial, sans-serif;
        }

        #groupDetailScrollArea {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 8px;
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

        #groupNameLabel {
            font-size: 18px;
            font-weight: bold;
            color: #212529;
            padding: 4px;
        }

        #groupInfoLabel {
            font-size: 14px;
            color: #6c757d;
            padding: 2px 4px;
        }

        #groupMemberList {
            background-color: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 4px;
        }

        #groupMemberList::item {
            padding: 8px;
            border-bottom: 1px solid #e9ecef;
        }

        #groupMemberList::item:hover {
            background-color: #e9ecef;
        }

        #inviteButton {
            background-color: #28a745;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 16px;
            font-size: 14px;
            font-weight: bold;
        }

        #inviteButton:hover {
            background-color: #218838;
        }

        #disbandButton {
            background-color: #dc3545;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 16px;
            font-size: 14px;
            font-weight: bold;
        }

        #disbandButton:hover {
            background-color: #c82333;
        }

        #leaveButton {
            background-color: #6c757d;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 16px;
            font-size: 14px;
            font-weight: bold;
        }

        #leaveButton:hover {
            background-color: #5a6268;
        }

        #emptyStateLabel {
            font-size: 16px;
            color: #6c757d;
            padding: 40px;
        }
    )";

    setStyleSheet(styles);
}

void GroupDetailWidget::showGroupDetail(int groupId, bool isCreator)
{
    qDebug() << "显示群聊详情:" << groupId << "是否创建者:" << isCreator;

    m_currentGroupId = groupId;
    m_isCreator = isCreator;

    // 隐藏空状态，显示详情
    m_emptyLabel->hide();
    m_basicInfoBox->show();
    m_membersBox->show();
    m_operationsBox->show();

    // 加载群聊信息
    QVariantMap groupInfo = m_database->getGroupInfo(groupId);
    if (groupInfo.isEmpty()) {
        qWarning() << "无法获取群聊信息:" << groupId;
        clearDetail();
        return;
    }

    // 更新基本信息
    m_groupNameLabel->setText(QString("群聊名称：%1").arg(groupInfo["group_name"].toString()));
    m_creatorLabel->setText(QString("创建者：%1 %2")
                                .arg(groupInfo["creator_name"].toString())
                                .arg(groupInfo["creator_type"].toString()));
    m_memberCountLabel->setText(QString("成员数量：%1人").arg(groupInfo["member_count"].toInt()));
    m_createTimeLabel->setText(QString("创建时间：%1")
                                   .arg(groupInfo["created_time"].toDateTime().toString("yyyy-MM-dd hh:mm")));

    // 加载成员列表
    loadGroupMembers();

    // 更新操作按钮
    updateOperationButtons(isCreator);
}

void GroupDetailWidget::clearDetail()
{
    m_currentGroupId = -1;
    m_isCreator = false;

    showEmptyState();
}

void GroupDetailWidget::showEmptyState()
{
    m_basicInfoBox->hide();
    m_membersBox->hide();
    m_operationsBox->hide();
    m_emptyLabel->show();
}

void GroupDetailWidget::loadGroupMembers()
{
    m_memberList->clear();

    if (m_currentGroupId <= 0) return;

    QList<QVariantMap> members = m_database->getGroupMembers(m_currentGroupId);

    for (const QVariantMap &member : members) {
        QString memberText;
        QString role = member["role"].toString();

        if (role == "创建者") {
            memberText = QString("👑 %1 %2 (%3)")
                             .arg(member["user_name"].toString())
                             .arg(member["user_type"].toString())
                             .arg(role);
        } else {
            memberText = QString("👤 %1 %2 (%3)")
                             .arg(member["user_name"].toString())
                             .arg(member["user_type"].toString())
                             .arg(role);
        }

        // 添加学院信息
        if (member.contains("user_college") && !member["user_college"].toString().isEmpty()) {
            memberText += QString(" - %1").arg(member["user_college"].toString());
        }

        QListWidgetItem *item = new QListWidgetItem(memberText);
        m_memberList->addItem(item);
    }

    qDebug() << "加载群聊成员完成，共" << members.size() << "人";
}

void GroupDetailWidget::updateOperationButtons(bool isCreator)
{
    if (isCreator) {
        // 创建者：显示邀请成员和解散群聊
        m_inviteButton->show();
        m_disbandButton->show();
        m_leaveButton->hide();
    } else {
        // 普通成员：只显示退出群聊
        m_inviteButton->hide();
        m_disbandButton->hide();
        m_leaveButton->show();
    }
}

void GroupDetailWidget::onInviteMember()
{
    if (m_currentGroupId <= 0) return;

    qDebug() << "打开邀请成员对话框，群聊ID:" << m_currentGroupId;

    // 创建邀请成员对话框
    InviteMemberDialog dialog(m_database, m_currentGroupId,
                              m_groupNameLabel->text().remove("群聊名称："),
                              m_userId, m_userType, this);

    if (dialog.exec() == QDialog::Accepted) {
        // 邀请成功，刷新群聊信息
        refreshGroupInfo();
        emit memberInvited(m_currentGroupId);
    }
}

void GroupDetailWidget::onDisbandGroup()
{
    if (m_currentGroupId <= 0) {
        QMessageBox::warning(this, "错误", "未选择群聊");
        return;
    }

    // 获取群聊名称用于确认
    QString groupName = m_groupNameLabel->text().remove("群聊名称：");

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认解散",
        QString("确定要解散群聊「%1」吗？\n\n此操作将：\n• 移除所有群聊成员\n• 删除所有群聊消息\n• 删除群聊记录\n\n此操作不可撤销！").arg(groupName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);  // 默认选择No

    if (reply == QMessageBox::Yes) {
        qDebug() << "用户确认解散群聊:" << m_currentGroupId;

        bool success = m_database->disbandGroup(m_currentGroupId, m_userId, m_userType);
        if (success) {
            QMessageBox::information(this, "操作成功", "群聊已解散");
            emit groupDisbanded(m_currentGroupId);
            clearDetail();
        } else {
            // 更详细的错误信息
            QString errorMsg = "解散群聊失败，可能原因：\n";
            errorMsg += "• 您不是群聊创建者\n";
            errorMsg += "• 数据库连接异常\n";
            errorMsg += "• 群聊不存在\n\n";
            errorMsg += "请检查网络连接后重试";

            QMessageBox::critical(this, "操作失败", errorMsg);
        }
    }
}

void GroupDetailWidget::onLeaveGroup()
{
    if (m_currentGroupId <= 0) {
        QMessageBox::warning(this, "错误", "未选择群聊");
        return;
    }

    // 获取群聊名称用于确认
    QString groupName = m_groupNameLabel->text().remove("群聊名称：");

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认退出",
        QString("确定要退出群聊「%1」吗？\n\n退出后您将无法接收群聊消息，需要重新申请加入。").arg(groupName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "用户确认退出群聊:" << m_currentGroupId;

        bool success = m_database->leaveGroup(m_currentGroupId, m_userId, m_userType);
        if (success) {
            QMessageBox::information(this, "操作成功", "已退出群聊");
            emit groupLeft(m_currentGroupId);
            clearDetail();
        } else {
            // 更详细的错误信息
            QString errorMsg = "退出群聊失败，可能原因：\n";
            errorMsg += "• 您是群聊创建者（创建者不能退出，只能解散）\n";
            errorMsg += "• 您不是群聊成员\n";
            errorMsg += "• 数据库连接异常\n";
            errorMsg += "• 群聊不存在\n\n";
            errorMsg += "请检查后重试";

            QMessageBox::critical(this, "操作失败", errorMsg);
        }
    }
}

void GroupDetailWidget::refreshGroupInfo()
{
    if (m_currentGroupId > 0) {
        showGroupDetail(m_currentGroupId, m_isCreator);
    }
}
