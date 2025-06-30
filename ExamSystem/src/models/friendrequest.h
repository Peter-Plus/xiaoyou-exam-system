#ifndef FRIENDREQUEST_H
#define FRIENDREQUEST_H

#include <QString>
#include <QDateTime>

class FriendRequest
{
public:
    enum RequestType {
        RECEIVED,   // 收到的请求
        SENT        // 发送的请求
    };

    enum RequestStatus {
        PENDING,    // 申请中
        ACCEPTED,   // 已同意
        REJECTED    // 已拒绝
    };

    FriendRequest();
    FriendRequest(int requestId, int userId, const QString &userType,
                  const QString &userName, const QString &userCollege,
                  const QString &userGrade = QString());

    // Getter方法
    int getRequestId() const { return m_requestId; }
    int getUserId() const { return m_userId; }
    QString getUserType() const { return m_userType; }
    QString getUserName() const { return m_userName; }
    QString getUserCollege() const { return m_userCollege; }
    QString getUserGrade() const { return m_userGrade; }
    RequestType getRequestType() const { return m_requestType; }
    RequestStatus getStatus() const { return m_status; }
    QDateTime getRequestTime() const { return m_requestTime; }

    // Setter方法
    void setRequestId(int requestId) { m_requestId = requestId; }
    void setUserId(int userId) { m_userId = userId; }
    void setUserType(const QString &userType) { m_userType = userType; }
    void setUserName(const QString &userName) { m_userName = userName; }
    void setUserCollege(const QString &userCollege) { m_userCollege = userCollege; }
    void setUserGrade(const QString &userGrade) { m_userGrade = userGrade; }
    void setRequestType(RequestType requestType) { m_requestType = requestType; }
    void setStatus(RequestStatus status) { m_status = status; }
    void setRequestTime(const QDateTime &requestTime) { m_requestTime = requestTime; }

    // 实用方法
    QString getDisplayName() const;
    QString getDisplayInfo() const;
    QString getStatusText() const;
    QString getRequestTypeText() const;
    QString getTimeText() const;
    bool isStudent() const { return m_userType == "学生"; }
    bool isTeacher() const { return m_userType == "老师"; }
    bool isPending() const { return m_status == PENDING; }
    bool isValid() const;

private:
    int m_requestId;
    int m_userId;
    QString m_userType;        // "学生" 或 "老师"
    QString m_userName;
    QString m_userCollege;
    QString m_userGrade;       // 仅学生有年级信息
    RequestType m_requestType; // 请求类型
    RequestStatus m_status;    // 请求状态
    QDateTime m_requestTime;
};

#endif // FRIENDREQUEST_H
