#ifndef TEACHERMAINWINDOW_H
#define TEACHERMAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include "teacher.h"
#include "questionmanager.h"
#include "exammanager.h"
#include "examgrading.h"
#include "classstatisticswindow.h"
#include "database.h"
#include "../../ui/friend/friendpage.h"
#include "../../ui/chat/chatpage.h"

class TeacherMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    TeacherMainWindow(const Teacher &teacher, Database *database, QWidget *parent = nullptr);
    ~TeacherMainWindow();

private slots:
    void onNavigationClicked();
    void onQuestionManagementClicked();
    void onExamManagementClicked();
    void onGradingClicked();
    void onScoreAnalysisClicked();
    void onLogoutClicked();
    // 好友相关槽函数
    void onFriendAdded(int friendId, const QString &friendType);
    void onFriendRemoved(int friendId, const QString &friendType);
    void onRequestProcessed();
    // 聊天相关槽函数
    void onMessageSent(int chatId);
    void onChatOpened(int friendId, const QString &friendName);
    void onFriendDoubleClickedToChat(int friendId, const QString &friendType, const QString &friendName);

private:
    void setupUI();
    void setupNavigationBar();
    void setupContentArea();
    void setupHeaderArea();

    void createChatPage();          // 2.0预留
    void createFriendPage();        // 2.0预留
    void createCoursePage();        // 2.0预留
    void createQuestionPage();      // 题库管理
    void createExamPage();          // 考试管理
    void createGradingPage();       // 阅卷
    void createStatisticsPage();    // 成绩统计
    void createSettingsPage();      // 设置

    void switchToPage(int pageIndex);
    void updateUserInfo();
    void setModernStyle();
    QPushButton* createFeatureCard(const QString &title, const QString &subtitle, const QString &buttonText);

    // 导航相关
    enum NavigationPage {
        PAGE_CHAT = 0,          // 聊天（2.0功能）
        PAGE_FRIEND = 1,        // 好友（2.0功能）
        PAGE_COURSE = 2,        // 课程（2.0功能）
        PAGE_QUESTION = 3,      // 题库管理
        PAGE_EXAM = 4,          // 考试管理
        PAGE_GRADING = 5,       // 阅卷
        PAGE_STATISTICS = 6,    // 成绩统计
        PAGE_SETTINGS = 7       // 设置
    };

    // 成员变量
    Teacher m_teacher;
    Database *m_database;
    FriendPage *m_friendPage;
    ChatPage *m_chatPage;

    // 子窗口指针
    QuestionManager *m_questionManager;
    ExamManager *m_examManager;
    ExamGrading *m_gradingWindow;
    ClassStatisticsWindow *m_statisticsWindow;

    // 主要UI组件
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QSplitter *mainSplitter;

    // 左侧导航栏
    QFrame *navigationFrame;
    QVBoxLayout *navLayout;
    QListWidget *navigationList;

    // 顶部用户信息
    QFrame *headerFrame;
    QLabel *userAvatarLabel;
    QLabel *userNameLabel;
    QLabel *userInfoLabel;
    QPushButton *settingsButton;
    QPushButton *logoutButton;

    // 中间内容区域
    QFrame *contentFrame;
    QStackedWidget *contentStack;

    // 各个页面
    QWidget *chatPage;
    QWidget *friendPage;
    QWidget *coursePage;
    QWidget *questionPage;
    QWidget *examPage;
    QWidget *gradingPage;
    QWidget *statisticsPage;
    QWidget *settingsPage;

signals:
    void logoutRequested();
};

#endif // TEACHERMAINWINDOW_H
