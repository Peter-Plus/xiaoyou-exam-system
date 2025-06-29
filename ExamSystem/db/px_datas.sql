-- 第三组测试数据插入脚本 data_insert3.sql
USE exam_system;

-- 1. 添加彭希老师
INSERT INTO teachers (teacher_id, name, college, password) VALUES
(20, '彭希', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'); -- 密码: password

-- 2. 添加彭希老师的两门课程
INSERT INTO courses (course_id, teacher_id, college, course_name) VALUES
(20, 20, '计算机学院', 'C语言程序设计'),
(21, 20, '计算机学院', '数据结构');

-- 3. 添加20个学生（为两个班准备，有2-3个重复）
INSERT INTO students (student_id, name, grade, college, password) VALUES
-- C语言程序设计班的学生
(2401, '李雨桐', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'), -- password
(2402, '陈浩然', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2403, '张欣怡', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2404, '王思远', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2405, '刘静雯', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2406, '赵宇航', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2407, '杨诗涵', '2024', '计算机学院', NULL), -- 无密码，可用于注册测试
(2408, '黄志豪', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2409, '徐梦琪', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2410, '孟凡宇', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),

-- 数据结构班的学生（包含3个重复学生）
(2411, '苏晓月', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2412, '邓振华', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2413, '高婷婷', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2414, '冯建国', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2415, '何美丽', '2024', '计算机学院', NULL), -- 无密码，可用于注册测试
(2416, '江文华', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8'),
(2417, '康雅琴', '2024', '计算机学院', '5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8');

-- 4. C语言程序设计班选课记录（10个学生）
INSERT INTO student_courses (student_id, course_id, exam_score) VALUES
(2401, 20, NULL),  -- 李雨桐
(2402, 20, NULL),  -- 陈浩然
(2403, 20, NULL),  -- 张欣怡
(2404, 20, NULL),  -- 王思远
(2405, 20, NULL),  -- 刘静雯
(2406, 20, NULL),  -- 赵宇航
(2407, 20, NULL),  -- 杨诗涵
(2408, 20, NULL),  -- 黄志豪
(2409, 20, NULL),  -- 徐梦琪
(2410, 20, NULL);  -- 孟凡宇

-- 5. 数据结构班选课记录（10个学生，包含3个重复学生）
INSERT INTO student_courses (student_id, course_id, exam_score) VALUES
-- 重复学生（同时选了两门课）
(2403, 21, NULL),  -- 张欣怡（重复）
(2405, 21, NULL),  -- 刘静雯（重复）
(2408, 21, NULL),  -- 黄志豪（重复）
-- 新学生
(2411, 21, NULL),  -- 苏晓月
(2412, 21, NULL),  -- 邓振华
(2413, 21, NULL),  -- 高婷婷
(2414, 21, NULL),  -- 冯建国
(2415, 21, NULL),  -- 何美丽
(2416, 21, NULL),  -- 江文华
(2417, 21, NULL);  -- 康雅琴

-- 6. 为彭希老师添加一些题目到题库
INSERT INTO questions (question_id, question_type, course_name, score, content, answer, creator_id, modifier_id, frequency, is_public) VALUES
-- C语言程序设计题目
(30, '单选', 'C语言程序设计', 5, 
'C语言中，下列哪个是正确的变量命名？\nA. 2var\nB. var-name\nC. var_name\nD. var name', 
'C', 20, 20, 0, TRUE),

(31, '单选', 'C语言程序设计', 5,
'C语言中printf函数的返回值类型是？\nA. void\nB. char\nC. int\nD. float',
'C', 20, 20, 0, TRUE),

(32, '填空', 'C语言程序设计', 10,
'C语言中，用于输入整数的格式控制符是____。',
'%d', 20, 20, 0, TRUE),

(33, '简答', 'C语言程序设计', 15,
'请解释C语言中指针的概念及其作用。',
'指针是一个变量，其值为另一个变量的地址。指针的作用包括：动态内存分配、函数参数传递、数据结构实现等。', 20, 20, 0, TRUE),

(34, '计算', 'C语言程序设计', 20,
'编写一个C语言函数，计算数组中所有元素的和。函数原型：int sum(int arr[], int n)',
'int sum(int arr[], int n) {\n    int total = 0;\n    for(int i = 0; i < n; i++) {\n        total += arr[i];\n    }\n    return total;\n}', 20, 20, 0, TRUE),

-- 数据结构题目
(35, '单选', '数据结构', 5,
'下列关于栈的叙述中，正确的是？\nA. 栈是先进先出的线性表\nB. 栈是先进后出的线性表\nC. 栈是随机存取的线性表\nD. 栈不是线性表',
'B', 20, 20, 0, TRUE),

(36, '多选', '数据结构', 10,
'下列哪些是线性数据结构？\nA. 数组\nB. 链表\nC. 栈\nD. 队列\nE. 二叉树',
'ABCD', 20, 20, 0, TRUE),

(37, '填空', '数据结构', 10,
'在单链表中，要访问第i个节点，最坏情况下需要____的时间复杂度。',
'O(n)', 20, 20, 0, TRUE),

(38, '简答', '数据结构', 15,
'请比较顺序存储和链式存储的优缺点。',
'顺序存储优点：支持随机访问，节省存储空间；缺点：插入删除需要移动元素。链式存储优点：插入删除高效；缺点：不支持随机访问，需要额外指针空间。', 20, 20, 0, TRUE),

(39, '论述', '数据结构', 20,
'请详细描述快速排序算法的原理、时间复杂度分析及其优化方法。',
'快速排序采用分治思想，选择基准元素进行分区，递归排序子数组。平均时间复杂度O(nlogn)，最坏情况O(n²)。优化方法包括：三数取中选择基准、尾递归优化、小数组用插入排序等。', 20, 20, 0, TRUE);

-- 7. 创建一些考试记录
INSERT INTO exams (exam_id, exam_name, course_id, exam_date, start_time, end_time, total_score, is_published, is_completed) VALUES
(20, 'C语言程序设计期中考试', 20, '2024-12-18', '10:00:00', '12:00:00', 80, FALSE, FALSE),
(21, '数据结构第一次月考', 21, '2024-12-22', '15:00:00', '17:00:00', 100, FALSE, FALSE);

-- 8. 设置考试权限（彭希老师为创建者）
INSERT INTO exam_permissions (exam_id, teacher_id, is_creator) VALUES
(20, 20, TRUE),  -- C语言考试
(21, 20, TRUE);  -- 数据结构考试

-- 9. 彭希老师收藏一些其他老师的题目
INSERT INTO question_favorites (question_id, teacher_id) VALUES
(1, 20),   -- 收藏张教授的数据结构题目
(2, 20),   -- 收藏张教授的数据结构题目
(5, 20),   -- 收藏张教授的算法题目
(20, 20),  -- 收藏李春燕的C++题目
(25, 20);  -- 收藏李春燕的数据库题目

-- 10. 添加一些学生之间的成绩差异（可选）
-- 为部分学生添加一些历史成绩，增加测试数据的真实性
UPDATE student_courses SET exam_score = 88.5 WHERE student_id = 2401 AND course_id = 20;
UPDATE student_courses SET exam_score = 92.0 WHERE student_id = 2403 AND course_id = 20;
UPDATE student_courses SET exam_score = 85.5 WHERE student_id = 2403 AND course_id = 21;

-- 查看插入结果统计
SELECT '新增教师' as 类型, COUNT(*) as 数量 FROM teachers WHERE teacher_id = 20
UNION ALL
SELECT '新增课程', COUNT(*) FROM courses WHERE teacher_id = 20
UNION ALL  
SELECT '新增学生', COUNT(*) FROM students WHERE student_id BETWEEN 2401 AND 2417
UNION ALL
SELECT 'C语言班选课', COUNT(*) FROM student_courses WHERE course_id = 20
UNION ALL
SELECT '数据结构班选课', COUNT(*) FROM student_courses WHERE course_id = 21
UNION ALL
SELECT '新增题目', COUNT(*) FROM questions WHERE creator_id = 20
UNION ALL
SELECT '新增考试', COUNT(*) FROM exams WHERE course_id IN (20, 21)
UNION ALL
SELECT '重复选课学生', COUNT(*) FROM students s 
WHERE EXISTS (SELECT 1 FROM student_courses sc1 WHERE sc1.student_id = s.student_id AND sc1.course_id = 20)
AND EXISTS (SELECT 1 FROM student_courses sc2 WHERE sc2.student_id = s.student_id AND sc2.course_id = 21)
UNION ALL
SELECT '2024级学生总数', COUNT(*) FROM students WHERE grade = '2024';