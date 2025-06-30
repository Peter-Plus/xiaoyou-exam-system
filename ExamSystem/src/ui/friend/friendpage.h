// ============================================================================
// friendpage.h 完整修复版本
// ============================================================================

#ifndef FRIENDPAGE_H
#define FRIENDPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QTimer>
#include <QFrame>

// 引入数据模型和管理器
#include "../../models/friendinfo.h"
#include "../../models/friendrequest.h"
#include "../../models/userinfo.h"
#include "../../core/database.h"

class FriendPage : public QWidget
{
    Q_OBJECT

public:
    explicit FriendPage(Database *database, int currentUserId, const QString &currentUserType, QWidget *parent = nullptr);
    ~FriendPage();

public slots:
    void refreshAll();               // 刷新所有数据
    void showFriendsList();          // 显示好友列表
    void showAddFriend();            // 显示添加好友页面
    void showRequests();             // 显示申请管理页面

signals:
    void friendAdded(int friendId, const QString &friendType);     // 好友添加成功信号
    void friendRemoved(int friendId, const QString &friendType);   // 好友删除信号
    void requestProcessed();         // 申请处理完成信号

private slots:
    // 导航相关
    void onNavigationClicked();

    // 好友列表相关
    void onFriendDoubleClicked(QListWidgetItem *item);
    void onRemoveFriendClicked();

    // 搜索添加好友相关
    void onSearchTextChanged();
    void onSearchButtonClicked();
    void onAddFriendClicked();

    // 申请处理相关
    void onRequestItemClicked(QListWidgetItem *item);
    void onAcceptRequestClicked();
    void onRejectRequestClicked();

    // 定时刷新
    void autoRefresh();

private:
    // 初始化界面
    void initUI();
    void setupNavigation();          // 设置导航区域
    void setupFriendsPage();         // 设置好友列表页面
    void setupAddFriendPage();       // 设置添加好友页面
    void setupRequestsPage();        // 设置申请管理页面

    // 数据更新
    void updateFriendsList();
    void updateSearchResults();
    void updatePendingRequests();
    void updateNavigationBadges();

    // 创建界面组件
    QWidget* createUserWidget(const UserInfo &userInfo);

    // 工具方法
    void showMessage(const QString &message, bool isError = false);
    UserInfo::RelationshipStatus checkUserRelationship(int userId, const QString &userType);
    void clearSearchResults(); // 清空搜索结果

private:
    // 核心数据
    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;

    // 主布局
    QHBoxLayout *m_mainLayout;

    // 左侧导航
    QWidget *m_navigationWidget;
    QVBoxLayout *m_navigationLayout;
    QPushButton *m_friendsListBtn;
    QPushButton *m_addFriendBtn;
    QPushButton *m_requestsBtn;
    QLabel *m_statisticsLabel;

    // 右侧内容区域
    QStackedWidget *m_contentStack;

    // 好友列表页面
    QWidget *m_friendsPage;
    QListWidget *m_friendsList;
    QLabel *m_friendsCountLabel;
    QPushButton *m_refreshFriendsBtn;

    // 添加好友页面
    QWidget *m_addFriendPage;
    QComboBox *m_searchMethodCombo;
    QLineEdit *m_searchLineEdit;
    QPushButton *m_searchButton;
    QScrollArea *m_searchResultsArea;
    QWidget *m_searchResultsWidget;
    QVBoxLayout *m_searchResultsLayout;
    QLabel *m_searchStatusLabel;

    // 申请管理页面
    QWidget *m_requestsPage;
    QListWidget *m_pendingRequestsList;
    QLabel *m_requestsCountLabel;
    QPushButton *m_refreshRequestsBtn;

    // 定时器
    QTimer *m_refreshTimer;
    QTimer *m_searchTimer;

    // 数据缓存
    QList<FriendInfo> m_friends;
    QList<UserInfo> m_searchResults;
    QList<FriendRequest> m_pendingRequests;

    // 统计信息
    int m_friendCount;
    int m_pendingRequestCount;

    // 当前选择
    int m_currentPage; // 0: 好友列表, 1: 添加好友, 2: 申请管理
};

#endif // FRIENDPAGE_H


