USE exam_system;

-- ============================================================================
-- 在线考试系统 2.0 版本数据库建表脚本
-- 包含 1.0 版本的 10 张表（扩展） + 2.0 版本新增的 10 张表
-- ============================================================================

-- 1. 学生表（保持1.0结构，改为手动设置student_id）
CREATE TABLE students(
    student_id INT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    grade VARCHAR(20) NOT NULL,
    college VARCHAR(100) NOT NULL,
    password VARCHAR(255) DEFAULT NULL
);

-- 2. 教师表（扩展：添加选课管理员标识）
CREATE TABLE teachers(
    teacher_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    college VARCHAR(100) NOT NULL,
    password VARCHAR(255) DEFAULT NULL,
    is_course_admin BOOLEAN DEFAULT FALSE COMMENT '是否为选课管理员'
);

-- 3. 课程表（大幅扩展：添加课程详细信息）
CREATE TABLE courses(
    course_id INT AUTO_INCREMENT PRIMARY KEY,
    teacher_id INT NOT NULL,
    college VARCHAR(100) NOT NULL,
    course_name VARCHAR(200) NOT NULL,
    description TEXT COMMENT '课程描述',
    credits INT DEFAULT 3 COMMENT '学分',
    course_hours INT DEFAULT 48 COMMENT '课时',
    semester VARCHAR(50) DEFAULT '2024-2025学年第一学期' COMMENT '开课学期',
    max_students INT DEFAULT 50 COMMENT '最大选课人数',
    status ENUM('开放选课', '关闭选课', '已结束') DEFAULT '开放选课' COMMENT '课程状态',
    FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE,
    INDEX idx_status (status),
    INDEX idx_semester (semester)
);

-- 4. 选课表（扩展：添加选课状态）
CREATE TABLE student_courses(
    student_id INT NOT NULL,
    course_id INT NOT NULL,
    exam_score DECIMAL(5,2) DEFAULT NULL,
    enrollment_status ENUM('申请中', '已通过') DEFAULT '已通过' COMMENT '选课状态',
    enrollment_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '选课时间',
    PRIMARY KEY (student_id, course_id),
    FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE,
    FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE,
    INDEX idx_enrollment_status (enrollment_status)
);

