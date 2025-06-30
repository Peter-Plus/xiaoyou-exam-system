#include "groupinfodialog.h"
#include "invitememberdialog.h"
#include <QDebug>

GroupInfoDialog::GroupInfoDialog(Database *database, int groupId, int userId, const QString &userType, QWidget *parent)
    : QDialog(parent)
    , m_database(database)
    , m_groupId(groupId)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
    , m_isCreator(false)
    , m_selectedMemberId(-1)
{
    setWindowTitle("群聊信息");
    setModal(true);
    resize(600, 700);

    // 检查权限
    m_isCreator = m_database->isGroupCreator(groupId, userId, userType);

    setupUI();
    setupStyles();
    loadGroupInfo();
    loadMembers();
    updateButtonStates();
}

void GroupInfoDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // 创建标签页
    m_tabWidget = new QTabWidget();

    // 基本信息标签页
    setupBasicInfoTab();

    // 成员管理标签页
    setupMembersTab();

    // 群聊设置标签页
    setupSettingsTab();

    // 底部按钮
    m_buttonLayout = new QHBoxLayout();

    m_saveButton = new QPushButton("保存");
    m_deleteButton = new QPushButton("解散群聊");
    m_cancelButton = new QPushButton("取消");

    m_saveButton->setObjectName("primaryButton");
    m_deleteButton->setObjectName("dangerButton");
    m_cancelButton->setObjectName("secondaryButton");

    // 只有创建者可以解散群聊
    m_deleteButton->setVisible(m_isCreator);

    m_buttonLayout->addWidget(m_deleteButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_saveButton);

    // 组装主布局
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addLayout(m_buttonLayout);

    // 连接信号
    connectSignals();
}

void GroupInfoDialog::setupBasicInfoTab()
{
    m_basicInfoTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_basicInfoTab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // 基本信息表单
    QGroupBox *basicGroup = new QGroupBox("基本信息");
    QFormLayout *formLayout = new QFormLayout(basicGroup);

    m_groupNameEdit = new QLineEdit();
    m_groupNameEdit->setMaxLength(50);
    m_groupNameEdit->setEnabled(m_isCreator); // 只有创建者可以修改

    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setMaximumHeight(100);
    m_descriptionEdit->setEnabled(m_isCreator);

    formLayout->addRow("群聊名称:", m_groupNameEdit);
    formLayout->addRow("群聊描述:", m_descriptionEdit);

    // 群聊详情
    QGroupBox *detailGroup = new QGroupBox("群聊详情");
    QFormLayout *detailLayout = new QFormLayout(detailGroup);

    m_groupIdLabel = new QLabel();
    m_creatorLabel = new QLabel();
    m_createTimeLabel = new QLabel();
    m_memberCountLabel = new QLabel();

    detailLayout->addRow("群聊ID:", m_groupIdLabel);
    detailLayout->addRow("创建者:", m_creatorLabel);
    detailLayout->addRow("创建时间:", m_createTimeLabel);
    detailLayout->addRow("成员数量:", m_memberCountLabel);

    layout->addWidget(basicGroup);
    layout->addWidget(detailGroup);
    layout->addStretch();

    m_tabWidget->addTab(m_basicInfoTab, "📋 基本信息");
}

