#ifndef COURSE_H
#define COURSE_H

#include <QString>

class Course
{
private:
    int courseId;
    int teacherId;
    QString college;
    QString courseName;
    QString teacherName; // 扩展字段
    QString description;    // 课程描述
    int credits;            // 学分
    int courseHours;        // 课时
    QString semester;       // 学期
    int maxStudents;        // 最大选课人数
    QString status;         // 选课状态 (开放选课/关闭选课/已结束)
    int studentCount;
public:
    // 构造函数
    Course();
    Course(int courseId, int teacherId, const QString &college, const QString &courseName,
               const QString &description, int credits, int courseHours,
               const QString &semester, int maxStudents, const QString &status);

    // Getter方法
    int getCourseId() const { return courseId; }
    int getTeacherId() const { return teacherId; }
    QString getCollege() const { return college; }
    QString getCourseName() const { return courseName; }
    QString getTeacherName() const { return teacherName; }
    QString getDescription() const;
    int     getCredits() const;
    int     getCourseHours() const;
    QString getSemester() const;
    int     getMaxStudents() const;
    QString getStatus() const;
    int getStudentCount() const { return studentCount; }

    // Setter方法
    void setCourseId(int id) { courseId = id; }
    void setTeacherId(int id) { teacherId = id; }
    void setCollege(const QString &col) { college = col; }
    void setCourseName(const QString &name) { courseName = name; }
    void setTeacherName(const QString &name) { teacherName = name; }
    void setDescription(const QString &desc);
    void setCredits(int c);
    void setCourseHours(int h);
    void setSemester(const QString &sem);
    void setMaxStudents(int m);
    void setStatus(const QString &s);
    void setStudentCount(int c);

    // 实用方法
    QString getDisplayName() const;
    bool isValid() const;
};

#endif // COURSE_H
