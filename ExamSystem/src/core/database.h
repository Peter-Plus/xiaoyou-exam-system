#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>
#include <QCryPtographicHash>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QDate>
#include <QTime>
#include <QDateTime>

class Question;//前向声明
class Exam;//前向声明
class Course;//前向声明
class Teacher;//前向声明
class StudentAnswer;//前向声明

class Database
{
public:
    Database();
    bool connectToDatabase();
    void createTables();
    void closeDatabase();

    //================用户管理相关方法================
    QString hashPassword(const QString& password);
    bool registerStudent(int studentId,const QString& name,const QString& grade,const QString& college,const QString& password);
    bool registerTeacher(int teacherId,const QString& name,const QString& college,const QString& password);
    bool authenticateStudent(int studentId,const QString& password);
    bool authenticateTeacher(int teacherId,const QString& password);
    bool isStudentRegistered(int studentId);
    bool isTeacherRegistered(int teacherId);

    //===============试题管理相关方法=================
    bool addQuestion(const QString &type, const QString &courseName, int score, const QString &content, const QString &answer, int creatorId, bool isPublic = true);
    bool updateQuestion(int questionId, const QString &type, const QString &courseName,
                        int score, const QString &content, const QString &answer,
                        int modifierId, bool isPublic);
    bool deleteQuestion(int questionId, int teacherId);
    QList<Question> getQuestionsByTeacher(int teacherId);
    QList<Question> getPublicQuestions(const QString &courseName = "");
    QList<Question> getFavoriteQuestions(int teacherId);
    QList<Question> getAllQuestions(); // 获取所有题目（管理员用）
    Question getQuestionById(int questionId);//单个题目查询
    //权限检查
    bool canModifyQuestion(int questionId,int teacherId);
    bool canDeleteQuestion(int questionId,int teacherId);
    //收藏相关
    bool addToFavorites(int questionId,int teacherId);
    bool removeFromFavorites(int questionId,int teacherId);
    bool isQuestionFavorited(int questionId,int teacherId);
    //其他统计和辅助方法
    QStringList getCourseNames();
    QStringList getQuestionTypes();
    QMap<QString,int> getQuestionStatsByType(int teacherId);
    QMap<QString,int> getQuestionStatsByCourse(int teacherId);
    //搜索相关
    QList<Question> searchQuestions(const QString &keyword, int teacherId = -1,
                                    const QString &courseName = "",
                                    const QString &questionType = "");
    //考频更新
    bool incrementQuestionFrequency(int questionId);

    //===============考试管理相关方法================================
    //考试管理相关方法
    // 考试CRUD操作
    bool addExam(const QString &examName, int courseId, const QDate &examDate,
                 const QTime &startTime, const QTime &endTime, int totalScore, int creatorId);
    bool updateExam(int examId, const QString &examName, const QDate &examDate,
                    const QTime &startTime, const QTime &endTime, int totalScore);
    bool deleteExam(int examId, int teacherId); // 检查权限
    QList<Exam> getExamsByTeacher(int teacherId); // 获取教师相关的考试
    QList<Exam> getExamsByCourse(int courseId); // 按课程获取考试

    // 考试状态管理
    QString getExamStatus(int examId); // 计算当前考试状态
    bool publishExam(int examId); // 发布考试
    bool terminateExam(int examId); // 终止考试
    QList<Exam> getExamsByStatus(int teacherId, const QString &status);

    // 组卷相关
    bool addQuestionToExam(int examId, int questionId, int questionOrder);
    bool removeQuestionFromExam(int examId, int questionId);
    bool updateQuestionOrder(int examId, int questionId, int newOrder);
    QList<Question> getExamQuestions(int examId); // 获取试卷题目
    int calculateExamTotalScore(int examId); // 计算试卷总分
    bool updateExamCompletion(int examId, bool isCompleted);

    // 权限管理
    bool addExamPermission(int examId, int teacherId, bool isCreator = false);
    bool removeExamPermission(int examId, int teacherId);
    QList<Teacher> getExamPermissions(int examId); // 获取有权限的教师
    bool hasExamPermission(int examId, int teacherId);
    bool canModifyExam(int examId, int teacherId);

    // 课程相关
    QList<Course> getTeacherCourses(int teacherId); // 获取教师课程列表
    QList<Course> getAllCourses();