void GroupInfoDialog::setupMembersTab()
{
    m_membersTab = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(m_membersTab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // 左侧成员列表
    QVBoxLayout *leftLayout = new QVBoxLayout();

    QLabel *memberListLabel = new QLabel("群聊成员");
    memberListLabel->setObjectName("sectionTitle");

    m_membersListWidget = new QListWidget();
    m_membersListWidget->setMinimumWidth(300);

    // 成员操作按钮
    QHBoxLayout *memberButtonsLayout = new QHBoxLayout();
    m_inviteMemberButton = new QPushButton("➕ 邀请成员");
    m_removeMemberButton = new QPushButton("➖ 移除成员");
    m_refreshMembersButton = new QPushButton("🔄 刷新");

    m_inviteMemberButton->setObjectName("actionButton");
    m_removeMemberButton->setObjectName("dangerActionButton");
    m_refreshMembersButton->setObjectName("actionButton");

    // 权限控制
    m_inviteMemberButton->setEnabled(m_isCreator);
    m_removeMemberButton->setEnabled(false);

    memberButtonsLayout->addWidget(m_inviteMemberButton);
    memberButtonsLayout->addWidget(m_removeMemberButton);
    memberButtonsLayout->addWidget(m_refreshMembersButton);

    leftLayout->addWidget(memberListLabel);
    leftLayout->addWidget(m_membersListWidget);
    leftLayout->addLayout(memberButtonsLayout);

    // 右侧成员详情
    QVBoxLayout *rightLayout = new QVBoxLayout();

    QLabel *memberDetailTitleLabel = new QLabel("成员详情");
    memberDetailTitleLabel->setObjectName("sectionTitle");

    QScrollArea *detailScrollArea = new QScrollArea();
    detailScrollArea->setWidgetResizable(true);
    detailScrollArea->setMinimumWidth(250);

    m_memberDetailLabel = new QLabel("选择左侧成员查看详细信息");
    m_memberDetailLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_memberDetailLabel->setWordWrap(true);
    m_memberDetailLabel->setObjectName("memberDetail");

    detailScrollArea->setWidget(m_memberDetailLabel);

    rightLayout->addWidget(memberDetailTitleLabel);
    rightLayout->addWidget(detailScrollArea);

    layout->addLayout(leftLayout);
    layout->addLayout(rightLayout);

    m_tabWidget->addTab(m_membersTab, "👥 成员管理");
}

void GroupInfoDialog::setupSettingsTab()
{
    m_settingsTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_settingsTab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // 群聊权限设置
    QGroupBox *permissionGroup = new QGroupBox("群聊权限");
    QVBoxLayout *permissionLayout = new QVBoxLayout(permissionGroup);

    m_allowMemberInviteCheck = new QCheckBox("允许成员邀请好友");
    m_requireApprovalCheck = new QCheckBox("新成员需要群主审核");

    // 只有创建者可以修改设置
    m_allowMemberInviteCheck->setEnabled(m_isCreator);
    m_requireApprovalCheck->setEnabled(m_isCreator);

    permissionLayout->addWidget(m_allowMemberInviteCheck);
    permissionLayout->addWidget(m_requireApprovalCheck);

    // 群聊限制设置
    QGroupBox *limitGroup = new QGroupBox("群聊限制");
    QFormLayout *limitLayout = new QFormLayout(limitGroup);

    m_maxMembersSpin = new QSpinBox();
    m_maxMembersSpin->setRange(3, 500);
    m_maxMembersSpin->setValue(100);
    m_maxMembersSpin->setEnabled(m_isCreator);

    m_groupTypeCombo = new QComboBox();
    m_groupTypeCombo->addItems({"普通群聊", "学习群", "工作群", "兴趣群"});
    m_groupTypeCombo->setEnabled(m_isCreator);

    limitLayout->addRow("最大成员数:", m_maxMembersSpin);
    limitLayout->addRow("群聊类型:", m_groupTypeCombo);

    // 提示信息
    QLabel *hintLabel = new QLabel("💡 只有群主可以修改群聊设置。普通成员只能查看当前设置。");
    hintLabel->setObjectName("hintLabel");
    hintLabel->setWordWrap(true);

    layout->addWidget(permissionGroup);
    layout->addWidget(limitGroup);
    layout->addWidget(hintLabel);
    layout->addStretch();

    m_tabWidget->addTab(m_settingsTab, "⚙️ 群聊设置");
}

void GroupInfoDialog::setupStyles()
{
    QString styles = R"(
        QDialog {
            background-color: #f5f5f5;
        }

        QTabWidget::pane {
            border: 1px solid #ddd;
            border-radius: 8px;
            background-color: white;
        }

        QTabWidget::tab-bar {
            left: 8px;
        }

        QTabBar::tab {
            background-color: #f0f0f0;
            border: 1px solid #ddd;
            border-bottom: none;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
        }

        QTabBar::tab:selected {
            background-color: white;
            border-bottom: 1px solid white;
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

        QLineEdit, QTextEdit {
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 14px;
        }

        QLineEdit:focus, QTextEdit:focus {
            border-color: #4CAF50;
        }

        QLineEdit:disabled, QTextEdit:disabled {
            background-color: #f0f0f0;
            color: #666;
        }

        QListWidget {
            border: 1px solid #ddd;
            border-radius: 4px;
            background-color: white;
        }

        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #f0f0f0;
        }

        QListWidget::item:selected {
            background-color: #e8f5e8;
            border-left: 3px solid #4CAF50;
        }

        #sectionTitle {
            font-size: 14px;
            font-weight: bold;
            color: #333;
            margin-bottom: 8px;
        }

        #memberDetail {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 12px;
            font-size: 12px;
            color: #666;
        }

        #primaryButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
            min-width: 80px;
        }

        #primaryButton:hover {
            background-color: #45a049;
        }

        #dangerButton {
            background-color: #f44336;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
            min-width: 100px;
        }

        #dangerButton:hover {
            background-color: #d32f2f;
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

        #actionButton {
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 12px;
        }

        #actionButton:hover {
            background-color: #1976D2;
        }

        #dangerActionButton {
            background-color: #ff9800;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 12px;
        }

        #dangerActionButton:hover {
            background-color: #f57c00;
        }

        #hintLabel {
            color: #666;
            font-size: 12px;
            background-color: #fff3cd;
            border: 1px solid #ffeaa7;
            border-radius: 4px;
            padding: 8px;
        }
    )";

    setStyleSheet(styles);
}

