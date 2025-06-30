#ifndef MESSAGEINFO_H
#define MESSAGEINFO_H

#include <QString>
#include <QDateTime>
#include <Qt>

class MessageInfo
{
public:
    enum MessageType { TEXT, SYSTEM };
    enum MessageStatus { SENDING, SENT, FAILED };

    // 构造函数
    MessageInfo();
    MessageInfo(int id, int sId, const QString &sType, const QString &sName);

    // 属性访问器
    int getMessageId() const { return messageId; }
    void setMessageId(int id) { messageId = id; }

    int getSenderId() const { return senderId; }
    void setSenderId(int id) { senderId = id; }

    QString getSenderType() const { return senderType; }
    void setSenderType(const QString &type) { senderType = type; }

    QString getSenderName() const { return senderName; }
    void setSenderName(const QString &name) { senderName = name; }

    QString getContent() const { return content; }
    void setContent(const QString &text) { content = text; }

    QDateTime getSendTime() const { return sendTime; }
    void setSendTime(const QDateTime &time) { sendTime = time; }

    MessageType getType() const { return type; }
    void setType(MessageType msgType) { type = msgType; }

    MessageStatus getStatus() const { return status; }
    void setStatus(MessageStatus msgStatus) { status = msgStatus; }

    bool getIsFromMe() const { return isFromMe; }
    void setIsFromMe(bool fromMe) { isFromMe = fromMe; }

    // 显示格式化方法
    QString getTimeDisplay() const;
    QString getSenderDisplay() const;
    QString getContentHtml() const;

    // 状态判断方法
    bool isSentByMe() const { return isFromMe; }
    bool isSystemMessage() const { return type == SYSTEM; }
    Qt::Alignment getAlignment() const;

    // 样式相关方法
    QString getBubbleColor() const;
    QString getTextColor() const;

    // 有效性检查
    bool isValid() const;

private:
    int messageId;
    int senderId;
    QString senderType;
    QString senderName;
    QString content;
    QDateTime sendTime;
    MessageType type;
    MessageStatus status;
    bool isFromMe;

    // 辅助方法
    QString formatMessageTime(const QDateTime &time) const;
    QString escapeHtml(const QString &text) const;
};

#endif // MESSAGEINFO_H