    //===============学生考试答题和阅卷相关方法================================
    // 学生考试相关
    QList<Exam> getAvailableExamsForStudent(int studentId); // 获取学生可参与的考试
    QList<Exam> getStudentExamHistory(int studentId); // 获取学生考试历史
    bool canStudentTakeExam(int studentId, int examId); // 检查学生是否可以参与考试
    bool hasStudentStartedExam(int studentId, int examId); // 检查是否已开始答题
    bool hasStudentSubmittedExam(int studentId, int examId); // 检查学生是否已提交考试
    bool isStudentExamFullyGraded(int examId, int studentId); // 检查学生考试是否完全批改
    bool hasStudentAnsweredAllQuestions(int studentId, int examId);
    int getStudentAnswerCount(int studentId, int examId);

    // 答题记录管理
    bool saveStudentAnswer(int examId, int questionId, int studentId,
                           const QString &answer); // 保存学生答案
    QString getStudentAnswer(int examId, int questionId, int studentId); // 获取已答题目
    QList<StudentAnswer> getStudentAnswers(int examId, int studentId); // 获取学生所有答案
    bool submitExam(int examId, int studentId); // 提交考试
    QDateTime getStudentSubmitTime(int examId, int studentId); // 获取提交时间

    // 自动批改
    bool autoGradeObjectiveQuestions(int examId, int studentId); // 自动批改客观题
    double calculateQuestionScore(const QString &studentAnswer,
                                  const QString &correctAnswer,
                                  const QString &questionType, int fullScore); // 计算题目得分

    // 阅卷相关
    QList<Exam> getExamsForGrading(int teacherId); // 获取待阅卷考试
    QList<StudentAnswer> getAnswersForGrading(int examId, const QString &questionType = ""); // 获取待批改答案
    bool updateAnswerScore(int examId, int questionId, int studentId,
                           double score, bool isGraded = true); // 更新答案分数
    bool isExamFullyGraded(int examId); // 检查考试是否完全批改完成
    QMap<QString, int> getGradingProgress(int examId); // 获取阅卷进度

    // 统计相关
    double getStudentExamTotalScore(int examId, int studentId); // 学生考试总分
    QMap<QString, QVariant> getExamStatistics(int examId); // 考试统计信息
    Exam getExamById(int examId);



    //===============班级成绩统计相关方法================================
    // 班级统计核心方法
    QMap<QString, QVariant> getClassStatistics(int examId); // 获取班级统计数据
    QList<QMap<QString, QVariant>> getFailingStudents(int examId); // 获取不及格学生名单
    QList<QMap<QString, QVariant>> getExamStudentScores(int examId); // 获取考试所有学生成绩








    // ============================================================================
    // 好友管理功能
    // ============================================================================

    // 好友关系管理
    bool addFriendship(int user1Id, const QString &user1Type, int user2Id, const QString &user2Type);
    bool removeFriendship(int user1Id, const QString &user1Type, int user2Id, const QString &user2Type);
    bool areFriends(int user1Id, const QString &user1Type, int user2Id, const QString &user2Type);
    QList<QVariantMap> getFriendsList(int userId, const QString &userType);

    // 好友申请管理
    bool sendFriendRequest(int requesterId, const QString &requesterType, int targetId, const QString &targetType);
    bool acceptFriendRequest(int requestId);
    bool rejectFriendRequest(int requestId);
    QList<QVariantMap> getReceivedFriendRequests(int userId, const QString &userType);
    QList<QVariantMap> getSentFriendRequests(int userId, const QString &userType);
    bool hasPendingFriendRequest(int requesterId, const QString &requesterType, int targetId, const QString &targetType);

    // 用户搜索功能
    QList<QVariantMap> searchUsersByKeyword(const QString &keyword, int currentUserId, const QString &currentUserType);
    QList<QVariantMap> searchUsersById(int userId, int currentUserId, const QString &currentUserType);
    QList<QVariantMap> getClassmates(int studentId);
    QList<QVariantMap> getColleagues(int teacherId);

    // 统计信息
    int getFriendCount(int userId, const QString &userType);
    int getPendingRequestCount(int userId, const QString &userType);






    // ============================================================================
    // 聊天功能
    // ============================================================================

    // 私聊关系管理（4个方法）
    int getOrCreatePrivateChat(int user1Id, const QString &user1Type,
                               int user2Id, const QString &user2Type);
    QList<QVariantMap> getPrivateChats(int userId, const QString &userType);
    bool updatePrivateChatLastMessage(int chatId);
    bool canChat(int user1Id, const QString &user1Type,
                 int user2Id, const QString &user2Type);

