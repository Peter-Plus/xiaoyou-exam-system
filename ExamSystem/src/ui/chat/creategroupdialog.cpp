#include "creategroupdialog.h"
#include <QDebug>

CreateGroupDialog::CreateGroupDialog(Database *database, int userId, const QString &userType, QWidget *parent)
    : QDialog(parent)
    , m_database(database)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
{
    setWindowTitle("创建群聊");
    setModal(true);
    resize(450, 600);

    setupUI();
    setupStyles();
    loadFriendsList();
    updateButtonState();
}

void CreateGroupDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 16, 16, 16);
    m_mainLayout->setSpacing(16);

    // 基本信息表单
    QGroupBox *basicInfoGroup = new QGroupBox("群聊基本信息");
    m_formLayout = new QFormLayout(basicInfoGroup);

    m_groupNameEdit = new QLineEdit();
    m_groupNameEdit->setPlaceholderText("请输入群聊名称（必填）");
    m_groupNameEdit->setMaxLength(50);

    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText("群聊描述（可选）");
    m_descriptionEdit->setMaximumHeight(80);

    m_formLayout->addRow("群聊名称:", m_groupNameEdit);
    m_formLayout->addRow("群聊描述:", m_descriptionEdit);

    // 邀请好友区域
    m_friendsGroupBox = new QGroupBox("邀请好友加入（可选）");
    QVBoxLayout *friendsLayout = new QVBoxLayout(m_friendsGroupBox);

    // 好友列表操作按钮
    QHBoxLayout *friendsButtonLayout = new QHBoxLayout();
    m_selectAllButton = new QPushButton("全选");
    m_selectNoneButton = new QPushButton("全不选");
    m_selectAllButton->setObjectName("friendsActionButton");
    m_selectNoneButton->setObjectName("friendsActionButton");

    friendsButtonLayout->addWidget(m_selectAllButton);
    friendsButtonLayout->addWidget(m_selectNoneButton);
    friendsButtonLayout->addStretch();

    // 好友列表
    m_friendsListWidget = new QListWidget();
    m_friendsListWidget->setMaximumHeight(200);
    m_friendsListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    friendsLayout->addLayout(friendsButtonLayout);
    friendsLayout->addWidget(m_friendsListWidget);

    // 提示信息
    m_hintLabel = new QLabel("💡 提示：创建群聊后，您将成为群主，可以管理群成员和群设置。");
    m_hintLabel->setObjectName("hintLabel");
    m_hintLabel->setWordWrap(true);

    // 底部按钮
    m_buttonLayout = new QHBoxLayout();
    m_createButton = new QPushButton("创建群聊");
    m_cancelButton = new QPushButton("取消");

    m_createButton->setObjectName("primaryButton");
    m_cancelButton->setObjectName("secondaryButton");

    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_createButton);

    // 组装主布局
    m_mainLayout->addWidget(basicInfoGroup);
    m_mainLayout->addWidget(m_friendsGroupBox);
    m_mainLayout->addWidget(m_hintLabel);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(m_buttonLayout);

    // 连接信号
    connect(m_groupNameEdit, &QLineEdit::textChanged,
            this, &CreateGroupDialog::onGroupNameChanged);
    connect(m_createButton, &QPushButton::clicked,
            this, &CreateGroupDialog::onCreateClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &CreateGroupDialog::onCancelClicked);
    connect(m_selectAllButton, &QPushButton::clicked,
            this, &CreateGroupDialog::onInviteFriendsClicked);
    connect(m_selectNoneButton, &QPushButton::clicked, [this]() {
        m_friendsListWidget->clearSelection();
    });
}

void CreateGroupDialog::setupStyles()
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
            border-color: #4CAF50;
        }

        QTextEdit {
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 8px;
            font-size: 14px;
        }

        QTextEdit:focus {
            border-color: #4CAF50;
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

        #friendsActionButton {
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 12px;
        }

        #friendsActionButton:hover {
            background-color: #1976D2;
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

void CreateGroupDialog::loadFriendsList()
{
    qDebug() << "加载好友列表";

    m_friendsList = m_database->getFriendsList(m_currentUserId, m_currentUserType);
    m_friendsListWidget->clear();

    if (m_friendsList.isEmpty()) {
        QListWidgetItem *emptyItem = new QListWidgetItem("暂无好友可邀请");
        emptyItem->setFlags(Qt::NoItemFlags);
        m_friendsListWidget->addItem(emptyItem);

        m_selectAllButton->setEnabled(false);
        m_selectNoneButton->setEnabled(false);
        return;
    }

    for (const QVariantMap &friendData : m_friendsList) {
        QString friendName = friendData["friend_name"].toString();
        QString friendType = friendData["friend_type"].toString();
        QString friendCollege = friendData["friend_college"].toString();
        QString friendGrade = friendData["friend_grade"].toString();

        QString displayText = friendName;
        if (!friendGrade.isEmpty()) {
            displayText += QString("（%1级 %2）").arg(friendGrade, friendCollege);
        } else {
            displayText += QString("（%1）").arg(friendCollege);
        }

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setData(Qt::UserRole, friendData["friend_id"]);
        item->setData(Qt::UserRole + 1, friendData["friend_type"]);

        m_friendsListWidget->addItem(item);
    }

    qDebug() << "好友列表加载完成，共" << m_friendsList.size() << "个好友";
}

void CreateGroupDialog::onCreateClicked()
{
    if (!validateInput()) {
        return;
    }

    QString groupName = m_groupNameEdit->text().trimmed();

    qDebug() << "创建群聊:" << groupName;

    // 创建群聊
    int groupId = m_database->createGroupChat(groupName, m_currentUserId, m_currentUserType);
    if (groupId <= 0) {
        QMessageBox::critical(this, "创建失败", "群聊创建失败，请重试。");
        return;
    }

    // 邀请选中的好友
    QList<QListWidgetItem*> selectedItems = m_friendsListWidget->selectedItems();
    int invitedCount = 0;

    for (QListWidgetItem *item : selectedItems) {
        int friendId = item->data(Qt::UserRole).toInt();
        QString friendType = item->data(Qt::UserRole + 1).toString();

        if (m_database->addGroupMember(groupId, friendId, friendType)) {
            invitedCount++;
        }
    }

    qDebug() << "群聊创建成功，ID:" << groupId << "邀请好友:" << invitedCount << "人";

    accept(); // 关闭对话框
}

void CreateGroupDialog::onCancelClicked()
{
    reject();
}

void CreateGroupDialog::onGroupNameChanged()
{
    updateButtonState();
}

void CreateGroupDialog::onInviteFriendsClicked()
{
    // 全选好友
    for (int i = 0; i < m_friendsListWidget->count(); ++i) {
        QListWidgetItem *item = m_friendsListWidget->item(i);
        if (item && item->flags() & Qt::ItemIsSelectable) {
            item->setSelected(true);
        }
    }
}

void CreateGroupDialog::updateButtonState()
{
    QString groupName = m_groupNameEdit->text().trimmed();
    m_createButton->setEnabled(!groupName.isEmpty());
}

bool CreateGroupDialog::validateInput()
{
    QString groupName = m_groupNameEdit->text().trimmed();

    if (groupName.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入群聊名称。");
        m_groupNameEdit->setFocus();
        return false;
    }

    if (groupName.length() > 50) {
        QMessageBox::warning(this, "输入错误", "群聊名称不能超过50个字符。");
        m_groupNameEdit->setFocus();
        return false;
    }

    return true;
}
