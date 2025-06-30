#ifndef GROUPLISTWIDGET_H
#define GROUPLISTWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include "../../core/database.h"
#include "../../models/groupinfo.h"

class GroupListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit GroupListWidget(Database *db, int userId, const QString &userType, QWidget *parent = nullptr);

    void refreshGroupList();
    void updateGroupItem(int groupId);

public slots:
    void onGroupItemClicked(QListWidgetItem *item);
    void onNewGroupMessage(int groupId);

signals:
    void groupSelected(int groupId, const QString &groupName);

private:
    void createGroupItem(const GroupInfo &group);
    QString formatGroupItemText(const GroupInfo &group);
    void updateUnreadBadge(int groupId, int count);

    Database *m_database;
    int m_currentUserId;
    QString m_currentUserType;
    QList<GroupInfo> m_groupList;
};

#endif // GROUPLISTWIDGET_H
