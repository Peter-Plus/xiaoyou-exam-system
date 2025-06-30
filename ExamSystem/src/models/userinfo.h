#ifndef USERINFO_H
#define USERINFO_H

#include <QString>

class UserInfo
{
public:
    enum RelationshipStatus {
        CAN_ADD,        // 可添加
        ALREADY_FRIEND, // 已是好友
        REQUEST_SENT,   // 已发送请求
        REQUEST_PENDING // 待处理请求
    };

    UserInfo();
    UserInfo(int userId, const QString &userType, const QString &userName,
             const QString &userCollege, const QString &userGrade = QString());

    // Getter方法
    int getUserId() const { return m_userId; }
    QString getUserType() const { return m_userType; }
    QString getUserName() const { return m_userName; }
    QString getUserCollege() const { return m_userCollege; }
    QString getUserGrade() const { return m_userGrade; }
    RelationshipStatus getRelationshipStatus() const { return m_relationshipStatus; }

    // Setter方法
    void setUserId(int userId) { m_userId = userId; }
    void setUserType(const QString &userType) { m_userType = userType; }
    void setUserName(const QString &userName) { m_userName = userName; }
    void setUserCollege(const QString &userCollege) { m_userCollege = userCollege; }
    void setUserGrade(const QString &userGrade) { m_userGrade = userGrade; }
    void setRelationshipStatus(RelationshipStatus status) { m_relationshipStatus = status; }

    // 实用方法
    QString getDisplayName() const;
    QString getDisplayInfo() const;
    QString getRelationshipText() const;
    QString getActionButtonText() const;
    bool isStudent() const { return m_userType == "学生"; }
    bool isTeacher() const { return m_userType == "老师"; }
    bool canAddFriend() const { return m_relationshipStatus == CAN_ADD; }
    bool isValid() const;

private:
    int m_userId;
    QString m_userType;           // "学生" 或 "老师"
    QString m_userName;
    QString m_userCollege;
    QString m_userGrade;          // 仅学生有年级信息
    RelationshipStatus m_relationshipStatus;
};

#endif // USERINFO_H
