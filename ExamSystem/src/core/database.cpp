#include "database.h"
#include "question.h"
#include "exam.h"
#include "course.h"
#include "teacher.h"
#include "studentanswer.h"
#include <algorithm>  // 用于std::reverse


Database::Database()
{
}

bool Database::connectToDatabase()
{//数据库连接
    db = QSqlDatabase::addDatabase("QODBC");
    // 使用64位ODBC驱动名称
    QStringList driverNames = {
        "MySQL ODBC 9.3 Unicode Driver",
        "MySQL ODBC 9.3 ANSI Driver"
    };

    bool connected = false;
    for (const QString &driverName : driverNames) {
        // 连接本地
        QString connectionString = QString("DRIVER={%1};"
                                           "SERVER=localhost;"
                                           "DATABASE=exam_system;"
                                           "UID=root;"
                                           "PWD=Zy202312138;"//    ====================记住改密码！！！！
                                           "PORT=3306;").arg(driverName);
        // 连接服务器
        // QString connectionString = QString("DRIVER={%1};"
        //                                    "SERVER=43.137.46.253;" //云服务器IP
        //                                    "DATABASE=exam_system;"
        //                                    "UID=xiaoyou_user;"
        //                                    "PWD=Zy202312138;"
        //                                    "PORT=3306;"
        //                                    "OPTION=3;").arg(driverName);

        db.setDatabaseName(connectionString);

        if (db.open()) {
            qDebug() << "数据库连接成功! 使用驱动:" << driverName;
            qDebug() << "连接服务器: 43.137.46.253";
            connected = true;
            break;
        } else {
            qDebug() << "尝试驱动" << driverName << "失败:" << db.lastError().text();
        }
    }

    if (!connected) {
        qDebug() << "所有驱动尝试失败，无法连接云数据库";
        return false;
    }

    return true;
}

void Database::createTables()
{//测试时用于创建测试表
    // QSqlQuery query;

    // QString createUsersTable =
    //     "CREATE TABLE IF NOT EXISTS users ("
    //     "id INT AUTO_INCREMENT PRIMARY KEY,"
    //     "username VARCHAR(50) UNIQUE NOT NULL,"
    //     "password VARCHAR(255) NOT NULL,"
    //     "role ENUM('student', 'teacher', 'admin') DEFAULT 'student',"
    //     "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
    //     ")";

    // if (!query.exec(createUsersTable)) {
    //     qDebug() << "创建用户表失败:" << query.lastError().text();
    // } else {
    //     qDebug() << "用户表创建成功";
    // }
}

void Database::closeDatabase()
{
    if (db.isOpen()) {
        db.close();
    }
}

