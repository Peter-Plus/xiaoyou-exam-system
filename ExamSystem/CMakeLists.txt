cmake_minimum_required(VERSION 3.16)

project(ExamSystem VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Core PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Core PrintSupport)

# 包含目录
include_directories(src)
include_directories(src/core)
include_directories(src/models)
include_directories(src/ui)
include_directories(src/ui/auth)
include_directories(src/ui/main)
include_directories(src/ui/student)
include_directories(src/ui/teacher/questions)
include_directories(src/ui/teacher/exams)
include_directories(src/ui/teacher/grading)
include_directories(src/ui/teacher/statistics)
include_directories(src/ui/social)
include_directories(src/utils)

set(PROJECT_SOURCES
            # 主程序文件
            main.cpp

            # 核心模块
            src/core/database.h
            src/core/database.cpp

            # 数据模型
            src/models/user.h
            src/models/user.cpp
            src/models/student.h
            src/models/student.cpp
            src/models/teacher.h
            src/models/teacher.cpp
            src/models/course.h
            src/models/course.cpp
            src/models/exam.h
            src/models/exam.cpp
            src/models/question.h
            src/models/question.cpp
            src/models/studentanswer.h
            src/models/studentanswer.cpp
            src/models/classstatistics.h
            src/models/classstatistics.cpp

            # 登录注册
            src/ui/auth/loginwindow.h
            src/ui/auth/loginwindow.cpp
            src/ui/auth/registerwindow.h
            src/ui/auth/registerwindow.cpp

            # 主界面
            src/ui/main/studentmainwindow.h
            src/ui/main/studentmainwindow.cpp
            src/ui/main/teachermainwindow.h
            src/ui/main/teachermainwindow.cpp

            # 学生界面
            src/ui/student/studentexamlist.h
            src/ui/student/studentexamlist.cpp
            src/ui/student/examtaking.h
            src/ui/student/examtaking.cpp
            src/ui/student/examresult.h
            src/ui/student/examresult.cpp

            # 教师界面 - 题库管理
            src/ui/teacher/questions/questionmanager.h
            src/ui/teacher/questions/questionmanager.cpp
            src/ui/teacher/questions/questiondialog.h
            src/ui/teacher/questions/questiondialog.cpp

            # 教师界面 - 考试管理
            src/ui/teacher/exams/exammanager.h
            src/ui/teacher/exams/exammanager.cpp
            src/ui/teacher/exams/examdialog.h
            src/ui/teacher/exams/examdialog.cpp
            src/ui/teacher/exams/papercomposer.h
            src/ui/teacher/exams/papercomposer.cpp
            src/ui/teacher/exams/paperpreview.h
            src/ui/teacher/exams/paperpreview.cpp
            src/ui/teacher/exams/permissiondialog.h
            src/ui/teacher/exams/permissiondialog.cpp
            src/ui/teacher/exams/examstatusmanager.h
            src/ui/teacher/exams/examstatusmanager.cpp

            # 教师界面 - 阅卷
            src/ui/teacher/grading/examgrading.h
            src/ui/teacher/grading/examgrading.cpp

            # 教师界面 - 成绩统计
            src/ui/teacher/statistics/classstatisticswindow.h
            src/ui/teacher/statistics/classstatisticswindow.cpp

            # 好友相关模型
            src/models/friendinfo.h
            src/models/friendinfo.cpp
            src/models/friendrequest.h
            src/models/friendrequest.cpp
            src/models/userinfo.h
            src/models/userinfo.cpp
            src/ui/friend/friendmanager.h
            src/ui/friend/friendmanager.cpp
            src/ui/friend/friendpage.h
            src/ui/friend/friendpage.cpp

            #私聊相关
            src/ui/chat/messagebubblewidget.h
            src/ui/chat/messagebubblewidget.cpp
            src/models/chatinfo.h
            src/models/chatinfo.cpp
            src/models/messageinfo.h
            src/models/messageinfo.cpp
            src/ui/chat/chatpage.h
            src/ui/chat/chatpage.cpp

            #群聊相关
            src/models/groupinfo.h
            src/models/groupinfo.cpp
            src/models/grouprequest.h
            src/models/grouprequest.cpp
            src/models/groupmember.h
            src/models/groupmember.cpp
            src/ui/chat/grouplistwidget.h
            src/ui/chat/grouplistwidget.cpp
            src/ui/chat/creategroupdialog.h
            src/ui/chat/creategroupdialog.cpp
            src/ui/chat/joingroupdialog.h
            src/ui/chat/joingroupdialog.cpp
            src/ui/chat/groupmanagewidget.h
            src/ui/chat/groupmanagewidget.cpp
            src/ui/chat/groupdetailwidget.h
            src/ui/chat/groupdetailwidget.cpp
            src/ui/chat/invitememberdialog.h
            src/ui/chat/invitememberdialog.cpp

            #课程管理模块
            src/models/coursenotice.h
            src/models/coursenotice.cpp
            src/models/courseassignment.h
            src/models/courseassignment.cpp
            src/models/assignmentsubmission.h
            src/models/assignmentsubmission.cpp
            src/models/enrollmentrequest.h
            src/models/erollmentrequest.cpp
            src/ui/course/coursepage.h
            src/ui/course/coursepage.cpp
            src/ui/course/enrollmentwidget.h
            src/ui/course/enrollmentwidget.cpp
            src/ui/course/enrollmentadminwidget.h
            src/ui/course/enrollmentadminwidget.cpp
            src/ui/course/coursenoticewidget.h
            src/ui/course/coursenoticewidget.cpp
            src/ui/course/courseassignmentwidget.h
            src/ui/course/courseassignmentwidget.cpp


)

# Windows资源文件（移到外面）
if(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} app.rc)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ExamSystem
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/ui/course/mycourseswidget.h
        src/ui/course/mycourseswidget.cpp
        src/utils/thememanager.h
        src/utils/thememanager.cpp
        src/utils/appstyle.h
        src/utils/appstyle.cpp
    )
else()
    if(ANDROID)
        add_library(ExamSystem SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(ExamSystem
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ExamSystem PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::PrintSupport)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ExamSystem)
endif()
set_target_properties(ExamSystem PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ExamSystem
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ExamSystem)
endif()
