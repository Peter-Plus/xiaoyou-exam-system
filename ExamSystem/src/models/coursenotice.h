#ifndef COURSENOTICE_H
#define COURSENOTICE_H

#include <QString>
#include <QDateTime>

class CourseNotice
{
private:
    int noticeId;
    int courseId;
    QString title;
    QString content;
    QDateTime publishTime;
    bool isPinned;

public:
    // 构造函数
    CourseNotice();
    CourseNotice(int noticeId, int courseId, const QString &title,
                 const QString &content, const QDateTime &publishTime, bool isPinned = false);

    // Getter方法
    int getNoticeId() const { return noticeId; }
    int getCourseId() const { return courseId; }
    QString getTitle() const { return title; }
    QString getContent() const { return content; }
    QDateTime getPublishTime() const { return publishTime; }
    bool getIsPinned() const { return isPinned; }

    // Setter方法
    void setNoticeId(int id) { noticeId = id; }
    void setCourseId(int id) { courseId = id; }
    void setTitle(const QString &t) { title = t; }
    void setContent(const QString &c) { content = c; }
    void setPublishTime(const QDateTime &time) { publishTime = time; }
    void setIsPinned(bool pinned) { isPinned = pinned; }

    // 实用方法
    QString getContentPreview(int maxLength = 50) const;
    QString getTimeString() const;
    QString getPinnedStatusText() const;
    bool isValid() const;
};

#endif // COURSENOTICE_H
