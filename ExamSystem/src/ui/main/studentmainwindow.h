#ifndef STUDENTMAINWINDOW_H
#define STUDENTMAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QFrame>
#include <QScrollArea>
#include "student.h"
#include "database.h"
#include "studentexamlist.h"
#include "examresult.h"
#include "../../ui/friend/friendpage.h"
#include "../../ui/chat/chatpage.h"
#include "../../ui/course/coursepage.h"
#include "../../ui/course/enrollmentwidget.h"
#include "../../ui/course/noticewidget.h"
#include "../../ui/course/assignmentwidget.h"

class StudentMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 新的构造函数：接受Student对象和Database
    explicit StudentMainWindow(const Student &student, Database *database, QWidget *parent = nullptr);

    // 保留原构造函数以保持兼容性
    explicit StudentMainWindow(int studentId, QWidget *parent = nullptr);

private slots:
    void onNavigationClicked();
    void onLogoutClicked();
    // 好友相关槽函数
    void onFriendAdded(int friendId, const QString &friendType);
    void onFriendRemoved(int friendId, const QString &friendType);
    void onRequestProcessed();
    // 聊天相关槽函数
    void onMessageSent(int chatId);
    void onChatOpened(int friendId, const QString &friendName);
    // 新增：处理好友双击事件的槽函数
    void onFriendDoubleClickedToChat(int friendId, const QString &friendType, const QString &friendName);
    void onCourseEnrolled(int courseId);
    void onEnrollmentProcessed(int studentId, int courseId, bool approved);
    void onNoticePublished(int courseId, const QString &title);
    void onAssignmentPublished(int courseId, const QString &title);



private:
    void setupUI();
    void setupNavigationBar();
    void setupContentArea();
    void setupHeaderArea();
    void createExamPage();
    void createScorePage();
    void createChatPage();        // 2.0预留
    void createFriendPage();      // 2.0预留
    void createCoursePage();      // 2.0预留
    void createSettingsPage();    // 2.0预留

    void switchToPage(int pageIndex);
    void updateUserInfo();
    void setModernStyle();

    // 导航相关
    enum NavigationPage {
        PAGE_CHAT = 0,      // 聊天（2.0功能）
        PAGE_FRIEND = 1,    // 好友（2.0功能）
        PAGE_COURSE = 2,    // 课程（2.0功能）
        PAGE_EXAM = 3,      // 考试（1.0功能）
        PAGE_SCORE = 4,     // 成绩（1.0功能）
        PAGE_SETTINGS = 5   // 设置
    };

    Student currentStudent;
    Database *database;
    FriendPage *m_friendPage;
    ChatPage *m_chatPage;
    CoursePage *m_coursePage;

    // 子窗口指针
    StudentExamList *examListWindow;

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
    QWidget *examPage;
    QWidget *scorePage;
    QWidget *settingsPage;

    // 考试页面组件
    QPushButton *startExamButton;
    QLabel *examStatusLabel;

    // 成绩页面组件
    QPushButton *viewScoreButton;
    QLabel *scoreInfoLabel;

signals:
    void logoutRequested();
};

#endif // STUDENTMAINWINDOW_H
