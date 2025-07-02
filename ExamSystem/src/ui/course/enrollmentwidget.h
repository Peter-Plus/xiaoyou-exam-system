#ifndef ENROLLMENTWIDGET_H
#define ENROLLMENTWIDGET_H

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
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>

#include "../../core/database.h"

// 前向声明，避免循环包含
class EnrollmentRequest;

class EnrollmentWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EnrollmentWidget(Database *database, int studentId, QWidget *parent = nullptr);

    void refreshData();

public slots:
    void showAvailableCourses();    // 显示可选课程
    void showMyCourses();           // 显示我的选课
    void onApplyForCourse();        // 申请选课
    void onCourseSearch();          // 搜索课程

signals:
    void enrollmentSubmitted(int courseId);

private slots:
    void onTabChanged(int index);
    void onCourseDoubleClicked(QListWidgetItem *item);
    void onSearchTextChanged();

private:
    void setupUI();
    void setupStyles();
    void setupAvailableCoursesTab();
    void setupMyCoursesTab();
    void updateAvailableCourses();
    void updateMyCourses();
    void createCourseListItem(const QVariantMap &course, QListWidget *listWidget, bool isEnrolled = false);
    void showMessage(const QString &message, bool isError = false);

    // 成员变量
    Database *m_database;
    int m_studentId;

    // UI组件
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;

    // 可选课程标签页
    QWidget *m_availableCoursesTab;
    QVBoxLayout *m_availableLayout;
    QHBoxLayout *m_searchLayout;
    QLineEdit *m_searchLineEdit;
    QPushButton *m_searchButton;
    QComboBox *m_collegeFilterCombo;
    QListWidget *m_availableCoursesList;
    QPushButton *m_applyButton;
    QLabel *m_availableCountLabel;

    // 我的选课标签页
    QWidget *m_myCoursesTab;
    QVBoxLayout *m_myCoursesLayout;
    QListWidget *m_myCoursesList;
    QLabel *m_myCoursesCountLabel;
    QPushButton *m_refreshButton;

    // 数据缓存
    QList<QVariantMap> m_availableCourses;
    QList<QVariantMap> m_myCourses;
    int m_selectedCourseId;
};

#endif // ENROLLMENTWIDGET_H
