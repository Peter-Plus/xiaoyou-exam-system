#ifndef CLASSSTATISTICSWINDOW_H
#define CLASSSTATISTICSWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFrame>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>


#include "database.h"
#include "classstatistics.h"
#include "exam.h"


class ClassStatisticsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ClassStatisticsWindow(Database *db, int teacherId, QWidget *parent = nullptr);
    ~ClassStatisticsWindow();

private slots:
    void onQueryStatistics();      // 查询统计
    void onExportExcel();          // 导出Excel
    void onRefresh();              // 刷新数据
    void onExamSelectionChanged(); // 考试选择变化
    void exportMultipleExams();

private:
    void setupUI();                // 初始化界面
    void loadExams();              // 加载考试列表
    void displayStatistics(const ClassStatistics &stats); // 显示统计信息
    void displayFailingStudents(const QList<StudentScore> &students); // 显示不及格学生
    void displayAllStudents(const QList<QMap<QString, QVariant>> &students); // 显示完整成绩
    void clearDisplay();           // 清空显示
    QFrame* createStatCard(const QString &title, const QString &value,
                           const QString &subtitle = ""); // 创建统计卡片
    void exportToCSV(const QString &filename); // 导出CSV文件
    void generateExcelReport(const QString &filename); // 生成Excel格式报告
    void formatExcelCell(QTextStream &out, const QString &text, bool isHeader = false); // 格式化单元格

    // 界面组件
    QVBoxLayout *mainLayout;
    QHBoxLayout *topLayout;
    QComboBox *examComboBox;
    QPushButton *queryButton;
    QPushButton *exportButton;
    QPushButton *refreshButton;

    // 统计卡片区域
    QHBoxLayout *cardLayout;
    QFrame *basicStatsCard;
    QFrame *scoreStatsCard;
    QFrame *passingStatsCard;

    // 不及格学生列表
    QGroupBox *failingGroup;
    QTableWidget *failingTable;

    // 完整成绩列表
    QGroupBox *allStudentsGroup;
    QTableWidget *allStudentsTable;

    // 数据
    Database *database;
    int currentTeacherId;
    ClassStatistics currentStats;
    QList<Exam> availableExams;
};

#endif // CLASSSTATISTICSWINDOW_H
