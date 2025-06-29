#ifndef QUESTIONDIALOG_H
#define QUESTIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include "database.h"
#include "question.h"

class QuestionDialog : public QDialog
{
    Q_OBJECT

public:
    // 对话框模式
    enum Mode {
        MODE_ADD,    // 新建模式
        MODE_EDIT,   // 编辑模式
        MODE_VIEW    // 查看模式（只读）
    };

    // 构造函数
    explicit QuestionDialog(Mode mode, int teacherId, QWidget *parent = nullptr);
    QuestionDialog(Mode mode, int teacherId, const Question &question, QWidget *parent = nullptr);

    // 获取题目数据（用于新建和编辑模式）
    Question getQuestion() const;

private slots:
    void onOkClicked();      // 确定按钮点击
    void onCancelClicked();  // 取消按钮点击

private:
    // 初始化方法
    void setupUI();          // 设置界面
    void setupFormLayout();  // 设置表单布局
    void setupButtonLayout(); // 设置按钮布局
    void loadQuestionData(); // 加载题目数据到界面
    void updateWindowTitle(); // 更新窗口标题

    // 验证和保存
    bool validateInput();    // 验证输入数据
    void showError(const QString &message); // 显示错误信息

    // 成员变量
    Mode m_mode;             // 对话框模式
    int m_teacherId;         // 教师ID
    Question m_question;     // 题目对象
    Database *m_database;    // 数据库操作对象

    // 界面控件
    QComboBox *m_typeCombo;     // 题型选择下拉框
    QComboBox *m_courseCombo;   // 课程选择下拉框
    QSpinBox *m_scoreSpinBox;   // 分值输入框
    QTextEdit *m_contentEdit;   // 题目内容输入框
    QTextEdit *m_answerEdit;    // 答案输入框
    QCheckBox *m_publicCheckBox; // 是否公开复选框

    QPushButton *m_okButton;     // 确定按钮
    QPushButton *m_cancelButton; // 取消按钮
};

#endif // QUESTIONDIALOG_H
