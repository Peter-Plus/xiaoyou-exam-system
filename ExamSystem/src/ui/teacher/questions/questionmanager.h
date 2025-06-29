#ifndef QUESTIONMANAGER_H
#define QUESTIONMANAGER_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include "database.h"
#include "question.h"

class QuestionManager : public QWidget
{
    Q_OBJECT

public:
    explicit QuestionManager(int teacherId, Database* database, QWidget *parent = nullptr);

private slots:
    // 基本操作
    void onAddQuestion();       // 新建题目按钮点击
    void onEditQuestion();      // 修改题目按钮点击
    void onDeleteQuestion();    // 删除题目按钮点击
    void onViewQuestion();      // 查看题目详情按钮点击

    // 收藏操作
    void onToggleFavorite();    // 收藏/取消收藏按钮点击

    // 筛选和搜索
    void onSearchTextChanged(); // 搜索框内容变化
    void onCourseFilterChanged(); // 课程筛选下拉框变化
    void onTypeFilterChanged();   // 题型筛选下拉框变化
    void onTabChanged(int index); // 标签页切换

    // 列表操作
    void onQuestionSelectionChanged(); // 题目列表选择变化
    void onQuestionDoubleClicked();    // 题目列表双击

private:
    // 界面初始化
    void setupUI();           // 设置界面布局
    void setupTabWidget();    // 设置标签页
    void setupFilterArea();   // 设置筛选区域
    void setupButtonArea();   // 设置按钮区域
    void setupQuestionTable(); // 设置题目表格

    // 数据操作
    void refreshCurrentTab();    // 刷新当前标签页数据
    void refreshMyQuestions();   // 刷新我的题库数据
    void refreshPublicQuestions(); // 刷新公共题库数据
    void refreshFavoriteQuestions(); // 刷新收藏题库数据
    void loadQuestionsToTable(const QList<Question> &questions); // 加载题目到表格

    // 辅助方法
    void updateButtonStates();   // 更新按钮状态
    void updateFilterOptions();  // 更新筛选选项
    Question getSelectedQuestion(); // 获取当前选中的题目
    bool hasSelection();         // 检查是否有选中项
    void showMessage(const QString &message); // 显示消息
    void applyFilters(QList<Question> &questions); // 应用筛选条件
    bool matchesFilter(const Question &question, const QString &keyword,
                       const QString &courseName, const QString &questionType); // 检查题目是否匹配筛选条件

    // 成员变量
    int m_teacherId;            // 当前教师ID
    Database *m_database;       // 数据库操作对象
    QTimer *m_searchTimer;      // 搜索延迟定时器

    // 界面组件
    QTabWidget *m_tabWidget;    // 标签页控件
    QTableWidget *m_questionTable; // 题目列表表格

    // 筛选控件
    QLineEdit *m_searchEdit;    // 搜索输入框
    QComboBox *m_courseCombo;   // 课程筛选下拉框
    QComboBox *m_typeCombo;     // 题型筛选下拉框

    // 按钮控件
    QPushButton *m_addButton;      // 新建题目按钮
    QPushButton *m_editButton;     // 修改按钮
    QPushButton *m_deleteButton;   // 删除按钮
    QPushButton *m_viewButton;     // 查看详情按钮
    QPushButton *m_favoriteButton; // 收藏/取消收藏按钮

    // 常量定义
    enum TabIndex {
        TAB_MY_QUESTIONS = 0,      // 我的题库标签页索引
        TAB_PUBLIC_QUESTIONS = 1,  // 公共题库标签页索引
        TAB_FAVORITE_QUESTIONS = 2 // 收藏题库标签页索引
    };

    enum TableColumn {
        COL_ID = 0,          // 题目ID列
        COL_TYPE = 1,        // 题型列
        COL_COURSE = 2,      // 课程列
        COL_SCORE = 3,       // 分值列
        COL_CONTENT = 4,     // 题干预览列
        COL_FREQUENCY = 5,   // 考频列
        COL_CREATOR = 6,     // 创建者列
        COL_STATUS = 7       // 状态列（公开/私有）
    };
};

#endif // QUESTIONMANAGER_H