QString Database::hashPassword(const QString& password)
{//密码哈希加密函数
    QByteArray data = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(data,QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool Database::registerStudent(int studentId,const QString& name,const QString& grade,const QString& college,const QString& password)
{//学生注册
    QSqlQuery query;
    //检查学生是否存在
    query.prepare("SELECT student_id FROM students WHERE student_id=?");
    query.addBindValue(studentId);//将参数内的值填入prepare中的？
    if(!query.exec())
    {
        qDebug()<<"查询失败："<<query.lastError().text();
        return false;
    }
    QString hashedPassword = hashPassword(password);
    if(query.next())
    {//学生已存在，激活账户，填入密码
        query.prepare("UPDATE students SET password = ? WHERE student_Id = ?");
        query.addBindValue(hashedPassword);
        query.addBindValue(studentId);
    }
    else
    {//学生不存在，插入新记录
        query.prepare("INSERT INTO students(student_id,name,grade,college,password)VALUES (?,?,?,?,?)");
        query.addBindValue(studentId);
        query.addBindValue(name);
        query.addBindValue(grade);
        query.addBindValue(college);
        query.addBindValue(hashedPassword);
    }

    if(query.exec())
    {
        qDebug()<<"学生注册成功："<<studentId;
        return true;
    }
    else
    {
        qDebug()<<"学生注册失败："<<query.lastError().text();
        return false;
    }
}

bool Database::registerTeacher(int teacherId,const QString& name,const QString& college,const QString& password)
{//教师注册
    QSqlQuery query;

    // 检查教师是否已存在
    query.prepare("SELECT teacher_id FROM teachers WHERE teacher_id = ?");
    query.addBindValue(teacherId);

    if (!query.exec()) {
        qDebug() << "查询教师失败:" << query.lastError().text();
        return false;
    }

    QString hashedPassword = hashPassword(password);
    if(query.next())
    {//教师已经存在，激活账户
        query.prepare("UPDATE teachers SET password = ? WHERE teacher_id=?");
        query.addBindValue(hashedPassword);
        query.addBindValue(teacherId);
    }
    else
    {//教师不存在，插入新记录
        query.prepare("INSERT INTO teachers (teacher_id,name,college,password) VALUES (?,?,?,?)");
        query.addBindValue(teacherId);
        query.addBindValue(name);
        query.addBindValue(college);
        query.addBindValue(hashedPassword);
    }

    if(query.exec())
    {
        qDebug()<<"教师注册成功:"<<teacherId;
        return true;
    }
    else
    {
        qDebug()<<"教师注册失败"<<query.lastError().text();
        return false;
    }

}

bool Database::authenticateStudent(int studentId,const QString& password)
{
    QSqlQuery query;
    QString hashedPassword = hashPassword(password);

    query.prepare("SELECT student_id FROM students WHERE student_id = ? AND password = ?");
    query.addBindValue(studentId);
    query.addBindValue(hashedPassword);
    if(query.exec()&&query.next())
    {
        qDebug()<<"学生登录验证成功:"<<studentId;
        return true;
    }
    qDebug()<<"学生登录验证失败："<<studentId;
    return false;
}

bool Database::authenticateTeacher(int teacherId,const QString& password)
{//教师登录验证
    QSqlQuery query;
    QString hashedPassword = hashPassword(password);
    query.prepare("SELECT teacher_id FROM teachers WHERE teacher_id=? AND password = ?");
    query.addBindValue(teacherId);
    query.addBindValue(hashedPassword);
    if(query.exec()&&query.next())
    {
        qDebug()<<"教师登录成功："<<teacherId;
        return true;
    }
    qDebug()<<"教师登录验证失败："<<teacherId;
    return false;

}

//==========试题管理方法===============================================

//添加题目
bool Database::addQuestion(const QString &type, const QString &courseName, int score,
                           const QString &content, const QString &answer,
                           int creatorId, bool isPublic)
{
    QSqlQuery query;
    query.prepare("INSERT INTO questions (question_type, course_name, score, content, answer, "
                  "creator_id, modifier_id, frequency, is_public) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, 0, ?)");
    query.addBindValue(type);
    query.addBindValue(courseName);
    query.addBindValue(score);
    query.addBindValue(content);
    query.addBindValue(answer);
    query.addBindValue(creatorId);
    query.addBindValue(creatorId);
    query.addBindValue(isPublic);

    if(query.exec())
    {
        qDebug()<<"题目添加成功！";
        return true;
    }
    else
    {
        qDebug()<<"题目添加失败："<<query.lastError().text();
        return false;
    }
}

//更新题目
bool Database::updateQuestion(int questionId, const QString &type, const QString &courseName,
                              int score, const QString &content, const QString &answer,
                              int modifierId, bool isPublic)
{
    //检查权限先
    if(!canModifyQuestion(questionId,modifierId))
    {
        qDebug()<<"无题目修改权限:"<<questionId;
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE questions SET question_type = ?, course_name = ?, score = ?, "
                  "content = ?, answer = ?, modifier_id = ?, is_public = ? "
                  "WHERE question_id = ?");
    query.addBindValue(type);
    query.addBindValue(courseName);
    query.addBindValue(score);
    query.addBindValue(content);
    query.addBindValue(answer);
    query.addBindValue(modifierId);
    query.addBindValue(isPublic);
    query.addBindValue(questionId);

    if(query.exec())
    {
        qDebug()<<"题目更新成功";
        return true;
    }
    else
    {
        qDebug()<<"题目更新失败:"<<query.lastError().text();
        return false;
    }
}

//删除权限
bool Database::deleteQuestion(int questionId,int teacherId)
{
    //检查删除权限，只有创建者可删除
    if(!canDeleteQuestion(questionId,teacherId))
    {
        qDebug()<<"无权限删除题目:"<<questionId;
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM questions WHERE question_id=?");
    query.addBindValue(questionId);

    if(query.exec())
    {
        qDebug()<<"题目删除成功";
        return true;
    }
    else
    {
        qDebug()<<"题目删除失败:"<<query.lastError().text();
        return false;
    }
}

QList<Question> Database::getQuestionsByTeacher(int teacherId)
{
    QList<Question> questions;
    QSqlQuery query;
    query.prepare("SELECT q.*, t1.name as creator_name, t2.name as modifier_name "
                  "FROM questions q "
                  "LEFT JOIN teachers t1 ON q.creator_id = t1.teacher_id "
                  "LEFT JOIN teachers t2 ON q.modifier_id = t2.teacher_id "
                  "WHERE q.creator_id = ? "
                  "ORDER BY q.question_id DESC");
    query.addBindValue(teacherId);

    if(query.exec())
    {
        while(query.next())
        {
            questions.append(createQuestionFromQuery(query));
        }
    }
    else
    {
        qDebug()<<"获取教师创建的题目失败："<<query.lastError().text();
    }
    return questions;
}

QList<Question> Database::getPublicQuestions(const QString &courseName)
{
    QList<Question> questions;
    QSqlQuery query;

    QString sql = "SELECT q.*, t1.name as creator_name, t2.name as modifier_name "
                  "FROM questions q "
                  "LEFT JOIN teachers t1 ON q.creator_id = t1.teacher_id "
                  "LEFT JOIN teachers t2 ON q.modifier_id = t2.teacher_id "
                  "WHERE q.is_public = true";

    if(!courseName.isEmpty())
    {
        sql+=" AND q.course_name = ?";
    }

    sql += " ORDER BY q.frequency DESC, q.question_id DESC";

    query.prepare(sql);
    if (!courseName.isEmpty()) {
        query.addBindValue(courseName);
    }

    if (query.exec()) {
        while (query.next()) {
            questions.append(createQuestionFromQuery(query));
        }
    } else {
        qDebug() << "获取公开题目失败:" << query.lastError().text();
    }

    return questions;
}

//获取教师收藏题目,并设置已收藏
QList<Question> Database::getFavoriteQuestions(int teacherId)
{
    QList<Question> questions;
    QSqlQuery query;

    query.prepare("SELECT q.*, t1.name as creator_name, t2.name as modifier_name "
                  "FROM questions q "
                  "INNER JOIN question_favorites qf ON q.question_id = qf.question_id "
                  "LEFT JOIN teachers t1 ON q.creator_id = t1.teacher_id "
                  "LEFT JOIN teachers t2 ON q.modifier_id = t2.teacher_id "
                  "WHERE qf.teacher_id = ? "
                  "ORDER BY q.frequency DESC, q.question_id DESC");
    query.addBindValue(teacherId);

    if (query.exec()) {
        while (query.next()) {
            Question question = createQuestionFromQuery(query);
            question.setIsFavorited(true); // 设置为已收藏
            questions.append(question);
        }
    } else {
        qDebug() << "获取收藏题目失败:" << query.lastError().text();
    }

    return questions;
}

//获取所有题目
QList<Question> Database::getAllQuestions()
{
    QList<Question> questions;
    QSqlQuery query;

    query.prepare("SELECT q.*, t1.name as creator_name, t2.name as modifier_name "
                  "FROM questions q "
                  "LEFT JOIN teachers t1 ON q.creator_id = t1.teacher_id "
                  "LEFT JOIN teachers t2 ON q.modifier_id = t2.teacher_id "
                  "ORDER BY q.question_id DESC");

    if (query.exec()) {
        while (query.next()) {
            questions.append(createQuestionFromQuery(query));
        }
    } else {
        qDebug() << "获取所有题目失败:" << query.lastError().text();
    }

    return questions;
}

//根据题目ID获取题目
Question Database::getQuestionById(int questionId)
{
    QSqlQuery query;
    query.prepare("SELECT q.*, t1.name as creator_name, t2.name as modifier_name "
                  "FROM questions q "
                  "LEFT JOIN teachers t1 ON q.creator_id = t1.teacher_id "
                  "LEFT JOIN teachers t2 ON q.modifier_id = t2.teacher_id "
                  "WHERE q.question_id = ?");
    query.addBindValue(questionId);

    if (query.exec() && query.next()) {
        return createQuestionFromQuery(query);
    } else {
        qDebug() << "获取题目详情失败:" << query.lastError().text();
        return Question(); // 返回空对象
    }
}

//检查有无修改题目权限
bool Database::canModifyQuestion(int questionId, int teacherId)
{
    QSqlQuery query;
    query.prepare("SELECT creator_id FROM questions WHERE question_id = ?");
    query.addBindValue(questionId);

    if (query.exec() && query.next()) {
        int creatorId = query.value(0).toInt();
        return creatorId == teacherId;
    }
    return false;
}

bool Database::canDeleteQuestion(int questionId, int teacherId)
{
    // 目前删除权限和修改权限相同，只有创建者可以删除
    return canModifyQuestion(questionId, teacherId);
}

//添加收藏题目
bool Database::addToFavorites(int questionId, int teacherId)
{
    // 先检查是否已经收藏
    if (isQuestionFavorited(questionId, teacherId)) {
        qDebug() << "题目已经在收藏夹中";
        return true;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO question_favorites (question_id, teacher_id) VALUES (?, ?)");
    query.addBindValue(questionId);
    query.addBindValue(teacherId);

    if (query.exec()) {
        qDebug() << "题目收藏成功";
        return true;
    } else {
        qDebug() << "题目收藏失败:" << query.lastError().text();
        return false;
    }
}

//移除收藏题目
bool Database::removeFromFavorites(int questionId, int teacherId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM question_favorites WHERE question_id = ? AND teacher_id = ?");
    query.addBindValue(questionId);
    query.addBindValue(teacherId);

    if (query.exec()) {
        qDebug() << "取消收藏成功";
        return true;
    } else {
        qDebug() << "取消收藏失败:" << query.lastError().text();
        return false;
    }
}

//检查题目是否已收藏
bool Database::isQuestionFavorited(int questionId, int teacherId)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM question_favorites WHERE question_id = ? AND teacher_id = ?");
    query.addBindValue(questionId);
    query.addBindValue(teacherId);

    if (query.exec() && query.next()) {
        return true;
    }

    return false;
}

//获取所有课程名称
QStringList Database::getCourseNames()
{
    QStringList courseNames;
    QSqlQuery query;

    query.prepare("SELECT DISTINCT course_name FROM courses ORDER BY course_name");

    if (query.exec()) {
        while (query.next()) {
            courseNames.append(query.value(0).toString());
        }
    } else {
        qDebug() << "获取课程名称失败:" << query.lastError().text();
    }

    return courseNames;
}

//获取所有题型
QStringList Database::getQuestionTypes()
{
    QStringList types;
    types << "单选" << "多选" << "填空" << "简答" << "论述" << "计算";
    return types;
}

//获取所有题型数量的map
QMap<QString,int> Database::getQuestionStatsByType(int teacherId)
{
    QMap<QString,int> stats;
    QSqlQuery query;

    query.prepare("SELECT question_type, COUNT(*) as count "
                  "FROM questions WHERE creator_id = ? "
                  "GROUP BY question_type ORDER BY question_type");
    query.addBindValue(teacherId);

    if (query.exec()) {
        while (query.next()) {
            stats[query.value(0).toString()] = query.value(1).toInt();
        }
    } else {
        qDebug() << "获取题型统计失败:" << query.lastError().text();
    }

    return stats;
}

//获取所有课程的题目数量的map
QMap<QString, int> Database::getQuestionStatsByCourse(int teacherId)
{
    QMap<QString, int> stats;
    QSqlQuery query;

    query.prepare("SELECT course_name, COUNT(*) as count "
                  "FROM questions WHERE creator_id = ? "
                  "GROUP BY course_name ORDER BY course_name");
    query.addBindValue(teacherId);

    if (query.exec()) {
        while (query.next()) {
            stats[query.value(0).toString()] = query.value(1).toInt();
        }
    } else {
        qDebug() << "获取课程统计失败:" << query.lastError().text();
    }

    return stats;
}

//搜索问题函数
QList<Question> Database::searchQuestions(const QString &keyword, int teacherId,
                                          const QString &courseName, const QString &questionType)
{
    QList<Question> questions;
    QSqlQuery query;

    QString sql = "SELECT q.*, t1.name as creator_name, t2.name as modifier_name "
                  "FROM questions q "
                  "LEFT JOIN teachers t1 ON q.creator_id = t1.teacher_id "
                  "LEFT JOIN teachers t2 ON q.modifier_id = t2.teacher_id "
                  "WHERE 1=1";

    QStringList conditions;
    QList<QVariant> bindValues;

    // 关键词搜索
    if (!keyword.isEmpty()) {
        conditions << "(q.content LIKE ? OR q.answer LIKE ?)";
        QString searchPattern = "%" + keyword + "%";
        bindValues << searchPattern << searchPattern;
    }

    // 教师筛选
    if (teacherId > 0) {
        conditions << "q.creator_id = ?";
        bindValues << teacherId;
    }

    // 课程筛选
    if (!courseName.isEmpty()) {
        conditions << "q.course_name = ?";
        bindValues << courseName;
    }

    // 题型筛选
    if (!questionType.isEmpty()) {
        conditions << "q.question_type = ?";
        bindValues << questionType;
    }

    // 拼接条件
    if (!conditions.isEmpty()) {
        sql += " AND " + conditions.join(" AND ");
    }

    sql += " ORDER BY q.frequency DESC, q.question_id DESC";

    query.prepare(sql);
    for (const QVariant &value : bindValues) {
        query.addBindValue(value);
    }

    if (query.exec()) {
        while (query.next()) {
            questions.append(createQuestionFromQuery(query));
        }
    } else {
        qDebug() << "搜索题目失败:" << query.lastError().text();
    }

    return questions;
}

//考频更新
bool Database::incrementQuestionFrequency(int questionId)
{
    QSqlQuery query;
    query.prepare("UPDATE questions SET frequency = frequency + 1 WHERE question_id = ?");
    query.addBindValue(questionId);

    if (query.exec()) {
        qDebug() << "题目考频更新成功";
        return true;
    } else {
        qDebug() << "题目考频更新失败:" << query.lastError().text();
        return false;
    }
}

//辅助方法实现
Question Database::createQuestionFromQuery(QSqlQuery &query)
{
    Question question;
    question.setQuestionId(query.value("question_id").toInt());
    question.setQuestionType(query.value("question_type").toString());
    question.setCourseName(query.value("course_name").toString());
    question.setScore(query.value("score").toInt());
    question.setContent(query.value("content").toString());
    question.setAnswer(query.value("answer").toString());
    question.setCreatorId(query.value("creator_id").toInt());
    question.setModifierId(query.value("modifier_id").toInt());
    question.setFrequency(query.value("frequency").toInt());
    question.setIsPublic(query.value("is_public").toBool());

    // 设置创建者和修改者姓名
    if (!query.value("creator_name").isNull()) {
        question.setCreatorName(query.value("creator_name").toString());
    }
    if (!query.value("modifier_name").isNull()) {
        question.setModifierName(query.value("modifier_name").toString());
    }

    return question;
}

//==========考试管理方法===============================================

// 添加考试
bool Database::addExam(const QString &examName, int courseId, const QDate &examDate,
                       const QTime &startTime, const QTime &endTime, int totalScore, int creatorId)
{
    QSqlQuery query;
    query.prepare("INSERT INTO exams (exam_name, course_id, exam_date, start_time, end_time, "
                  "total_score, is_published, is_completed) "
                  "VALUES (?, ?, ?, ?, ?, ?, FALSE, FALSE)");
    query.addBindValue(examName);
    query.addBindValue(courseId);
    query.addBindValue(examDate);
    query.addBindValue(startTime);
    query.addBindValue(endTime);
    query.addBindValue(totalScore);

    if (!query.exec()) {
        qDebug() << "添加考试失败:" << query.lastError().text();
        return false;
    }

    // 获取新插入的考试ID
    int examId = query.lastInsertId().toInt();

    // 添加创建者权限
    if (!addExamPermission(examId, creatorId, true)) {
        qDebug() << "添加考试权限失败";
        return false;
    }

    qDebug() << "考试添加成功:" << examName;
    return true;
}

// 更新考试
bool Database::updateExam(int examId, const QString &examName, const QDate &examDate,
                          const QTime &startTime, const QTime &endTime, int totalScore)
{
    QSqlQuery query;
    query.prepare("UPDATE exams SET exam_name = ?, exam_date = ?, start_time = ?, "
                  "end_time = ?, total_score = ? WHERE exam_id = ?");
    query.addBindValue(examName);
    query.addBindValue(examDate);
    query.addBindValue(startTime);
    query.addBindValue(endTime);
    query.addBindValue(totalScore);
    query.addBindValue(examId);

    if (query.exec()) {
        qDebug() << "考试更新成功";
        return true;
    } else {
        qDebug() << "考试更新失败:" << query.lastError().text();
        return false;
    }
}

// 删除考试
bool Database::deleteExam(int examId, int teacherId)
{
    // 检查权限
    if (!canModifyExam(examId, teacherId)) {
        qDebug() << "无权限删除考试:" << examId;
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM exams WHERE exam_id = ?");
    query.addBindValue(examId);

    if (query.exec()) {
        qDebug() << "考试删除成功";
        return true;
    } else {
        qDebug() << "考试删除失败:" << query.lastError().text();
        return false;
    }
}

// 获取教师相关的考试
QList<Exam> Database::getExamsByTeacher(int teacherId)
{
    QList<Exam> exams;
    QSqlQuery query;

    query.prepare("SELECT e.*, c.course_name, t.name as teacher_name "
                  "FROM exams e "
                  "INNER JOIN exam_permissions ep ON e.exam_id = ep.exam_id "
                  "INNER JOIN courses c ON e.course_id = c.course_id "
                  "INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
                  "WHERE ep.teacher_id = ? "
                  "ORDER BY e.exam_date DESC, e.start_time DESC");
    query.addBindValue(teacherId);

    if (query.exec()) {
        while (query.next()) {
            exams.append(createExamFromQuery(query));
        }
    } else {
        qDebug() << "获取教师考试失败:" << query.lastError().text();
    }

    return exams;
}

// 获取课程的考试
QList<Exam> Database::getExamsByCourse(int courseId)
{
    QList<Exam> exams;
    QSqlQuery query;

    query.prepare("SELECT e.*, c.course_name, t.name as teacher_name "
                  "FROM exams e "
                  "INNER JOIN courses c ON e.course_id = c.course_id "
                  "INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
                  "WHERE e.course_id = ? "
                  "ORDER BY e.exam_date DESC, e.start_time DESC");
    query.addBindValue(courseId);

    if (query.exec()) {
        while (query.next()) {
            exams.append(createExamFromQuery(query));
        }
    } else {
        qDebug() << "获取课程考试失败:" << query.lastError().text();
    }

    return exams;
}

// 计算考试状态
QString Database::getExamStatus(int examId)
{
    QSqlQuery query;
    query.prepare("SELECT exam_date, start_time, end_time, is_published FROM exams WHERE exam_id = ?");
    query.addBindValue(examId);

    if (!query.exec() || !query.next()) {
        return "未知";
    }

    QDate examDate = query.value("exam_date").toDate();
    QTime startTime = query.value("start_time").toTime();
    QTime endTime = query.value("end_time").toTime();
    bool isPublished = query.value("is_published").toBool();

    QDateTime examStart = QDateTime(examDate, startTime);
    QDateTime examEnd = QDateTime(examDate, endTime);
    QDateTime now = QDateTime::currentDateTime();

    if (!isPublished) {
        return "未发布";
    }
    if (now < examStart) {
        return "已发布";
    }
    if (now >= examStart && now <= examEnd) {
        return "进行中";
    }
    return "已结束";
}

// 发布考试
bool Database::publishExam(int examId)
{
    QSqlQuery query;
    query.prepare("UPDATE exams SET is_published = TRUE WHERE exam_id = ?");
    query.addBindValue(examId);

    if (query.exec()) {
        qDebug() << "考试发布成功";
        return true;
    } else {
        qDebug() << "考试发布失败:" << query.lastError().text();
        return false;
    }
}

// 终止考试
bool Database::terminateExam(int examId)
{
    QSqlQuery query;
    query.prepare("UPDATE exams SET end_time = ? WHERE exam_id = ?");
    query.addBindValue(QTime::currentTime());
    query.addBindValue(examId);

    if (query.exec()) {
        qDebug() << "考试终止成功";
        return true;
    } else {
        qDebug() << "考试终止失败:" << query.lastError().text();
        return false;
    }
}

// 按状态获取考试
QList<Exam> Database::getExamsByStatus(int teacherId, const QString &status)
{
    QList<Exam> allExams = getExamsByTeacher(teacherId);
    QList<Exam> filteredExams;

    for (const Exam &exam : allExams) {
        if (getExamStatus(exam.getExamId()) == status) {
            filteredExams.append(exam);
        }
    }

    return filteredExams;
}

// 添加题目到考试
bool Database::addQuestionToExam(int examId, int questionId, int questionOrder)
{
    QSqlQuery query;
    query.prepare("INSERT INTO exam_questions (exam_id, question_id, question_order) VALUES (?, ?, ?)");
    query.addBindValue(examId);
    query.addBindValue(questionId);
    query.addBindValue(questionOrder);

    if (query.exec()) {
        // 更新题目考频
        incrementQuestionFrequency(questionId);
        qDebug() << "题目添加到试卷成功";
        return true;
    } else {
        qDebug() << "题目添加到试卷失败:" << query.lastError().text();
        return false;
    }
}

// 从考试中移除题目
bool Database::removeQuestionFromExam(int examId, int questionId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM exam_questions WHERE exam_id = ? AND question_id = ?");
    query.addBindValue(examId);
    query.addBindValue(questionId);

    if (query.exec()) {
        qDebug() << "题目从试卷移除成功";
        return true;
    } else {
        qDebug() << "题目从试卷移除失败:" << query.lastError().text();
        return false;
    }
}

// 更新题目顺序
bool Database::updateQuestionOrder(int examId, int questionId, int newOrder)
{
    QSqlQuery query;
    query.prepare("UPDATE exam_questions SET question_order = ? WHERE exam_id = ? AND question_id = ?");
    query.addBindValue(newOrder);
    query.addBindValue(examId);
    query.addBindValue(questionId);

    if (query.exec()) {
        qDebug() << "题目顺序更新成功";
        return true;
    } else {
        qDebug() << "题目顺序更新失败:" << query.lastError().text();
        return false;
    }
}

// 获取考试题目
QList<Question> Database::getExamQuestions(int examId)
{
    QList<Question> questions;
    QSqlQuery query;

    query.prepare("SELECT q.*, t1.name as creator_name, t2.name as modifier_name, eq.question_order "
                  "FROM questions q "
                  "INNER JOIN exam_questions eq ON q.question_id = eq.question_id "
                  "LEFT JOIN teachers t1 ON q.creator_id = t1.teacher_id "
                  "LEFT JOIN teachers t2 ON q.modifier_id = t2.teacher_id "
                  "WHERE eq.exam_id = ? "
                  "ORDER BY eq.question_order");
    query.addBindValue(examId);

    if (query.exec()) {
        while (query.next()) {
            questions.append(createQuestionFromQuery(query));
        }
    } else {
        qDebug() << "获取考试题目失败:" << query.lastError().text();
    }

    return questions;
}

// 计算试卷总分
int Database::calculateExamTotalScore(int examId)
{
    QSqlQuery query;
    query.prepare("SELECT SUM(q.score) as total_score "
                  "FROM questions q "
                  "INNER JOIN exam_questions eq ON q.question_id = eq.question_id "
                  "WHERE eq.exam_id = ?");
    query.addBindValue(examId);

    if (query.exec() && query.next()) {
        return query.value("total_score").toInt();
    }

    return 0;
}

// 更新考试完成状态
bool Database::updateExamCompletion(int examId, bool isCompleted)
{
    QSqlQuery query;
    query.prepare("UPDATE exams SET is_completed = ? WHERE exam_id = ?");
    query.addBindValue(isCompleted);
    query.addBindValue(examId);

    if (query.exec()) {
        qDebug() << "考试完成状态更新成功";
        return true;
    } else {
        qDebug() << "考试完成状态更新失败:" << query.lastError().text();
        return false;
    }
}

// 添加考试权限
bool Database::addExamPermission(int examId, int teacherId, bool isCreator)
{
    QSqlQuery query;
    query.prepare("INSERT INTO exam_permissions (exam_id, teacher_id, is_creator) VALUES (?, ?, ?)");
    query.addBindValue(examId);
    query.addBindValue(teacherId);
    query.addBindValue(isCreator);

    if (query.exec()) {
        qDebug() << "考试权限添加成功";
        return true;
    } else {
        qDebug() << "考试权限添加失败:" << query.lastError().text();
        return false;
    }
}

// 移除考试权限
bool Database::removeExamPermission(int examId, int teacherId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM exam_permissions WHERE exam_id = ? AND teacher_id = ? AND is_creator = FALSE");
    query.addBindValue(examId);
    query.addBindValue(teacherId);

    if (query.exec()) {
        qDebug() << "考试权限移除成功";
        return true;
    } else {
        qDebug() << "考试权限移除失败:" << query.lastError().text();
        return false;
    }
}

// 获取考试权限的教师
QList<Teacher> Database::getExamPermissions(int examId)
{
    QList<Teacher> teachers;
    QSqlQuery query;

    query.prepare("SELECT t.*, ep.is_creator "
                  "FROM teachers t "
                  "INNER JOIN exam_permissions ep ON t.teacher_id = ep.teacher_id "
                  "WHERE ep.exam_id = ? "
                  "ORDER BY ep.is_creator DESC, t.name");
    query.addBindValue(examId);

    if (query.exec()) {
        while (query.next()) {
            teachers.append(createTeacherFromQuery(query));
        }
    } else {
        qDebug() << "获取考试权限失败:" << query.lastError().text();
    }

    return teachers;
}

// 检查是否有考试权限
bool Database::hasExamPermission(int examId, int teacherId)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM exam_permissions WHERE exam_id = ? AND teacher_id = ?");
    query.addBindValue(examId);
    query.addBindValue(teacherId);

    return query.exec() && query.next();
}

// 检查是否可以修改考试
bool Database::canModifyExam(int examId, int teacherId)
{
    QSqlQuery query;
    query.prepare("SELECT is_creator FROM exam_permissions WHERE exam_id = ? AND teacher_id = ?");
    query.addBindValue(examId);
    query.addBindValue(teacherId);

    if (query.exec() && query.next()) {
        return query.value("is_creator").toBool();
    }

    return false;
}

// 获取教师课程
QList<Course> Database::getTeacherCourses(int teacherId)
{
    QList<Course> courses;
    QSqlQuery query;

    query.prepare("SELECT c.*, t.name as teacher_name "
                  "FROM courses c "
                  "INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
                  "WHERE c.teacher_id = ? "
                  "ORDER BY c.course_name");
    query.addBindValue(teacherId);

    if (query.exec()) {
        while (query.next()) {
            courses.append(createCourseFromQuery(query));
        }
    } else {
        qDebug() << "获取教师课程失败:" << query.lastError().text();
    }

    return courses;
}

QList<Course> Database::getAllCourses()
{
    QList<Course> list;
    QSqlQuery query;
    query.prepare(
    "SELECT c.*, t.name AS teacher_name "
    "  FROM courses c "
    "  INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
    " ORDER BY c.course_name"
        );
    // 如果不需要老师名字，也可以只 SELECT c.* 并在 Course 构造里去掉 teacherName

    if (!query.exec()) {
        qWarning() << "getAllCourses exec failed:" << query.lastError();
            return list;
        }
    while (query.next()) {
            list.append( createCourseFromQuery(query) );
        }
    return list;
}

// 辅助方法：从查询结果创建Exam对象
Exam Database::createExamFromQuery(QSqlQuery &query)
{
    Exam exam;
    exam.setExamId(query.value("exam_id").toInt());
    exam.setExamName(query.value("exam_name").toString());
    exam.setCourseId(query.value("course_id").toInt());
    exam.setExamDate(query.value("exam_date").toDate());
    exam.setStartTime(query.value("start_time").toTime());
    exam.setEndTime(query.value("end_time").toTime());
    exam.setTotalScore(query.value("total_score").toInt());
    exam.setIsPublished(query.value("is_published").toBool());
    exam.setIsCompleted(query.value("is_completed").toBool());

    // 设置扩展字段
    if (!query.value("course_name").isNull()) {
        exam.setCourseName(query.value("course_name").toString());
    }
    if (!query.value("teacher_name").isNull()) {
        exam.setTeacherName(query.value("teacher_name").toString());
    }

    return exam;
}

// 辅助方法：从查询结果创建Course对象
Course Database::createCourseFromQuery(QSqlQuery &query)
{
    Course course;
    course.setCourseId(query.value("course_id").toInt());
    course.setTeacherId(query.value("teacher_id").toInt());
    course.setCollege(query.value("college").toString());
    course.setCourseName(query.value("course_name").toString());

    if (!query.value("teacher_name").isNull()) {
        course.setTeacherName(query.value("teacher_name").toString());
    }

    return course;
}

// 辅助方法：从查询结果创建Teacher对象
Teacher Database::createTeacherFromQuery(QSqlQuery &query)
{
    Teacher teacher;
    teacher.setTeacherId(query.value("teacher_id").toInt());
    teacher.setName(query.value("name").toString());
    teacher.setCollege(query.value("college").toString());

    return teacher;
}











//==========学生考试答题和阅卷相关方法===============================================





// 获取学生可参与的考试
QList<Exam> Database::getAvailableExamsForStudent(int studentId)
{
    QList<Exam> exams;
    QSqlQuery query;

    query.prepare("SELECT DISTINCT e.*, c.course_name, t.name as teacher_name "
                  "FROM exams e "
                  "INNER JOIN courses c ON e.course_id = c.course_id "
                  "INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
                  "INNER JOIN student_courses sc ON c.course_id = sc.course_id "
                  "WHERE sc.student_id = ? AND e.is_published = TRUE "
                  "ORDER BY e.exam_date ASC, e.start_time ASC");
    query.addBindValue(studentId);

    if (query.exec()) {
        while (query.next()) {
            exams.append(createExamFromQuery(query));
        }
    } else {
        qDebug() << "获取学生可参与考试失败:" << query.lastError().text();
    }

    return exams;
}

// 获取学生考试历史
QList<Exam> Database::getStudentExamHistory(int studentId)
{
    QList<Exam> exams;
    QSqlQuery query;

    query.prepare("SELECT DISTINCT e.*, c.course_name, t.name as teacher_name "
                  "FROM exams e "
                  "INNER JOIN courses c ON e.course_id = c.course_id "
                  "INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
                  "INNER JOIN student_courses sc ON c.course_id = sc.course_id "
                  "INNER JOIN student_answers sa ON e.exam_id = sa.exam_id "
                  "WHERE sc.student_id = ? AND sa.student_id = ? "
                  "ORDER BY e.exam_date DESC, e.start_time DESC");
    query.addBindValue(studentId);
    query.addBindValue(studentId);

    if (query.exec()) {
        while (query.next()) {
            exams.append(createExamFromQuery(query));
        }
    } else {
        qDebug() << "获取学生考试历史失败:" << query.lastError().text();
    }

    return exams;
}

// 检查学生是否可以参与考试
bool Database::canStudentTakeExam(int studentId, int examId)
{
    QSqlQuery query;

    // 检查学生是否选修了该课程
    query.prepare("SELECT 1 FROM student_courses sc "
                  "INNER JOIN exams e ON sc.course_id = e.course_id "
                  "WHERE sc.student_id = ? AND e.exam_id = ?");
    query.addBindValue(studentId);
    query.addBindValue(examId);

    if (!query.exec() || !query.next()) {
        qDebug() << "学生未选修该课程";
        return false;
    }

    // 检查考试是否已发布且在进行时间内
    QString status = getExamStatus(examId);
    return (status == "已发布" || status == "进行中");
}

// 检查学生是否已开始答题
bool Database::hasStudentStartedExam(int studentId, int examId)
{
    QSqlQuery query;

    // 检查学生是否有任何答题记录（无论考试是否结束）
    query.prepare("SELECT COUNT(*) FROM student_answers "
                  "WHERE student_id = :studentId AND exam_id = :examId");
    query.bindValue(":studentId", studentId);
    query.bindValue(":examId", examId);

    if (query.exec() && query.next()) {
        int answerCount = query.value(0).toInt();
        return answerCount > 0; // 有答题记录就说明学生已经参与并提交了
    }

    return false;
}
// 保存学生答案
bool Database::saveStudentAnswer(int examId, int questionId, int studentId, const QString &answer)
{
    QSqlQuery query;

    // 使用INSERT ... ON DUPLICATE KEY UPDATE或者先DELETE再INSERT的方式
    // 确保每个学生每道题只有一条记录
    query.prepare("INSERT INTO student_answers (exam_id, question_id, student_id, student_answer, score, is_graded) "
                  "VALUES (:examId, :questionId, :studentId, :answer, 0, 0) "
                  "ON DUPLICATE KEY UPDATE "
                  "student_answer = VALUES(student_answer)");

    query.bindValue(":examId", examId);
    query.bindValue(":questionId", questionId);
    query.bindValue(":studentId", studentId);
    query.bindValue(":answer", answer);

    if (!query.exec()) {
        qDebug() << "保存学生答案失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "成功保存学生答案: 学生" << studentId << "考试" << examId << "题目" << questionId;
    return true;
}

// 获取学生单题答案
QString Database::getStudentAnswer(int examId, int questionId, int studentId)
{
    QSqlQuery query;
    query.prepare("SELECT student_answer FROM student_answers "
                  "WHERE exam_id = ? AND question_id = ? AND student_id = ?");
    query.addBindValue(examId);
    query.addBindValue(questionId);
    query.addBindValue(studentId);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return QString();
}

// 获取学生所有答案
QList<StudentAnswer> Database::getStudentAnswers(int examId, int studentId)
{
    QList<StudentAnswer> answers;
    QSqlQuery query;

    query.prepare("SELECT sa.*, q.content, q.answer as correct_answer, q.question_type, q.score as full_score, "
                  "s.name as student_name "
                  "FROM student_answers sa "
                  "INNER JOIN questions q ON sa.question_id = q.question_id "
                  "INNER JOIN students s ON sa.student_id = s.student_id "
                  "WHERE sa.exam_id = ? AND sa.student_id = ? "
                  "ORDER BY sa.question_id");
    query.addBindValue(examId);
    query.addBindValue(studentId);

    if (query.exec()) {
        while (query.next()) {
            answers.append(createStudentAnswerFromQuery(query));
        }
    } else {
        qDebug() << "获取学生答案失败:" << query.lastError().text();
    }

    return answers;
}

// 提交考试
bool Database::submitExam(int examId, int studentId)
{
    // 先自动批改客观题
    autoGradeObjectiveQuestions(examId, studentId);

    qDebug() << "考试提交成功，客观题已自动批改";
    return true;
}

// 获取提交时间（这里简化实现，实际可以记录最后答题时间）
QDateTime Database::getStudentSubmitTime(int examId, int studentId)
{
    // 简化实现：返回当前时间，实际应该记录具体的提交时间
    return QDateTime::currentDateTime();
}

// 自动批改客观题
bool Database::autoGradeObjectiveQuestions(int examId, int studentId)
{
    QSqlQuery query;
    query.prepare("SELECT sa.exam_id, sa.question_id, sa.student_id, sa.student_answer, "
                  "q.answer as correct_answer, q.question_type, q.score as full_score "
                  "FROM student_answers sa "
                  "INNER JOIN questions q ON sa.question_id = q.question_id "
                  "WHERE sa.exam_id = ? AND sa.student_id = ? "
                  "AND q.question_type IN ('单选', '多选', '填空') "
                  "AND sa.is_graded = FALSE");
    query.addBindValue(examId);
    query.addBindValue(studentId);

    if (!query.exec()) {
        qDebug() << "查询客观题失败:" << query.lastError().text();
        return false;
    }

    int gradedCount = 0;
    while (query.next()) {
        QString studentAnswer = query.value("student_answer").toString();
        QString correctAnswer = query.value("correct_answer").toString();
        QString questionType = query.value("question_type").toString();
        int fullScore = query.value("full_score").toInt();
        int questionId = query.value("question_id").toInt();

        double score = calculateQuestionScore(studentAnswer, correctAnswer, questionType, fullScore);
        updateAnswerScore(examId, questionId, studentId, score, true);
        gradedCount++;
    }

    qDebug() << "客观题自动批改完成，共批改了" << gradedCount << "道题";
    return true;
}
// 计算题目得分
double Database::calculateQuestionScore(const QString &studentAnswer, const QString &correctAnswer,
                                        const QString &questionType, int fullScore)
{
    if (studentAnswer.isEmpty()) {
        return 0.0;
    }

    QString cleanStudentAnswer = studentAnswer.trimmed().toUpper();
    QString cleanCorrectAnswer = correctAnswer.trimmed().toUpper();

    if (questionType == "单选") {
        // 单选题：完全匹配
        return (cleanStudentAnswer == cleanCorrectAnswer) ? fullScore : 0.0;
    } else if (questionType == "多选") {
        // 多选题：完全匹配（可以考虑部分分数）
        return (cleanStudentAnswer == cleanCorrectAnswer) ? fullScore : 0.0;
    } else if (questionType == "填空") {
        // 填空题：支持多个正确答案，用|分隔
        QStringList correctAnswers = cleanCorrectAnswer.split("|");
        for (const QString &answer : correctAnswers) {
            if (cleanStudentAnswer == answer.trimmed()) {
                return fullScore;
            }
        }
        return 0.0;
    }

    // 主观题不自动批改
    return 0.0;
}

// 获取待阅卷考试
QList<Exam> Database::getExamsForGrading(int teacherId)
{
    QList<Exam> exams;
    QSqlQuery query;

    query.prepare("SELECT DISTINCT e.*, c.course_name, t.name as teacher_name "
                  "FROM exams e "
                  "INNER JOIN exam_permissions ep ON e.exam_id = ep.exam_id "
                  "INNER JOIN courses c ON e.course_id = c.course_id "
                  "INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
                  "WHERE ep.teacher_id = ? AND e.is_published = TRUE "
                  "AND EXISTS (SELECT 1 FROM student_answers sa WHERE sa.exam_id = e.exam_id) "
                  "ORDER BY e.exam_date DESC");
    query.addBindValue(teacherId);

    if (query.exec()) {
        while (query.next()) {
            exams.append(createExamFromQuery(query));
        }
    } else {
        qDebug() << "获取待阅卷考试失败:" << query.lastError().text();
    }

    return exams;
}

// 获取待批改答案
QList<StudentAnswer> Database::getAnswersForGrading(int examId, const QString &questionType)
{
    QList<StudentAnswer> answers;
    QSqlQuery query;

    QString sql = "SELECT sa.*, q.content, q.answer as correct_answer, q.question_type, q.score as full_score, "
                  "s.name as student_name "
                  "FROM student_answers sa "
                  "INNER JOIN questions q ON sa.question_id = q.question_id "
                  "INNER JOIN students s ON sa.student_id = s.student_id "
                  "WHERE sa.exam_id = ?";

    if (!questionType.isEmpty()) {
        sql += " AND q.question_type = ?";
    }

    sql += " ORDER BY sa.question_id, s.name";

    query.prepare(sql);
    query.addBindValue(examId);
    if (!questionType.isEmpty()) {
        query.addBindValue(questionType);
    }

    if (query.exec()) {
        while (query.next()) {
            answers.append(createStudentAnswerFromQuery(query));
        }
    } else {
        qDebug() << "获取待批改答案失败:" << query.lastError().text();
    }

    return answers;
}

// 更新答案分数
bool Database::updateAnswerScore(int examId, int questionId, int studentId, double score, bool isGraded)
{
    QSqlQuery query;
    query.prepare("UPDATE student_answers SET score = ?, is_graded = ? "
                  "WHERE exam_id = ? AND question_id = ? AND student_id = ?");
    query.addBindValue(score);
    query.addBindValue(isGraded);
    query.addBindValue(examId);
    query.addBindValue(questionId);
    query.addBindValue(studentId);

    if (query.exec()) {
        qDebug() << "答案分数更新成功";
        return true;
    } else {
        qDebug() << "答案分数更新失败:" << query.lastError().text();
        return false;
    }
}

// 检查考试是否完全批改完成
bool Database::isExamFullyGraded(int examId)
{
    QSqlQuery query;

    // 获取该考试中所有参与学生的答题记录总数
    query.prepare("SELECT COUNT(*) FROM student_answers "
                  "WHERE exam_id = :examId");
    query.bindValue(":examId", examId);

    if (!query.exec() || !query.next()) {
        qDebug() << "查询考试答题记录失败:" << query.lastError().text();
        return false;
    }
    int totalAnswers = query.value(0).toInt();

    if (totalAnswers == 0) {
        qDebug() << "考试" << examId << "没有学生参与";
        return true; // 没有学生参与考试，可以认为是"已完成"
    }

    // 获取已批改的答案数量
    query.prepare("SELECT COUNT(*) FROM student_answers "
                  "WHERE exam_id = :examId AND is_graded = 1");
    query.bindValue(":examId", examId);

    if (query.exec() && query.next()) {
        int gradedAnswers = query.value(0).toInt();
        bool fullyGraded = (gradedAnswers == totalAnswers);

        qDebug() << "考试" << examId << "批改进度检查:";
        qDebug() << "  - 总答题记录:" << totalAnswers;
        qDebug() << "  - 已批改记录:" << gradedAnswers;
        qDebug() << "  - 是否完全批改:" << fullyGraded;

        return fullyGraded; // 所有答案都已批改
    }

    qDebug() << "查询已批改答案数量失败:" << query.lastError().text();
    return false;
}


// 获取阅卷进度
QMap<QString, int> Database::getGradingProgress(int examId)
{
    QMap<QString, int> progress;
    QSqlQuery query;

    query.prepare("SELECT q.question_type, "
                  "COUNT(*) as total, "
                  "SUM(CASE WHEN sa.is_graded = TRUE THEN 1 ELSE 0 END) as graded "
                  "FROM student_answers sa "
                  "INNER JOIN questions q ON sa.question_id = q.question_id "
                  "WHERE sa.exam_id = ? "
                  "GROUP BY q.question_type");
    query.addBindValue(examId);

    if (query.exec()) {
        while (query.next()) {
            QString type = query.value("question_type").toString();
            int total = query.value("total").toInt();
            int graded = query.value("graded").toInt();

            progress[type + "_total"] = total;
            progress[type + "_graded"] = graded;
        }
    } else {
        qDebug() << "获取阅卷进度失败:" << query.lastError().text();
    }

    return progress;
}

// 获取学生考试总分
double Database::getStudentExamTotalScore(int examId, int studentId)
{
    QSqlQuery query;
    query.prepare("SELECT SUM(score) as total_score FROM student_answers "
                  "WHERE exam_id = ? AND student_id = ?");
    query.addBindValue(examId);
    query.addBindValue(studentId);

    if (query.exec() && query.next()) {
        return query.value("total_score").toDouble();
    }

    return 0.0;
}

// 获取考试统计信息
QMap<QString, QVariant> Database::getExamStatistics(int examId)
{
    QMap<QString, QVariant> stats;
    QSqlQuery query;

    // 获取参考学生数
    query.prepare("SELECT COUNT(DISTINCT student_id) as student_count FROM student_answers WHERE exam_id = ?");
    query.addBindValue(examId);
    if (query.exec() && query.next()) {
        stats["student_count"] = query.value("student_count").toInt();
    }

    // 获取成绩统计
    query.prepare("SELECT "
                  "MAX(total_score) as max_score, "
                  "MIN(total_score) as min_score, "
                  "AVG(total_score) as avg_score "
                  "FROM ("
                  "  SELECT student_id, SUM(score) as total_score "
                  "  FROM student_answers "
                  "  WHERE exam_id = ? "
                  "  GROUP BY student_id"
                  ") as scores");
    query.addBindValue(examId);
    if (query.exec() && query.next()) {
        stats["max_score"] = query.value("max_score").toDouble();
        stats["min_score"] = query.value("min_score").toDouble();
        stats["avg_score"] = query.value("avg_score").toDouble();
    }

    return stats;
}

// 辅助方法：从查询结果创建StudentAnswer对象
StudentAnswer Database::createStudentAnswerFromQuery(QSqlQuery &query)
{
    StudentAnswer answer;
    answer.setExamId(query.value("exam_id").toInt());
    answer.setQuestionId(query.value("question_id").toInt());
    answer.setStudentId(query.value("student_id").toInt());
    answer.setStudentAnswer(query.value("student_answer").toString());
    answer.setScore(query.value("score").toDouble());
    answer.setIsGraded(query.value("is_graded").toBool());

    // 设置扩展字段
    if (!query.value("content").isNull()) {
        answer.setQuestionContent(query.value("content").toString());
    }
    if (!query.value("correct_answer").isNull()) {
        answer.setCorrectAnswer(query.value("correct_answer").toString());
    }
    if (!query.value("question_type").isNull()) {
        answer.setQuestionType(query.value("question_type").toString());
    }
    if (!query.value("full_score").isNull()) {
        answer.setFullScore(query.value("full_score").toInt());
    }
    if (!query.value("student_name").isNull()) {
        answer.setStudentName(query.value("student_name").toString());
    }

    return answer;
}

Exam Database::getExamById(int examId)
{
    QSqlQuery query;
    query.prepare("SELECT e.*, c.course_name, t.name as teacher_name "
                  "FROM exams e "
                  "INNER JOIN courses c ON e.course_id = c.course_id "
                  "INNER JOIN teachers t ON c.teacher_id = t.teacher_id "
                  "WHERE e.exam_id = ?");
    query.addBindValue(examId);

    if (query.exec() && query.next()) {
        return createExamFromQuery(query);
    } else {
        qDebug() << "获取考试信息失败:" << query.lastError().text();
        return Exam(); // 返回空对象
    }
}

bool Database::hasStudentSubmittedExam(int studentId, int examId)
{
    QSqlQuery query;

    // 首先检查考试是否已经结束
    QString examStatus = getExamStatus(examId);
    if (examStatus != "已结束") {
        return false; // 考试未结束，不算提交
    }

    // 检查学生是否有答题记录
    query.prepare("SELECT COUNT(*) FROM student_answers "
                  "WHERE student_id = :studentId AND exam_id = :examId");
    query.bindValue(":studentId", studentId);
    query.bindValue(":examId", examId);

    if (query.exec() && query.next()) {
        int answerCount = query.value(0).toInt();
        return answerCount > 0; // 有答题记录就算已提交
    }

    return false;
}

bool Database::isStudentExamFullyGraded(int examId, int studentId)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as total, "
                  "SUM(CASE WHEN is_graded = TRUE THEN 1 ELSE 0 END) as graded "
                  "FROM student_answers "
                  "WHERE exam_id = ? AND student_id = ?");
    query.addBindValue(examId);
    query.addBindValue(studentId);

    if (query.exec() && query.next()) {
        int total = query.value("total").toInt();
        int graded = query.value("graded").toInt();

        qDebug() << "学生" << studentId << "考试" << examId << "批改状态:";
        qDebug() << "  - 总题目:" << total;
        qDebug() << "  - 已批改:" << graded;

        return total > 0 && total == graded;
    }

    return false;
}

bool Database::hasStudentAnsweredAllQuestions(int studentId, int examId)
{
    QSqlQuery query;

    // 获取考试总题目数
    query.prepare("SELECT COUNT(*) FROM exam_questions WHERE exam_id = :examId");
    query.bindValue(":examId", examId);

    if (!query.exec() || !query.next()) {
        return false;
    }
    int totalQuestions = query.value(0).toInt();

    // 获取学生已答题目数（有效答案）
    query.prepare("SELECT COUNT(*) FROM student_answers "
                  "WHERE student_id = :studentId AND exam_id = :examId "
                  "AND student_answer IS NOT NULL AND student_answer != ''");
    query.bindValue(":studentId", studentId);
    query.bindValue(":examId", examId);

    if (query.exec() && query.next()) {
        int answeredQuestions = query.value(0).toInt();
        return answeredQuestions == totalQuestions;
    }

    return false;
}

int Database::getStudentAnswerCount(int studentId, int examId)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM student_answers "
                  "WHERE student_id = :studentId AND exam_id = :examId "
                  "AND student_answer IS NOT NULL AND student_answer != ''");
    query.bindValue(":studentId", studentId);
    query.bindValue(":examId", examId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}




//==========班级成绩统计相关方法===============================================

// 获取班级统计数据
QMap<QString, QVariant> Database::getClassStatistics(int examId)
{
    QMap<QString, QVariant> stats;
    QSqlQuery query;

    // 获取考试基本信息
    query.prepare("SELECT exam_name, total_score FROM exams WHERE exam_id = ?");
    query.addBindValue(examId);
    if (query.exec() && query.next()) {
        stats["exam_name"] = query.value("exam_name").toString();
        stats["total_score"] = query.value("total_score").toInt();
    }

    // 获取所有选修该课程的学生及其成绩统计
    query.prepare("SELECT "
                  "COUNT(DISTINCT all_students.student_id) as total_students, "
                  "IFNULL(MAX(scores.student_total), 0) as highest_score, "
                  "IFNULL(MIN(scores.student_total), 0) as lowest_score, "
                  "IFNULL(AVG(scores.student_total), 0) as average_score, "
                  "SUM(CASE WHEN scores.student_total >= (e.total_score * 0.6) THEN 1 ELSE 0 END) as passing_count "
                  "FROM ("
                  "  SELECT DISTINCT sc.student_id "
                  "  FROM student_courses sc "
                  "  INNER JOIN exams e ON sc.course_id = e.course_id "
                  "  WHERE e.exam_id = ?"
                  ") as all_students "
                  "LEFT JOIN ("
                  "  SELECT sa.student_id, SUM(sa.score) as student_total "
                  "  FROM student_answers sa "
                  "  WHERE sa.exam_id = ? "
                  "  GROUP BY sa.student_id"
                  ") as scores ON all_students.student_id = scores.student_id "
                  "CROSS JOIN exams e "
                  "WHERE e.exam_id = ?");
    query.addBindValue(examId);
    query.addBindValue(examId);
    query.addBindValue(examId);

    if (query.exec() && query.next()) {
        int totalStudents = query.value("total_students").toInt();
        double highestScore = query.value("highest_score").toDouble();
        double lowestScore = query.value("lowest_score").toDouble();
        double averageScore = query.value("average_score").toDouble();
        int passingCount = query.value("passing_count").toInt();

        stats["total_students"] = totalStudents;
        stats["highest_score"] = highestScore;
        stats["lowest_score"] = lowestScore;
        stats["average_score"] = averageScore;
        stats["passing_count"] = passingCount;

        // 计算及格率
        double passingRate = (totalStudents > 0) ? (double(passingCount) / totalStudents * 100) : 0.0;
        stats["passing_rate"] = passingRate;

        qDebug() << "班级统计 - 考试ID:" << examId
                 << "应考人数:" << totalStudents
                 << "最高分:" << highestScore
                 << "最低分:" << lowestScore
                 << "平均分:" << averageScore
                 << "及格人数:" << passingCount
                 << "及格率:" << passingRate << "%";
    } else {
        qDebug() << "获取班级统计失败:" << query.lastError().text();
        // 设置默认值
        stats["total_students"] = 0;
        stats["highest_score"] = 0.0;
        stats["lowest_score"] = 0.0;
        stats["average_score"] = 0.0;
        stats["passing_count"] = 0;
        stats["passing_rate"] = 0.0;
    }

    return stats;
}

// 获取不及格学生名单（总分60%以下）
QList<QMap<QString, QVariant>> Database::getFailingStudents(int examId)
{
    QList<QMap<QString, QVariant>> failingStudents;
    QSqlQuery query;

    query.prepare("SELECT "
                  "s.student_id, "
                  "s.name as student_name, "
                  "IFNULL(scores.student_total, 0) as student_total, "
                  "ROUND((IFNULL(scores.student_total, 0) / e.total_score * 100), 2) as percentage, "
                  "CASE "
                  "  WHEN scores.student_total IS NULL THEN '缺考' "
                  "  WHEN scores.has_ungraded > 0 THEN '批改中' "
                  "  ELSE '不及格' "
                  "END as status "
                  "FROM ("
                  "  SELECT DISTINCT sc.student_id "
                  "  FROM student_courses sc "
                  "  INNER JOIN exams e ON sc.course_id = e.course_id "
                  "  WHERE e.exam_id = ?"
                  ") as eligible_students "
                  "INNER JOIN students s ON eligible_students.student_id = s.student_id "
                  "LEFT JOIN ("
                  "  SELECT sa.student_id, "
                  "         SUM(sa.score) as student_total, "
                  "         SUM(CASE WHEN sa.is_graded = FALSE THEN 1 ELSE 0 END) as has_ungraded "
                  "  FROM student_answers sa "
                  "  WHERE sa.exam_id = ? "
                  "  GROUP BY sa.student_id"
                  ") as scores ON eligible_students.student_id = scores.student_id "
                  "CROSS JOIN exams e "
                  "WHERE e.exam_id = ? "
                  "AND (scores.student_total IS NULL OR scores.student_total < (e.total_score * 0.6)) "
                  "ORDER BY IFNULL(scores.student_total, 0) ASC");
    query.addBindValue(examId);
    query.addBindValue(examId);
    query.addBindValue(examId);

    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> student;
            student["student_id"] = query.value("student_id").toInt();
            student["student_name"] = query.value("student_name").toString();
            student["total_score"] = query.value("student_total").toDouble();
            student["percentage"] = query.value("percentage").toDouble();
            student["status"] = query.value("status").toString();
            failingStudents.append(student);
        }
        qDebug() << "不及格/缺考学生查询完成，共" << failingStudents.size() << "人";
    } else {
        qDebug() << "获取不及格学生失败:" << query.lastError().text();
    }

    return failingStudents;
}

