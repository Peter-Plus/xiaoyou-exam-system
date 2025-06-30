#ifndef GROUPINFO_H
#define GROUPINFO_H

#include <QString>
#include <QDateTime>

class GroupInfo
{
public:
    enum GroupRole {
        CREATOR,    // 创建者
        MEMBER      // 成员
    };

    // 构造函数
    GroupInfo();
    GroupInfo(int groupId, const QString &groupName, int creatorId, const QString &creatorType,
              const QString &creatorName, int memberCount, const QDateTime &createdTime,
              const QDateTime &lastMessageTime = QDateTime(), GroupRole userRole = MEMBER);

    // Getter方法
    int getGroupId() const { return m_groupId; }
    QString getGroupName() const { return m_groupName; }
    int getCreatorId() const { return m_creatorId; }
    QString getCreatorType() const { return m_creatorType; }
    QString getCreatorName() const { return m_creatorName; }
    int getMemberCount() const { return m_memberCount; }
    QDateTime getCreatedTime() const { return m_createdTime; }
    QDateTime getLastMessageTime() const { return m_lastMessageTime; }
    GroupRole getUserRole() const { return m_userRole; }

    // Setter方法
    void setGroupId(int groupId) { m_groupId = groupId; }
    void setGroupName(const QString &groupName) { m_groupName = groupName; }
    void setCreatorId(int creatorId) { m_creatorId = creatorId; }
    void setCreatorType(const QString &creatorType) { m_creatorType = creatorType; }
    void setCreatorName(const QString &creatorName) { m_creatorName = creatorName; }
    void setMemberCount(int memberCount) { m_memberCount = memberCount; }
    void setCreatedTime(const QDateTime &createdTime) { m_createdTime = createdTime; }
    void setLastMessageTime(const QDateTime &lastMessageTime) { m_lastMessageTime = lastMessageTime; }
    void setUserRole(GroupRole userRole) { m_userRole = userRole; }

    // 便于UI显示的格式化方法
    QString getDisplayName() const;
    QString getMemberCountText() const;
    QString getLastActiveText() const;
    QString getRoleText() const;
    QString getCreatorDisplayText() const;
    QString getGroupDescription() const;

    // 实用方法
    bool isValid() const;
    bool isUserCreator() const;
    bool hasRecentActivity() const; // 最近24小时内有活动

private:
    // 私有辅助方法
    QString getTimeDisplayText(const QDateTime &dateTime) const;

    // 成员变量
    int m_groupId;
    QString m_groupName;
    int m_creatorId;
    QString m_creatorType;
    QString m_creatorName;
    int m_memberCount;
    QDateTime m_createdTime;
    QDateTime m_lastMessageTime;
    GroupRole m_userRole;
};

#endif // GROUPINFO_H
