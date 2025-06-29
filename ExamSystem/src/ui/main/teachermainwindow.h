#ifndef TEACHERMAINWINDOW_H
#define TEACHERMAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QMessageBox>
#include "teacher.h"
#include "questionmanager.h"
#include "exammanager.h"
#include "examgrading.h"
#include "classstatisticswindow.h"  // 新增：班级成绩统计窗口
#include "database.h"

class TeacherMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    TeacherMainWindow(const Teacher &teacher, Database *database, QWidget *parent = nullptr);
    ~TeacherMainWindow();

private slots:
    void onQuestionManagementClicked();  // 题库管理按钮点击
    void onExamManagementClicked();      // 考试管理按钮点击
    void onGradingClicked();             // 阅卷按钮点击
    void onScoreAnalysisClicked();       // 班级成绩分析按钮点击
    void onLogoutClicked();              // 退出登录按钮点击

signals:
    void logoutRequested();              // 退出登录信号

private:
    void setupUI();                      // 设置界面
    void setupWelcomeArea();             // 设置欢迎区域
    void setupFunctionArea();            // 设置功能区域
    void setupBottomArea();              // 设置底部区域
    QPushButton* createFunctionButton(const QString &text, const QString &description); // 创建功能按钮

    // 成员变量
    Teacher m_teacher;                   // 当前教师信息
    QuestionManager *m_questionManager; // 题库管理窗口
    ExamManager *m_examManager;          // 考试管理窗口
    ExamGrading *m_gradingWindow;        // 阅卷窗口
    ClassStatisticsWindow *m_statisticsWindow; // 班级成绩统计窗口
    Database *m_database;                // 数据库连接

    // 界面组件
    QLabel *m_welcomeLabel;              // 欢迎标签
    QLabel *m_infoLabel;                 // 信息标签
    QPushButton *m_questionMgmtBtn;      // 题库管理按钮
    QPushButton *m_examMgmtBtn;          // 考试管理按钮
    QPushButton *m_gradingBtn;           // 阅卷按钮
    QPushButton *m_scoreAnalysisBtn;     // 成绩分析按钮
    QPushButton *m_logoutBtn;            // 退出登录按钮
};

#endif // TEACHERMAINWINDOW_H