// 获取考试所有学生成绩
QList<QMap<QString, QVariant>> Database::getExamStudentScores(int examId)
{
    QList<QMap<QString, QVariant>> studentScores;
    QSqlQuery query;

    query.prepare("SELECT "
                  "s.student_id, "
                  "s.name as student_name, "
                  "IFNULL(scores.student_total, 0) as student_total, "
                  "ROUND((IFNULL(scores.student_total, 0) / e.total_score * 100), 2) as percentage, "
                  "CASE "
                  "  WHEN scores.student_total IS NULL THEN '缺考' "
                  "  WHEN scores.has_ungraded > 0 THEN '批改中' "
                  "  WHEN scores.student_total >= (e.total_score * 0.6) THEN '及格' "
                  "  ELSE '不及格' "
                  "END as status "
                  "FROM ("
                  "  SELECT DISTINCT sc.student_id "
                  "  FROM student_courses sc "
                  "  INNER JOIN exams e ON sc.course_id = e.course_id "
                  "  WHERE e.exam_id = ?"
                  ") as eligible_students "
                  "INNER JOIN students s ON eligible_students.student_id = s.student_id "
                  "LEFT JOIN ("
                  "  SELECT sa.student_id, "
                  "         SUM(sa.score) as student_total, "
                  "         SUM(CASE WHEN sa.is_graded = FALSE THEN 1 ELSE 0 END) as has_ungraded "
                  "  FROM student_answers sa "
                  "  WHERE sa.exam_id = ? "
                  "  GROUP BY sa.student_id"
                  ") as scores ON eligible_students.student_id = scores.student_id "
                  "CROSS JOIN exams e "
                  "WHERE e.exam_id = ? "
                  "ORDER BY IFNULL(scores.student_total, 0) DESC");
    query.addBindValue(examId);
    query.addBindValue(examId);
    query.addBindValue(examId);

    if (query.exec()) {
        int rank = 1;
        while (query.next()) {
            QMap<QString, QVariant> student;
            student["rank"] = rank++;
            student["student_id"] = query.value("student_id").toInt();
            student["student_name"] = query.value("student_name").toString();
            student["total_score"] = query.value("student_total").toDouble();
            student["percentage"] = query.value("percentage").toDouble();
            student["status"] = query.value("status").toString();
            studentScores.append(student);
        }
        qDebug() << "完整成绩查询完成，共" << studentScores.size() << "人";
    } else {
        qDebug() << "获取学生成绩失败:" << query.lastError().text();
    }

    return studentScores;
}







