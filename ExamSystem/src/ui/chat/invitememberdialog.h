// invitememberdialog.h
#ifndef INVITEMEMBERDIALOG_H
#define INVITEMEMBERDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QTimer>
#include <QMessageBox>
#include "../../core/database.h"
#include "../../models/userinfo.h"

class InviteMemberDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InviteMemberDialog(Database *database, int groupId, const QString &groupName,
                                int userId, const QString &userType, QWidget *parent = nullptr);

private slots:
    void onSearchTextChanged();
    void onSearchButtonClicked();
    void onUserItemClicked();
    void onInviteButtonClicked();
    void onCloseButtonClicked();
    void performSearch();

private:
    void setupUI();
    void setupStyles();
    void loadFriendsList();
    void addUserToList(const UserInfo &user);
    void clearSearchResults();
    bool isUserAlreadyMember(int userId, const QString &userType);

    Database *m_database;
    int m_groupId;
    QString m_groupName;
    int m_currentUserId;
    QString m_currentUserType;

    // 当前群聊成员ID列表（用于过滤）
    QList<QPair<int, QString>> m_currentMembers;

    // UI组件
    QVBoxLayout *m_mainLayout;

    // 搜索区域
    QGroupBox *m_searchBox;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;

    // 好友列表区域
    QGroupBox *m_friendsBox;
    QListWidget *m_friendsList;

    // 搜索结果区域
    QGroupBox *m_resultsBox;
    QListWidget *m_resultsList;

    // 按钮区域
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_inviteButton;
    QPushButton *m_closeButton;

    // 状态
    int m_selectedUserId;
    QString m_selectedUserType;
    QString m_selectedUserName;

    // 搜索防抖定时器
    QTimer *m_searchTimer;
};

#endif // INVITEMEMBERDIALOG_H
