#include "papercomposer.h"
#include "paperpreview.h"
#include "permissiondialog.h"

PaperComposer::PaperComposer(int examId, int teacherId, Database* database, QWidget *parent)
    : QWidget(parent), examId(examId), teacherId(teacherId), database(database), currentTotalScore(0)
{
    // 获取考试信息
    QList<Exam> exams = database->getExamsByTeacher(teacherId);
    for (const Exam& exam : exams) {
        if (exam.getExamId() == examId) {
            currentExam = exam;
            break;
        }
    }

    targetTotalScore = currentExam.getTotalScore();

    setupUI();
    loadAvailableQuestions();
    loadSelectedQuestions();

    // 设置延迟搜索定时器
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    connect(searchTimer, &QTimer::timeout, this, &PaperComposer::delayedSearch);

    // 添加发布状态检查
    checkExamPublishStatus();
}

void PaperComposer::setupUI()
{
    setWindowTitle(QString("组卷 - %1").arg(currentExam.getExamName()));
    setMinimumSize(1400, 800);
    resize(1600, 900);

    mainLayout = new QHBoxLayout(this);

    // 创建分割器
    splitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(splitter);

    setupLeftPanel();
    setupRightPanel();

    // 设置分割比例：左侧60%，右侧40%
    splitter->setStretchFactor(0, 6);
    splitter->setStretchFactor(1, 4);
}

void PaperComposer::setupLeftPanel()
{
    leftPanel = new QWidget();
    leftLayout = new QVBoxLayout(leftPanel);

    // 题目来源标签页
    sourceTabWidget = new QTabWidget();
    sourceTabWidget->addTab(new QWidget(), "公共题库");
    sourceTabWidget->addTab(new QWidget(), "我的题库");
    sourceTabWidget->addTab(new QWidget(), "收藏题库");
    leftLayout->addWidget(sourceTabWidget);

    // 筛选区域
    QHBoxLayout* filterLayout = new QHBoxLayout();

    QLabel* courseLabel = new QLabel("课程:");
    courseFilterCombo = new QComboBox();
    courseFilterCombo->setMinimumWidth(120);

    QLabel* typeLabel = new QLabel("题型:");
    typeFilterCombo = new QComboBox();
    typeFilterCombo->setMinimumWidth(100);

    QLabel* searchLabel = new QLabel("搜索:");
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("输入关键词搜索...");
    searchEdit->setMinimumWidth(150);

    filterLayout->addWidget(courseLabel);
    filterLayout->addWidget(courseFilterCombo);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(typeLabel);
    filterLayout->addWidget(typeFilterCombo);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(searchLabel);
    filterLayout->addWidget(searchEdit);
    filterLayout->addStretch();

    leftLayout->addLayout(filterLayout);

    // 题目列表表格
    questionTable = new QTableWidget();
    questionTable->setColumnCount(5);
    QStringList headers;
    headers << "题型" << "分值" << "题干预览" << "考频" << "创建者";
    questionTable->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    questionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    questionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    questionTable->setAlternatingRowColors(true);
    questionTable->setSortingEnabled(true);

    // 设置列宽
    QHeaderView* header = questionTable->horizontalHeader();
    header->resizeSection(0, 80);  // 题型
    header->resizeSection(1, 60);  // 分值
    header->resizeSection(2, 300); // 题干预览
    header->resizeSection(3, 60);  // 考频
    header->resizeSection(4, 100); // 创建者
    header->setStretchLastSection(false);

    leftLayout->addWidget(questionTable);

    // 添加题目按钮
    addQuestionBtn = new QPushButton("添加到试卷");
    addQuestionBtn->setEnabled(false);
    leftLayout->addWidget(addQuestionBtn);

    splitter->addWidget(leftPanel);

    // 加载筛选选项
    QStringList courseNames = database->getCourseNames();
    courseFilterCombo->addItem("全部课程");
    courseFilterCombo->addItems(courseNames);

    QStringList questionTypes = database->getQuestionTypes();
    typeFilterCombo->addItem("全部题型");
    typeFilterCombo->addItems(questionTypes);

    // 连接信号
    connect(sourceTabWidget, &QTabWidget::currentChanged, this, &PaperComposer::onTabChanged);
    connect(courseFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PaperComposer::onCourseFilterChanged);
    connect(typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PaperComposer::onTypeFilterChanged);
    connect(searchEdit, &QLineEdit::textChanged, this, &PaperComposer::onSearchTextChanged);
    connect(questionTable, &QTableWidget::cellDoubleClicked, this, &PaperComposer::onQuestionDoubleClicked);
    connect(questionTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PaperComposer::onSelectedQuestionChanged);
    connect(addQuestionBtn, &QPushButton::clicked, this, &PaperComposer::onAddQuestionClicked);
}