// ============================================================================
// 好友管理功能实现
// ============================================================================

bool Database::addFriendship(int user1Id, const QString &user1Type, int user2Id, const QString &user2Type)
{
    QSqlQuery query;

    // 检查是否已经是好友
    if (areFriends(user1Id, user1Type, user2Id, user2Type)) {
        qDebug() << "用户已经是好友关系";
        return false;
    }

    // 标准化好友关系（确保较小的ID在前面，保持一致性）
    int firstUserId, secondUserId;
    QString firstUserType, secondUserType;

    if (user1Id < user2Id || (user1Id == user2Id && user1Type < user2Type)) {
        firstUserId = user1Id;
        firstUserType = user1Type;
        secondUserId = user2Id;
        secondUserType = user2Type;
    } else {
        firstUserId = user2Id;
        firstUserType = user2Type;
        secondUserId = user1Id;
        secondUserType = user1Type;
    }

    query.prepare("INSERT INTO friend_relationships (user1_id, user1_type, user2_id, user2_type) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(firstUserId);
    query.addBindValue(firstUserType);
    query.addBindValue(secondUserId);
    query.addBindValue(secondUserType);

    if (!query.exec()) {
        qDebug() << "添加好友关系失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "成功添加好友关系:" << firstUserId << firstUserType << "-" << secondUserId << secondUserType;
    return true;
}

bool Database::removeFriendship(int user1Id, const QString &user1Type, int user2Id, const QString &user2Type)
{
    QSqlQuery query;

    query.prepare("DELETE FROM friend_relationships WHERE "
                  "((user1_id = ? AND user1_type = ? AND user2_id = ? AND user2_type = ?) OR "
                  "(user1_id = ? AND user1_type = ? AND user2_id = ? AND user2_type = ?))");
    query.addBindValue(user1Id);
    query.addBindValue(user1Type);
    query.addBindValue(user2Id);
    query.addBindValue(user2Type);
    query.addBindValue(user2Id);
    query.addBindValue(user2Type);
    query.addBindValue(user1Id);
    query.addBindValue(user1Type);

    if (!query.exec()) {
        qDebug() << "删除好友关系失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "成功删除好友关系";
    return true;
}

bool Database::areFriends(int user1Id, const QString &user1Type, int user2Id, const QString &user2Type)
{
    QSqlQuery query;

    query.prepare("SELECT COUNT(*) FROM friend_relationships WHERE "
                  "((user1_id = ? AND user1_type = ? AND user2_id = ? AND user2_type = ?) OR "
                  "(user1_id = ? AND user1_type = ? AND user2_id = ? AND user2_type = ?))");
    query.addBindValue(user1Id);
    query.addBindValue(user1Type);
    query.addBindValue(user2Id);
    query.addBindValue(user2Type);
    query.addBindValue(user2Id);
    query.addBindValue(user2Type);
    query.addBindValue(user1Id);
    query.addBindValue(user1Type);

    if (!query.exec() || !query.next()) {
        qDebug() << "检查好友关系失败:" << query.lastError().text();
        return false;
    }

    int count = query.value(0).toInt();
    qDebug() << "检查好友关系:" << user1Id << user1Type << "和" << user2Id << user2Type << "结果:" << (count > 0);
    return count > 0;
}

QList<QVariantMap> Database::getFriendsList(int userId, const QString &userType)
{
    QList<QVariantMap> friends;
    QSqlQuery query;

    // 简化查询逻辑，分别处理 user1 和 user2 的情况
    query.prepare(
        "SELECT "
        "    CASE "
        "        WHEN fr.user1_id = ? AND fr.user1_type = ? THEN fr.user2_id "
        "        ELSE fr.user1_id "
        "    END as friend_id, "
        "    CASE "
        "        WHEN fr.user1_id = ? AND fr.user1_type = ? THEN fr.user2_type "
        "        ELSE fr.user1_type "
        "    END as friend_type, "
        "    fr.created_time "
        "FROM friend_relationships fr "
        "WHERE (fr.user1_id = ? AND fr.user1_type = ?) OR (fr.user2_id = ? AND fr.user2_type = ?) "
        "ORDER BY fr.created_time DESC"
        );

    // 绑定参数
    query.addBindValue(userId);
    query.addBindValue(userType);
    query.addBindValue(userId);
    query.addBindValue(userType);
    query.addBindValue(userId);
    query.addBindValue(userType);
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec()) {
        qDebug() << "获取好友列表失败:" << query.lastError().text();
        return friends;
    }

    // 获取好友基本信息
    while (query.next()) {
        int friendId = query.value("friend_id").toInt();
        QString friendType = query.value("friend_type").toString();
        QDateTime createdTime = query.value("created_time").toDateTime();

        // 根据好友类型获取详细信息
        QSqlQuery detailQuery;
        QString friendName, friendCollege, friendGrade;

        if (friendType == "学生") {
            detailQuery.prepare("SELECT name, college, grade FROM students WHERE student_id = ?");
            detailQuery.addBindValue(friendId);
            if (detailQuery.exec() && detailQuery.next()) {
                friendName = detailQuery.value("name").toString();
                friendCollege = detailQuery.value("college").toString();
                friendGrade = detailQuery.value("grade").toString();
            }
        } else if (friendType == "老师") {
            detailQuery.prepare("SELECT name, college FROM teachers WHERE teacher_id = ?");
            detailQuery.addBindValue(friendId);
            if (detailQuery.exec() && detailQuery.next()) {
                friendName = detailQuery.value("name").toString();
                friendCollege = detailQuery.value("college").toString();
                friendGrade = QString(); // 老师没有年级
            }
        }

        // 添加到结果列表
        if (!friendName.isEmpty()) {
            QVariantMap friendInfo;
            friendInfo["friend_id"] = friendId;
            friendInfo["friend_type"] = friendType;
            friendInfo["friend_name"] = friendName;
            friendInfo["friend_college"] = friendCollege;
            friendInfo["friend_grade"] = friendGrade;
            friendInfo["created_time"] = createdTime;
            friends.append(friendInfo);
        }
    }

    qDebug() << "用户" << userId << userType << "的好友数量:" << friends.size();
    return friends;
}


bool Database::sendFriendRequest(int requesterId, const QString &requesterType, int targetId, const QString &targetType)
{
    // 检查是否已经是好友
    if (areFriends(requesterId, requesterType, targetId, targetType)) {
        qDebug() << "用户已经是好友关系，无法发送请求";
        return false;
    }

    // 检查是否已经有待处理的请求
    if (hasPendingFriendRequest(requesterId, requesterType, targetId, targetType)) {
        qDebug() << "已存在待处理的好友请求";
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO friend_requests (requester_id, requester_type, target_id, target_type, status) "
                  "VALUES (?, ?, ?, ?, '申请中')");
    query.addBindValue(requesterId);
    query.addBindValue(requesterType);
    query.addBindValue(targetId);
    query.addBindValue(targetType);

    if (!query.exec()) {
        qDebug() << "发送好友请求失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "成功发送好友请求";
    return true;
}

bool Database::acceptFriendRequest(int requestId)
{
    QSqlDatabase::database().transaction();

    try {
        // 获取请求信息
        QSqlQuery selectQuery;
        selectQuery.prepare("SELECT requester_id, requester_type, target_id, target_type FROM friend_requests "
                            "WHERE request_id = ? AND status = '申请中'");
        selectQuery.addBindValue(requestId);

        if (!selectQuery.exec() || !selectQuery.next()) {
            throw std::runtime_error("请求不存在或已处理");
        }

        int requesterId = selectQuery.value("requester_id").toInt();
        QString requesterType = selectQuery.value("requester_type").toString();
        int targetId = selectQuery.value("target_id").toInt();
        QString targetType = selectQuery.value("target_type").toString();

        // 建立好友关系
        if (!addFriendship(requesterId, requesterType, targetId, targetType)) {
            throw std::runtime_error("建立好友关系失败");
        }

        // 更新请求状态
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE friend_requests SET status = '已同意' WHERE request_id = ?");
        updateQuery.addBindValue(requestId);

        if (!updateQuery.exec()) {
            throw std::runtime_error("更新请求状态失败");
        }

        QSqlDatabase::database().commit();
        qDebug() << "成功接受好友请求";
        return true;

    } catch (const std::exception &e) {
        QSqlDatabase::database().rollback();
        qDebug() << "接受好友请求失败:" << e.what();
        return false;
    }
}

bool Database::rejectFriendRequest(int requestId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM friend_requests WHERE request_id = ? AND status = '申请中'");
    query.addBindValue(requestId);

    if (!query.exec()) {
        qDebug() << "拒绝好友请求失败:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "请求不存在或已处理";
        return false;
    }

    qDebug() << "成功拒绝好友请求";
    return true;
}

QList<QVariantMap> Database::getReceivedFriendRequests(int userId, const QString &userType)
{
    QList<QVariantMap> requests;
    QSqlQuery query;

    query.prepare(
        "SELECT fr.request_id, fr.requester_id, fr.requester_type, "
        "       CASE fr.requester_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as requester_name, "
        "       CASE fr.requester_type "
        "           WHEN '学生' THEN s.college "
        "           WHEN '老师' THEN t.college "
        "       END as requester_college, "
        "       CASE fr.requester_type "
        "           WHEN '学生' THEN s.grade "
        "           ELSE NULL "
        "       END as requester_grade, "
        "       fr.request_time "
        "FROM friend_requests fr "
        "LEFT JOIN students s ON (fr.requester_id = s.student_id AND fr.requester_type = '学生') "
        "LEFT JOIN teachers t ON (fr.requester_id = t.teacher_id AND fr.requester_type = '老师') "
        "WHERE fr.target_id = ? AND fr.target_type = ? AND fr.status = '申请中' "
        "ORDER BY fr.request_time DESC"
        );
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec()) {
        qDebug() << "获取收到的好友请求失败:" << query.lastError().text();
        return requests;
    }

    while (query.next()) {
        QVariantMap request;
        request["request_id"] = query.value("request_id");
        request["requester_id"] = query.value("requester_id");
        request["requester_type"] = query.value("requester_type");
        request["requester_name"] = query.value("requester_name");
        request["requester_college"] = query.value("requester_college");
        request["requester_grade"] = query.value("requester_grade");
        request["request_time"] = query.value("request_time");
        requests.append(request);
    }

    return requests;
}

QList<QVariantMap> Database::getSentFriendRequests(int userId, const QString &userType)
{
    QList<QVariantMap> requests;
    QSqlQuery query;

    query.prepare(
        "SELECT fr.request_id, fr.target_id, fr.target_type, "
        "       CASE fr.target_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as target_name, "
        "       CASE fr.target_type "
        "           WHEN '学生' THEN s.college "
        "           WHEN '老师' THEN t.college "
        "       END as target_college, "
        "       fr.status, fr.request_time "
        "FROM friend_requests fr "
        "LEFT JOIN students s ON (fr.target_id = s.student_id AND fr.target_type = '学生') "
        "LEFT JOIN teachers t ON (fr.target_id = t.teacher_id AND fr.target_type = '老师') "
        "WHERE fr.requester_id = ? AND fr.requester_type = ? "
        "ORDER BY fr.request_time DESC"
        );
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec()) {
        qDebug() << "获取发送的好友请求失败:" << query.lastError().text();
        return requests;
    }

    while (query.next()) {
        QVariantMap request;
        request["request_id"] = query.value("request_id");
        request["target_id"] = query.value("target_id");
        request["target_type"] = query.value("target_type");
        request["target_name"] = query.value("target_name");
        request["target_college"] = query.value("target_college");
        request["status"] = query.value("status");
        request["request_time"] = query.value("request_time");
        requests.append(request);
    }

    return requests;
}

bool Database::hasPendingFriendRequest(int requesterId, const QString &requesterType, int targetId, const QString &targetType)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM friend_requests WHERE "
                  "((requester_id = ? AND requester_type = ? AND target_id = ? AND target_type = ?) OR "
                  "(requester_id = ? AND requester_type = ? AND target_id = ? AND target_type = ?)) "
                  "AND status = '申请中'");
    query.addBindValue(requesterId);
    query.addBindValue(requesterType);
    query.addBindValue(targetId);
    query.addBindValue(targetType);
    query.addBindValue(targetId);
    query.addBindValue(targetType);
    query.addBindValue(requesterId);
    query.addBindValue(requesterType);

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

