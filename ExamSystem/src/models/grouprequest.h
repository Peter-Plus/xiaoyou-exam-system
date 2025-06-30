#ifndef GROUPREQUEST_H
#define GROUPREQUEST_H

#include <QString>
#include <QDateTime>

class GroupRequest
{
public:
    enum RequestType {
        SENT,       // 我发送的申请
        RECEIVED    // 我收到的申请
    };

    enum RequestStatus {
        PENDING,    // 申请中
        ACCEPTED,   // 已同意
        REJECTED    // 已拒绝
    };

    // 构造函数
    GroupRequest();
    GroupRequest(int requestId, int groupId, const QString &groupName,
                 int requesterId, const QString &requesterType, const QString &requesterName,
                 const QString &requesterCollege, const QString &requesterGrade,
                 int creatorId, const QString &creatorType, const QString &creatorName,
                 RequestStatus status, const QDateTime &requestTime, RequestType type = RECEIVED);

    // Getter方法
    int getRequestId() const { return m_requestId; }
    int getGroupId() const { return m_groupId; }
    QString getGroupName() const { return m_groupName; }
    int getRequesterId() const { return m_requesterId; }
    QString getRequesterType() const { return m_requesterType; }
    QString getRequesterName() const { return m_requesterName; }
    QString getRequesterCollege() const { return m_requesterCollege; }
    QString getRequesterGrade() const { return m_requesterGrade; }
    int getCreatorId() const { return m_creatorId; }
    QString getCreatorType() const { return m_creatorType; }
    QString getCreatorName() const { return m_creatorName; }
    RequestStatus getStatus() const { return m_status; }
    QDateTime getRequestTime() const { return m_requestTime; }
    RequestType getType() const { return m_type; }

    // Setter方法
    void setRequestId(int requestId) { m_requestId = requestId; }
    void setGroupId(int groupId) { m_groupId = groupId; }
    void setGroupName(const QString &groupName) { m_groupName = groupName; }
    void setRequesterId(int requesterId) { m_requesterId = requesterId; }
    void setRequesterType(const QString &requesterType) { m_requesterType = requesterType; }
    void setRequesterName(const QString &requesterName) { m_requesterName = requesterName; }
    void setRequesterCollege(const QString &requesterCollege) { m_requesterCollege = requesterCollege; }
    void setRequesterGrade(const QString &requesterGrade) { m_requesterGrade = requesterGrade; }
    void setCreatorId(int creatorId) { m_creatorId = creatorId; }
    void setCreatorType(const QString &creatorType) { m_creatorType = creatorType; }
    void setCreatorName(const QString &creatorName) { m_creatorName = creatorName; }
    void setStatus(RequestStatus status) { m_status = status; }
    void setRequestTime(const QDateTime &requestTime) { m_requestTime = requestTime; }
    void setType(RequestType type) { m_type = type; }

    // 便于UI显示的格式化方法
    QString getRequesterDisplayText() const;
    QString getGroupDisplayText() const;
    QString getStatusText() const;
    QString getRequestTimeText() const;
    QString getTypeText() const;
    QString getFullDescription() const;

    // 状态判断方法
    bool isPending() const;
    bool isAccepted() const;
    bool isRejected() const;
    bool isSentByMe() const;
    bool isReceivedByMe() const;
    bool canBeProcessed() const; // 是否可以处理（同意/拒绝）

    // 实用方法
    bool isValid() const;
    QString getActionDescription() const; // 获取操作描述

private:
    int m_requestId;
    int m_groupId;
    QString m_groupName;
    int m_requesterId;
    QString m_requesterType;
    QString m_requesterName;
    QString m_requesterCollege;
    QString m_requesterGrade;
    int m_creatorId;
    QString m_creatorType;
    QString m_creatorName;
    RequestStatus m_status;
    QDateTime m_requestTime;
    RequestType m_type;

    // 私有辅助方法
    QString formatTimeDisplay(const QDateTime &dateTime) const;
};

#endif // GROUPREQUEST_H
