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
    // 选课管理相关方法 (8个方法)
    // ============================================================================

    /**
     * @brief 获取选课申请列表
     * @param adminId 选课管理员ID（为0时获取所有申请）
     * @return 选课申请列表，包含申请者、课程、状态等信息
     */
    QList<QVariantMap> getEnrollmentRequests(int adminId = 0);

    /**
     * @brief 处理选课申请
     * @param requestId 申请记录的主键（student_id + course_id）
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @param approved 是否批准（true=通过，false=拒绝）
     * @param adminId 处理的管理员ID
     * @return 是否处理成功
     */
    bool processEnrollmentRequest(int studentId, int courseId, bool approved, int adminId);

    /**
     * @brief 学生提交选课申请
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @return 是否提交成功
     */
    bool submitEnrollmentRequest(int studentId, int courseId);

    /**
     * @brief 获取学生已选课程列表
     * @param studentId 学生ID
     * @param includeApplying 是否包含申请中的课程
     * @return 课程列表，包含课程详细信息和选课状态
     */
    QList<QVariantMap> getCoursesByStudent(int studentId, bool includeApplying = false);

    /**
     * @brief 获取教师教授的课程列表
     * @param teacherId 教师ID
     * @return 课程列表，包含课程详细信息和学生数量
     */
    QList<QVariantMap> getCoursesByTeacher(int teacherId);

    /**
     * @brief 获取可选课程列表（学生用）
     * @param studentId 学生ID
     * @return 可选课程列表，排除已选和申请中的课程
     */
    QList<QVariantMap> getAvailableCourses(int studentId);

    /**
     * @brief 检查学生是否已选择或申请某课程
     * @param studentId 学生ID
     * @param courseId 课程ID
     * @return 选课状态：0=未选，1=已通过，2=申请中
     */
    int getEnrollmentStatus(int studentId, int courseId);

    /**
     * @brief 获取选课统计信息
     * @param courseId 课程ID（为0时获取总体统计）
     * @return 统计信息：申请中数量、已通过数量等
     */
    QVariantMap getEnrollmentStats(int courseId = 0);

    // ============================================================================
    // 课程通知相关方法 (6个方法)
    // ============================================================================

    /**
     * @brief 发布课程通知
     * @param courseId 课程ID
     * @param title 通知标题
     * @param content 通知内容
     * @param isPinned 是否置顶
     * @param teacherId 发布教师ID
     * @return 是否发布成功
     */
    bool addCourseNotice(int courseId, const QString &title, const QString &content,
                         bool isPinned = false, int teacherId = 0);

    /**
     * @brief 获取课程通知列表
     * @param courseId 课程ID
     * @param studentId 学生ID（用于权限检查，为0表示教师查询）
     * @return 通知列表，置顶通知在前
     */
    QList<QVariantMap> getCourseNotices(int courseId, int studentId = 0);

    /**
     * @brief 获取学生的所有课程通知
     * @param studentId 学生ID
     * @param limit 限制数量（最新N条）
     * @return 所有已选课程的通知列表
     */
    QList<QVariantMap> getStudentAllNotices(int studentId, int limit = 50);

    /**
     * @brief 更新课程通知
     * @param noticeId 通知ID
     * @param title 新标题
     * @param content 新内容
     * @param isPinned 是否置顶
     * @param teacherId 修改者ID（权限检查）
     * @return 是否更新成功
     */
    bool updateCourseNotice(int noticeId, const QString &title, const QString &content,
                            bool isPinned, int teacherId);

    /**
     * @brief 删除课程通知
     * @param noticeId 通知ID
     * @param teacherId 删除者ID（权限检查）
     * @return 是否删除成功
     */
    bool deleteCourseNotice(int noticeId, int teacherId);

    /**
     * @brief 检查教师是否有课程通知权限
     * @param teacherId 教师ID
     * @param courseId 课程ID
     * @return 是否有权限
     */
    bool canManageCourseNotices(int teacherId, int courseId);

    // ============================================================================
    // 作业管理相关方法 (10个方法)
    // ============================================================================

    /**
     * @brief 发布课程作业
     * @param courseId 课程ID
     * @param title 作业标题
     * @param description 作业描述
     * @param deadline 截止时间
     * @param maxScore 最大分数
     * @param teacherId 发布教师ID
     * @return 是否发布成功，返回作业ID（失败返回-1）
     */
    int addAssignment(int courseId, const QString &title, const QString &description,
                      const QDateTime &deadline, int maxScore, int teacherId);

    /**
     * @brief 获取课程作业列表
     * @param courseId 课程ID
     * @param studentId 学生ID（用于获取提交状态，为0表示教师查询）
     * @return 作业列表，包含作业信息和提交状态
     */
    QList<QVariantMap> getAssignments(int courseId, int studentId = 0);

    /**
     * @brief 获取学生的所有作业
     * @param studentId 学生ID
     * @param includeSubmitted 是否包含已提交的作业
     * @return 所有已选课程的作业列表
     */
    QList<QVariantMap> getStudentAllAssignments(int studentId, bool includeSubmitted = true);

    /**
     * @brief 学生提交作业
     * @param assignmentId 作业ID
     * @param studentId 学生ID
     * @param content 作业内容
     * @return 是否提交成功
     */
    bool submitAssignment(int assignmentId, int studentId, const QString &content);

    /**
     * @brief 教师批改作业
     * @param assignmentId 作业ID
     * @param studentId 学生ID
     * @param score 给分
     * @param feedback 评语
     * @param teacherId 批改教师ID
     * @return 是否批改成功
     */
    bool gradeAssignment(int assignmentId, int studentId, double score,
                         const QString &feedback, int teacherId);

    /**
     * @brief 获取作业提交记录
     * @param assignmentId 作业ID
     * @param teacherId 教师ID（权限检查）
     * @return 提交记录列表，包含学生信息和提交状态
     */
    QList<QVariantMap> getSubmissions(int assignmentId, int teacherId = 0);

    /**
     * @brief 获取学生的作业提交详情
     * @param assignmentId 作业ID
     * @param studentId 学生ID
     * @return 提交详情，包含内容、分数、评语等
     */
    QVariantMap getSubmissionDetail(int assignmentId, int studentId);

    /**
     * @brief 检查作业是否可以提交
     * @param assignmentId 作业ID
     * @param studentId 学生ID
     * @return 是否可以提交（未截止且学生已选课程）
     */
    bool canSubmitAssignment(int assignmentId, int studentId);

    /**
     * @brief 获取作业统计信息
     * @param assignmentId 作业ID
     * @return 统计信息：总人数、已提交、已批改、平均分等
     */
    QVariantMap getAssignmentStats(int assignmentId);

    /**
     * @brief 检查教师是否有作业管理权限
     * @param teacherId 教师ID
     * @param courseId 课程ID
     * @return 是否有权限
     */
    bool canManageAssignments(int teacherId, int courseId);

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
