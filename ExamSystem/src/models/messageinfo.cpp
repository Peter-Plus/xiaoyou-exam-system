#include "messageinfo.h"
#include <QTime>

MessageInfo::MessageInfo()
    : messageId(-1)
    , senderId(-1)
    , senderType("")
    , senderName("")
    , content("")
    , sendTime()
    , type(TEXT)
    , status(SENT)
    , isFromMe(false)
{
}

MessageInfo::MessageInfo(int id, int sId, const QString &sType, const QString &sName)
    : messageId(id)
    , senderId(sId)
    , senderType(sType)
    , senderName(sName)
    , content("")
    , sendTime()
    , type(TEXT)
    , status(SENT)
    , isFromMe(false)
{
}

QString MessageInfo::getTimeDisplay() const
{
    if (!sendTime.isValid()) {
        return "";
    }

    return formatMessageTime(sendTime);
}

QString MessageInfo::getSenderDisplay() const
{
    if (isSystemMessage()) {
        return "系统消息";
    }

    if (senderName.isEmpty()) {
        return QString("%1用户").arg(senderType);
    }

    return senderName;
}

QString MessageInfo::getContentHtml() const
{
    if (isSystemMessage()) {
        return QString("<i style='color: #999;'>%1</i>").arg(escapeHtml(content));
    }

    // 简单的文本处理，支持换行
    QString htmlContent = escapeHtml(content);
    htmlContent.replace("\n", "<br>");

    return htmlContent;
}

Qt::Alignment MessageInfo::getAlignment() const
{
    if (isSystemMessage()) {
        return Qt::AlignCenter;
    }

    return isFromMe ? Qt::AlignRight : Qt::AlignLeft;
}

QString MessageInfo::getBubbleColor() const
{
    if (isSystemMessage()) {
        return "#f0f0f0";
    }

    if (isFromMe) {
        return "#007AFF"; // iOS蓝色
    } else {
        return "#E5E5EA"; // 浅灰色
    }
}

QString MessageInfo::getTextColor() const
{
    if (isSystemMessage()) {
        return "#999999";
    }

    if (isFromMe) {
        return "#FFFFFF"; // 白色文字
    } else {
        return "#000000"; // 黑色文字
    }
}

bool MessageInfo::isValid() const
{
    return messageId > 0 && senderId > 0 && !content.isEmpty();
}

QString MessageInfo::formatMessageTime(const QDateTime &time) const
{
    if (!time.isValid()) {
        return "";
    }

    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();
    QDate messageDate = time.date();

    if (messageDate == today) {
        // 今天：显示时间
        return time.toString("hh:mm");
    } else if (messageDate == today.addDays(-1)) {
        // 昨天
        return QString("昨天 %1").arg(time.toString("hh:mm"));
    } else if (messageDate.year() == today.year()) {
        // 今年：显示月日和时间
        return time.toString("MM-dd hh:mm");
    } else {
        // 其他年份：显示完整日期时间
        return time.toString("yyyy-MM-dd hh:mm");
    }
}

QString MessageInfo::escapeHtml(const QString &text) const
{
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&#39;");
    return escaped;
}