QList<QVariantMap> Database::searchUsersByKeyword(const QString &keyword, int currentUserId, const QString &currentUserType)
{
    QList<QVariantMap> users;

    if (keyword.trimmed().isEmpty()) {
        return users;
    }

    QSqlQuery query;

    // 搜索学生和教师
    query.prepare(
        "SELECT student_id as user_id, '学生' as user_type, name, college, grade "
        "FROM students "
        "WHERE (name LIKE ? OR CAST(student_id AS CHAR) LIKE ?) AND NOT (student_id = ? AND ? = '学生') "
        "UNION ALL "
        "SELECT teacher_id as user_id, '老师' as user_type, name, college, NULL as grade "
        "FROM teachers "
        "WHERE (name LIKE ? OR CAST(teacher_id AS CHAR) LIKE ?) AND NOT (teacher_id = ? AND ? = '老师') "
        "ORDER BY user_type, name"
        );

    QString searchPattern = "%" + keyword + "%";
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);
    query.addBindValue(currentUserId);
    query.addBindValue(currentUserType);
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);
    query.addBindValue(currentUserId);
    query.addBindValue(currentUserType);

    if (!query.exec()) {
        qDebug() << "按关键词搜索用户失败:" << query.lastError().text();
        return users;
    }

    while (query.next()) {
        QVariantMap user;
        user["user_id"] = query.value("user_id");
        user["user_type"] = query.value("user_type");
        user["user_name"] = query.value("name");      // 修改：改为 user_name
        user["user_college"] = query.value("college"); // 修改：改为 user_college
        user["user_grade"] = query.value("grade");     // 修改：改为 user_grade

        // 检查关系状态
        int userId = user["user_id"].toInt();
        QString userType = user["user_type"].toString();

        if (areFriends(currentUserId, currentUserType, userId, userType)) {
            user["relationship_status"] = "已是好友";   // 修改：改为 relationship_status
        } else if (hasPendingFriendRequest(currentUserId, currentUserType, userId, userType)) {
            user["relationship_status"] = "已发送请求"; // 修改：改为 relationship_status
        } else if (hasPendingFriendRequest(userId, userType, currentUserId, currentUserType)) {
            user["relationship_status"] = "待处理请求"; // 修改：改为 relationship_status
        } else {
            user["relationship_status"] = "可添加";     // 修改：改为 relationship_status
        }

        users.append(user);
    }

    qDebug() << "搜索用户完成，关键词:" << keyword << "找到" << users.size() << "个用户";
    return users;
}

