#ifndef JOINGROUPDIALOG_H
#define JOINGROUPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QTimer>
#include "../../core/database.h"
#include "../../models/groupinfo.h"

class JoinGroupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JoinGroupDialog(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);

private slots:
    void onSearchClicked();
    void onJoinClicked();
    void onCancelClicked();
    void onSearchTextChanged();
    void onGroupItemClicked();
    void onDelayedSearch();

private:
    void setupUI();
    void setupStyles();
    void searchGroups();
    void clearSearchResults();
    void createGroupSearchItem(const QVariantMap &group);
    QString getGroupStatusText(int groupId);

    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;

    // UI组件
    QVBoxLayout *m_mainLayout;

    QGroupBox *m_searchGroupBox;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QLabel *m_searchHintLabel;

    QGroupBox *m_resultsGroupBox;
    QListWidget *m_resultsListWidget;
    QLabel *m_statusLabel;

    QHBoxLayout *m_buttonLayout;
    QPushButton *m_joinButton;
    QPushButton *m_cancelButton;

    // 状态
    QTimer *m_searchTimer;
    int m_selectedGroupId;
    QString m_selectedGroupName;
    QList<QVariantMap> m_searchResults;
};

#endif // JOINGROUPDIALOG_H
