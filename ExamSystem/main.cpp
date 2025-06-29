#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "database.h"
#include "loginwindow.h"
#include "studentmainwindow.h"
#include "teachermainwindow.h"
#include "student.h"
#include "teacher.h"

class AppController
{
public:
    AppController() : currentMainWindow(nullptr), database(nullptr) {}

    ~AppController()
    {
        if (database) {
            delete database;
        }
        if (currentMainWindow) {
            delete currentMainWindow;
        }
    }

    void start()
    {
        // 检查可用驱动
        QStringList drivers = QSqlDatabase::drivers();
        qDebug() << "可用驱动:" << drivers;

        // 创建数据库连接
        database = new Database();
        if (database->connectToDatabase())
        {
            qDebug() << "数据库连接成功";
        } else
        {
            qDebug() << "数据库连接失败，程序继续运行";
        }

        // 显示登录窗口
        showLoginWindow();
    }

    // 处理登录成功
    void onLoginSuccess(int userId, bool isStudent)
    {
        qDebug() << "用户登录成功，ID:" << userId << "是否为学生:" << isStudent;

        // 关闭当前主窗口（如果存在）
        if (currentMainWindow) {
            currentMainWindow->close();
            delete currentMainWindow;
            currentMainWindow = nullptr;
        }

        // 根据用户类型创建对应的主界面
        if (isStudent) {
            // 创建学生对象（这里需要从数据库获取学生信息）
            Student student = getStudentInfo(userId);
            // 使用新的构造函数，传递database参数
            StudentMainWindow *studentWindow = new StudentMainWindow(student, database);
            currentMainWindow = studentWindow;
            QObject::connect(studentWindow, &StudentMainWindow::logoutRequested, [this]() {
                this->onLogout();
            });
        }
        else
        {
            // 创建教师对象（这里需要从数据库获取教师信息）
            Teacher teacher = getTeacherInfo(userId);
            TeacherMainWindow *teacherWindow = new TeacherMainWindow(teacher, database);
            currentMainWindow = teacherWindow;
            QObject::connect(teacherWindow, &TeacherMainWindow::logoutRequested, [this]()
                             {
                                 this->onLogout();
                             });
        }

        currentMainWindow->show();
    }

    // 处理退出登录
    void onLogout()
    {
        qDebug() << "用户退出登录";

        // 关闭当前主窗口
        if (currentMainWindow) {
            currentMainWindow->close();
            delete currentMainWindow;
            currentMainWindow = nullptr;
        }

        // 重新显示登录窗口
        showLoginWindow();
    }

private:
    void showLoginWindow()
    {
        LoginWindow *loginWindow = new LoginWindow();
        QObject::connect(loginWindow, &LoginWindow::loginSuccess, [this](int userId, bool isStudent) {
            this->onLoginSuccess(userId, isStudent);
        });
        loginWindow->show();
    }

    // 从数据库获取学生信息
    Student getStudentInfo(int studentId)
    {
        if (!database) {
            // 如果数据库连接不存在，返回默认学生对象
            Student defaultStudent;
            defaultStudent.setId(studentId);
            defaultStudent.setName("未知学生");
            defaultStudent.setGrade("未知");
            defaultStudent.setCollege("未知学院");
            return defaultStudent;
        }

        QSqlQuery query;
        query.prepare("SELECT name, grade, college FROM students WHERE student_id = ?");
        query.addBindValue(studentId);

        if (query.exec() && query.next()) {
            Student student;
            student.setId(studentId);
            student.setName(query.value("name").toString());
            student.setGrade(query.value("grade").toString());
            student.setCollege(query.value("college").toString());
            return student;
        }

        // 如果查询失败，返回默认学生对象
        Student defaultStudent;
        defaultStudent.setId(studentId);
        defaultStudent.setName("未知学生");
        defaultStudent.setGrade("未知");
        defaultStudent.setCollege("未知学院");
        return defaultStudent;
    }

    Teacher getTeacherInfo(int teacherId)
    {
        if (!database) {
            // 如果数据库连接不存在，返回默认教师对象
            Teacher defaultTeacher;
            defaultTeacher.setId(teacherId);
            defaultTeacher.setName("未知教师");
            defaultTeacher.setCollege("未知学院");
            return defaultTeacher;
        }

        QSqlQuery query;
        query.prepare("SELECT name, college FROM teachers WHERE teacher_id = ?");
        query.addBindValue(teacherId);

        if (query.exec() && query.next()) {
            Teacher teacher;
            teacher.setId(teacherId);
            teacher.setName(query.value("name").toString());
            teacher.setCollege(query.value("college").toString());
            return teacher;
        }

        // 如果查询失败，返回默认教师对象
        Teacher defaultTeacher;
        defaultTeacher.setId(teacherId);
        defaultTeacher.setName("未知教师");
        defaultTeacher.setCollege("未知学院");
        return defaultTeacher;
    }

    QMainWindow *currentMainWindow;
    Database *database;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //创建应用控制器并启动
    AppController controller;
    controller.start();
    return app.exec();
}
