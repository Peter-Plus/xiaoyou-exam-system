#ifndef ENROLLMENTWIDGET_H
#define ENROLLMENTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTableWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QHeaderView>
#include <QTimer>
#include <QMessageBox>
#include "../core/database.h"
#include "../models/enrollmentrequest.h"

class EnrollmentWidget : public QWidget
{
    Q_OBJECT

public:
    enum UserType {
        STUDENT = 0,
        TEACHER = 1
    };

    EnrollmentWidget(Database *database, int userId, UserType userType, QWidget *parent = nullptr);
    ~EnrollmentWidget();

public slots:
    void refreshData();
    void onSearchTextChanged();

signals:
    void enrollmentSubmitted(int courseId);
    void requestProcessed(int studentId, int courseId, bool approved);

private slots:
    void onCourseSelectionChanged();
    void onRequestSelectionChanged();
    void onSubmitApplication();
    void onApproveRequest();
    void onRejectRequest();
    void onBatchApprove();
    void onBatchReject();
    void autoRefresh();

private:
    void setupUI();
    void setupStudentUI();
    void setupTeacherUI();
    void loadAvailableCourses();
    void loadEnrollmentRequests();
    void loadMyApplications();
    void updateCourseTable();
    void updateRequestTable();
    void updateApplicationTable();
    void updateStatistics();
    void showCourseDetails(int courseId);
    void showRequestDetails(const EnrollmentRequest &request);
    bool validateEnrollment(int courseId);
    void processSelectedRequests(bool approve);

    // UI组件
    QVBoxLayout *m_mainLayout;
    QSplitter *m_splitter;

    // 学生端UI
    QWidget *m_studentWidget;
    QVBoxLayout *m_studentLayout;
    QHBoxLayout *m_searchLayout;
    QLineEdit *m_searchEdit;
    QComboBox *m_collegeFilter;
    QTableWidget *m_courseTable;
    QGroupBox *m_courseDetailGroup;
    QLabel *m_courseDetailLabel;
    QPushButton *m_submitButton;

    // 学生申请状态
    QGroupBox *m_applicationGroup;
    QTableWidget *m_applicationTable;

    // 教师端UI
    QWidget *m_teacherWidget;
    QVBoxLayout *m_teacherLayout;
    QHBoxLayout *m_filterLayout;
    QComboBox *m_statusFilter;
    QComboBox *m_courseFilter;
    QTableWidget *m_requestTable;
    QGroupBox *m_requestDetailGroup;
    QTextEdit *m_requestDetailText;
    QHBoxLayout *m_actionLayout;
    QPushButton *m_approveButton;
    QPushButton *m_rejectButton;
    QPushButton *m_batchApproveButton;
    QPushButton *m_batchRejectButton;

    // 统计信息
    QGroupBox *m_statsGroup;
    QLabel *m_statsLabel;
    QPushButton *m_refreshButton;

    // 数据
    Database *m_database;
    int m_userId;
    UserType m_userType;
    QTimer *m_refreshTimer;

    // 数据缓存
    QList<QVariantMap> m_availableCourses;
    QList<EnrollmentRequest> m_enrollmentRequests;
    QList<QVariantMap> m_myApplications;

    // 当前选择
    int m_selectedCourseId;
    EnrollmentRequest m_selectedRequest;

    // 统计数据
    int m_totalCourses;
    int m_enrolledCourses;
    int m_pendingApplications;
    int m_totalRequests;
    int m_approvedRequests;
};

#endif // ENROLLMENTWIDGET_H
