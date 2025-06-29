#include "database.h"
#include "question.h"
#include "exam.h"
#include "course.h"
#include "teacher.h"
#include "studentanswer.h"


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
