-- 扩展测试数据插入脚本 data_insert2.sql
USE exam_system;

-- 1. 添加李春燕老师
INSERT INTO teachers (teacher_id, name, college, password) VALUES
(10, '李春燕', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'); -- 密码: password

-- 2. 添加李春燕老师的两门课程
INSERT INTO courses (course_id, teacher_id, college, course_name) VALUES
(10, 10, '计算机学院', 'C++面向对象程序设计'),
(11, 10, '计算机学院', '数据库原理');

-- 3. 添加20个学生（为两个班准备，有2-3个重复）
INSERT INTO students (student_id, name, grade, college, password) VALUES
-- C++面向对象程序设计班的学生
(2301, '陈思雨', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'), -- password
(2302, '王志强', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2303, '刘美琪', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2304, '张晓东', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2305, '赵雅芳', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2306, '孙建华', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2307, '周文静', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2308, '吴俊杰', '2023', '计算机学院', NULL), -- 无密码，可用于注册测试
(2309, '郑小丽', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2310, '马天宇', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),

-- 数据库原理班的学生（包含3个重复学生）
(2311, '林梦瑶', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2312, '胡永强', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2313, '罗晓霞', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2314, '蒋文博', '2023', '计算机学院', NULL), -- 无密码，可用于注册测试
(2315, '沈佳慧', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2316, '韩志明', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2317, '曹雪梅', '2023', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8');

-- 4. C++面向对象程序设计班选课记录（10个学生）
INSERT INTO student_courses (student_id, course_id, exam_score) VALUES
(2301, 10, NULL),  -- 陈思雨
(2302, 10, NULL),  -- 王志强  
(2303, 10, NULL),  -- 刘美琪
(2304, 10, NULL),  -- 张晓东
(2305, 10, NULL),  -- 赵雅芳
(2306, 10, NULL),  -- 孙建华
(2307, 10, NULL),  -- 周文静
(2308, 10, NULL),  -- 吴俊杰
(2309, 10, NULL),  -- 郑小丽
(2310, 10, NULL);  -- 马天宇

-- 5. 数据库原理班选课记录（10个学生，包含3个重复学生）
INSERT INTO student_courses (student_id, course_id, exam_score) VALUES
-- 重复学生（同时选了两门课）
(2303, 11, NULL),  -- 刘美琪（重复）
(2305, 11, NULL),  -- 赵雅芳（重复）
(2307, 11, NULL),  -- 周文静（重复）
-- 新学生
(2311, 11, NULL),  -- 林梦瑶
(2312, 11, NULL),  -- 胡永强
(2313, 11, NULL),  -- 罗晓霞
(2314, 11, NULL),  -- 蒋文博
(2315, 11, NULL),  -- 沈佳慧
(2316, 11, NULL),  -- 韩志明
(2317, 11, NULL);  -- 曹雪梅

-- 6. 为李春燕老师添加一些题目到题库
INSERT INTO questions (question_id, question_type, course_name, score, content, answer, creator_id, modifier_id, frequency, is_public) VALUES
-- C++面向对象程序设计题目
(20, '单选', 'C++面向对象程序设计', 5, 
'C++中哪个关键字用于定义类？\nA. struct\nB. class\nC. object\nD. type', 
'B', 10, 10, 0, TRUE),

(21, '单选', 'C++面向对象程序设计', 5,
'下列哪个是C++的访问控制符？\nA. public\nB. private\nC. protected\nD. 以上都是',
'D', 10, 10, 0, TRUE),

(22, '填空', 'C++面向对象程序设计', 10,
'C++中，构造函数的名称必须与____相同。',
'类名', 10, 10, 0, TRUE),

(23, '简答', 'C++面向对象程序设计', 15,
'请简述C++中继承的概念及其作用。',
'继承是面向对象编程的重要特性，允许一个类获得另一个类的属性和方法，实现代码复用和层次化设计。', 10, 10, 0, TRUE),

(24, '计算', 'C++面向对象程序设计', 20,
'编写一个简单的C++类Student，包含姓名和年龄两个私有成员变量，以及相应的构造函数和访问函数。',
'class Student {\nprivate:\n    string name;\n    int age;\npublic:\n    Student(string n, int a) : name(n), age(a) {}\n    string getName() { return name; }\n    int getAge() { return age; }\n};', 10, 10, 0, TRUE),

-- 数据库原理题目
(25, '单选', '数据库原理', 5,
'关系数据库中，用于确保实体完整性的是？\nA. 外键约束\nB. 主键约束\nC. 检查约束\nD. 非空约束',
'B', 10, 10, 0, TRUE),

(26, '多选', '数据库原理', 10,
'SQL语言包括哪些部分？\nA. 数据定义语言(DDL)\nB. 数据操作语言(DML)\nC. 数据控制语言(DCL)\nD. 数据查询语言(DQL)',
'ABCD', 10, 10, 0, TRUE),

(27, '填空', '数据库原理', 10,
'在关系数据库中，用于维护表与表之间联系的是____约束。',
'外键', 10, 10, 0, TRUE),

(28, '简答', '数据库原理', 15,
'请解释数据库事务的ACID特性。',
'ACID是指原子性(Atomicity)、一致性(Consistency)、隔离性(Isolation)、持久性(Durability)四个特性，确保数据库事务的可靠性。', 10, 10, 0, TRUE),

(29, '论述', '数据库原理', 20,
'请论述数据库范式化的目的和第一、二、三范式的要求。',
'范式化目的是消除数据冗余，避免插入、删除、更新异常。1NF要求属性不可分割；2NF要求非主属性完全依赖于主键；3NF要求非主属性不传递依赖于主键。', 10, 10, 0, TRUE);

-- 7. 创建一些考试记录
INSERT INTO exams (exam_id, exam_name, course_id, exam_date, start_time, end_time, total_score, is_published, is_completed) VALUES
(10, 'C++面向对象程序设计期中考试', 10, '2024-12-15', '09:00:00', '11:00:00', 100, FALSE, FALSE),
(11, '数据库原理期末考试', 11, '2024-12-20', '14:00:00', '16:30:00', 120, FALSE, FALSE);

-- 8. 设置考试权限（李春燕老师为创建者）
INSERT INTO exam_permissions (exam_id, teacher_id, is_creator) VALUES
(10, 10, TRUE),  -- C++考试
(11, 10, TRUE);  -- 数据库考试

-- 9. 李春燕老师收藏一些其他老师的题目
INSERT INTO question_favorites (question_id, teacher_id) VALUES
(1, 10),   -- 收藏张教授的数据结构题目
(7, 10),   -- 收藏李老师的数据库题目
(10, 10);  -- 收藏李老师的Java题目

-- 查看插入结果统计
SELECT '新增教师' as 类型, COUNT(*) as 数量 FROM teachers WHERE teacher_id = 10
UNION ALL
SELECT '新增课程', COUNT(*) FROM courses WHERE teacher_id = 10
UNION ALL  
SELECT '新增学生', COUNT(*) FROM students WHERE student_id BETWEEN 2301 AND 2317
UNION ALL
SELECT 'C++班选课', COUNT(*) FROM student_courses WHERE course_id = 10
UNION ALL
SELECT '数据库班选课', COUNT(*) FROM student_courses WHERE course_id = 11
UNION ALL
SELECT '新增题目', COUNT(*) FROM questions WHERE creator_id = 10
UNION ALL
SELECT '新增考试', COUNT(*) FROM exams WHERE course_id IN (10, 11)
UNION ALL
SELECT '重复选课学生', COUNT(*) FROM students s 
WHERE EXISTS (SELECT 1 FROM student_courses sc1 WHERE sc1.student_id = s.student_id AND sc1.course_id = 10)
AND EXISTS (SELECT 1 FROM student_courses sc2 WHERE sc2.student_id = s.student_id AND sc2.course_id = 11);