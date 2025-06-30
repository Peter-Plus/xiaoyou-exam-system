#ifndef GROUPMEMBER_H
#define GROUPMEMBER_H

#include <QString>
#include <QDateTime>

class GroupMember
{
public:
    enum MemberRole {
        CREATOR,    // 创建者
        ADMIN,      // 管理员（预留）
        MEMBER      // 普通成员
    };

    // 构造函数
    GroupMember();
    GroupMember(int userId, const QString &userType, const QString &userName,
                const QString &userCollege, const QString &userGrade,
                MemberRole role, const QDateTime &joinTime);

    // Getter方法
    int getUserId() const { return m_userId; }
    QString getUserType() const { return m_userType; }
    QString getUserName() const { return m_userName; }
    QString getUserCollege() const { return m_userCollege; }
    QString getUserGrade() const { return m_userGrade; }
    MemberRole getRole() const { return m_role; }
    QDateTime getJoinTime() const { return m_joinTime; }

    // Setter方法
    void setUserId(int userId) { m_userId = userId; }
    void setUserType(const QString &userType) { m_userType = userType; }
    void setUserName(const QString &userName) { m_userName = userName; }
    void setUserCollege(const QString &userCollege) { m_userCollege = userCollege; }
    void setUserGrade(const QString &userGrade) { m_userGrade = userGrade; }
    void setRole(MemberRole role) { m_role = role; }
    void setJoinTime(const QDateTime &joinTime) { m_joinTime = joinTime; }

    // 便于UI显示的格式化方法
    QString getDisplayName() const;
    QString getUserInfo() const;
    QString getRoleText() const;
    QString getJoinTimeText() const;
    QString getFullDescription() const;

    // 状态判断方法
    bool isCreator() const;
    bool isAdmin() const;
    bool isMember() const;
    bool canBeRemoved() const; // 是否可以被移除
    bool hasManagePermission() const; // 是否有管理权限

    // 实用方法
    bool isValid() const;

private:
    int m_userId;
    QString m_userType;
    QString m_userName;
    QString m_userCollege;
    QString m_userGrade;
    MemberRole m_role;
    QDateTime m_joinTime;

    // 私有辅助方法
    QString formatTimeDisplay(const QDateTime &dateTime) const;
};

#endif // GROUPMEMBER_H
