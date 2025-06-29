#ifndef EXAMSTATUSMANAGER_H
#define EXAMSTATUSMANAGER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include "database.h"
#include "exam.h"

class ExamStatusManager : public QObject
{
    Q_OBJECT

public:
    explicit ExamStatusManager(Database* database, QObject *parent = nullptr);

    // 启动和停止状态检查
    void startStatusCheck();
    void stopStatusCheck();

    // 设置检查间隔（秒）
    void setCheckInterval(int seconds);

    // 手动触发检查
    void checkAllExams();

public slots:
    void checkExamStatus();      // 定期检查考试状态
    void autoPublishExams();     // 自动发布到时间的考试
    void autoTerminateExams();   // 自动结束超时的考试

signals:
    void examStatusChanged(int examId, const QString &oldStatus, const QString &newStatus);
    void examAutoPublished(int examId, const QString &examName);
    void examAutoTerminated(int examId, const QString &examName);
    void statusCheckCompleted(int totalExams, int changedExams);

private:
    void processExam(const Exam& exam);
    QString calculateExamStatus(const Exam& exam, const QDateTime& currentTime);
    bool shouldAutoPublish(const Exam& exam, const QDateTime& currentTime);
    bool shouldAutoTerminate(const Exam& exam, const QDateTime& currentTime);

private:
    Database* database;
    QTimer* statusCheckTimer;
    int checkInterval; // 检查间隔（秒）
    bool isRunning;

    // 统计信息
    int lastTotalExams;
    int lastChangedExams;
};

#endif // EXAMSTATUSMANAGER_H
