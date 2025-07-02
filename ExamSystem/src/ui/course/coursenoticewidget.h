#ifndef COURSENOTICEWIDGET_H
#define COURSENOTICEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QDialog>

#include "../../core/database.h"

// 前向声明，避免循环包含
class NoticeDetailWidget;
class PublishNoticeDialog;
class CourseNotice;

class CourseNoticeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CourseNoticeWidget(Database *database, int userId, const QString &userType, QWidget *parent = nullptr);

    void refreshData();

public slots:
    void onNoticeSelected(QListWidgetItem *item);
    void onPublishNotice();          // 发布通知（教师）
    void onEditNotice();             // 编辑通知（教师）
    void onDeleteNotice();           // 删除通知（教师）
    void onRefreshClicked();

signals:
    void noticePublished(int courseId);
    void noticeUpdated(int noticeId);
    void noticeDeleted(int noticeId);

private slots:
    void onCourseFilterChanged();
    void onSearchTextChanged();
    void onNoticeDoubleClicked(QListWidgetItem *item);

private:
    void setupUI();
    void setupStyles();
    void setupTeacherUI();          // 教师端界面
    void setupStudentUI();          // 学生端界面
    void updateNoticeList();
    void updateCourseFilter();
    void createNoticeListItem(const QVariantMap &notice);
    void showNoticeDetail(const QVariantMap &notice);
    void showMessage(const QString &message, bool isError = false);
    QList<QVariantMap> getFilteredNotices();

    // 成员变量
    Database *m_database;
    int m_userId;
    QString m_userType;
    bool m_isTeacher;
    QTimer *m_searchTimer;

    // UI组件 - 主布局
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 左侧通知列表区域
    QWidget *m_leftWidget;
    QVBoxLayout *m_leftLayout;

    // 筛选区域
    QGroupBox *m_filterGroup;
    QHBoxLayout *m_filterLayout;
    QComboBox *m_courseFilterCombo;
    QLineEdit *m_searchLineEdit;
    QPushButton *m_searchButton;
    QPushButton *m_refreshButton;

    // 通知列表
    QGroupBox *m_listGroup;
    QListWidget *m_noticeList;
    QLabel *m_noticeCountLabel;

    // 操作按钮（教师端）
    QGroupBox *m_actionGroup;
    QHBoxLayout *m_actionLayout;
    QPushButton *m_publishButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;

    // 右侧通知详情区域
    NoticeDetailWidget *m_noticeDetailWidget;

    // 数据缓存
    QList<QVariantMap> m_notices;
    QList<QVariantMap> m_teacherCourses;  // 教师课程列表
    QList<QVariantMap> m_studentCourses;  // 学生课程列表
    int m_selectedNoticeId;

    // 对话框
    PublishNoticeDialog *m_publishDialog;
};

// ============================================================================
// NoticeDetailWidget - 通知详情组件
// ============================================================================

class NoticeDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoticeDetailWidget(QWidget *parent = nullptr);

    void showNotice(const QVariantMap &notice);
    void clearContent();

private:
    void setupUI();
    void setupStyles();

    // UI组件
    QVBoxLayout *m_mainLayout;

    // 头部信息
    QWidget *m_headerWidget;
    QVBoxLayout *m_headerLayout;
    QLabel *m_titleLabel;
    QLabel *m_infoLabel;        // 课程、时间、置顶状态

    // 内容区域
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;
    QTextEdit *m_contentTextEdit;

    // 欢迎页面
    QWidget *m_welcomeWidget;
    QLabel *m_welcomeLabel;
};

// ============================================================================
// PublishNoticeDialog - 发布通知对话框
// ============================================================================

class PublishNoticeDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode { PUBLISH, EDIT };

    explicit PublishNoticeDialog(Database *database, int teacherId, Mode mode = PUBLISH, QWidget *parent = nullptr);

    void setNoticeData(const QVariantMap &notice);  // 编辑模式时设置数据
    QVariantMap getNoticeData() const;

public slots:
    void onPublishClicked();
    void onCancelClicked();
    void onPreviewClicked();

private:
    void setupUI();
    void setupStyles();
    void updateCourseCombo();
    bool validateInput();

    // 成员变量
    Database *m_database;
    int m_teacherId;
    Mode m_mode;
    int m_noticeId;  // 编辑模式时的通知ID

    // UI组件
    QVBoxLayout *m_mainLayout;

    // 基本信息
    QGroupBox *m_basicGroup;
    QComboBox *m_courseCombo;
    QLineEdit *m_titleLineEdit;
    QCheckBox *m_pinnedCheckBox;

    // 内容区域
    QGroupBox *m_contentGroup;
    QTextEdit *m_contentTextEdit;

    // 操作按钮
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_publishButton;
    QPushButton *m_previewButton;
    QPushButton *m_cancelButton;

    // 数据
    QList<QVariantMap> m_teacherCourses;
};

#endif // COURSENOTICEWIDGET_H
