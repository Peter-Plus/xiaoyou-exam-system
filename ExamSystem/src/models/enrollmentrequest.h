#ifndef ENROLLMENTREQUEST_H
#define ENROLLMENTREQUEST_H

#include <QString>
#include <QDateTime>

/**
 * @brief 选课申请实体类
 * 封装选课申请的完整信息，提供便于UI使用的格式化方法
 */
class EnrollmentRequest
{
public:
    /**
     * @brief 申请状态枚举
     */
    enum RequestStatus {
        PENDING = 0,     // 申请中
        APPROVED = 1     // 已通过
    };

    /**
     * @brief 默认构造函数
     */
    EnrollmentRequest();

    /**
     * @brief 完整构造函数
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @param enrollmentTime 申请时间
     * @param status 申请状态
     * @param studentName 学生姓名
     * @param studentGrade 学生年级
     * @param studentCollege 学生学院
     * @param courseName 课程名称
     * @param courseCollege 课程学院
     * @param teacherName 任课教师姓名
     */
    EnrollmentRequest(int studentId, int courseId, const QDateTime &enrollmentTime,
                      RequestStatus status = PENDING, const QString &studentName = "",
                      const QString &studentGrade = "", const QString &studentCollege = "",
                      const QString &courseName = "", const QString &courseCollege = "",
                      const QString &teacherName = "");

    // Getter 方法
    int getStudentId() const { return m_studentId; }
    int getCourseId() const { return m_courseId; }
    QDateTime getEnrollmentTime() const { return m_enrollmentTime; }
    RequestStatus getStatus() const { return m_status; }
    QString getStudentName() const { return m_studentName; }
    QString getStudentGrade() const { return m_studentGrade; }
    QString getStudentCollege() const { return m_studentCollege; }
    QString getCourseName() const { return m_courseName; }
    QString getCourseCollege() const { return m_courseCollege; }
    QString getTeacherName() const { return m_teacherName; }

    // Setter 方法
    void setStudentId(int studentId) { m_studentId = studentId; }
    void setCourseId(int courseId) { m_courseId = courseId; }
    void setEnrollmentTime(const QDateTime &enrollmentTime) { m_enrollmentTime = enrollmentTime; }
    void setStatus(RequestStatus status) { m_status = status; }
    void setStudentName(const QString &studentName) { m_studentName = studentName; }
    void setStudentGrade(const QString &studentGrade) { m_studentGrade = studentGrade; }
    void setStudentCollege(const QString &studentCollege) { m_studentCollege = studentCollege; }
    void setCourseName(const QString &courseName) { m_courseName = courseName; }
    void setCourseCollege(const QString &courseCollege) { m_courseCollege = courseCollege; }
    void setTeacherName(const QString &teacherName) { m_teacherName = teacherName; }

    // 静态方法
    static RequestStatus statusFromString(const QString &statusStr);
    static QString statusToString(RequestStatus status);

    // 实用方法

    /**
     * @brief 获取申请时间的显示文本
     * @return 格式化的时间字符串
     */
    QString getEnrollmentTimeText() const;

    /**
     * @brief 获取相对时间显示
     * @return 相对时间字符串（如"2小时前"）
     */
    QString getRelativeTimeText() const;

    /**
     * @brief 获取状态显示文本
     * @return 状态的中文描述
     */
    QString getStatusText() const;

    /**
     * @brief 获取状态图标
     * @return 用于显示的图标字符串
     */
    QString getStatusIcon() const;

    /**
     * @brief 获取状态颜色（用于UI样式）
     * @return 颜色字符串
     */
    QString getStatusColor() const;

    /**
     * @brief 获取学生完整信息
     * @return 格式化的学生信息字符串
     */
    QString getStudentInfo() const;

    /**
     * @brief 获取课程完整信息
     * @return 格式化的课程信息字符串
     */
    QString getCourseInfo() const;

    /**
     * @brief 获取申请优先级（用于排序）
     * @return 优先级数值，越大越优先
     */
    int getPriority() const;

    /**
     * @brief 检查申请是否有效
     * @return 是否有效
     */
    bool isValid() const;

    /**
     * @brief 检查是否为待处理状态
     * @return 是否待处理
     */
    bool isPending() const;

    /**
     * @brief 检查是否已通过
     * @return 是否已通过
     */
    bool isApproved() const;

    /**
     * @brief 检查是否为新申请（24小时内）
     * @return 是否为新申请
     */
    bool isNew() const;

    /**
     * @brief 获取申请描述（用于审核界面）
     * @return 完整的申请描述
     */
    QString getRequestDescription() const;

    /**
     * @brief 比较操作符（用于排序：申请时间降序）
     * @param other 另一个申请对象
     * @return 是否应该排在前面
     */
    bool operator<(const EnrollmentRequest &other) const;

    /**
     * @brief 等于操作符
     * @param other 另一个申请对象
     * @return 是否相等
     */
    bool operator==(const EnrollmentRequest &other) const;

private:
    int m_studentId;              // 学生ID
    int m_courseId;               // 课程ID
    QDateTime m_enrollmentTime;   // 申请时间
    RequestStatus m_status;       // 申请状态
    QString m_studentName;        // 学生姓名
    QString m_studentGrade;       // 学生年级
    QString m_studentCollege;     // 学生学院
    QString m_courseName;         // 课程名称
    QString m_courseCollege;      // 课程学院
    QString m_teacherName;        // 任课教师姓名
};

#endif // ENROLLMENTREQUEST_H