QList<QVariantMap> Database::searchUsersById(int userId, int currentUserId, const QString &currentUserType)
{
    QList<QVariantMap> users;
    QSqlQuery query;

    // 先搜索学生
    query.prepare("SELECT student_id as user_id, '学生' as user_type, name, college, grade "
                  "FROM students WHERE student_id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        QVariantMap user;
        user["user_id"] = query.value("user_id");
        user["user_type"] = query.value("user_type");
        user["user_name"] = query.value("name");      // 修改：改为 user_name
        user["user_college"] = query.value("college"); // 修改：改为 user_college
        user["user_grade"] = query.value("grade");     // 修改：改为 user_grade

        // 添加关系状态检查
        int searchUserId = user["user_id"].toInt();
        QString searchUserType = user["user_type"].toString();

        if (areFriends(currentUserId, currentUserType, searchUserId, searchUserType)) {
            user["relationship_status"] = "已是好友";   // 修改：改为 relationship_status
        } else if (hasPendingFriendRequest(currentUserId, currentUserType, searchUserId, searchUserType)) {
            user["relationship_status"] = "已发送请求"; // 修改：改为 relationship_status
        } else if (hasPendingFriendRequest(searchUserId, searchUserType, currentUserId, currentUserType)) {
            user["relationship_status"] = "待处理请求"; // 修改：改为 relationship_status
        } else {
            user["relationship_status"] = "可添加";     // 修改：改为 relationship_status
        }

        users.append(user);
        qDebug() << "按ID搜索学生成功:" << userId;
        return users;
    }

    // 再搜索教师
    query.prepare("SELECT teacher_id as user_id, '老师' as user_type, name, college, NULL as grade "
                  "FROM teachers WHERE teacher_id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        QVariantMap user;
        user["user_id"] = query.value("user_id");
        user["user_type"] = query.value("user_type");
        user["user_name"] = query.value("name");      // 修改：改为 user_name
        user["user_college"] = query.value("college"); // 修改：改为 user_college
        user["user_grade"] = query.value("grade");     // 修改：改为 user_grade

        // 添加关系状态检查
        int searchUserId = user["user_id"].toInt();
        QString searchUserType = user["user_type"].toString();

        if (areFriends(currentUserId, currentUserType, searchUserId, searchUserType)) {
            user["relationship_status"] = "已是好友";   // 修改：改为 relationship_status
        } else if (hasPendingFriendRequest(currentUserId, currentUserType, searchUserId, searchUserType)) {
            user["relationship_status"] = "已发送请求"; // 修改：改为 relationship_status
        } else if (hasPendingFriendRequest(searchUserId, searchUserType, currentUserId, currentUserType)) {
            user["relationship_status"] = "待处理请求"; // 修改：改为 relationship_status
        } else {
            user["relationship_status"] = "可添加";     // 修改：改为 relationship_status
        }

        users.append(user);
        qDebug() << "按ID搜索教师成功:" << userId;
    }

    return users;
}

QList<QVariantMap> Database::getClassmates(int studentId)
{
    QList<QVariantMap> classmates;
    QSqlQuery query;

    // 获取同班同学（同一课程的学生）
    query.prepare(
        "SELECT DISTINCT s.student_id as user_id, '学生' as user_type, s.name, s.college, s.grade "
        "FROM students s "
        "JOIN student_courses sc1 ON s.student_id = sc1.student_id "
        "JOIN student_courses sc2 ON sc1.course_id = sc2.course_id "
        "WHERE sc2.student_id = ? AND s.student_id != ? AND sc1.enrollment_status = '已通过' AND sc2.enrollment_status = '已通过' "
        "ORDER BY s.name"
        );
    query.addBindValue(studentId);
    query.addBindValue(studentId);

    if (!query.exec()) {
        qDebug() << "获取同班同学失败:" << query.lastError().text();
        return classmates;
    }

    while (query.next()) {
        QVariantMap classmate;
        classmate["user_id"] = query.value("user_id");
        classmate["user_type"] = query.value("user_type");
        classmate["name"] = query.value("name");
        classmate["college"] = query.value("college");
        classmate["grade"] = query.value("grade");
        classmates.append(classmate);
    }

    return classmates;
}

QList<QVariantMap> Database::getColleagues(int teacherId)
{
    QList<QVariantMap> colleagues;
    QSqlQuery query;

    // 获取同学院的教师
    query.prepare(
        "SELECT DISTINCT t2.teacher_id as user_id, '老师' as user_type, t2.name, t2.college, NULL as grade "
        "FROM teachers t1 "
        "JOIN teachers t2 ON t1.college = t2.college "
        "WHERE t1.teacher_id = ? AND t2.teacher_id != ? "
        "ORDER BY t2.name"
        );
    query.addBindValue(teacherId);
    query.addBindValue(teacherId);

    if (!query.exec()) {
        qDebug() << "获取同事失败:" << query.lastError().text();
        return colleagues;
    }

    while (query.next()) {
        QVariantMap colleague;
        colleague["user_id"] = query.value("user_id");
        colleague["user_type"] = query.value("user_type");
        colleague["name"] = query.value("name");
        colleague["college"] = query.value("college");
        colleague["grade"] = query.value("grade");
        colleagues.append(colleague);
    }

    return colleagues;
}

int Database::getFriendCount(int userId, const QString &userType)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM friend_relationships WHERE "
                  "(user1_id = ? AND user1_type = ?) OR (user2_id = ? AND user2_type = ?)");
    query.addBindValue(userId);
    query.addBindValue(userType);
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}

int Database::getPendingRequestCount(int userId, const QString &userType)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM friend_requests WHERE "
                  "target_id = ? AND target_type = ? AND status = '申请中'");
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}



// ============================================================================
// 聊天功能实现
// ============================================================================

int Database::getOrCreatePrivateChat(int user1Id, const QString &user1Type,
                                     int user2Id, const QString &user2Type)
{
    QSqlQuery query;

    // 首先检查私聊关系是否已存在（双向查询）
    query.prepare("SELECT chat_id FROM private_chats WHERE "
                  "((user1_id = ? AND user1_type = ? AND user2_id = ? AND user2_type = ?) OR "
                  "(user1_id = ? AND user1_type = ? AND user2_id = ? AND user2_type = ?))");
    query.addBindValue(user1Id);
    query.addBindValue(user1Type);
    query.addBindValue(user2Id);
    query.addBindValue(user2Type);
    query.addBindValue(user2Id);
    query.addBindValue(user2Type);
    query.addBindValue(user1Id);
    query.addBindValue(user1Type);

    if (!query.exec()) {
        qDebug() << "查询私聊关系失败:" << query.lastError().text();
        return -1;
    }

    // 如果已存在，返回chat_id
    if (query.next()) {
        int chatId = query.value("chat_id").toInt();
        qDebug() << "私聊关系已存在，chat_id:" << chatId;
        return chatId;
    }

    // 标准化存储（较小ID在前）
    int firstUserId, secondUserId;
    QString firstUserType, secondUserType;

    if (user1Id < user2Id || (user1Id == user2Id && user1Type < user2Type)) {
        firstUserId = user1Id;
        firstUserType = user1Type;
        secondUserId = user2Id;
        secondUserType = user2Type;
    } else {
        firstUserId = user2Id;
        firstUserType = user2Type;
        secondUserId = user1Id;
        secondUserType = user1Type;
    }

    // 创建新的私聊关系
    query.prepare("INSERT INTO private_chats (user1_id, user1_type, user2_id, user2_type) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(firstUserId);
    query.addBindValue(firstUserType);
    query.addBindValue(secondUserId);
    query.addBindValue(secondUserType);

    if (!query.exec()) {
        qDebug() << "创建私聊关系失败:" << query.lastError().text();
        return -1;
    }

    int chatId = query.lastInsertId().toInt();
    qDebug() << "成功创建私聊关系，chat_id:" << chatId;
    return chatId;
}

QList<QVariantMap> Database::getPrivateChats(int userId, const QString &userType)
{
    QList<QVariantMap> chats;
    QSqlQuery query;

    query.prepare(
        "SELECT pc.chat_id, "
        "       CASE "
        "           WHEN pc.user1_id = ? AND pc.user1_type = ? THEN pc.user2_id "
        "           ELSE pc.user1_id "
        "       END as friend_id, "
        "       CASE "
        "           WHEN pc.user1_id = ? AND pc.user1_type = ? THEN pc.user2_type "
        "           ELSE pc.user1_type "
        "       END as friend_type, "
        "       pc.created_time, pc.last_message_time "
        "FROM private_chats pc "
        "WHERE (pc.user1_id = ? AND pc.user1_type = ?) OR (pc.user2_id = ? AND pc.user2_type = ?) "
        "ORDER BY IFNULL(pc.last_message_time, pc.created_time) DESC"
        );

    query.addBindValue(userId);
    query.addBindValue(userType);
    query.addBindValue(userId);
    query.addBindValue(userType);
    query.addBindValue(userId);
    query.addBindValue(userType);
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec()) {
        qDebug() << "获取私聊列表失败:" << query.lastError().text();
        return chats;
    }

    while (query.next()) {
        QVariantMap chat;
        chat["chat_id"] = query.value("chat_id");
        chat["friend_id"] = query.value("friend_id");
        chat["friend_type"] = query.value("friend_type");
        chat["created_time"] = query.value("created_time");
        chat["last_message_time"] = query.value("last_message_time");

        // 获取好友详细信息
        int friendId = chat["friend_id"].toInt();
        QString friendType = chat["friend_type"].toString();

        QSqlQuery detailQuery;
        if (friendType == "学生") {
            detailQuery.prepare("SELECT name, college, grade FROM students WHERE student_id = ?");
            detailQuery.addBindValue(friendId);
            if (detailQuery.exec() && detailQuery.next()) {
                chat["friend_name"] = detailQuery.value("name");
                chat["friend_college"] = detailQuery.value("college");
                chat["friend_grade"] = detailQuery.value("grade");
            }
        } else if (friendType == "老师") {
            detailQuery.prepare("SELECT name, college FROM teachers WHERE teacher_id = ?");
            detailQuery.addBindValue(friendId);
            if (detailQuery.exec() && detailQuery.next()) {
                chat["friend_name"] = detailQuery.value("name");
                chat["friend_college"] = detailQuery.value("college");
                chat["friend_grade"] = QString(); // 教师没有年级
            }
        }

        // 获取最后一条消息
        QVariantMap lastMessage = getLastMessage(chat["chat_id"].toInt(), "私聊");
        if (!lastMessage.isEmpty()) {
            chat["last_message"] = lastMessage["content"];
            chat["last_message_time"] = lastMessage["send_time"];
        }

        chats.append(chat);
    }

    qDebug() << "用户" << userId << userType << "的私聊数量:" << chats.size();
    return chats;
}

