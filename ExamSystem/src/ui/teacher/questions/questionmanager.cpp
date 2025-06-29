#include "questionmanager.h"
#include "questiondialog.h" // 需要在第四步创建
#include <QHeaderView>
#include <QSplitter>
#include <QTimer>

QuestionManager::QuestionManager(int teacherId, Database* database, QWidget *parent)
    : QWidget(parent)
    , m_teacherId(teacherId)
    , m_database(database)
    , m_searchTimer(new QTimer(this))
{
    // 设置窗口属性
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_DeleteOnClose);

    setupUI();
    // 不需要再调用 m_database->connectToDatabase()，因为外部已经连接了

    // 设置搜索延迟定时器
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(500); // 500毫秒延迟
    connect(m_searchTimer, &QTimer::timeout, this, &QuestionManager::refreshCurrentTab);

    updateFilterOptions();
    refreshCurrentTab();
}

void QuestionManager::setupUI()
{
    setWindowTitle("题库管理");
    setMinimumSize(1200, 700); // 增加最小窗口大小
    resize(1400, 800); // 设置默认窗口大小

    setupTabWidget();
    setupFilterArea();
    setupQuestionTable();
    setupButtonArea();

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    mainLayout->addWidget(m_tabWidget);

    // 筛选区域
    QWidget *filterWidget = new QWidget();
    filterWidget->setMaximumHeight(60); // 限制筛选区域高度
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->addWidget(new QLabel("搜索:"));
    filterLayout->addWidget(m_searchEdit);
    filterLayout->addWidget(new QLabel("课程:"));
    filterLayout->addWidget(m_courseCombo);
    filterLayout->addWidget(new QLabel("题型:"));
    filterLayout->addWidget(m_typeCombo);
    filterLayout->addStretch();
    filterLayout->addWidget(m_addButton);

    // 表格和按钮区域
    QWidget *tableWidget = new QWidget();
    QVBoxLayout *tableLayout = new QVBoxLayout(tableWidget);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->addWidget(m_questionTable);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_viewButton);
    buttonLayout->addWidget(m_favoriteButton);
    buttonLayout->addStretch();

    tableLayout->addLayout(buttonLayout);

    mainLayout->addWidget(filterWidget);
    mainLayout->addWidget(tableWidget, 1); // 给表格区域更多空间
}

void QuestionManager::setupTabWidget()
{
    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(new QWidget(), "我的题库");
    m_tabWidget->addTab(new QWidget(), "公共题库");
    m_tabWidget->addTab(new QWidget(), "收藏题库");

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &QuestionManager::onTabChanged);
}

void QuestionManager::setupFilterArea()
{
    // 搜索框
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("请输入关键词");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &QuestionManager::onSearchTextChanged);

    // 课程筛选
    m_courseCombo = new QComboBox();
    connect(m_courseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuestionManager::onCourseFilterChanged);

    // 题型筛选
    m_typeCombo = new QComboBox();
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QuestionManager::onTypeFilterChanged);

    // 新建题目按钮
    m_addButton = new QPushButton("新建题目");
    connect(m_addButton, &QPushButton::clicked, this, &QuestionManager::onAddQuestion);
}

void QuestionManager::setupButtonArea()
{
    m_editButton = new QPushButton("修改");
    m_deleteButton = new QPushButton("删除");
    m_viewButton = new QPushButton("查看详情");
    m_favoriteButton = new QPushButton("收藏");

    connect(m_editButton, &QPushButton::clicked, this, &QuestionManager::onEditQuestion);
    connect(m_deleteButton, &QPushButton::clicked, this, &QuestionManager::onDeleteQuestion);
    connect(m_viewButton, &QPushButton::clicked, this, &QuestionManager::onViewQuestion);
    connect(m_favoriteButton, &QPushButton::clicked, this, &QuestionManager::onToggleFavorite);

    updateButtonStates();
}