-- 5. 题目表（保持1.0结构不变）
CREATE TABLE questions(
    question_id INT AUTO_INCREMENT PRIMARY KEY,
    question_type VARCHAR(40) NOT NULL,
    course_name VARCHAR(200) NOT NULL,
    score INT NOT NULL,
    content TEXT NOT NULL,
    answer TEXT NOT NULL,
    creator_id INT NOT NULL,
    modifier_id INT NOT NULL,
    frequency INT DEFAULT 0,
    is_public BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (creator_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE,
    FOREIGN KEY (modifier_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE,
    INDEX idx_course_name (course_name),
    INDEX idx_question_type (question_type),
    INDEX idx_is_public (is_public)
);

-- 6. 收藏题库表（保持1.0结构不变）
CREATE TABLE question_favorites(
    question_id INT NOT NULL,
    teacher_id INT NOT NULL,
    PRIMARY KEY (question_id, teacher_id),
    FOREIGN KEY (question_id) REFERENCES questions(question_id) ON DELETE CASCADE,
    FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE
);

-- 7. 考试表（保持1.0结构不变）
CREATE TABLE exams(
    exam_id INT AUTO_INCREMENT PRIMARY KEY,
    exam_name VARCHAR(200) NOT NULL,
    course_id INT NOT NULL,
    exam_date DATE NOT NULL,
    start_time TIME NOT NULL,
    end_time TIME NOT NULL,
    total_score INT NOT NULL,
    is_published BOOLEAN DEFAULT FALSE,
    is_completed BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE,
    INDEX idx_exam_date (exam_date),
    INDEX idx_is_published (is_published),
    INDEX idx_is_completed (is_completed)
);

-- 8. 试题组卷关联表（保持1.0结构不变）
CREATE TABLE exam_questions(
    exam_id INT NOT NULL,
    question_id INT NOT NULL,
    question_order INT NOT NULL,
    PRIMARY KEY (exam_id, question_id),
    FOREIGN KEY (exam_id) REFERENCES exams(exam_id) ON DELETE CASCADE,
    FOREIGN KEY (question_id) REFERENCES questions(question_id) ON DELETE CASCADE,
    INDEX idx_question_order (question_order)
);

-- 9. 试卷权限表（保持1.0结构不变）
CREATE TABLE exam_permissions(
    exam_id INT NOT NULL,
    teacher_id INT NOT NULL,
    is_creator BOOLEAN DEFAULT FALSE,
    PRIMARY KEY (exam_id, teacher_id),
    FOREIGN KEY (exam_id) REFERENCES exams(exam_id) ON DELETE CASCADE,
    FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE
);

-- 10. 学生答题记录表（保持1.0结构不变）
CREATE TABLE student_answers(
    exam_id INT NOT NULL,
    question_id INT NOT NULL,
    student_id INT NOT NULL,
    student_answer TEXT DEFAULT NULL,
    score DECIMAL(5,2) DEFAULT 0,
    is_graded BOOLEAN DEFAULT FALSE,
    PRIMARY KEY (exam_id, question_id, student_id),
    FOREIGN KEY (exam_id) REFERENCES exams(exam_id) ON DELETE CASCADE,
    FOREIGN KEY (question_id) REFERENCES questions(question_id) ON DELETE CASCADE,
    FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE,
    INDEX idx_is_graded (is_graded)
);

-- ============================================================================
-- 2.0 版本新增表（好友功能）
-- ============================================================================

-- 11. 好友关系表
CREATE TABLE friend_relationships(
    friendship_id INT AUTO_INCREMENT PRIMARY KEY,
    user1_id INT NOT NULL COMMENT '用户1ID',
    user1_type ENUM('学生', '老师') NOT NULL COMMENT '用户1类型',
    user2_id INT NOT NULL COMMENT '用户2ID', 
    user2_type ENUM('学生', '老师') NOT NULL COMMENT '用户2类型',
    created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '建立友谊时间',
    UNIQUE INDEX idx_friendship (user1_id, user1_type, user2_id, user2_type),
    INDEX idx_user1 (user1_id, user1_type),
    INDEX idx_user2 (user2_id, user2_type)
);

-- 12. 好友申请表
CREATE TABLE friend_requests(
    request_id INT AUTO_INCREMENT PRIMARY KEY,
    requester_id INT NOT NULL COMMENT '申请者ID',
    requester_type ENUM('学生', '老师') NOT NULL COMMENT '申请者类型',
    target_id INT NOT NULL COMMENT '被申请者ID',
    target_type ENUM('学生', '老师') NOT NULL COMMENT '被申请者类型',
    status ENUM('申请中', '已同意') DEFAULT '申请中' COMMENT '申请状态',
    request_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '申请时间',
    INDEX idx_requester (requester_id, requester_type),
    INDEX idx_target (target_id, target_type),
    INDEX idx_status (status),
    UNIQUE INDEX idx_unique_request (requester_id, requester_type, target_id, target_type)
);

-- ============================================================================
-- 2.0 版本新增表（聊天系统）
-- ============================================================================

-- 13. 私聊关系表
CREATE TABLE private_chats(
    chat_id INT AUTO_INCREMENT PRIMARY KEY,
    user1_id INT NOT NULL COMMENT '用户1ID',
    user1_type ENUM('学生', '老师') NOT NULL COMMENT '用户1类型',
    user2_id INT NOT NULL COMMENT '用户2ID',
    user2_type ENUM('学生', '老师') NOT NULL COMMENT '用户2类型',
    created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '私聊创建时间',
    last_message_time TIMESTAMP NULL COMMENT '最后消息时间',
    UNIQUE INDEX idx_private_chat (user1_id, user1_type, user2_id, user2_type),
    INDEX idx_user1 (user1_id, user1_type),
    INDEX idx_user2 (user2_id, user2_type)
);

-- 14. 群聊表
CREATE TABLE group_chats(
    group_id INT AUTO_INCREMENT PRIMARY KEY,
    group_name VARCHAR(100) NOT NULL COMMENT '群聊名称',
    creator_id INT NOT NULL COMMENT '创建者ID',
    creator_type ENUM('学生', '老师') NOT NULL COMMENT '创建者类型',
    created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    last_message_time TIMESTAMP NULL COMMENT '最后消息时间',
    member_count INT DEFAULT 1 COMMENT '成员数量',
    INDEX idx_creator (creator_id, creator_type),
    INDEX idx_group_name (group_name)
);

-- 15. 群聊成员表
CREATE TABLE group_members(
    group_id INT NOT NULL,
    user_id INT NOT NULL COMMENT '用户ID',
    user_type ENUM('学生', '老师') NOT NULL COMMENT '用户类型',
    join_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '加入时间',
    PRIMARY KEY (group_id, user_id, user_type),
    FOREIGN KEY (group_id) REFERENCES group_chats(group_id) ON DELETE CASCADE,
    INDEX idx_user (user_id, user_type)
);

-- 16. 统一消息表
CREATE TABLE messages(
    message_id INT AUTO_INCREMENT PRIMARY KEY,
    chat_type ENUM('私聊', '群聊') NOT NULL COMMENT '聊天类型',
    chat_id INT NOT NULL COMMENT '聊天ID',
    sender_id INT NOT NULL COMMENT '发送方ID',
    sender_type ENUM('学生', '老师') NOT NULL COMMENT '发送方类型',
    content TEXT NOT NULL COMMENT '消息内容',
    send_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '发送时间',
    INDEX idx_chat (chat_type, chat_id),
    INDEX idx_sender (sender_id, sender_type),
    INDEX idx_send_time (send_time)
);

-- 17. 群聊申请表
CREATE TABLE group_requests(
    request_id INT AUTO_INCREMENT PRIMARY KEY,
    group_id INT NOT NULL COMMENT '群聊ID',
    requester_id INT NOT NULL COMMENT '申请者ID',
    requester_type ENUM('学生', '老师') NOT NULL COMMENT '申请者类型',
    status ENUM('申请中', '已同意') DEFAULT '申请中' COMMENT '申请状态',
    request_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '申请时间',
    FOREIGN KEY (group_id) REFERENCES group_chats(group_id) ON DELETE CASCADE,
    INDEX idx_group (group_id),
    INDEX idx_requester (requester_id, requester_type),
    INDEX idx_status (status),
    UNIQUE INDEX idx_unique_group_request (group_id, requester_id, requester_type)
);

-- ============================================================================
-- 2.0 版本新增表（课程管理）
-- ============================================================================

-- 18. 课程通知表
CREATE TABLE course_notices(
    notice_id INT AUTO_INCREMENT PRIMARY KEY,
    course_id INT NOT NULL COMMENT '课程ID',
    title VARCHAR(200) NOT NULL COMMENT '通知标题',
    content TEXT NOT NULL COMMENT '通知内容',
    publish_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '发布时间',
    is_pinned BOOLEAN DEFAULT FALSE COMMENT '是否置顶',
    FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE,
    INDEX idx_course (course_id),
    INDEX idx_publish_time (publish_time),
    INDEX idx_is_pinned (is_pinned)
);

-- 19. 课程作业表
CREATE TABLE course_assignments(
    assignment_id INT AUTO_INCREMENT PRIMARY KEY,
    course_id INT NOT NULL COMMENT '课程ID',
    title VARCHAR(200) NOT NULL COMMENT '作业标题',
    description TEXT NOT NULL COMMENT '作业描述',
    publish_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '发布时间',
    deadline TIMESTAMP NOT NULL COMMENT '截止时间',
    max_score INT DEFAULT 100 COMMENT '最大分数',
    status ENUM('开放提交', '已截止') DEFAULT '开放提交' COMMENT '作业状态',
    FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE,
    INDEX idx_course (course_id),
    INDEX idx_deadline (deadline),
    INDEX idx_status (status)
);

-- 20. 作业提交表
CREATE TABLE assignment_submissions(
    assignment_id INT NOT NULL,
    student_id INT NOT NULL,
    content TEXT NOT NULL COMMENT '提交内容',
    submit_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '提交时间',
    score DECIMAL(5,2) DEFAULT NULL COMMENT '批改分数',
    feedback TEXT DEFAULT NULL COMMENT '批改评语',
    grade_time TIMESTAMP NULL COMMENT '批改时间',
    status ENUM('已提交', '已批改', '逾期提交') DEFAULT '已提交' COMMENT '提交状态',
    PRIMARY KEY (assignment_id, student_id),
    FOREIGN KEY (assignment_id) REFERENCES course_assignments(assignment_id) ON DELETE CASCADE,
    FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE,
    INDEX idx_submit_time (submit_time),
    INDEX idx_status (status)
);

-- ============================================================================
-- 重建有用的视图
-- ============================================================================

-- 视图1：学生考试成绩汇总（保持1.0版本不变）
CREATE VIEW student_exam_scores AS
SELECT
    sa.exam_id,
    sa.student_id,
    s.name as student_name,
    e.exam_name,
    c.course_name,
    SUM(sa.score) as total_score,
    e.total_score as max_score,
    ROUND(SUM(sa.score)/e.total_score * 100,2) as percentage,
    COUNT(sa.question_id) as answered_questions,
    SUM(CASE WHEN sa.is_graded=TRUE THEN 1 ELSE 0 END) as graded_questions
FROM student_answers sa
JOIN students s ON sa.student_id = s.student_id
JOIN exams e ON sa.exam_id = e.exam_id
JOIN courses c ON e.course_id = c.course_id
GROUP BY sa.exam_id, sa.student_id;

-- 视图2：教师题目统计（保持1.0版本不变）
CREATE VIEW teacher_question_stats AS
SELECT 
    t.teacher_id,
    t.name as teacher_name,
    COUNT(DISTINCT q.question_id) as total_questions,
    COUNT(DISTINCT CASE WHEN q.is_public=TRUE THEN q.question_id END) as public_questions,
    COUNT(DISTINCT qf.question_id) as favorited_questions,
    AVG(q.frequency) as avg_frequency
FROM teachers t
LEFT JOIN questions q ON t.teacher_id = q.creator_id
LEFT JOIN question_favorites qf ON t.teacher_id = qf.teacher_id
GROUP BY t.teacher_id, t.name;

-- 视图3：用户好友关系统计（2.0新增）
CREATE VIEW user_friend_stats AS
SELECT 
    'teacher' as user_type,
    t.teacher_id as user_id,
    t.name as user_name,
    COUNT(DISTINCT CASE WHEN fr.user1_id = t.teacher_id AND fr.user1_type = '老师' THEN CONCAT(fr.user2_id, '-', fr.user2_type)
                       WHEN fr.user2_id = t.teacher_id AND fr.user2_type = '老师' THEN CONCAT(fr.user1_id, '-', fr.user1_type)
                       END) as friend_count
FROM teachers t
LEFT JOIN friend_relationships fr ON (fr.user1_id = t.teacher_id AND fr.user1_type = '老师') 
                                   OR (fr.user2_id = t.teacher_id AND fr.user2_type = '老师')
GROUP BY t.teacher_id, t.name
UNION ALL
SELECT 
    'student' as user_type,
    s.student_id as user_id,
    s.name as user_name,
    COUNT(DISTINCT CASE WHEN fr.user1_id = s.student_id AND fr.user1_type = '学生' THEN CONCAT(fr.user2_id, '-', fr.user2_type)
                       WHEN fr.user2_id = s.student_id AND fr.user2_type = '学生' THEN CONCAT(fr.user1_id, '-', fr.user1_type)
                       END) as friend_count
FROM students s
LEFT JOIN friend_relationships fr ON (fr.user1_id = s.student_id AND fr.user1_type = '学生') 
                                   OR (fr.user2_id = s.student_id AND fr.user2_type = '学生')
GROUP BY s.student_id, s.name;

-- 视图4：课程统计（2.0新增）
CREATE VIEW course_stats AS
SELECT 
    c.course_id,
    c.course_name,
    c.teacher_id,
    t.name as teacher_name,
    c.status,
    COUNT(DISTINCT sc.student_id) as enrolled_students,
    COUNT(DISTINCT CASE WHEN sc.enrollment_status = '申请中' THEN sc.student_id END) as pending_applications,
    COUNT(DISTINCT cn.notice_id) as notice_count,
    COUNT(DISTINCT ca.assignment_id) as assignment_count,
    COUNT(DISTINCT e.exam_id) as exam_count
FROM courses c
JOIN teachers t ON c.teacher_id = t.teacher_id
LEFT JOIN student_courses sc ON c.course_id = sc.course_id
LEFT JOIN course_notices cn ON c.course_id = cn.course_id
LEFT JOIN course_assignments ca ON c.course_id = ca.course_id
LEFT JOIN exams e ON c.course_id = e.course_id
GROUP BY c.course_id, c.course_name, c.teacher_id, t.name, c.status;

-- ============================================================================
-- 显示所有表
-- ============================================================================
SHOW TABLES;