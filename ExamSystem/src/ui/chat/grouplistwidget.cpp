#include "grouplistwidget.h"
#include <QDebug>

GroupListWidget::GroupListWidget(Database *db, int userId, const QString &userType, QWidget *parent)
    : QListWidget(parent)
    , m_database(db)
    , m_currentUserId(userId)
    , m_currentUserType(userType)
{
    setObjectName("groupListWidget");
    setMinimumHeight(200);

    // 设置样式
    QString listStyle = R"(
        QListWidget {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            outline: none;
        }

        QListWidget::item {
            padding: 12px;
            border-bottom: 1px solid #f0f0f0;
            min-height: 60px;
        }

        QListWidget::item:selected {
            background-color: #e8f5e8;
            border-left: 4px solid #4CAF50;
        }

        QListWidget::item:hover {
            background-color: #f5f5f5;
        }
    )";
    setStyleSheet(listStyle);

    // 连接信号
    connect(this, &QListWidget::itemClicked,
            this, &GroupListWidget::onGroupItemClicked);
}

void GroupListWidget::refreshGroupList()
{
    qDebug() << "GroupListWidget刷新群聊列表";

    clear();
    m_groupList.clear();

    // 获取用户的群聊列表
    QList<QVariantMap> groups = m_database->getUserGroups(m_currentUserId, m_currentUserType);

    for (const QVariantMap &groupData : groups) {
        GroupInfo group;
        group.setGroupId(groupData["group_id"].toInt());
        group.setGroupName(groupData["group_name"].toString());
        group.setCreatorId(groupData["creator_id"].toInt());
        group.setCreatorType(groupData["creator_type"].toString());
        group.setCreatorName(groupData["creator_name"].toString());
        group.setMemberCount(groupData["member_count"].toInt());
        group.setCreatedTime(groupData["created_time"].toDateTime());

        if (groupData.contains("last_message_time")) {
            group.setLastMessageTime(groupData["last_message_time"].toDateTime());
        }

        // 设置用户角色
        QString userRole = groupData["user_role"].toString();
        if (userRole == "创建者") {
            group.setUserRole(GroupInfo::CREATOR);
        } else {
            group.setUserRole(GroupInfo::MEMBER);
        }

        m_groupList.append(group);
        createGroupItem(group);
    }

    qDebug() << "群聊列表刷新完成，共" << m_groupList.size() << "个群聊";
}

void GroupListWidget::createGroupItem(const GroupInfo &group)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(formatGroupItemText(group));
    item->setData(Qt::UserRole, group.getGroupId());
    item->setData(Qt::UserRole + 1, group.getGroupName());
    item->setData(Qt::UserRole + 2, group.getRoleText());

    addItem(item);
}

QString GroupListWidget::formatGroupItemText(const GroupInfo &group)
{
    QString displayName = group.getDisplayName();
    QString memberInfo = group.getMemberCountText();
    QString roleInfo = group.getRoleText();
    QString timeDisplay = group.getLastActiveText();

    // 获取最后一条消息
    QVariantMap lastMessage = m_database->getLastMessage(group.getGroupId(), "群聊");
    QString lastMsgPreview = "暂无消息";

    if (!lastMessage.isEmpty()) {
        QString content = lastMessage["content"].toString();
        QString senderName = lastMessage["sender_name"].toString();

        // 限制消息预览长度
        if (content.length() > 20) {
            content = content.left(20) + "...";
        }

        lastMsgPreview = QString("%1: %2").arg(senderName, content);
    }

    QString itemText = QString("%1 (%2) [%3]\n%4  %5")
                           .arg(displayName)
                           .arg(memberInfo)
                           .arg(roleInfo)
                           .arg(lastMsgPreview)
                           .arg(timeDisplay);

    return itemText;
}

void GroupListWidget::onGroupItemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int groupId = item->data(Qt::UserRole).toInt();
    QString groupName = item->data(Qt::UserRole + 1).toString();

    qDebug() << "群聊列表项被点击:" << groupId << groupName;
    emit groupSelected(groupId, groupName);
}

void GroupListWidget::updateGroupItem(int groupId)
{
    // 查找并更新指定群聊项
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *item = this->item(i);
        if (item && item->data(Qt::UserRole).toInt() == groupId) {
            // 重新获取群聊信息并更新
            refreshGroupList();
            break;
        }
    }
}

void GroupListWidget::onNewGroupMessage(int groupId)
{
    updateGroupItem(groupId);
}

void GroupListWidget::updateUnreadBadge(int groupId, int count)
{
    // 暂时不实现未读消息角标，留待后续扩展
    Q_UNUSED(groupId)
    Q_UNUSED(count)
}
