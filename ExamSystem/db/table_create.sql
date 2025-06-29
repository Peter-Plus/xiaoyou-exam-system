USE exam_system;
-- 1学生表
CREATE TABLE students(
student_id INT PRIMARY KEY,
name VARCHAR(100) NOT NULL,
grade VARCHAR(20) NOT NULL,
college VARCHAR(100) NOT NULL,
password VARCHAR(255) DEFAULT NULL
);

-- 2教师表
CREATE TABLE teachers(
teacher_id INT AUTO_INCREMENT PRIMARY KEY,
name VARCHAR(100) NOT NULL,
college VARCHAR(100) NOT NULL,
password VARCHAR(255) DEFAULT NULL
);

-- 3课程表
CREATE TABLE courses(
course_id INT AUTO_INCREMENT PRIMARY KEY,
teacher_id INT NOT NULL,
college VARCHAR(100) NOT NULL,
course_name VARCHAR(200) NOT NULL,
FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE
);

-- 4选课表
CREATE TABLE student_courses(
student_id INT NOT NULL,
course_id INT NOT NULL,
exam_score DECIMAL(5,2) DEFAULT NULL,
PRIMARY KEY (student_id,course_id),
FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE,
FOREIGN KEY (course_id) REFERENCES courses(course_id) ON DELETE CASCADE
);

-- 5题目表
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
INDEX idx_question_type(question_type),
INDEX idx_is_public(is_public)
);

-- 6收藏题库表
CREATE TABLE question_favorites(
question_id INT NOT NULL,
teacher_id INT NOT NULL,
PRIMARY KEY (question_id,teacher_id),
FOREIGN KEY (question_id) REFERENCES questions(question_id) ON DELETE CASCADE,
FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE
);

-- 7考试表
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

-- 8试题组卷关联表
CREATE TABLE exam_questions(
exam_id INT NOT NULL,
question_id INT NOT NULL,
question_order INT NOT NULL,
PRIMARY KEY (exam_id,question_id),
FOREIGN KEY (exam_id) REFERENCES exams(exam_id) ON DELETE CASCADE,
FOREIGN KEY (question_id) REFERENCES questions(question_id) ON DELETE CASCADE,
INDEX idx_question_order (question_order)
);

-- 9试卷权限表
CREATE TABLE exam_permissions(
exam_id INT NOT NULL,
teacher_id INT NOT NULL,
is_creator BOOLEAN DEFAULT FALSE,
PRIMARY KEY (exam_id,teacher_id),
FOREIGN KEY (exam_id) REFERENCES exams(exam_id) ON DELETE CASCADE,
FOREIGN KEY (teacher_id) REFERENCES teachers(teacher_id) ON DELETE CASCADE
);

-- 10学生答题记录表
CREATE TABLE student_answers(
exam_id INT NOT NULL,
question_id INT NOT NULL,
student_id INT NOT NULL,
student_answer TEXT DEFAULT NULL,
score DECIMAL(5,2) DEFAULT 0,
is_graded BOOLEAN DEFAULT FALSE,
PRIMARY KEY (exam_id,question_id,student_id),
FOREIGN KEY (exam_id) REFERENCES exams(exam_id) ON DELETE CASCADE,
FOREIGN KEY (question_id) REFERENCES questions(question_id) ON DELETE CASCADE,
FOREIGN KEY (student_id) REFERENCES students(student_id) ON DELETE CASCADE,
INDEX idx_is_graded (is_graded)
);


-- 创建一些有用的视图

-- 视图1 学生考试成绩汇总
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
		SUM(CASE WHEN sa.is_graded=TRUE THEN 1 ELSE 0 END)as graded_questions
FROM student_answers sa
JOIN students s ON sa.student_id = s.student_id
JOIN exams e ON sa.exam_id = e.exam_id
JOIN courses c ON e.course_id = c.course_id
GROUP BY sa.exam_id,sa.student_id;

-- 视图2 教师题目统计
CREATE VIEW teacher_question_stats AS
SELECT 
		t.teacher_id,
		t.name as teacher_name,
		COUNT(DISTINCT q.question_id) as total_questions,
		COUNT(DISTINCT CASE WHEN q.is_public=TRUE THEN q.question_id END)as public_questions,
		COUNT(DISTINCT qf.question_id) as favorited_questions,
		AVG(q.frequency) as avg_frequency
FROM teachers t
LEFT JOIN questions q ON t.teacher_id = q.creator_id
LEFT JOIN question_favorites qf ON t.teacher_id = qf.teacher_id
GROUP BY t.teacher_id, t.name;


-- 显示所有表
SHOW TABLES;