void PaperComposer::setupRightPanel()
{
    rightPanel = new QWidget();
    rightLayout = new QVBoxLayout(rightPanel);

    // 试卷信息区域
    examInfoGroup = new QGroupBox("试卷信息");
    QVBoxLayout* infoLayout = new QVBoxLayout(examInfoGroup);

    examNameLabel = new QLabel(QString("考试: %1").arg(currentExam.getExamName()));
    examNameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    currentScoreLabel = new QLabel("当前总分: 0/100");
    currentScoreLabel->setStyleSheet("font-size: 13px; color: #007bff;");

    questionCountLabel = new QLabel("题目数量: 0");
    questionCountLabel->setStyleSheet("font-size: 13px; color: #28a745;");

    infoLayout->addWidget(examNameLabel);
    infoLayout->addWidget(currentScoreLabel);
    infoLayout->addWidget(questionCountLabel);

    rightLayout->addWidget(examInfoGroup);

    // 试卷题目列表
    QLabel* listLabel = new QLabel("试卷题目列表:");
    listLabel->setStyleSheet("font-weight: bold;");
    rightLayout->addWidget(listLabel);

    selectedQuestionsList = new QListWidget();
    selectedQuestionsList->setAlternatingRowColors(true);
    rightLayout->addWidget(selectedQuestionsList);

    // 操作按钮
    QHBoxLayout* buttonLayout1 = new QHBoxLayout();
    removeQuestionBtn = new QPushButton("删除题目");
    moveUpBtn = new QPushButton("上移");
    moveDownBtn = new QPushButton("下移");

    removeQuestionBtn->setEnabled(false);
    moveUpBtn->setEnabled(false);
    moveDownBtn->setEnabled(false);

    buttonLayout1->addWidget(removeQuestionBtn);
    buttonLayout1->addWidget(moveUpBtn);
    buttonLayout1->addWidget(moveDownBtn);
    rightLayout->addLayout(buttonLayout1);

    // 主要操作按钮
    QHBoxLayout* buttonLayout2 = new QHBoxLayout();
    savePaperBtn = new QPushButton("保存试卷");
    previewPaperBtn = new QPushButton("预览试卷");
    sharePermissionBtn = new QPushButton("分享权限");

    savePaperBtn->setStyleSheet("QPushButton { background-color: #007bff; color: white; font-weight: bold; padding: 8px; }");
    previewPaperBtn->setStyleSheet("QPushButton { background-color: #28a745; color: white; padding: 8px; }");
    sharePermissionBtn->setStyleSheet("QPushButton { background-color: #17a2b8; color: white; padding: 8px; }");

    buttonLayout2->addWidget(savePaperBtn);
    buttonLayout2->addWidget(previewPaperBtn);
    buttonLayout2->addWidget(sharePermissionBtn);
    rightLayout->addLayout(buttonLayout2);

    splitter->addWidget(rightPanel);

    // 连接信号
    connect(selectedQuestionsList, &QListWidget::currentRowChanged, this, &PaperComposer::updateButtonStates);
    connect(removeQuestionBtn, &QPushButton::clicked, this, &PaperComposer::onRemoveQuestionClicked);
    connect(moveUpBtn, &QPushButton::clicked, this, &PaperComposer::onMoveUpClicked);
    connect(moveDownBtn, &QPushButton::clicked, this, &PaperComposer::onMoveDownClicked);
    connect(savePaperBtn, &QPushButton::clicked, this, &PaperComposer::onSavePaperClicked);
    connect(previewPaperBtn, &QPushButton::clicked, this, &PaperComposer::onPreviewPaperClicked);
    connect(sharePermissionBtn, &QPushButton::clicked, this, &PaperComposer::onSharePermissionClicked);
}

