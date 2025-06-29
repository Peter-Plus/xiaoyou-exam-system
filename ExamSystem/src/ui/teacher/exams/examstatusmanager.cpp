#include "examstatusmanager.h"

ExamStatusManager::ExamStatusManager(Database* database, QObject *parent)
    : QObject(parent), database(database), checkInterval(30), isRunning(false),
    lastTotalExams(0), lastChangedExams(0)
{
    statusCheckTimer = new QTimer(this);
    statusCheckTimer->setSingleShot(false);

    // 连接定时器信号
    connect(statusCheckTimer, &QTimer::timeout, this, &ExamStatusManager::checkExamStatus);

    qDebug() << "ExamStatusManager 初始化完成，检查间隔:" << checkInterval << "秒";
}

void ExamStatusManager::startStatusCheck()
{
    if (!isRunning) {
        statusCheckTimer->start(checkInterval * 1000); // 转换为毫秒
        isRunning = true;
        qDebug() << "考试状态自动检查已启动";

        // 立即执行一次检查
        checkExamStatus();
    }
}

void ExamStatusManager::stopStatusCheck()
{
    if (isRunning) {
        statusCheckTimer->stop();
        isRunning = false;
        qDebug() << "考试状态自动检查已停止";
    }
}

void ExamStatusManager::setCheckInterval(int seconds)
{
    checkInterval = qMax(10, seconds); // 最小10秒间隔

    if (isRunning) {
        statusCheckTimer->setInterval(checkInterval * 1000);
    }

    qDebug() << "检查间隔已设置为:" << checkInterval << "秒";
}

void ExamStatusManager::checkAllExams()
{
    checkExamStatus();
}

void ExamStatusManager::checkExamStatus()
{
    if (!database) {
        qDebug() << "数据库连接无效，跳过状态检查";
        return;
    }

    qDebug() << "开始检查考试状态...";

    QDateTime currentTime = QDateTime::currentDateTime();
    int totalExams = 0;
    int changedExams = 0;

    // 获取所有考试（这里需要一个获取所有考试的方法）
    // 由于没有getAllExams方法，我们需要遍历所有教师的考试
    // 这里简化处理，假设有一个方法可以获取所有考试

    // 临时解决方案：检查特定范围内的考试
    QList<Exam> allExams;

    // 这里需要Database类提供getAllExams方法
    // 现在先用一个空列表，实际使用时需要实现这个方法
    // allExams = database->getAllExams();

    for (const Exam& exam : allExams) {
        totalExams++;

        QString oldStatus = exam.getStatusText();
        QString newStatus = calculateExamStatus(exam, currentTime);

        if (oldStatus != newStatus) {
            changedExams++;
            qDebug() << "考试状态变化:" << exam.getExamName()
                     << "从" << oldStatus << "变为" << newStatus;

            emit examStatusChanged(exam.getExamId(), oldStatus, newStatus);
        }

        // 处理自动发布和终止
        processExam(exam);
    }

    lastTotalExams = totalExams;
    lastChangedExams = changedExams;

    emit statusCheckCompleted(totalExams, changedExams);

    if (changedExams > 0) {
        qDebug() << "状态检查完成，共检查" << totalExams << "个考试，"
                 << changedExams << "个考试状态发生变化";
    }
}

void ExamStatusManager::autoPublishExams()
{
    if (!database) return;

    QDateTime currentTime = QDateTime::currentDateTime();

    // 这里需要获取所有未发布但应该自动发布的考试
    // 由于数据库接口限制，这里提供实现思路

    qDebug() << "检查需要自动发布的考试...";

    // 实际实现需要数据库支持
    // QList<Exam> unpublishedExams = database->getUnpublishedExams();

    // for (const Exam& exam : unpublishedExams) {
    //     if (shouldAutoPublish(exam, currentTime)) {
    //         if (database->publishExam(exam.getExamId())) {
    //             qDebug() << "自动发布考试:" << exam.getExamName();
    //             emit examAutoPublished(exam.getExamId(), exam.getExamName());
    //         }
    //     }
    // }
}

void ExamStatusManager::autoTerminateExams()
{
    if (!database) return;

    QDateTime currentTime = QDateTime::currentDateTime();

    qDebug() << "检查需要自动终止的考试...";

    // 实际实现需要数据库支持
    // QList<Exam> ongoingExams = database->getOngoingExams();

    // for (const Exam& exam : ongoingExams) {
    //     if (shouldAutoTerminate(exam, currentTime)) {
    //         if (database->terminateExam(exam.getExamId())) {
    //             qDebug() << "自动终止考试:" << exam.getExamName();
    //             emit examAutoTerminated(exam.getExamId(), exam.getExamName());
    //         }
    //     }
    // }
}

void ExamStatusManager::processExam(const Exam& exam)
{
    QDateTime currentTime = QDateTime::currentDateTime();

    // 检查是否需要自动发布
    if (shouldAutoPublish(exam, currentTime)) {
        if (database->publishExam(exam.getExamId())) {
            qDebug() << "自动发布考试:" << exam.getExamName();
            emit examAutoPublished(exam.getExamId(), exam.getExamName());
        }
    }

    // 检查是否需要自动终止
    if (shouldAutoTerminate(exam, currentTime)) {
        if (database->terminateExam(exam.getExamId())) {
            qDebug() << "自动终止考试:" << exam.getExamName();
            emit examAutoTerminated(exam.getExamId(), exam.getExamName());
        }
    }
}

QString ExamStatusManager::calculateExamStatus(const Exam& exam, const QDateTime& currentTime)
{
    QDateTime examStart = QDateTime(exam.getExamDate(), exam.getStartTime());
    QDateTime examEnd = QDateTime(exam.getExamDate(), exam.getEndTime());

    if (!exam.getIsPublished()) {
        return "未发布";
    }

    if (currentTime < examStart) {
        return "已发布";
    }

    if (currentTime >= examStart && currentTime <= examEnd) {
        return "进行中";
    }

    return "已结束";
}

bool ExamStatusManager::shouldAutoPublish(const Exam& exam, const QDateTime& currentTime)
{
    // 如果已经发布，不需要自动发布
    if (exam.getIsPublished()) {
        return false;
    }

    // 如果考试还没有完成组卷，不自动发布
    if (!exam.getIsCompleted()) {
        return false;
    }

    QDateTime examStart = QDateTime(exam.getExamDate(), exam.getStartTime());

    // 在考试开始时间自动发布
    // 这里可以设置提前发布时间，比如提前5分钟
    QDateTime autoPublishTime = examStart.addSecs(-300); // 提前5分钟

    return currentTime >= autoPublishTime;
}

bool ExamStatusManager::shouldAutoTerminate(const Exam& exam, const QDateTime& currentTime)
{
    // 只有已发布的考试才能被终止
    if (!exam.getIsPublished()) {
        return false;
    }

    QDateTime examEnd = QDateTime(exam.getExamDate(), exam.getEndTime());

    // 在考试结束时间自动终止
    return currentTime >= examEnd;
}
