#ifndef COURSENOTICE_H
#define COURSENOTICE_H

#include <QString>
#include <QDateTime>

/**
 * @brief 课程通知实体类
 * 封装课程通知的完整信息，提供便于UI使用的格式化方法
 */
class CourseNotice
{
public:
    /**
     * @brief 默认构造函数
     */
    CourseNotice();

    /**
     * @brief 完整构造函数
     * @param noticeId 通知ID
     * @param courseId 课程ID
     * @param title 通知标题
     * @param content 通知内容
     * @param publishTime 发布时间
     * @param isPinned 是否置顶
     * @param courseName 课程名称（可选）
     */
    CourseNotice(int noticeId, int courseId, const QString &title, const QString &content,
                 const QDateTime &publishTime, bool isPinned = false, const QString &courseName = "");

    // Getter 方法
    int getNoticeId() const { return m_noticeId; }
    int getCourseId() const { return m_courseId; }
    QString getTitle() const { return m_title; }
    QString getContent() const { return m_content; }
    QDateTime getPublishTime() const { return m_publishTime; }
    bool isPinned() const { return m_isPinned; }
    QString getCourseName() const { return m_courseName; }

    // Setter 方法
    void setNoticeId(int noticeId) { m_noticeId = noticeId; }
    void setCourseId(int courseId) { m_courseId = courseId; }
    void setTitle(const QString &title) { m_title = title; }
    void setContent(const QString &content) { m_content = content; }
    void setPublishTime(const QDateTime &publishTime) { m_publishTime = publishTime; }
    void setPinned(bool isPinned) { m_isPinned = isPinned; }
    void setCourseName(const QString &courseName) { m_courseName = courseName; }

    // 实用方法

    /**
     * @brief 获取内容预览
     * @param maxLength 最大长度
     * @return 截断后的内容预览
     */
    QString getContentPreview(int maxLength = 50) const;

    /**
     * @brief 获取发布时间的显示文本
     * @return 格式化的时间字符串
     */
    QString getPublishTimeText() const;

    /**
     * @brief 获取相对时间显示
     * @return 相对时间字符串（如"2小时前"）
     */
    QString getRelativeTimeText() const;

    /**
     * @brief 获取置顶状态文本
     * @return 置顶状态的显示文本
     */
    QString getPinnedStatusText() const;

    /**
     * @brief 获取通知类型图标
     * @return 用于显示的图标字符串
     */
    QString getTypeIcon() const;

    /**
     * @brief 获取完整的显示标题（包含置顶标识）
     * @return 带置顶标识的标题
     */
    QString getDisplayTitle() const;

    /**
     * @brief 检查通知是否有效
     * @return 是否有效
     */
    bool isValid() const;

    /**
     * @brief 检查是否为新通知（24小时内）
     * @return 是否为新通知
     */
    bool isNew() const;

    /**
     * @brief 比较操作符（用于排序：置顶优先，然后按时间降序）
     * @param other 另一个通知对象
     * @return 是否应该排在前面
     */
    bool operator<(const CourseNotice &other) const;

    /**
     * @brief 等于操作符
     * @param other 另一个通知对象
     * @return 是否相等
     */
    bool operator==(const CourseNotice &other) const;

private:
    int m_noticeId;           // 通知ID
    int m_courseId;           // 课程ID
    QString m_title;          // 通知标题
    QString m_content;        // 通知内容
    QDateTime m_publishTime;  // 发布时间
    bool m_isPinned;          // 是否置顶
    QString m_courseName;     // 课程名称（用于跨课程显示）
};

#endif // COURSENOTICE_H
