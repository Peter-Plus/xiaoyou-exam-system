USE exam_system;
-- 1 插入老师测试数据
INSERT INTO teachers(name,college,password)VALUES
('张教授', '计算机学院', '$2y$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi'), --
('李老师', '计算机学院', '$2y$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi'), 
('王教授', '数学学院', NULL); 

-- 2 插入学生测试数据
INSERT INTO students (name,grade,college,password) VALUES
('小明', '2023', '计算机学院', '$2y$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi'), 
('小红', '2023', '计算机学院', '$2y$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi'), 
('小华', '2022', '计算机学院', NULL), 
('小丽', '2023', '软件学院', '$2y$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi'), 
('小强', '2024', '计算机学院', NULL), 
('小芳', '2023', '数学学院', '$2y$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi'); 

-- 3.插入课程数据
INSERT INTO courses (teacher_id,college,course_name) VALUES
(1, '计算机学院', '数据结构'),
(1, '计算机学院', '算法分析'), 
(2, '计算机学院', '数据库原理'),
(2, '计算机学院', 'Java程序设计'),
(3, '数学学院', '高等数学'); 

-- 4.插入选课数据
INSERT INTO student_courses(student_id,course_id,exam_score) VALUES
(1, 1, 85.5), 
(1, 2, NULL), 
(1, 3, 78.0), 
(2, 1, 92.0), 
(2, 3, NULL), 
(2, 4, 88.5), 
(3, 1, NULL), 
(3, 2, 76.5), 
(4, 3, NULL), 
(4, 4, 90.0), 
(5, 1, NULL), 
(6, 1, 82.0), 
(6, 5, 95.0);

-- 5.插入题目数据
INSERT INTO questions(question_type,course_name,score,content,answer,creator_id,modifier_id,frequency,is_public) VALUES
('单选', '数据结构', 5, '以下哪种数据结构遵循先进先出的原则？\nA. 栈\nB. 队列\nC. 链表\nD. 树', 'B', 1, 1, 2, TRUE),
('单选', '数据结构', 5, '二叉树中，具有n个节点的完全二叉树的高度为？\nA. log₂n\nB. ⌊log₂n⌋+1\nC. n\nD. n-1', 'B', 1, 1, 1, TRUE),
('填空', '数据结构', 10, '在顺序栈中，栈顶指针top指向栈顶元素的位置，当栈为空时，top的值为____。', '-1', 1, 1, 0, TRUE),
('简答', '数据结构', 15, '请简述链表相比数组的优缺点。', '优点：动态分配内存，插入删除高效；缺点：不支持随机访问，额外内存开销', 1, 1, 0, TRUE),
('单选', '算法分析', 5, '冒泡排序的时间复杂度是？\nA. O(n)\nB. O(n log n)\nC. O(n²)\nD. O(n³)', 'C', 1, 1, 1, TRUE),
('计算', '算法分析', 20, '分析以下递归函数的时间复杂度：\nint fibonacci(int n) {\n    if (n <= 1) return n;\n    return fibonacci(n-1) + fibonacci(n-2);\n}', 'T(n) = T(n-1) + T(n-2) + O(1)，时间复杂度为O(2^n)', 1, 1, 0, TRUE),
('单选', '数据库原理', 5, 'SQL中用于查询数据的关键字是？\nA. INSERT\nB. UPDATE\nC. SELECT\nD. DELETE', 'C', 2, 2, 3, TRUE),
('多选', '数据库原理', 10, '关系数据库的ACID特性包括？\nA. 原子性(Atomicity)\nB. 一致性(Consistency)\nC. 隔离性(Isolation)\nD. 持久性(Durability)\nE. 可用性(Availability)', 'ABCD', 2, 2, 1, TRUE),
('简答', '数据库原理', 15, '请解释什么是数据库范式，并说明第一范式的要求。', '数据库范式是设计关系数据库时遵循的规范。第一范式要求关系中的每个属性都是不可分割的原子值。', 2, 2, 0, TRUE),
('单选', 'Java程序设计', 5, 'Java中哪个关键字用于继承？\nA. implements\nB. extends\nC. inherits\nD. super', 'B', 2, 2, 2, TRUE),
('填空', 'Java程序设计', 10, 'Java中，____方法是程序的入口点。', 'main', 2, 2, 1, TRUE),
('单选', '高等数学', 5, '函数f(x)=x²在x=0处的导数是？\nA. 0\nB. 1\nC. 2\nD. 不存在', 'A', 3, 3, 1, TRUE),
('计算', '高等数学', 15, '计算定积分∫₀¹ x²dx', '∫₀¹ x²dx = [x³/3]₀¹ = 1/3 - 0 = 1/3', 3, 3, 0, TRUE),
('单选', '数据结构', 5, '这是张教授的私有题目，关于红黑树的性质', '私有题目答案', 1, 1, 0, FALSE),
('简答', '数据库原理', 20, '李老师的高级数据库题目（未公开）', '高级答案内容', 2, 2, 0, FALSE);

