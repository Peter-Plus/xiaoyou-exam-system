#ifndef FRIENDMANAGER_H
#define FRIENDMANAGER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QTimer>
#include <QMenu>
#include <QListWidgetItem>

// 引入数据模型
#include "../../models/friendinfo.h"
#include "../../models/friendrequest.h"
#include "../../models/userinfo.h"
#include "../../core/database.h"

class FriendManager : public QWidget
{
    Q_OBJECT

public:
    explicit FriendManager(Database *database, int currentUserId, const QString &currentUserType, QWidget *parent = nullptr);
    ~FriendManager();

public slots:
    void refreshData();              // 刷新所有数据
    void refreshFriendsList();       // 刷新好友列表
    void refreshRequests();          // 刷新申请列表
    void onTabChanged(int index);    // 标签页切换

private slots:
    // 好友列表相关
    void onFriendItemClicked(QListWidgetItem *item);
    void onFriendContextMenu(const QPoint &pos);
    void removeFriend();

    // 搜索功能相关
    void onSearchTextChanged();
    void onSearchMethodChanged();
    void onSearchButtonClicked();
    void onAddFriendClicked();

    // 申请处理相关
    void onReceivedRequestClicked(QListWidgetItem *item);
    void onSentRequestClicked(QListWidgetItem *item);
    void acceptRequest();
    void rejectRequest();

    // 班级成员相关
    void onClassmateClicked(QListWidgetItem *item);
    void onAddClassmateClicked();

private:
    // 初始化界面
    void initUI();
    void setupFriendsTab();          // 设置好友列表标签页
    void setupAddFriendTab();        // 设置添加好友标签页
    void setupRequestsTab();         // 设置申请管理标签页
    void setupClassmatesTab();       // 设置班级成员标签页

    // 界面更新
    void updateFriendsList();
    void updateSearchResults();
    void updateReceivedRequests();
    void updateSentRequests();
    void updateClassmates();
    void updateStatistics();

    // 创建列表项
    QListWidgetItem* createFriendItem(const FriendInfo &friendInfo);
    QListWidgetItem* createUserItem(const UserInfo &userInfo);
    QListWidgetItem* createRequestItem(const FriendRequest &request, bool isReceived);

    // 工具方法
    void showMessage(const QString &title, const QString &message);
    void showError(const QString &message);
    UserInfo::RelationshipStatus getUserRelationshipStatus(int userId, const QString &userType);

    // 获取选中项的用户信息
    QVariantMap getSelectedFriend();
    QVariantMap getSelectedUser();
    QVariantMap getSelectedRequest();

private:
    // 核心数据
    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;

    // 主界面布局
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;

    // 好友列表标签页
    QWidget *m_friendsTab;
    QListWidget *m_friendsList;
    QLabel *m_friendsCountLabel;
    QPushButton *m_refreshFriendsBtn;

    // 添加好友标签页
    QWidget *m_addFriendTab;
    QComboBox *m_searchMethodCombo;
    QLineEdit *m_searchLineEdit;
    QPushButton *m_searchButton;
    QListWidget *m_searchResultsList;
    QLabel *m_searchStatusLabel;

    // 申请管理标签页
    QWidget *m_requestsTab;
    QTabWidget *m_requestsTabWidget;
    QListWidget *m_receivedRequestsList;
    QListWidget *m_sentRequestsList;
    QLabel *m_receivedCountLabel;
    QLabel *m_sentCountLabel;

    // 班级成员标签页（仅学生可见）
    QWidget *m_classmatesTab;
    QListWidget *m_classmatesList;
    QLabel *m_classmatesCountLabel;
    QPushButton *m_refreshClassmatesBtn;

    // 定时刷新
    QTimer *m_refreshTimer;

    // 数据缓存
    QList<FriendInfo> m_friends;
    QList<UserInfo> m_searchResults;
    QList<FriendRequest> m_receivedRequests;
    QList<FriendRequest> m_sentRequests;
    QList<UserInfo> m_classmates;

    // 统计信息
    int m_friendCount;
    int m_pendingRequestCount;
};

#endif // FRIENDMANAGER_H
