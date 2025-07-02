#include "coursenotice.h"

CourseNotice::CourseNotice()
    : noticeId(0), courseId(0), isPinned(false)
{
}

CourseNotice::CourseNotice(int noticeId, int courseId, const QString &title,
                           const QString &content, const QDateTime &publishTime, bool isPinned)
    : noticeId(noticeId), courseId(courseId), title(title),
    content(content), publishTime(publishTime), isPinned(isPinned)
{
}

QString CourseNotice::getContentPreview(int maxLength) const
{
    if (content.length() <= maxLength) {
        return content;
    }
    return content.left(maxLength) + "...";
}

QString CourseNotice::getTimeString() const
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsAgo = publishTime.secsTo(now);

    if (secondsAgo < 60) {
        return "刚刚";
    } else if (secondsAgo < 3600) {
        return QString("%1分钟前").arg(secondsAgo / 60);
    } else if (secondsAgo < 86400) {
        return QString("%1小时前").arg(secondsAgo / 3600);
    } else if (secondsAgo < 604800) {
        return QString("%1天前").arg(secondsAgo / 86400);
    } else {
        return publishTime.toString("yyyy-MM-dd hh:mm");
    }
}

QString CourseNotice::getPinnedStatusText() const
{
    return isPinned ? "置顶" : "";
}

bool CourseNotice::isValid() const
{
    return noticeId > 0 &&
           courseId > 0 &&
           !title.isEmpty() &&
           !content.isEmpty() &&
           publishTime.isValid();
}