bool Database::updatePrivateChatLastMessage(int chatId)
{
    QSqlQuery query;
    query.prepare("UPDATE private_chats SET last_message_time = NOW() WHERE chat_id = ?");
    query.addBindValue(chatId);

    if (!query.exec()) {
        qDebug() << "更新私聊最后消息时间失败:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::canChat(int user1Id, const QString &user1Type,
                       int user2Id, const QString &user2Type)
{
    // 检查是否为好友关系
    return areFriends(user1Id, user1Type, user2Id, user2Type);
}

int Database::sendMessage(int chatId, const QString &chatType,
                          int senderId, const QString &senderType,
                          const QString &content)
{
    QSqlQuery query;
    query.prepare("INSERT INTO messages (chat_type, chat_id, sender_id, sender_type, content) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(chatType);
    query.addBindValue(chatId);
    query.addBindValue(senderId);
    query.addBindValue(senderType);
    query.addBindValue(content);

    if (!query.exec()) {
        qDebug() << "发送消息失败:" << query.lastError().text();
        return -1;
    }

    int messageId = query.lastInsertId().toInt();

    // 更新聊天最后消息时间
    if (chatType == "私聊") {
        updatePrivateChatLastMessage(chatId);
    }

    qDebug() << "成功发送消息，message_id:" << messageId;
    return messageId;
}

QList<QVariantMap> Database::getChatMessages(int chatId, const QString &chatType,
                                             int limit, int offset)
{
    QList<QVariantMap> messages;
    QSqlQuery query;

    query.prepare(
        "SELECT m.message_id, m.sender_id, m.sender_type, m.content, m.send_time, "
        "       CASE m.sender_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as sender_name "
        "FROM messages m "
        "LEFT JOIN students s ON (m.sender_id = s.student_id AND m.sender_type = '学生') "
        "LEFT JOIN teachers t ON (m.sender_id = t.teacher_id AND m.sender_type = '老师') "
        "WHERE m.chat_id = ? AND m.chat_type = ? "
        "ORDER BY m.send_time DESC "
        "LIMIT ? OFFSET ?"
        );
    query.addBindValue(chatId);
    query.addBindValue(chatType);
    query.addBindValue(limit);
    query.addBindValue(offset);

    if (!query.exec()) {
        qDebug() << "获取聊天记录失败:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        QVariantMap message;
        message["message_id"] = query.value("message_id");
        message["sender_id"] = query.value("sender_id");
        message["sender_type"] = query.value("sender_type");
        message["sender_name"] = query.value("sender_name");
        message["content"] = query.value("content");
        message["send_time"] = query.value("send_time");
        messages.append(message);
    }

    // 反转顺序，使最新消息在最后
    std::reverse(messages.begin(), messages.end());

    qDebug() << "获取到" << messages.size() << "条聊天记录";
    return messages;
}

QVariantMap Database::getLastMessage(int chatId, const QString &chatType)
{
    QVariantMap lastMessage;
    QSqlQuery query;

    query.prepare(
        "SELECT m.message_id, m.sender_id, m.sender_type, m.content, m.send_time, "
        "       CASE m.sender_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as sender_name "
        "FROM messages m "
        "LEFT JOIN students s ON (m.sender_id = s.student_id AND m.sender_type = '学生') "
        "LEFT JOIN teachers t ON (m.sender_id = t.teacher_id AND m.sender_type = '老师') "
        "WHERE m.chat_id = ? AND m.chat_type = ? "
        "ORDER BY m.send_time DESC "
        "LIMIT 1"
        );
    query.addBindValue(chatId);
    query.addBindValue(chatType);

    if (!query.exec()) {
        qDebug() << "获取最后消息失败:" << query.lastError().text();
        return lastMessage;
    }

    if (query.next()) {
        lastMessage["message_id"] = query.value("message_id");
        lastMessage["sender_id"] = query.value("sender_id");
        lastMessage["sender_type"] = query.value("sender_type");
        lastMessage["sender_name"] = query.value("sender_name");
        lastMessage["content"] = query.value("content");
        lastMessage["send_time"] = query.value("send_time");
    }

    return lastMessage;
}

bool Database::deleteMessage(int messageId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM messages WHERE message_id = ?");
    query.addBindValue(messageId);

    if (!query.exec()) {
        qDebug() << "删除消息失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "成功删除消息:" << messageId;
    return true;
}

int Database::getUnreadMessageCount(int userId, const QString &userType)
{
    // 注意：由于当前数据库设计中没有已读状态字段，
    // 这里暂时返回0，实际项目中可以扩展消息表添加已读状态
    // 或者创建单独的已读记录表

    qDebug() << "获取未读消息数量 - 当前版本暂不支持已读状态";
    return 0;
}

bool Database::markMessagesAsRead(int chatId, int userId, const QString &userType)
{
    // 注意：由于当前数据库设计中没有已读状态字段，
    // 这里暂时返回true，实际项目中可以扩展消息表添加已读状态
    // 或者创建单独的已读记录表

    qDebug() << "标记消息为已读 - 当前版本暂不支持已读状态";
    return true;
}

// ============================================================================
// 在 database.h 中添加的群聊管理功能声明
// ============================================================================

// 在 Database 类的 public 部分添加以下方法声明：

// ============================================================================
// 群聊功能（阶段5新增）
// ============================================================================

// 群聊管理（4个方法）
int createGroupChat(const QString &groupName, int creatorId, const QString &creatorType);
QList<QVariantMap> getUserGroups(int userId, const QString &userType);
QList<QVariantMap> getAllGroups(); // 用于群聊搜索
bool deleteGroupChat(int groupId, int userId, const QString &userType);

// 成员管理（5个方法）
bool addGroupMember(int groupId, int userId, const QString &userType);
bool removeGroupMember(int groupId, int userId, const QString &userType);
QList<QVariantMap> getGroupMembers(int groupId);
bool isGroupMember(int groupId, int userId, const QString &userType);
int getGroupMemberCount(int groupId);

// 申请管理（4个方法）
bool sendGroupRequest(int groupId, int userId, const QString &userType);
bool processGroupRequest(int requestId, bool accept);
QList<QVariantMap> getGroupRequests(int groupId);
QList<QVariantMap> getUserGroupRequests(int userId, const QString &userType);

// 权限验证（3个方法）
bool isGroupCreator(int groupId, int userId, const QString &userType);
bool canManageGroup(int groupId, int userId, const QString &userType);
QVariantMap getGroupInfo(int groupId);

// ============================================================================
// 在 database.cpp 中添加的群聊管理功能实现
// ============================================================================

// 群聊管理（4个方法）
int Database::createGroupChat(const QString &groupName, int creatorId, const QString &creatorType)
{
    QSqlQuery query;

    // 创建群聊
    query.prepare("INSERT INTO group_chats (group_name, creator_id, creator_type) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(groupName);
    query.addBindValue(creatorId);
    query.addBindValue(creatorType);

    if (!query.exec()) {
        qDebug() << "创建群聊失败:" << query.lastError().text();
        return -1;
    }

    int groupId = query.lastInsertId().toInt();

    // 将创建者添加为群成员
    if (!addGroupMember(groupId, creatorId, creatorType)) {
        qDebug() << "将创建者添加为群成员失败";
        // 删除刚创建的群聊
        query.prepare("DELETE FROM group_chats WHERE group_id = ?");
        query.addBindValue(groupId);
        query.exec();
        return -1;
    }

    qDebug() << "成功创建群聊:" << groupName << "群聊ID:" << groupId;
    return groupId;
}

QList<QVariantMap> Database::getUserGroups(int userId, const QString &userType)
{
    QList<QVariantMap> groups;
    QSqlQuery query;

    query.prepare(
        "SELECT gc.group_id, gc.group_name, gc.creator_id, gc.creator_type, "
        "       gc.created_time, gc.last_message_time, gc.member_count, "
        "       CASE gc.creator_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as creator_name, "
        "       CASE "
        "           WHEN gc.creator_id = ? AND gc.creator_type = ? THEN '创建者' "
        "           ELSE '成员' "
        "       END as user_role "
        "FROM group_chats gc "
        "INNER JOIN group_members gm ON gc.group_id = gm.group_id "
        "LEFT JOIN students s ON (gc.creator_id = s.student_id AND gc.creator_type = '学生') "
        "LEFT JOIN teachers t ON (gc.creator_id = t.teacher_id AND gc.creator_type = '老师') "
        "WHERE gm.user_id = ? AND gm.user_type = ? "
        "ORDER BY IFNULL(gc.last_message_time, gc.created_time) DESC"
        );

    query.addBindValue(userId);
    query.addBindValue(userType);
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec()) {
        qDebug() << "获取用户群聊失败:" << query.lastError().text();
        return groups;
    }

    while (query.next()) {
        QVariantMap group;
        group["group_id"] = query.value("group_id");
        group["group_name"] = query.value("group_name");
        group["creator_id"] = query.value("creator_id");
        group["creator_type"] = query.value("creator_type");
        group["creator_name"] = query.value("creator_name");
        group["member_count"] = query.value("member_count");
        group["user_role"] = query.value("user_role");
        group["created_time"] = query.value("created_time");
        group["last_message_time"] = query.value("last_message_time");

        // 获取最后一条消息
        QVariantMap lastMessage = getLastMessage(group["group_id"].toInt(), "群聊");
        if (!lastMessage.isEmpty()) {
            group["last_message"] = lastMessage["content"];
            group["last_sender_name"] = lastMessage["sender_name"];
        }

        groups.append(group);
    }

    qDebug() << "用户" << userId << userType << "的群聊数量:" << groups.size();
    return groups;
}

QList<QVariantMap> Database::getAllGroups()
{
    QList<QVariantMap> groups;
    QSqlQuery query;

    query.prepare(
        "SELECT gc.group_id, gc.group_name, gc.creator_id, gc.creator_type, "
        "       gc.member_count, gc.created_time, "
        "       CASE gc.creator_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as creator_name "
        "FROM group_chats gc "
        "LEFT JOIN students s ON (gc.creator_id = s.student_id AND gc.creator_type = '学生') "
        "LEFT JOIN teachers t ON (gc.creator_id = t.teacher_id AND gc.creator_type = '老师') "
        "ORDER BY gc.created_time DESC"
        );

    if (!query.exec()) {
        qDebug() << "获取所有群聊失败:" << query.lastError().text();
        return groups;
    }

    while (query.next()) {
        QVariantMap group;
        group["group_id"] = query.value("group_id");
        group["group_name"] = query.value("group_name");
        group["creator_id"] = query.value("creator_id");
        group["creator_type"] = query.value("creator_type");
        group["creator_name"] = query.value("creator_name");
        group["member_count"] = query.value("member_count");
        group["created_time"] = query.value("created_time");
        groups.append(group);
    }

    return groups;
}

bool Database::deleteGroupChat(int groupId, int userId, const QString &userType)
{
    // 检查权限：只有创建者可以删除群聊
    if (!isGroupCreator(groupId, userId, userType)) {
        qDebug() << "无权限删除群聊:" << groupId;
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM group_chats WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec()) {
        qDebug() << "删除群聊失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "成功删除群聊:" << groupId;
    return true;
}

// 成员管理（5个方法）
bool Database::addGroupMember(int groupId, int userId, const QString &userType)
{
    // 检查是否已经是成员
    if (isGroupMember(groupId, userId, userType)) {
        qDebug() << "用户已经是群成员";
        return true;
    }

    QSqlDatabase::database().transaction();

    try {
        // 添加群成员
        QSqlQuery query;
        query.prepare("INSERT INTO group_members (group_id, user_id, user_type) "
                      "VALUES (?, ?, ?)");
        query.addBindValue(groupId);
        query.addBindValue(userId);
        query.addBindValue(userType);

        if (!query.exec()) {
            throw std::runtime_error("添加群成员失败");
        }

        // 更新群聊成员数量
        query.prepare("UPDATE group_chats SET member_count = member_count + 1 "
                      "WHERE group_id = ?");
        query.addBindValue(groupId);

        if (!query.exec()) {
            throw std::runtime_error("更新群成员数量失败");
        }

        QSqlDatabase::database().commit();
        qDebug() << "成功添加群成员:" << userId << userType << "到群聊:" << groupId;
        return true;

    } catch (const std::exception &e) {
        QSqlDatabase::database().rollback();
        qDebug() << "添加群成员失败:" << e.what();
        return false;
    }
}

bool Database::removeGroupMember(int groupId, int userId, const QString &userType)
{
    // 检查是否为群成员
    if (!isGroupMember(groupId, userId, userType)) {
        qDebug() << "用户不是群成员";
        return false;
    }

    // 检查是否为创建者（创建者不能被移除，只能删除群聊）
    if (isGroupCreator(groupId, userId, userType)) {
        qDebug() << "不能移除群创建者";
        return false;
    }

    QSqlDatabase::database().transaction();

    try {
        // 移除群成员
        QSqlQuery query;
        query.prepare("DELETE FROM group_members WHERE group_id = ? AND user_id = ? AND user_type = ?");
        query.addBindValue(groupId);
        query.addBindValue(userId);
        query.addBindValue(userType);

        if (!query.exec()) {
            throw std::runtime_error("移除群成员失败");
        }

        // 更新群聊成员数量
        query.prepare("UPDATE group_chats SET member_count = member_count - 1 "
                      "WHERE group_id = ?");
        query.addBindValue(groupId);

        if (!query.exec()) {
            throw std::runtime_error("更新群成员数量失败");
        }

        QSqlDatabase::database().commit();
        qDebug() << "成功移除群成员:" << userId << userType << "从群聊:" << groupId;
        return true;

    } catch (const std::exception &e) {
        QSqlDatabase::database().rollback();
        qDebug() << "移除群成员失败:" << e.what();
        return false;
    }
}

QList<QVariantMap> Database::getGroupMembers(int groupId)
{
    QList<QVariantMap> members;
    QSqlQuery query;

    query.prepare(
        "SELECT gm.user_id, gm.user_type, gm.join_time, "
        "       CASE gm.user_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as user_name, "
        "       CASE gm.user_type "
        "           WHEN '学生' THEN s.college "
        "           WHEN '老师' THEN t.college "
        "       END as user_college, "
        "       CASE gm.user_type "
        "           WHEN '学生' THEN s.grade "
        "           ELSE NULL "
        "       END as user_grade, "
        "       CASE "
        "           WHEN gc.creator_id = gm.user_id AND gc.creator_type = gm.user_type THEN '创建者' "
        "           ELSE '成员' "
        "       END as role "
        "FROM group_members gm "
        "INNER JOIN group_chats gc ON gm.group_id = gc.group_id "
        "LEFT JOIN students s ON (gm.user_id = s.student_id AND gm.user_type = '学生') "
        "LEFT JOIN teachers t ON (gm.user_id = t.teacher_id AND gm.user_type = '老师') "
        "WHERE gm.group_id = ? "
        "ORDER BY "
        "  CASE WHEN gc.creator_id = gm.user_id AND gc.creator_type = gm.user_type THEN 0 ELSE 1 END, "
        "  gm.join_time ASC"
        );
    query.addBindValue(groupId);

    if (!query.exec()) {
        qDebug() << "获取群成员失败:" << query.lastError().text();
        return members;
    }

    while (query.next()) {
        QVariantMap member;
        member["user_id"] = query.value("user_id");
        member["user_type"] = query.value("user_type");
        member["user_name"] = query.value("user_name");
        member["user_college"] = query.value("user_college");
        member["user_grade"] = query.value("user_grade");
        member["role"] = query.value("role");
        member["join_time"] = query.value("join_time");
        members.append(member);
    }

    qDebug() << "群聊" << groupId << "成员数量:" << members.size();
    return members;
}

bool Database::isGroupMember(int groupId, int userId, const QString &userType)
{
    if (!QSqlDatabase::database().isOpen()) {  // 修改：使用静态方法
        return false;
    }

    QSqlQuery query;  // 修改：不指定数据库连接
    query.prepare("SELECT COUNT(*) as count FROM group_members WHERE group_id = ? AND user_id = ? AND user_type = ?");
    query.bindValue(0, groupId);
    query.bindValue(1, userId);
    query.bindValue(2, userType);

    if (query.exec() && query.next()) {
        return query.value("count").toInt() > 0;
    }

    return false;
}

int Database::getGroupMemberCount(int groupId)
{
    QSqlQuery query;
    query.prepare("SELECT member_count FROM group_chats WHERE group_id = ?");
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}

// 申请管理（4个方法）
bool Database::sendGroupRequest(int groupId, int userId, const QString &userType)
{
    // 检查是否已经是群成员
    if (isGroupMember(groupId, userId, userType)) {
        qDebug() << "用户已经是群成员，无法发送申请";
        return false;
    }

    // 检查是否已有待处理申请
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM group_requests "
                       "WHERE group_id = ? AND requester_id = ? AND requester_type = ? AND status = '申请中'");
    checkQuery.addBindValue(groupId);
    checkQuery.addBindValue(userId);
    checkQuery.addBindValue(userType);

    if (!checkQuery.exec() || !checkQuery.next()) {
        qDebug() << "检查群聊申请失败:" << checkQuery.lastError().text();
        return false;
    }

    if (checkQuery.value(0).toInt() > 0) {
        qDebug() << "已存在待处理的群聊申请";
        return false;
    }

    // 发送申请
    QSqlQuery query;
    query.prepare("INSERT INTO group_requests (group_id, requester_id, requester_type, status) "
                  "VALUES (?, ?, ?, '申请中')");
    query.addBindValue(groupId);
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec()) {
        qDebug() << "发送群聊申请失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "成功发送群聊申请";
    return true;
}

bool Database::processGroupRequest(int requestId, bool accept)
{
    QSqlDatabase::database().transaction();

    try {
        // 获取申请信息
        QSqlQuery selectQuery;
        selectQuery.prepare("SELECT group_id, requester_id, requester_type FROM group_requests "
                            "WHERE request_id = ? AND status = '申请中'");
        selectQuery.addBindValue(requestId);

        if (!selectQuery.exec() || !selectQuery.next()) {
            throw std::runtime_error("申请不存在或已处理");
        }

        int groupId = selectQuery.value("group_id").toInt();
        int requesterId = selectQuery.value("requester_id").toInt();
        QString requesterType = selectQuery.value("requester_type").toString();

        if (accept) {
            // 同意申请：添加用户到群聊
            if (!addGroupMember(groupId, requesterId, requesterType)) {
                throw std::runtime_error("添加群成员失败");
            }

            // 更新申请状态为已同意
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE group_requests SET status = '已同意' WHERE request_id = ?");
            updateQuery.addBindValue(requestId);

            if (!updateQuery.exec()) {
                throw std::runtime_error("更新申请状态失败");
            }
        } else {
            // 拒绝申请：直接删除申请记录
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM group_requests WHERE request_id = ?");
            deleteQuery.addBindValue(requestId);

            if (!deleteQuery.exec()) {
                throw std::runtime_error("删除申请记录失败");
            }
        }

        QSqlDatabase::database().commit();
        qDebug() << "成功处理群聊申请:" << (accept ? "同意" : "拒绝");
        return true;

    } catch (const std::exception &e) {
        QSqlDatabase::database().rollback();
        qDebug() << "处理群聊申请失败:" << e.what();
        return false;
    }
}

QList<QVariantMap> Database::getGroupRequests(int groupId)
{
    QList<QVariantMap> requests;
    QSqlQuery query;

    query.prepare(
        "SELECT gr.request_id, gr.requester_id, gr.requester_type, gr.request_time, "
        "       CASE gr.requester_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as requester_name, "
        "       CASE gr.requester_type "
        "           WHEN '学生' THEN s.college "
        "           WHEN '老师' THEN t.college "
        "       END as requester_college, "
        "       CASE gr.requester_type "
        "           WHEN '学生' THEN s.grade "
        "           ELSE NULL "
        "       END as requester_grade "
        "FROM group_requests gr "
        "LEFT JOIN students s ON (gr.requester_id = s.student_id AND gr.requester_type = '学生') "
        "LEFT JOIN teachers t ON (gr.requester_id = t.teacher_id AND gr.requester_type = '老师') "
        "WHERE gr.group_id = ? AND gr.status = '申请中' "
        "ORDER BY gr.request_time DESC"
        );
    query.addBindValue(groupId);

    if (!query.exec()) {
        qDebug() << "获取群聊申请失败:" << query.lastError().text();
        return requests;
    }

    while (query.next()) {
        QVariantMap request;
        request["request_id"] = query.value("request_id");
        request["requester_id"] = query.value("requester_id");
        request["requester_type"] = query.value("requester_type");
        request["requester_name"] = query.value("requester_name");
        request["requester_college"] = query.value("requester_college");
        request["requester_grade"] = query.value("requester_grade");
        request["request_time"] = query.value("request_time");
        requests.append(request);
    }

    return requests;
}

QList<QVariantMap> Database::getUserGroupRequests(int userId, const QString &userType)
{
    QList<QVariantMap> requests;
    QSqlQuery query;

    query.prepare(
        "SELECT gr.request_id, gr.group_id, gr.status, gr.request_time, "
        "       gc.group_name, gc.creator_id, gc.creator_type, "
        "       CASE gc.creator_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as creator_name "
        "FROM group_requests gr "
        "INNER JOIN group_chats gc ON gr.group_id = gc.group_id "
        "LEFT JOIN students s ON (gc.creator_id = s.student_id AND gc.creator_type = '学生') "
        "LEFT JOIN teachers t ON (gc.creator_id = t.teacher_id AND gc.creator_type = '老师') "
        "WHERE gr.requester_id = ? AND gr.requester_type = ? "
        "ORDER BY gr.request_time DESC"
        );
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec()) {
        qDebug() << "获取用户群聊申请失败:" << query.lastError().text();
        return requests;
    }

    while (query.next()) {
        QVariantMap request;
        request["request_id"] = query.value("request_id");
        request["group_id"] = query.value("group_id");
        request["group_name"] = query.value("group_name");
        request["creator_id"] = query.value("creator_id");
        request["creator_type"] = query.value("creator_type");
        request["creator_name"] = query.value("creator_name");
        request["status"] = query.value("status");
        request["request_time"] = query.value("request_time");
        requests.append(request);
    }

    return requests;
}

// 权限验证（3个方法）
bool Database::isGroupCreator(int groupId, int userId, const QString &userType)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM group_chats "
                  "WHERE group_id = ? AND creator_id = ? AND creator_type = ?");
    query.addBindValue(groupId);
    query.addBindValue(userId);
    query.addBindValue(userType);

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

bool Database::canManageGroup(int groupId, int userId, const QString &userType)
{
    // 目前只有创建者可以管理群聊
    // 未来可以扩展管理员权限
    return isGroupCreator(groupId, userId, userType);
}

QVariantMap Database::getGroupInfo(int groupId)
{
    QVariantMap groupInfo;
    QSqlQuery query;

    query.prepare(
        "SELECT gc.group_id, gc.group_name, gc.creator_id, gc.creator_type, "
        "       gc.member_count, gc.created_time, "
        "       CASE gc.creator_type "
        "           WHEN '学生' THEN s.name "
        "           WHEN '老师' THEN t.name "
        "       END as creator_name "
        "FROM group_chats gc "
        "LEFT JOIN students s ON (gc.creator_id = s.student_id AND gc.creator_type = '学生') "
        "LEFT JOIN teachers t ON (gc.creator_id = t.teacher_id AND gc.creator_type = '老师') "
        "WHERE gc.group_id = ?"
        );
    query.addBindValue(groupId);

    if (!query.exec() || !query.next()) {
        qDebug() << "获取群聊信息失败:" << query.lastError().text();
        return groupInfo;
    }

    groupInfo["group_id"] = query.value("group_id");
    groupInfo["group_name"] = query.value("group_name");
    groupInfo["creator_id"] = query.value("creator_id");
    groupInfo["creator_type"] = query.value("creator_type");
    groupInfo["creator_name"] = query.value("creator_name");
    groupInfo["member_count"] = query.value("member_count");
    groupInfo["created_time"] = query.value("created_time");

    return groupInfo;
}

bool Database::disbandGroup(int groupId, int userId, const QString &userType)
{
    qDebug() << "解散群聊:" << groupId << "操作者:" << userId << userType;

    // 修改：使用静态数据库连接，与 addGroupMember 保持一致
    if (!QSqlDatabase::database().isOpen()) {
        qWarning() << "数据库未连接";
        return false;
    }

    QSqlQuery query;  // 修改：不指定数据库连接，使用默认连接

    // 检查是否为群聊创建者
    query.prepare("SELECT creator_id, creator_type FROM group_chats WHERE group_id = ?");
    query.bindValue(0, groupId);

    if (!query.exec()) {
        qWarning() << "检查群聊创建者失败:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        qWarning() << "群聊不存在:" << groupId;
        return false;
    }

    int creatorId = query.value("creator_id").toInt();
    QString creatorType = query.value("creator_type").toString();

    if (creatorId != userId || creatorType != userType) {
        qWarning() << "只有创建者可以解散群聊";
        qWarning() << "期望创建者:" << creatorId << creatorType;
        qWarning() << "实际操作者:" << userId << userType;
        return false;
    }

    // 开始事务 - 修改：使用静态方法
    if (!QSqlDatabase::database().transaction()) {
        qWarning() << "开始事务失败";
        return false;
    }

    bool success = true;

    // 1. 删除群聊成员
    query.prepare("DELETE FROM group_members WHERE group_id = ?");
    query.bindValue(0, groupId);
    if (!query.exec()) {
        qWarning() << "删除群聊成员失败:" << query.lastError().text();
        success = false;
    }

    // 2. 删除群聊申请
    if (success) {
        query.prepare("DELETE FROM group_requests WHERE group_id = ?");
        query.bindValue(0, groupId);
        if (!query.exec()) {
            qWarning() << "删除群聊申请失败:" << query.lastError().text();
            success = false;
        }
    }

    // 3. 删除群聊消息
    if (success) {
        query.prepare("DELETE FROM messages WHERE chat_type = '群聊' AND chat_id = ?");
        query.bindValue(0, groupId);
        if (!query.exec()) {
            qWarning() << "删除群聊消息失败:" << query.lastError().text();
            success = false;
        }
    }

    // 4. 删除群聊
    if (success) {
        query.prepare("DELETE FROM group_chats WHERE group_id = ?");
        query.bindValue(0, groupId);
        if (!query.exec()) {
            qWarning() << "删除群聊失败:" << query.lastError().text();
            success = false;
        }
    }

    // 提交或回滚事务 - 修改：使用静态方法
    if (success) {
        if (QSqlDatabase::database().commit()) {
            qDebug() << "群聊解散成功:" << groupId;
            return true;
        } else {
            qWarning() << "提交事务失败:" << QSqlDatabase::database().lastError().text();
        }
    }

    QSqlDatabase::database().rollback();
    return false;
}

bool Database::leaveGroup(int groupId, int userId, const QString &userType)
{
    qDebug() << "退出群聊:" << groupId << "用户:" << userId << userType;

    // 修改：使用静态数据库连接，与 addGroupMember 保持一致
    if (!QSqlDatabase::database().isOpen()) {
        qWarning() << "数据库未连接";
        return false;
    }

    QSqlQuery query;  // 修改：不指定数据库连接，使用默认连接

    // 检查是否为群聊创建者
    query.prepare("SELECT creator_id, creator_type FROM group_chats WHERE group_id = ?");
    query.bindValue(0, groupId);

    if (!query.exec()) {
        qWarning() << "检查群聊信息失败:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        qWarning() << "群聊不存在:" << groupId;
        return false;
    }

    int creatorId = query.value("creator_id").toInt();
    QString creatorType = query.value("creator_type").toString();

    if (creatorId == userId && creatorType == userType) {
        qWarning() << "创建者不能退出群聊，只能解散群聊";
        qWarning() << "当前用户:" << userId << userType;
        qWarning() << "群聊创建者:" << creatorId << creatorType;
        return false;
    }

    // 检查是否为群聊成员
    query.prepare("SELECT COUNT(*) as count FROM group_members WHERE group_id = ? AND user_id = ? AND user_type = ?");
    query.bindValue(0, groupId);
    query.bindValue(1, userId);
    query.bindValue(2, userType);

    if (!query.exec()) {
        qWarning() << "检查群聊成员失败:" << query.lastError().text();
        return false;
    }

    if (!query.next() || query.value("count").toInt() == 0) {
        qWarning() << "用户不是群聊成员";
        return false;
    }

    // 开始事务 - 修改：使用静态方法
    if (!QSqlDatabase::database().transaction()) {
        qWarning() << "开始事务失败";
        return false;
    }

    bool success = true;

    // 1. 从群聊成员中移除
    query.prepare("DELETE FROM group_members WHERE group_id = ? AND user_id = ? AND user_type = ?");
    query.bindValue(0, groupId);
    query.bindValue(1, userId);
    query.bindValue(2, userType);

    if (!query.exec()) {
        qWarning() << "移除群聊成员失败:" << query.lastError().text();
        success = false;
    }

    // 2. 更新群聊成员数量
    if (success) {
        query.prepare("UPDATE group_chats SET member_count = member_count - 1 WHERE group_id = ?");
        query.bindValue(0, groupId);
        if (!query.exec()) {
            qWarning() << "更新群聊成员数量失败:" << query.lastError().text();
            success = false;
        }
    }

    // 提交或回滚事务 - 修改：使用静态方法
    if (success) {
        if (QSqlDatabase::database().commit()) {
            qDebug() << "退出群聊成功:" << groupId;
            return true;
        } else {
            qWarning() << "提交事务失败:" << QSqlDatabase::database().lastError().text();
        }
    }

    QSqlDatabase::database().rollback();
    return false;
}