    // 消息管理（6个方法）
    int sendMessage(int chatId, const QString &chatType,
                    int senderId, const QString &senderType,
                    const QString &content);
    QList<QVariantMap> getChatMessages(int chatId, const QString &chatType,
                                       int limit = 50, int offset = 0);
    QVariantMap getLastMessage(int chatId, const QString &chatType);
    bool deleteMessage(int messageId);
    int getUnreadMessageCount(int userId, const QString &userType);
    bool markMessagesAsRead(int chatId, int userId, const QString &userType);


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

    // 群聊操作方法
    bool disbandGroup(int groupId, int userId, const QString &userType);
    bool leaveGroup(int groupId, int userId, const QString &userType);



    // ============================================================================
    // 课程管理功能（阶段6新增）
    // ============================================================================

    // 选课管理（5个方法）
    bool applyForCourse(int studentId, int courseId);                      // 学生申请选课
    bool approveEnrollment(int studentId, int courseId);                   // 选课管理员审核通过
    bool rejectEnrollment(int studentId, int courseId);                    // 选课管理员拒绝申请
    QList<QVariantMap> getPendingEnrollments();                           // 获取待审核选课申请
    QList<QVariantMap> getStudentCourses(int studentId);                  // 获取学生选课列表

    // 课程通知管理（4个方法）
    int publishCourseNotice(int courseId, const QString &title,
                            const QString &content, bool isPinned = false); // 发布课程通知
    bool updateCourseNotice(int noticeId, const QString &title,
                            const QString &content, bool isPinned);         // 更新通知
    bool deleteCourseNotice(int noticeId);                                 // 删除通知
    QList<QVariantMap> getCourseNotices(int courseId);                    // 获取课程通知列表

    // 作业管理（6个方法）
    int publishAssignment(int courseId, const QString &title,
                          const QString &description, const QDateTime &deadline,
                          int maxScore = 100);                              // 发布作业
    bool updateAssignment(int assignmentId, const QString &title,
                          const QString &description, const QDateTime &deadline,
                          int maxScore);                                     // 更新作业
    bool closeAssignment(int assignmentId);                               // 关闭作业提交
    QList<QVariantMap> getCourseAssignments(int courseId);               // 获取课程作业列表
    QList<QVariantMap> getStudentAssignments(int studentId);             // 获取学生作业列表
    bool deleteAssignment(int assignmentId);                             // 删除作业

    // 作业提交管理（5个方法）
    bool submitAssignment(int assignmentId, int studentId,
                          const QString &content);                         // 学生提交作业
    bool gradeAssignment(int assignmentId, int studentId,
                         double score, const QString &feedback = "");      // 教师批改作业
    QList<QVariantMap> getAssignmentSubmissions(int assignmentId);       // 获取作业提交列表
    QVariantMap getStudentSubmission(int assignmentId, int studentId);   // 获取学生提交详情
    bool hasSubmittedAssignment(int assignmentId, int studentId);        // 检查是否已提交


    // 权限验证（3个方法）
    bool isTeacherCourseAdmin(int teacherId);                                          // 检查是否为选课管理员
    bool canManageCourse(int teacherId, int courseId);                    // 检查课程管理权限
    bool isStudentEnrolled(int studentId, int courseId);                 // 检查学生是否已选课

    // 作业批改辅助方法（新增）
    int getAssignmentMaxScore(int assignmentId);                           // 获取作业最大分数
    QString getAssignmentTitle(int assignmentId);                          // 获取作业标题
    bool canTeacherGradeAssignment(int teacherId, int assignmentId);      // 检查批改权限
    QVariantMap getAssignmentStats(int assignmentId);                     // 获取作业统计信息


    QList<QVariantMap> getCourseMembers(int courseId);                    // 获取课程所有学生成员
    // 获取课程扩展信息
    QVariantMap getCourseExtendedInfo(int courseId);                      // 获取课程完整信息（包含扩展字段）
    // 更新课程信息（教师专用）
    bool updateCourseInfo(int courseId, const QString &courseName,        // 更新课程基本信息
                          const QString &description, int credits,
                          int courseHours, const QString &semester,
                          int maxStudents, const QString &status);

    // 获取课程统计信息
    QVariantMap getCourseStats(int courseId);                            // 获取课程统计数据

private:
    QSqlDatabase db;

    //辅助方法
    Question createQuestionFromQuery(QSqlQuery &query);
    QString buildSearchCondition(const QString &keyword, int teacherId,
                                 const QString &courseName, const QString &questionType);
    Exam createExamFromQuery(QSqlQuery &query);
    Course createCourseFromQuery(QSqlQuery &query);
    Teacher createTeacherFromQuery(QSqlQuery &query);
    StudentAnswer createStudentAnswerFromQuery(QSqlQuery &query);
};


#endif // DATABASE_H