void PaperComposer::loadAvailableQuestions()
{
    int currentTab = sourceTabWidget->currentIndex();

    if (currentTab == 0) { // 公共题库
        availableQuestions = database->getPublicQuestions();
    } else if (currentTab == 1) { // 我的题库
        availableQuestions = database->getQuestionsByTeacher(teacherId);
    } else if (currentTab == 2) { // 收藏题库
        availableQuestions = database->getFavoriteQuestions(teacherId);
    }

    refreshQuestionTable();
}

void PaperComposer::loadSelectedQuestions()
{
    selectedQuestions = database->getExamQuestions(examId);
    refreshSelectedList();
    updateScoreInfo();
}

void PaperComposer::refreshQuestionTable()
{
    questionTable->setRowCount(0);

    for (const Question& question : availableQuestions) {
        if (!matchesFilter(question)) {
            continue;
        }

        int row = questionTable->rowCount();
        questionTable->insertRow(row);

        questionTable->setItem(row, 0, new QTableWidgetItem(question.getQuestionType()));
        questionTable->setItem(row, 1, new QTableWidgetItem(QString::number(question.getScore())));
        questionTable->setItem(row, 2, new QTableWidgetItem(question.getContentPreview()));
        questionTable->setItem(row, 3, new QTableWidgetItem(QString::number(question.getFrequency())));
        questionTable->setItem(row, 4, new QTableWidgetItem(question.getCreatorName()));

        // 存储题目ID
        questionTable->item(row, 0)->setData(Qt::UserRole, question.getQuestionId());
    }

    updateButtonStates();
}

void PaperComposer::refreshSelectedList()
{
    selectedQuestionsList->clear();

    for (int i = 0; i < selectedQuestions.size(); ++i) {
        const Question& question = selectedQuestions[i];
        QString itemText = QString("%1. [%2][%3分] %4")
                               .arg(i + 1)
                               .arg(question.getQuestionType())
                               .arg(question.getScore())
                               .arg(question.getContentPreview());

        QListWidgetItem* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, question.getQuestionId());
        selectedQuestionsList->addItem(item);
    }

    updateButtonStates();
}

void PaperComposer::updateScoreInfo()
{
    currentTotalScore = 0;
    for (const Question& question : selectedQuestions) {
        currentTotalScore += question.getScore();
    }

    currentScoreLabel->setText(QString("当前总分: %1/%2").arg(currentTotalScore).arg(targetTotalScore));
    questionCountLabel->setText(QString("题目数量: %1").arg(selectedQuestions.size()));

    // 根据分数设置颜色
    if (currentTotalScore == targetTotalScore) {
        currentScoreLabel->setStyleSheet("font-size: 13px; color: #28a745;"); // 绿色
    } else if (currentTotalScore > targetTotalScore) {
        currentScoreLabel->setStyleSheet("font-size: 13px; color: #dc3545;"); // 红色
    } else {
        currentScoreLabel->setStyleSheet("font-size: 13px; color: #007bff;"); // 蓝色
    }
}

void PaperComposer::updateButtonStates()
{
    // 如果考试已发布，强制禁用所有编辑按钮
    if (currentExam.getIsPublished()) {
        addQuestionBtn->setEnabled(false);
        removeQuestionBtn->setEnabled(false);
        moveUpBtn->setEnabled(false);
        moveDownBtn->setEnabled(false);
        savePaperBtn->setEnabled(false);
        return;
    }
    // 添加题目按钮
    bool hasQuestionSelection = questionTable->currentRow() >= 0;
    addQuestionBtn->setEnabled(hasQuestionSelection);

    // 选中题目操作按钮
    bool hasSelectedQuestion = selectedQuestionsList->currentRow() >= 0;
    int currentRow = selectedQuestionsList->currentRow();
    int totalRows = selectedQuestionsList->count();

    removeQuestionBtn->setEnabled(hasSelectedQuestion);
    moveUpBtn->setEnabled(hasSelectedQuestion && currentRow > 0);
    moveDownBtn->setEnabled(hasSelectedQuestion && currentRow < totalRows - 1);

    // 保存和预览按钮
    bool hasQuestions = selectedQuestions.size() > 0;
    savePaperBtn->setEnabled(hasQuestions);
    previewPaperBtn->setEnabled(hasQuestions);
}