void QuestionManager::setupQuestionTable()
{
    m_questionTable = new QTableWidget();

    // 设置列数和列标题
    m_questionTable->setColumnCount(8);
    QStringList headers;
    headers << "ID" << "题型" << "课程" << "分值" << "题干预览" << "考频" << "创建者" << "状态";
    m_questionTable->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    m_questionTable->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选择
    m_questionTable->setSelectionMode(QAbstractItemView::SingleSelection); // 单选
    m_questionTable->setAlternatingRowColors(true); // 交替行颜色
    m_questionTable->setSortingEnabled(true); // 允许排序
    m_questionTable->setWordWrap(true); // 允许文字换行

    // 设置水平和垂直滚动条
    m_questionTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_questionTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 设置列宽 - 根据内容自适应
    QHeaderView *header = m_questionTable->horizontalHeader();
    header->setDefaultSectionSize(100); // 默认列宽

    // 设置特定列的宽度
    m_questionTable->setColumnWidth(COL_ID, 60);
    m_questionTable->setColumnWidth(COL_TYPE, 80);
    m_questionTable->setColumnWidth(COL_COURSE, 120);
    m_questionTable->setColumnWidth(COL_SCORE, 60);
    m_questionTable->setColumnWidth(COL_CONTENT, 350); // 增加题干预览列宽
    m_questionTable->setColumnWidth(COL_FREQUENCY, 60);
    m_questionTable->setColumnWidth(COL_CREATOR, 100);
    m_questionTable->setColumnWidth(COL_STATUS, 80);

    // 设置列的拉伸模式
    header->setSectionResizeMode(COL_CONTENT, QHeaderView::Stretch); // 题干预览列自动拉伸
    header->setSectionResizeMode(COL_TYPE, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(COL_COURSE, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(COL_CREATOR, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(COL_STATUS, QHeaderView::ResizeToContents);

    // 隐藏ID列
    m_questionTable->setColumnHidden(COL_ID, true);

    // 设置行高自适应内容
    m_questionTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_questionTable->verticalHeader()->setDefaultSectionSize(40); // 默认行高

    // 连接信号
    connect(m_questionTable, &QTableWidget::itemSelectionChanged,
            this, &QuestionManager::onQuestionSelectionChanged);
    connect(m_questionTable, &QTableWidget::itemDoubleClicked,
            this, &QuestionManager::onQuestionDoubleClicked);
}

// 基本操作槽函数
void QuestionManager::onAddQuestion()
{
    QuestionDialog dialog(QuestionDialog::MODE_ADD, m_teacherId, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshCurrentTab(); // 刷新当前标签页数据
    }
}

void QuestionManager::onEditQuestion()
{
    if (!hasSelection()) {
        showMessage("请先选择要修改的题目");
        return;
    }

    Question question = getSelectedQuestion();
    if (!m_database->canModifyQuestion(question.getQuestionId(), m_teacherId)) {
        showMessage("您没有权限修改此题目");
        return;
    }

    QuestionDialog dialog(QuestionDialog::MODE_EDIT, m_teacherId, question, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshCurrentTab(); // 刷新当前标签页数据
    }
}

void QuestionManager::onDeleteQuestion()
{
    if (!hasSelection()) {
        showMessage("请先选择要删除的题目");
        return;
    }

    Question question = getSelectedQuestion();
    if (!m_database->canDeleteQuestion(question.getQuestionId(), m_teacherId)) {
        showMessage("您没有权限删除此题目");
        return;
    }

    // 确认删除
    int ret = QMessageBox::question(this, "确认删除",
                                    QString("确定要删除题目「%1」吗？").arg(question.getContentPreview()),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_database->deleteQuestion(question.getQuestionId(), m_teacherId)) {
            showMessage("删除成功");
            refreshCurrentTab();
        } else {
            showMessage("删除失败");
        }
    }
}

void QuestionManager::onViewQuestion()
{
    if (!hasSelection()) {
        showMessage("请先选择要查看的题目");
        return;
    }

    Question question = getSelectedQuestion();
    QuestionDialog dialog(QuestionDialog::MODE_VIEW, m_teacherId, question, this);
    dialog.exec(); // 查看模式不需要处理返回值
}

void QuestionManager::onToggleFavorite()
{
    if (!hasSelection()) {
        showMessage("请先选择题目");
        return;
    }

    Question question = getSelectedQuestion();
    int questionId = question.getQuestionId();

    if (question.getIsFavorited()) {
        // 取消收藏
        if (m_database->removeFromFavorites(questionId, m_teacherId)) {
            showMessage("取消收藏成功");
            refreshCurrentTab();
        } else {
            showMessage("取消收藏失败");
        }
    } else {
        // 添加收藏
        if (m_database->addToFavorites(questionId, m_teacherId)) {
            showMessage("收藏成功");
            refreshCurrentTab();
        } else {
            showMessage("收藏失败");
        }
    }
}

// 筛选和搜索槽函数
void QuestionManager::onSearchTextChanged()
{
    // 重启定时器，延迟搜索
    m_searchTimer->start();
}

void QuestionManager::onCourseFilterChanged()
{
    refreshCurrentTab(); // 立即刷新
}

void QuestionManager::onTypeFilterChanged()
{
    refreshCurrentTab(); // 立即刷新
}

void QuestionManager::onTabChanged(int index)
{
    refreshCurrentTab();
    updateButtonStates();
}

// 列表操作槽函数
void QuestionManager::onQuestionSelectionChanged()
{
    updateButtonStates();
}

void QuestionManager::onQuestionDoubleClicked()
{
    onViewQuestion(); // 双击查看详情
}

// 数据操作方法
void QuestionManager::refreshCurrentTab()
{
    int currentTab = m_tabWidget->currentIndex();

    switch (currentTab) {
    case TAB_MY_QUESTIONS:
        refreshMyQuestions();
        break;
    case TAB_PUBLIC_QUESTIONS:
        refreshPublicQuestions();
        break;
    case TAB_FAVORITE_QUESTIONS:
        refreshFavoriteQuestions();
        break;
    }
}

void QuestionManager::refreshMyQuestions()
{
    // 获取筛选条件
    QString keyword = m_searchEdit->text().trimmed();
    QString courseName = m_courseCombo->currentText();
    QString questionType = m_typeCombo->currentText();

    // 处理"全部"选项
    if (courseName == "全部") courseName = "";
    if (questionType == "全部") questionType = "";

    QList<Question> questions;

    if (keyword.isEmpty() && courseName.isEmpty() && questionType.isEmpty()) {
        // 无筛选条件，获取教师的所有题目
        questions = m_database->getQuestionsByTeacher(m_teacherId);
    } else {
        // 有筛选条件，使用搜索功能
        questions = m_database->searchQuestions(keyword, m_teacherId, courseName, questionType);
    }

    loadQuestionsToTable(questions);
}

void QuestionManager::refreshPublicQuestions()
{
    // 获取筛选条件
    QString keyword = m_searchEdit->text().trimmed();
    QString courseName = m_courseCombo->currentText();
    QString questionType = m_typeCombo->currentText();

    // 处理"全部"选项
    if (courseName == "全部") courseName = "";
    if (questionType == "全部") questionType = "";

    QList<Question> questions;

    if (keyword.isEmpty() && courseName.isEmpty() && questionType.isEmpty()) {
        // 无筛选条件，获取所有公开题目
        questions = m_database->getPublicQuestions();
    } else {
        // 有筛选条件，使用搜索功能
        questions = m_database->searchQuestions(keyword, -1, courseName, questionType);

        // 只保留公开题目
        QList<Question> publicQuestions;
        for (const Question &question : questions) {
            if (question.getIsPublic()) {
                publicQuestions.append(question);
            }
        }
        questions = publicQuestions;
    }

    // 设置收藏状态
    for (Question &question : questions) {
        bool isFavorited = m_database->isQuestionFavorited(question.getQuestionId(), m_teacherId);
        question.setIsFavorited(isFavorited);
    }

    loadQuestionsToTable(questions);
}

void QuestionManager::refreshFavoriteQuestions()
{
    QList<Question> questions = m_database->getFavoriteQuestions(m_teacherId);
    applyFilters(questions); // 应用筛选条件
    loadQuestionsToTable(questions);
}

void QuestionManager::loadQuestionsToTable(const QList<Question> &questions)
{
    m_questionTable->setRowCount(questions.size());

    for (int i = 0; i < questions.size(); ++i) {
        const Question &question = questions[i];

        m_questionTable->setItem(i, COL_ID, new QTableWidgetItem(QString::number(question.getQuestionId())));
        m_questionTable->setItem(i, COL_TYPE, new QTableWidgetItem(question.getQuestionType()));
        m_questionTable->setItem(i, COL_COURSE, new QTableWidgetItem(question.getCourseName()));
        m_questionTable->setItem(i, COL_SCORE, new QTableWidgetItem(QString::number(question.getScore())));
        m_questionTable->setItem(i, COL_CONTENT, new QTableWidgetItem(question.getContentPreview()));
        m_questionTable->setItem(i, COL_FREQUENCY, new QTableWidgetItem(QString::number(question.getFrequency())));
        m_questionTable->setItem(i, COL_CREATOR, new QTableWidgetItem(question.getCreatorName()));
        m_questionTable->setItem(i, COL_STATUS, new QTableWidgetItem(question.getPublicStatusText()));
    }
}

// 辅助方法
void QuestionManager::updateButtonStates()
{
    bool hasSelect = hasSelection();
    int currentTab = m_tabWidget->currentIndex();

    m_editButton->setEnabled(hasSelect && currentTab == TAB_MY_QUESTIONS);
    m_deleteButton->setEnabled(hasSelect && currentTab == TAB_MY_QUESTIONS);
    m_viewButton->setEnabled(hasSelect);

    // 收藏按钮
    if (hasSelect && currentTab != TAB_MY_QUESTIONS) {
        Question question = getSelectedQuestion();
        m_favoriteButton->setEnabled(true);
        m_favoriteButton->setText(question.getIsFavorited() ? "取消收藏" : "收藏");
    } else {
        m_favoriteButton->setEnabled(false);
        m_favoriteButton->setText("收藏");
    }
}

void QuestionManager::updateFilterOptions()
{
    // 更新课程下拉框
    m_courseCombo->clear();
    m_courseCombo->addItem("全部");
    QStringList courses = m_database->getCourseNames();
    m_courseCombo->addItems(courses);

    // 更新题型下拉框
    m_typeCombo->clear();
    m_typeCombo->addItem("全部");
    QStringList types = m_database->getQuestionTypes();
    m_typeCombo->addItems(types);
}

Question QuestionManager::getSelectedQuestion()
{
    int currentRow = m_questionTable->currentRow();
    if (currentRow < 0) return Question();

    int questionId = m_questionTable->item(currentRow, COL_ID)->text().toInt();
    return m_database->getQuestionById(questionId);
}

bool QuestionManager::hasSelection()
{
    return m_questionTable->currentRow() >= 0;
}

void QuestionManager::showMessage(const QString &message)
{
    QMessageBox::information(this, "提示", message);
}

// 应用筛选条件到题目列表
void QuestionManager::applyFilters(QList<Question> &questions)
{
    QString keyword = m_searchEdit->text().trimmed();
    QString courseName = m_courseCombo->currentText();
    QString questionType = m_typeCombo->currentText();

    // 处理"全部"选项
    if (courseName == "全部") courseName = "";
    if (questionType == "全部") questionType = "";

    // 如果没有筛选条件，直接返回
    if (keyword.isEmpty() && courseName.isEmpty() && questionType.isEmpty()) {
        return;
    }

    // 筛选题目
    QList<Question> filteredQuestions;
    for (const Question &question : questions) {
        if (matchesFilter(question, keyword, courseName, questionType)) {
            filteredQuestions.append(question);
        }
    }

    questions = filteredQuestions;
}

// 检查题目是否匹配筛选条件
bool QuestionManager::matchesFilter(const Question &question, const QString &keyword,
                                    const QString &courseName, const QString &questionType)
{
    // 关键词筛选
    if (!keyword.isEmpty()) {
        QString content = question.getContent().toLower();
        QString answer = question.getAnswer().toLower();
        QString searchKey = keyword.toLower();
        if (!content.contains(searchKey) && !answer.contains(searchKey)) {
            return false;
        }
    }

    // 课程筛选
    if (!courseName.isEmpty() && question.getCourseName() != courseName) {
        return false;
    }

    // 题型筛选
    if (!questionType.isEmpty() && question.getQuestionType() != questionType) {
        return false;
    }

    return true;
}
