#ifndef ENROLLMENTADMINWIDGET_H
#define ENROLLMENTADMINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QHeaderView>
#include <QCheckBox>

#include "../../core/database.h"
#include "../../models/enrollmentrequest.h"

class EnrollmentAdminWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EnrollmentAdminWidget(Database *database, int adminId, QWidget *parent = nullptr);

    void refreshData();

public slots:
    void onApproveEnrollment();     // 审核通过
    void onRejectEnrollment();      // 审核拒绝
    void onBatchApprove();          // 批量通过
    void onBatchReject();           // 批量拒绝
    void onFilterChanged();         // 筛选条件改变

signals:
    void enrollmentProcessed(int studentId, int courseId, bool approved);

private slots:
    void onTableSelectionChanged();
    void onTableItemChanged(QTableWidgetItem *item);
    void onRefreshClicked();

private:
    void setupUI();
    void setupStyles();
    void updateEnrollmentTable();
    void updateStatistics();
    void createTableRow(const QVariantMap &enrollment, int row);
    void showMessage(const QString &message, bool isError = false);
    QList<int> getSelectedRows();
    void selectAllItems(bool select);

    // 成员变量
    Database *m_database;
    int m_adminId;

    // UI组件
    QVBoxLayout *m_mainLayout;

    // 筛选区域
    QGroupBox *m_filterGroup;
    QHBoxLayout *m_filterLayout;
    QComboBox *m_collegeFilterCombo;
    QComboBox *m_statusFilterCombo;
    QLineEdit *m_searchLineEdit;
    QPushButton *m_searchButton;
    QPushButton *m_refreshButton;

    // 统计信息
    QGroupBox *m_statsGroup;
    QLabel *m_statsLabel;

    // 申请列表
    QGroupBox *m_tableGroup;
    QTableWidget *m_enrollmentTable;

    // 操作区域
    QGroupBox *m_actionGroup;
    QHBoxLayout *m_actionLayout;
    QCheckBox *m_selectAllCheckBox;
    QPushButton *m_approveButton;
    QPushButton *m_rejectButton;
    QPushButton *m_batchApproveButton;
    QPushButton *m_batchRejectButton;
    QLabel *m_selectedCountLabel;

    // 数据缓存
    QList<QVariantMap> m_pendingEnrollments;
    QList<int> m_selectedStudentIds;
    QList<int> m_selectedCourseIds;
};

#endif // ENROLLMENTADMINWIDGET_H
