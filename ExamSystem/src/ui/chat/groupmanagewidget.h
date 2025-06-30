#ifndef GROUPMANAGEWIDGET_H
#define GROUPMANAGEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
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
    void onGroupItemClicked();
    void onRequestItemClicked();
    void onAcceptRequestClicked();
    void onRejectRequestClicked();
    void onManageGroupClicked();

signals:
    void groupRequestProcessed();

private:
    void setupUI();
    void setupStyles();
    void loadCreatedGroups();
    void loadJoinedGroups();
    void loadPendingRequests();
    void updateStatistics();
    void showGroupDetails(int groupId);
    void clearGroupDetails();
    QString findGroupName(int groupId);  // 添加这个方法声明

    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;

    // UI组件
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧管理区域
    QWidget *m_leftWidget;
    QVBoxLayout *m_leftLayout;

    QGroupBox *m_createdGroupsBox;
    QListWidget *m_createdGroupsList;
    QPushButton *m_manageCreatedButton;

    QGroupBox *m_joinedGroupsBox;
    QListWidget *m_joinedGroupsList;
    QPushButton *m_manageJoinedButton;

    QGroupBox *m_requestsBox;
    QListWidget *m_requestsList;
    QHBoxLayout *m_requestButtonsLayout;
    QPushButton *m_acceptButton;
    QPushButton *m_rejectButton;

    QLabel *m_statisticsLabel;

    // 右侧详情区域
    QWidget *m_rightWidget;
    QVBoxLayout *m_rightLayout;
    QScrollArea *m_detailsArea;
    QWidget *m_detailsContent;
    QVBoxLayout *m_detailsLayout;
    QLabel *m_detailsLabel;

    // 状态
    int m_selectedGroupId;
    int m_selectedRequestId;
    QList<QVariantMap> m_createdGroups;
    QList<QVariantMap> m_joinedGroups;
    QList<QVariantMap> m_pendingRequests;
};

#endif // GROUPMANAGEWIDGET_H
