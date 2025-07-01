#ifndef NOTICEWIDGET_H
#define NOTICEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QHeaderView>
#include <QTimer>
#include <QMessageBox>
#include <QDialog>
#include "../core/database.h"
#include "../models/coursenotice.h"

class NoticeEditDialog;

class NoticeWidget : public QWidget
{
    Q_OBJECT

public:
    enum UserType {
        STUDENT = 0,
        TEACHER = 1
    };

    NoticeWidget(Database *database, int userId, UserType userType, int courseId = -1, QWidget *parent = nullptr);
    ~NoticeWidget();

public slots:
    void refreshData();
    void setCourseId(int courseId);

signals:
    void noticePublished(int courseId, const QString &title);
    void noticeUpdated(int noticeId, const QString &title);
    void noticeDeleted(int noticeId);

private slots:
    void onNoticeSelectionChanged();
    void onCourseSelectionChanged();
    void onPublishNotice();
    void onEditNotice();
    void onDeleteNotice();
    void onTogglePin();
    void onSearchTextChanged();
    void autoRefresh();

private:
    void setupUI();
    void setupStudentUI();
    void setupTeacherUI();
    void loadNotices();
    void loadCourseList();
    void updateNoticeTable();
    void updateStatistics();
    void showNoticeContent(const CourseNotice &notice);
    void clearNoticeContent();
    bool validateNoticePermissions(int courseId);

    // UI组件
    QVBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧通知列表
    QWidget *m_listWidget;
    QVBoxLayout *m_listLayout;
    QHBoxLayout *m_controlLayout;
    QComboBox *m_courseCombo;          // 课程选择（教师端）
    QLineEdit *m_searchEdit;           // 搜索框
    QPushButton *m_publishButton;      // 发布按钮（教师端）
    QTableWidget *m_noticeTable;       // 通知表格

    // 右侧通知内容
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    QGroupBox *m_noticeGroup;
    QLabel *m_titleLabel;              // 通知标题
    QLabel *m_infoLabel;               // 通知信息（课程、时间等）
    QTextEdit *m_contentText;          // 通知内容
    QHBoxLayout *m_actionLayout;
    QPushButton *m_editButton;         // 编辑按钮（教师端）
    QPushButton *m_deleteButton;       // 删除按钮（教师端）
    QPushButton *m_pinButton;          // 置顶按钮（教师端）

    // 统计信息
    QGroupBox *m_statsGroup;
    QLabel *m_statsLabel;
    QPushButton *m_refreshButton;

    // 数据
    Database *m_database;
    int m_userId;
    UserType m_userType;
    int m_currentCourseId;
    QTimer *m_refreshTimer;

    // 数据缓存
    QList<CourseNotice> m_notices;
    QList<QVariantMap> m_courseList;
    CourseNotice m_selectedNotice;

    // 统计数据
    int m_totalNotices;
    int m_pinnedNotices;
    int m_recentNotices;  // 7天内的通知
};

// 通知编辑对话框
class NoticeEditDialog : public QDialog
{
    Q_OBJECT

public:
    NoticeEditDialog(const CourseNotice &notice, bool isEdit = true, QWidget *parent = nullptr);

    CourseNotice getNotice() const;

private slots:
    void onAccept();
    void onCancel();

private:
    void setupUI();
    void loadNoticeData();
    bool validateInput();

    // UI组件
    QVBoxLayout *m_mainLayout;
    QLineEdit *m_titleEdit;
    QTextEdit *m_contentEdit;
    QCheckBox *m_pinnedCheck;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    // 数据
    CourseNotice m_notice;
    bool m_isEdit;
};

#endif // NOTICEWIDGET_H