void GroupInfoDialog::connectSignals()
{
    connect(m_groupNameEdit, &QLineEdit::textChanged,
            this, &GroupInfoDialog::onGroupNameChanged);
    connect(m_saveButton, &QPushButton::clicked,
            this, &GroupInfoDialog::onSaveClicked);
    connect(m_deleteButton, &QPushButton::clicked,
            this, &GroupInfoDialog::onDeleteGroupClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &GroupInfoDialog::onCancelClicked);
    connect(m_inviteMemberButton, &QPushButton::clicked,
            this, &GroupInfoDialog::onInviteMemberClicked);
    connect(m_removeMemberButton, &QPushButton::clicked,
            this, &GroupInfoDialog::onRemoveMemberClicked);
    connect(m_refreshMembersButton, &QPushButton::clicked,
            this, &GroupInfoDialog::onRefreshMembersClicked);
    connect(m_membersListWidget, &QListWidget::itemClicked,
            this, &GroupInfoDialog::onMemberItemClicked);
}

void GroupInfoDialog::loadGroupInfo()
{
    QVariantMap groupData = m_database->getGroupInfo(m_groupId);
    if (groupData.isEmpty()) {
        QMessageBox::critical(this, "错误", "无法加载群聊信息");
        reject();
        return;
    }

    // 设置基本信息
    m_groupNameEdit->setText(groupData["group_name"].toString());
    // 群聊描述暂时留空，数据库中暂未存储

    // 设置详情信息
    m_groupIdLabel->setText(QString::number(groupData["group_id"].toInt()));
    m_creatorLabel->setText(QString("%1 %2")
                                .arg(groupData["creator_name"].toString())
                                .arg(groupData["creator_type"].toString()));
    m_createTimeLabel->setText(groupData["created_time"].toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    m_memberCountLabel->setText(QString("%1人").arg(groupData["member_count"].toInt()));

    // 设置默认群聊设置（暂时硬编码，可以扩展数据库字段）
    m_allowMemberInviteCheck->setChecked(false);
    m_requireApprovalCheck->setChecked(true);
    m_maxMembersSpin->setValue(groupData["member_count"].toInt() + 50); // 当前人数+50
}

void GroupInfoDialog::loadMembers()
{
    m_members = m_database->getGroupMembers(m_groupId);
    m_membersListWidget->clear();

    for (const QVariantMap &member : m_members) {
        QString displayText = QString("%1 %2 (%3)")
        .arg(member["user_name"].toString())
            .arg(member["user_type"].toString())
            .arg(member["role"].toString());

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, member["user_id"]);
        item->setData(Qt::UserRole + 1, member["user_type"]);
        item->setData(Qt::UserRole + 2, member["role"]);

        // 创建者用特殊颜色标识
        if (member["role"].toString() == "创建者") {
            item->setBackground(QBrush(QColor(255, 248, 220))); // 浅金色
        }

        m_membersListWidget->addItem(item);
    }

    // 更新成员数量显示
    m_memberCountLabel->setText(QString("%1人").arg(m_members.size()));
}

void GroupInfoDialog::onMemberItemClicked()
{
    QListWidgetItem *item = m_membersListWidget->currentItem();
    if (!item) return;

    m_selectedMemberId = item->data(Qt::UserRole).toInt();
    m_selectedMemberType = item->data(Qt::UserRole + 1).toString();
    QString role = item->data(Qt::UserRole + 2).toString();

    // 显示成员详情
    for (const QVariantMap &member : m_members) {
        if (member["user_id"].toInt() == m_selectedMemberId &&
            member["user_type"].toString() == m_selectedMemberType) {

            QString detailText = QString(
                                     "<h3>成员详情</h3>"
                                     "<p><b>姓名:</b> %1</p>"
                                     "<p><b>类型:</b> %2</p>"
                                     "<p><b>学院:</b> %3</p>"
                                     "<p><b>年级:</b> %4</p>"
                                     "<p><b>角色:</b> %5</p>"
                                     "<p><b>加入时间:</b> %6</p>"
                                     ).arg(member["user_name"].toString())
                                     .arg(member["user_type"].toString())
                                     .arg(member["user_college"].toString())
                                     .arg(member["user_grade"].toString().isEmpty() ? "无" : member["user_grade"].toString())
                                     .arg(role)
                                     .arg(member["join_time"].toDateTime().toString("yyyy-MM-dd hh:mm"));

            m_memberDetailLabel->setText(detailText);
            break;
        }
    }

    // 更新移除按钮状态
    bool canRemove = m_isCreator && role != "创建者" &&
                     !(m_selectedMemberId == m_currentUserId && m_selectedMemberType == m_currentUserType);
    m_removeMemberButton->setEnabled(canRemove);
}