bool PaperComposer::matchesFilter(const Question& question)
{
    // 课程筛选
    QString courseFilter = courseFilterCombo->currentText();
    if (courseFilter != "全部课程" && question.getCourseName() != courseFilter) {
        return false;
    }

    // 题型筛选
    QString typeFilter = typeFilterCombo->currentText();
    if (typeFilter != "全部题型" && question.getQuestionType() != typeFilter) {
        return false;
    }

    // 关键词搜索
    QString searchText = searchEdit->text().trimmed().toLower();
    if (!searchText.isEmpty()) {
        if (!question.getContent().toLower().contains(searchText) &&
            !question.getAnswer().toLower().contains(searchText)) {
            return false;
        }
    }

    return true;
}

// 槽函数实现
void PaperComposer::onTabChanged(int index)
{
    Q_UNUSED(index)
    loadAvailableQuestions();
}

void PaperComposer::onCourseFilterChanged()
{
    refreshQuestionTable();
}

void PaperComposer::onTypeFilterChanged()
{
    refreshQuestionTable();
}

void PaperComposer::onSearchTextChanged()
{
    // 延迟搜索，避免频繁查询
    searchTimer->stop();
    searchTimer->start(500);
}

void PaperComposer::delayedSearch()
{
    refreshQuestionTable();
}

void PaperComposer::onAddQuestionClicked()
{
    if (currentExam.getIsPublished()) {
        QMessageBox::warning(this, "操作被禁止", "考试已发布，无法添加题目！");
        return;
    }

    int currentRow = questionTable->currentRow();
    if (currentRow >= 0) {
        int questionId = questionTable->item(currentRow, 0)->data(Qt::UserRole).toInt();

        // 检查题目是否已经在试卷中
        for (const Question& q : selectedQuestions) {
            if (q.getQuestionId() == questionId) {
                QMessageBox::warning(this, "警告", "该题目已经在试卷中！");
                return;
            }
        }

        // 查找题目详情
        Question selectedQuestion;
        for (const Question& q : availableQuestions) {
            if (q.getQuestionId() == questionId) {
                selectedQuestion = q;
                break;
            }
        }

        // 检查总分限制
        if (currentTotalScore + selectedQuestion.getScore() > targetTotalScore) {
            int ret = QMessageBox::question(this, "分数超限",
                                            QString("添加此题目将使总分超过目标分数(%1分)，是否继续？").arg(targetTotalScore),
                                            QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes) {
                return;
            }
        }

        // 添加到试卷
        selectedQuestions.append(selectedQuestion);
        refreshSelectedList();
        updateScoreInfo();
    }
}

void PaperComposer::onRemoveQuestionClicked()
{
    int currentRow = selectedQuestionsList->currentRow();
    if (currentRow >= 0 && currentRow < selectedQuestions.size()) {
        selectedQuestions.removeAt(currentRow);
        refreshSelectedList();
        updateScoreInfo();
    }
}

void PaperComposer::onMoveUpClicked()
{
    if (currentExam.getIsPublished()) {
        QMessageBox::warning(this, "操作被禁止", "考试已发布，无法调整题目顺序！");
        return;
    }

    if (currentExam.getIsPublished()) {
        QMessageBox::warning(this, "操作被禁止", "考试已发布，无法删除题目！");
        return;
    }

    int currentRow = selectedQuestionsList->currentRow();
    if (currentRow > 0) {
        selectedQuestions.swapItemsAt(currentRow - 1, currentRow);
        refreshSelectedList();
        selectedQuestionsList->setCurrentRow(currentRow - 1);
    }
}

void PaperComposer::onMoveDownClicked()
{
    if (currentExam.getIsPublished()) {
        QMessageBox::warning(this, "操作被禁止", "考试已发布，无法调整题目顺序！");
        return;
    }

    int currentRow = selectedQuestionsList->currentRow();
    if (currentRow >= 0 && currentRow < selectedQuestions.size() - 1) {
        selectedQuestions.swapItemsAt(currentRow, currentRow + 1);
        refreshSelectedList();
        selectedQuestionsList->setCurrentRow(currentRow + 1);
    }
}