-- 6 插入题目收藏数据
INSERT INTO question_favorites (question_id,teacher_id) VALUES
(7,1),
(8,1),
(10,1),
(1,2),
(2,2),
(4,2),
(1,3),
(5,3);

-- 7. 插入考试数据
INSERT INTO exams(exam_name,course_id,exam_date,start_time,end_time,total_score,is_published,is_completed)VALUES
('数据结构期中考试', 1, '2024-11-15', '09:00:00', '11:00:00', 100, TRUE, TRUE),
('Java程序设计第一次测验', 4, '2024-11-20', '14:00:00', '15:30:00', 50, TRUE, TRUE),
('数据库原理期中考试', 3, '2024-12-20', '09:00:00', '11:00:00', 80, TRUE, TRUE),
('算法分析单元测试', 2, '2024-12-25', '10:00:00', '12:00:00', 60, TRUE, TRUE),
('高等数学期末考试', 5, '2025-01-10', '09:00:00', '11:00:00', 120, FALSE, FALSE);

-- 8. 插入试题组卷关联数据
INSERT INTO exam_questions(exam_id,question_id,question_order)VALUES
(1, 1, 1), 
(1, 2, 2),
(1, 3, 3), 
(1, 4, 4), 
(1, 13, 5), 
(2, 10, 1), 
(2, 11, 2), 
(3, 7, 1), 
(3, 8, 2), 
(3, 9, 3), 
(4, 5, 1), 
(4, 6, 2), 
(5, 12, 1), 
(5, 15, 2);

-- 9. 插入考试权限数据
INSERT INTO exam_permissions (exam_id,teacher_id,is_creator)VALUES
(1, 1, TRUE), 
(2, 2, TRUE), 
(3, 2, TRUE), 
(4, 1, TRUE), 
(5, 3, TRUE), 
(1, 2, FALSE), 
(3, 1, FALSE);

-- 10. 插入学生答题记录数据
INSERT INTO student_answers (exam_id,question_id,student_id,student_answer,score,is_graded) VALUES
(1, 1, 1, 'B', 5.0, TRUE), 
(1, 2, 1, 'B', 5.0, TRUE), 
(1, 3, 1, '-1', 10.0, TRUE), -- 填空题老师已批改
(1, 4, 1, '链表的优点是动态分配内存，插入删除效率高；缺点是不能随机访问，有额外的指针开销', 12.0, TRUE), 
(1, 13, 1, '红黑树是平衡二叉搜索树', 3.0, TRUE), 
(2, 10, 2, 'B', 5.0, TRUE), 
(2, 11, 2, 'main', 10.0, TRUE), 
(3, 7, 3, 'C', 5.0, TRUE), 
(3, 8, 3, 'ABCD', 10.0, TRUE), 
(3, 9, 3, '数据库范式是规范，第一范式要求属性不可分割', 0.0, FALSE), 
(3, 7, 4, 'C', 5.0, TRUE), 
(3, 8, 4, 'ABC', 7.0, TRUE), 
(3, 9, 4, '还未作答', 0.0, FALSE); 



SELECT '教师数量' as 统计项, COUNT(*) as 数量 FROM teachers
UNION ALL
SELECT '学生数量', COUNT(*) FROM students  
UNION ALL
SELECT '课程数量', COUNT(*) FROM courses
UNION ALL
SELECT '选课记录', COUNT(*) FROM student_courses
UNION ALL
SELECT '题目数量', COUNT(*) FROM questions
UNION ALL
SELECT '收藏记录', COUNT(*) FROM question_favorites
UNION ALL
SELECT '考试数量', COUNT(*) FROM exams
UNION ALL
SELECT '组卷记录', COUNT(*) FROM exam_questions
UNION ALL
SELECT '权限记录', COUNT(*) FROM exam_permissions
UNION ALL
SELECT '答题记录', COUNT(*) FROM student_answers;














