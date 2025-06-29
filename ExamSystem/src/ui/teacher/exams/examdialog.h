#ifndef EXAMDIALOG_H
#define EXAMDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include "database.h"
#include "exam.h"
#include "course.h"

class ExamDialog : public QDialog
{
    Q_OBJECT

public:
    // 新建考试模式
    explicit ExamDialog(int teacherId, Database* database, QWidget *parent = nullptr);

    // 编辑考试模式
    ExamDialog(const Exam &exam, int teacherId, Database* database, QWidget *parent = nullptr);

    Exam getExamData() const;

private slots:
    void onAcceptClicked();
    void onRejectClicked();

private:
    void setupUI();
    void loadCourses();
    bool validateInput();
    void showValidationError(const QString &message, QWidget *focusWidget);

private:
    int teacherId;
    Database* database;
    bool isEditMode;
    int examId; // 编辑模式下的考试ID

    // UI组件
    QVBoxLayout* mainLayout;
    QFormLayout* formLayout;
    QHBoxLayout* buttonLayout;

    QLineEdit* examNameEdit;
    QComboBox* courseComboBox;
    QDateEdit* examDateEdit;
    QTimeEdit* startTimeEdit;
    QTimeEdit* endTimeEdit;
    QSpinBox* totalScoreSpinBox;

    QPushButton* okButton;
    QPushButton* cancelButton;

    // 数据
    QList<Course> courses;
};

#endif // EXAMDIALOG_H