void PaperComposer::onSavePaperClicked()
{
    if (currentExam.getIsPublished()) {
        QMessageBox::warning(this, "操作被禁止", "考试已发布，试卷内容无法修改！");
        return;
    }

    if (selectedQuestions.isEmpty()) {
        QMessageBox::warning(this, "警告", "试卷中没有题目，无法保存！");
        return;
    }

    // 先清除原有的试卷题目
    for (const Question& question : database->getExamQuestions(examId)) {
        database->removeQuestionFromExam(examId, question.getQuestionId());
    }

    // 添加新的试卷题目
    bool success = true;
    for (int i = 0; i < selectedQuestions.size(); ++i) {
        const Question& question = selectedQuestions[i];
        if (!database->addQuestionToExam(examId, question.getQuestionId(), i + 1)) {
            success = false;
            break;
        }
    }

    // 更新考试完成状态
    database->updateExamCompletion(examId, selectedQuestions.size() > 0);

    if (success) {
        QMessageBox::information(this, "成功", "试卷保存成功！");
    } else {
        QMessageBox::warning(this, "失败", "试卷保存失败！");
    }
}

void PaperComposer::onPreviewPaperClicked()
{
    if (selectedQuestions.isEmpty()) {
        QMessageBox::warning(this, "警告", "试卷中没有题目，无法预览！");
        return;
    }

    // 已发布的考试不需要保存，直接预览
    if (!currentExam.getIsPublished()) {
        // 先保存当前试卷（确保预览的是最新内容）
        if (selectedQuestions.size() > 0) {
            // 先清除原有的试卷题目
            for (const Question& question : database->getExamQuestions(examId)) {
                database->removeQuestionFromExam(examId, question.getQuestionId());
            }

            // 添加新的试卷题目
            for (int i = 0; i < selectedQuestions.size(); ++i) {
                const Question& question = selectedQuestions[i];
                database->addQuestionToExam(examId, question.getQuestionId(), i + 1);
            }

            // 更新考试完成状态
            database->updateExamCompletion(examId, selectedQuestions.size() > 0);
        }
    }

    // 打开试卷预览对话框
    PaperPreview previewDialog(examId, database, this);
    previewDialog.exec();
}

void PaperComposer::onSharePermissionClicked()
{
    // 检查是否是创建者
    if (!database->canModifyExam(examId, teacherId)) {
        QMessageBox::warning(this, "权限不足", "只有考试创建者可以分享编辑权限！");
        return;
    }

    // 打开权限分享对话框
    PermissionDialog permissionDialog(examId, teacherId, database, this);
    permissionDialog.exec();
}

void PaperComposer::onQuestionDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    if (row >= 0) {
        onAddQuestionClicked();
    }
}

void PaperComposer::onSelectedQuestionChanged()
{
    updateButtonStates();
}

void PaperComposer::checkExamPublishStatus()
{
    if (currentExam.getIsPublished()) {
        // 禁用所有编辑功能
        addQuestionBtn->setEnabled(false);
        removeQuestionBtn->setEnabled(false);
        moveUpBtn->setEnabled(false);
        moveDownBtn->setEnabled(false);
        savePaperBtn->setEnabled(false);

        // 禁用表格双击添加
        questionTable->setSelectionMode(QAbstractItemView::NoSelection);

        // 更新窗口标题显示已发布状态
        setWindowTitle(QString("组卷 - %1 [已发布 - 只读模式]").arg(currentExam.getExamName()));

        // 在试卷信息区域添加提示
        QLabel* warningLabel = new QLabel("⚠️ 考试已发布，试卷内容不可修改");
        warningLabel->setStyleSheet("font-weight: bold; color: #dc3545; background-color: #f8d7da; padding: 8px; border-radius: 4px; margin: 5px 0;");

        // 将警告标签插入到试卷信息组的布局中
        QVBoxLayout* infoLayout = qobject_cast<QVBoxLayout*>(examInfoGroup->layout());
        if (infoLayout) {
            infoLayout->insertWidget(0, warningLabel);
        }

        // 修改保存按钮为只读提示
        savePaperBtn->setText("试卷已锁定");
        savePaperBtn->setStyleSheet("QPushButton { background-color: #6c757d; color: white; font-weight: bold; padding: 8px; }");
    }
}
