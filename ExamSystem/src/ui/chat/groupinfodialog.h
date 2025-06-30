#ifndef GROUPINFODIALOG_H
#define GROUPINFODIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QListWidget>
#include <QTabWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QScrollArea>
#include "../../core/database.h"
#include "../../models/groupinfo.h"

class GroupInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GroupInfoDialog(Database *database, int groupId, int userId, const QString &userType, QWidget *parent = nullptr);

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onDeleteGroupClicked();
    void onInviteMemberClicked();
    void onRemoveMemberClicked();
    void onMemberItemClicked();
    void onGroupNameChanged();
    void onRefreshMembersClicked();

private:
    void setupUI();
    void setupStyles();
    void loadGroupInfo();
    void loadMembers();
    void updateButtonStates();
    bool validateInput();
    bool hasPermissionToEdit();

    Database *m_database;
    int m_groupId;
    int m_currentUserId;
    QString m_currentUserType;
    bool m_isCreator;

    // 群聊信息
    GroupInfo m_groupInfo;
    QList<QVariantMap> m_members;
    int m_selectedMemberId;
    QString m_selectedMemberType;

    // UI组件
    QTabWidget *m_tabWidget;

    // 基本信息标签页
    QWidget *m_basicInfoTab;
    QLineEdit *m_groupNameEdit;
    QTextEdit *m_descriptionEdit;
    QLabel *m_groupIdLabel;
    QLabel *m_creatorLabel;
    QLabel *m_createTimeLabel;
    QLabel *m_memberCountLabel;

    // 成员管理标签页
    QWidget *m_membersTab;
    QListWidget *m_membersListWidget;
    QPushButton *m_inviteMemberButton;
    QPushButton *m_removeMemberButton;
    QPushButton *m_refreshMembersButton;
    QLabel *m_memberDetailLabel;

    // 群聊设置标签页
    QWidget *m_settingsTab;
    QCheckBox *m_allowMemberInviteCheck;
    QCheckBox *m_requireApprovalCheck;
    QSpinBox *m_maxMembersSpin;
    QComboBox *m_groupTypeCombo;

    // 底部按钮
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_saveButton;
    QPushButton *m_deleteButton;
    QPushButton *m_cancelButton;
};

#endif // GROUPINFODIALOG_H
