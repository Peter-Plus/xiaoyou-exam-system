#ifndef GROUPMANAGEWIDGET_H
#define GROUPMANAGEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include "../../core/database.h"

class GroupManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GroupManageWidget(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);

    void refreshData();

public slots:
    void onCreatedGroupClicked();
    void onJoinedGroupClicked();
    void onRequestsClicked();
    void onAcceptRequestClicked();
    void onRejectRequestClicked();

signals:
    void groupSelected(int groupId, bool isCreator);  // 新增：群聊被选中
    void noGroupSelected();                           // 新增：取消选中
    void groupRequestProcessed();

private:
    void setupUI();
    void setupStyles();
    void loadCreatedGroups();
    void loadJoinedGroups();
    void loadPendingRequests();
    void updateStatistics();
    void clearSelection();  // 新增：清除所有选择
    QString findGroupName(int groupId);

    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;

    // UI组件 - 移除分割器和右侧区域
    QVBoxLayout *m_mainLayout;

    // 我创建的群聊
    QGroupBox *m_createdGroupsBox;
    QListWidget *m_createdGroupsList;
    // 移除：QPushButton *m_manageCreatedButton;

    // 我加入的群聊
    QGroupBox *m_joinedGroupsBox;
    QListWidget *m_joinedGroupsList;
    // 移除：QPushButton *m_manageJoinedButton;

    // 待处理申请
    QGroupBox *m_requestsBox;
    QListWidget *m_requestsList;
    QHBoxLayout *m_requestButtonsLayout;
    QPushButton *m_acceptButton;
    QPushButton *m_rejectButton;

    QLabel *m_statisticsLabel;

    // 状态
    int m_selectedGroupId;
    int m_selectedRequestId;
    QList<QVariantMap> m_createdGroups;
    QList<QVariantMap> m_joinedGroups;
    QList<QVariantMap> m_pendingRequests;
};

#endif // GROUPMANAGEWIDGET_H
