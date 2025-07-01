#include "coursenotice.h"
#include <QDebug>

CourseNotice::CourseNotice()
    : m_noticeId(-1), m_courseId(-1), m_isPinned(false)
{
}

CourseNotice::CourseNotice(int noticeId, int courseId, const QString &title, const QString &content,
                           const QDateTime &publishTime, bool isPinned, const QString &courseName)
    : m_noticeId(noticeId), m_courseId(courseId), m_title(title), m_content(content),
    m_publishTime(publishTime), m_isPinned(isPinned), m_courseName(courseName)
{
}

QString CourseNotice::getContentPreview(int maxLength) const
{
    if (m_content.length() <= maxLength) {
        return m_content;
    }

    return m_content.left(maxLength) + "...";
}

QString CourseNotice::getPublishTimeText() const
{
    if (!m_publishTime.isValid()) {
        return "未知时间";
    }

    return m_publishTime.toString("yyyy-MM-dd hh:mm");
}

QString CourseNotice::getRelativeTimeText() const
{
    if (!m_publishTime.isValid()) {
        return "未知时间";
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 seconds = m_publishTime.secsTo(currentTime);

    if (seconds < 0) {
        return "刚刚";
    }

    if (seconds < 60) {
        return "刚刚";
    } else if (seconds < 3600) {
        int minutes = seconds / 60;
        return QString("%1分钟前").arg(minutes);
    } else if (seconds < 86400) {
        int hours = seconds / 3600;
        return QString("%1小时前").arg(hours);
    } else if (seconds < 2592000) {  // 30天
        int days = seconds / 86400;
        return QString("%1天前").arg(days);
    } else {
        return getPublishTimeText();
    }
}

QString CourseNotice::getPinnedStatusText() const
{
    return m_isPinned ? "置顶" : "普通";
}

QString CourseNotice::getTypeIcon() const
{
    if (m_isPinned) {
        return "📌";  // 置顶图标
    }
    return "📢";      // 普通通知图标
}

QString CourseNotice::getDisplayTitle() const
{
    if (m_isPinned) {
        return QString("[置顶] %1").arg(m_title);
    }
    return m_title;
}

bool CourseNotice::isValid() const
{
    return m_noticeId > 0 && m_courseId > 0 && !m_title.isEmpty() && m_publishTime.isValid();
}

bool CourseNotice::isNew() const
{
    if (!m_publishTime.isValid()) {
        return false;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    qint64 hours = m_publishTime.secsTo(currentTime) / 3600;

    return hours <= 24;
}

bool CourseNotice::operator<(const CourseNotice &other) const
{
    // 置顶通知优先
    if (m_isPinned != other.m_isPinned) {
        return m_isPinned;
    }

    // 相同置顶状态下，按发布时间降序排列
    return m_publishTime > other.m_publishTime;
}

bool CourseNotice::operator==(const CourseNotice &other) const
{
    return m_noticeId == other.m_noticeId && m_courseId == other.m_courseId;
}
