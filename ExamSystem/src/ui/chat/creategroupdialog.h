#ifndef CREATEGROUPDIALOG_H
#define CREATEGROUPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QListWidget>
#include <QCheckBox>
#include <QMessageBox>
#include "../../core/database.h"

class CreateGroupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateGroupDialog(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onGroupNameChanged();
    void onInviteFriendsClicked();
    void updateButtonState();

private:
    void setupUI();
    void setupStyles();
    void loadFriendsList();
    bool validateInput();

    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;

    // UI组件
    QVBoxLayout *m_mainLayout;
    QFormLayout *m_formLayout;

    QLineEdit *m_groupNameEdit;
    QTextEdit *m_descriptionEdit;

    QGroupBox *m_friendsGroupBox;
    QListWidget *m_friendsListWidget;
    QPushButton *m_selectAllButton;
    QPushButton *m_selectNoneButton;

    QHBoxLayout *m_buttonLayout;
    QPushButton *m_createButton;
    QPushButton *m_cancelButton;

    QLabel *m_hintLabel;

    QList<QVariantMap> m_friendsList;
    QList<int> m_selectedFriends;
};

#endif // CREATEGROUPDIALOG_H
