#ifndef FRIENDINFO_H
#define FRIENDINFO_H

#include <QString>
#include <QDateTime>

class FriendInfo
{
public:
    FriendInfo();
    FriendInfo(int friendId, const QString &friendType, const QString &friendName,
               const QString &friendCollege, const QString &friendGrade = QString());

    // Getter方法
    int getFriendId() const { return m_friendId; }
    QString getFriendType() const { return m_friendType; }
    QString getFriendName() const { return m_friendName; }
    QString getFriendCollege() const { return m_friendCollege; }
    QString getFriendGrade() const { return m_friendGrade; }
    QDateTime getCreatedTime() const { return m_createdTime; }

    // Setter方法
    void setFriendId(int friendId) { m_friendId = friendId; }
    void setFriendType(const QString &friendType) { m_friendType = friendType; }
    void setFriendName(const QString &friendName) { m_friendName = friendName; }
    void setFriendCollege(const QString &friendCollege) { m_friendCollege = friendCollege; }
    void setFriendGrade(const QString &friendGrade) { m_friendGrade = friendGrade; }
    void setCreatedTime(const QDateTime &createdTime) { m_createdTime = createdTime; }

    // 实用方法
    QString getDisplayName() const;
    QString getDisplayInfo() const;
    QString getFriendTypeText() const;
    bool isStudent() const { return m_friendType == "学生"; }
    bool isTeacher() const { return m_friendType == "老师"; }
    bool isValid() const;

private:
    int m_friendId;
    QString m_friendType;      // "学生" 或 "老师"
    QString m_friendName;
    QString m_friendCollege;
    QString m_friendGrade;     // 仅学生有年级信息
    QDateTime m_createdTime;
};

#endif // FRIENDINFO_H
