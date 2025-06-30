#ifndef GROUPDETAILWIDGET_H
#define GROUPDETAILWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include "../../core/database.h"
#include "../../models/groupinfo.h"
#include "../../models/groupmember.h"
#include "invitememberdialog.h"

class GroupDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GroupDetailWidget(Database *database, int userId,
                               const QString &userType, QWidget *parent = nullptr);

    void showGroupDetail(int groupId, bool isCreator);
    void clearDetail();

public slots:
    void onInviteMember();     // 邀请成员
    void onDisbandGroup();     // 解散群聊
    void onLeaveGroup();       // 退出群聊
    void refreshGroupInfo();   // 刷新群聊信息

signals:
    void groupDisbanded(int groupId);
    void groupLeft(int groupId);
    void memberInvited(int groupId);

private:
    void setupUI();
    void setupStyles();
    void loadGroupMembers();
    void updateOperationButtons(bool isCreator);
    void showEmptyState();

private:
    Database *m_database;
    int m_userId;
    QString m_userType;
    int m_currentGroupId;
    bool m_isCreator;

    // UI组件
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;

    // 群聊基本信息
    QGroupBox *m_basicInfoBox;
    QLabel *m_groupNameLabel;
    QLabel *m_creatorLabel;
    QLabel *m_memberCountLabel;
    QLabel *m_createTimeLabel;

    // 成员列表
    QGroupBox *m_membersBox;
    QListWidget *m_memberList;

    // 操作按钮
    QGroupBox *m_operationsBox;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_inviteButton;
    QPushButton *m_disbandButton;
    QPushButton *m_leaveButton;

    // 空状态
    QLabel *m_emptyLabel;
};

#endif // GROUPDETAILWIDGET_H
