#ifndef PERMISSIONDIALOG_H
#define PERMISSIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QListWidgetItem>
#include "database.h"
#include "teacher.h"

class PermissionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PermissionDialog(int examId, int currentTeacherId, Database* database, QWidget *parent = nullptr);

private slots:
    void onAddPermissionClicked();
    void onRemovePermissionClicked();
    void onCurrentTeacherChanged();
    void onAvailableTeacherChanged();

private:
    void setupUI();
    void loadTeachersWithPermission();
    void loadAvailableTeachers();
    void updateButtonStates();
    Teacher getSelectedCurrentTeacher();
    Teacher getSelectedAvailableTeacher();

private:
    int examId;
    int currentTeacherId;
    Database* database;

    // UI组件
    QVBoxLayout* mainLayout;
    QHBoxLayout* contentLayout;
    QHBoxLayout* buttonLayout;

    // 已有权限区域
    QGroupBox* currentPermissionGroup;
    QVBoxLayout* currentLayout;
    QListWidget* currentTeacherList;
    QLabel* currentCountLabel;

    // 可添加教师区域
    QGroupBox* availableTeacherGroup;
    QVBoxLayout* availableLayout;
    QComboBox* collegeFilterCombo;
    QListWidget* availableTeacherList;
    QLabel* availableCountLabel;

    // 操作按钮
    QPushButton* addPermissionBtn;
    QPushButton* removePermissionBtn;
    QPushButton* closeBtn;

    // 数据
    QList<Teacher> currentTeachers;
    QList<Teacher> availableTeachers;
};

#endif // PERMISSIONDIALOG_H