void GroupInfoDialog::onSaveClicked()
{
    if (!validateInput()) {
        return;
    }

    if (!hasPermissionToEdit()) {
        QMessageBox::warning(this, "权限不足", "只有群主可以修改群聊信息");
        return;
    }

    // 暂时只支持修改群聊名称
    QString newGroupName = m_groupNameEdit->text().trimmed();

    // 这里需要扩展Database类添加updateGroupInfo方法
    // bool success = m_database->updateGroupInfo(m_groupId, newGroupName, ...);

    QMessageBox::information(this, "提示", "群聊信息修改功能开发中...");

    // 暂时直接关闭对话框
    accept();
}

void GroupInfoDialog::onDeleteGroupClicked()
{
    if (!m_isCreator) {
        QMessageBox::warning(this, "权限不足", "只有群主可以解散群聊");
        return;
    }

    int ret = QMessageBox::question(this, "确认解散",
                                    "确定要解散这个群聊吗？\n\n解散后所有成员将被移除，聊天记录将被清除，此操作无法撤销。",
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        bool success = m_database->deleteGroupChat(m_groupId, m_currentUserId, m_currentUserType);
        if (success) {
            QMessageBox::information(this, "成功", "群聊已解散");
            accept();
        } else {
            QMessageBox::critical(this, "失败", "解散群聊失败，请重试");
        }
    }
}

void GroupInfoDialog::onInviteMemberClicked()
{
    if (!m_isCreator) {
        QMessageBox::warning(this, "权限不足", "只有群主可以邀请成员");
        return;
    }

    // 这里需要创建邀请成员对话框
    QMessageBox::information(this, "提示", "邀请成员功能开发中...");
}

void GroupInfoDialog::onRemoveMemberClicked()
{
    if (m_selectedMemberId <= 0) {
        QMessageBox::warning(this, "请选择成员", "请先选择要移除的成员");
        return;
    }

    if (!m_isCreator) {
        QMessageBox::warning(this, "权限不足", "只有群主可以移除成员");
        return;
    }

    QString memberName;
    for (const QVariantMap &member : m_members) {
        if (member["user_id"].toInt() == m_selectedMemberId) {
            memberName = member["user_name"].toString();
            break;
        }
    }

    int ret = QMessageBox::question(this, "确认移除",
                                    QString("确定要移除成员 %1 吗？").arg(memberName),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        bool success = m_database->removeGroupMember(m_groupId, m_selectedMemberId, m_selectedMemberType);
        if (success) {
            QMessageBox::information(this, "成功", "成员已移除");
            loadMembers(); // 刷新成员列表
            m_memberDetailLabel->setText("选择左侧成员查看详细信息");
            m_removeMemberButton->setEnabled(false);
        } else {
            QMessageBox::critical(this, "失败", "移除成员失败，请重试");
        }
    }
}

void GroupInfoDialog::onRefreshMembersClicked()
{
    loadMembers();
    m_memberDetailLabel->setText("选择左侧成员查看详细信息");
    m_removeMemberButton->setEnabled(false);
}

void GroupInfoDialog::onCancelClicked()
{
    reject();
}

void GroupInfoDialog::onGroupNameChanged()
{
    updateButtonStates();
}

void GroupInfoDialog::updateButtonStates()
{
    QString groupName = m_groupNameEdit->text().trimmed();
    m_saveButton->setEnabled(!groupName.isEmpty() && hasPermissionToEdit());
}

bool GroupInfoDialog::validateInput()
{
    QString groupName = m_groupNameEdit->text().trimmed();

    if (groupName.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "群聊名称不能为空");
        m_groupNameEdit->setFocus();
        return false;
    }

    if (groupName.length() > 50) {
        QMessageBox::warning(this, "输入错误", "群聊名称不能超过50个字符");
        m_groupNameEdit->setFocus();
        return false;
    }

    return true;
}

bool GroupInfoDialog::hasPermissionToEdit()
{
    return m_isCreator;
}
