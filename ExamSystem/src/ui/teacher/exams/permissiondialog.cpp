#include "permissiondialog.h"

PermissionDialog::PermissionDialog(int examId, int currentTeacherId, Database* database, QWidget *parent)
    : QDialog(parent), examId(examId), currentTeacherId(currentTeacherId), database(database)
{
    setupUI();
    loadTeachersWithPermission();
    loadAvailableTeachers();
}

void PermissionDialog::setupUI()
{
    setWindowTitle("分享编辑权限");
    setModal(true);
    setFixedSize(600, 500);

    mainLayout = new QVBoxLayout(this);

    // 标题说明
    QLabel* titleLabel = new QLabel("管理试卷编辑权限");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QLabel* descLabel = new QLabel("您可以将试卷编辑权限分享给其他教师，被分享的教师可以修改试卷内容。");
    descLabel->setStyleSheet("font-size: 12px; color: #7f8c8d; margin-bottom: 15px;");
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(descLabel);

    // 主要内容区域
    contentLayout = new QHBoxLayout();

    // 左侧：已有权限的教师
    currentPermissionGroup = new QGroupBox("已有编辑权限");
    currentLayout = new QVBoxLayout(currentPermissionGroup);

    currentCountLabel = new QLabel("共0位教师");
    currentCountLabel->setStyleSheet("font-size: 12px; color: #6c757d;");
    currentLayout->addWidget(currentCountLabel);

    currentTeacherList = new QListWidget();
    currentTeacherList->setAlternatingRowColors(true);
    currentTeacherList->setSelectionMode(QAbstractItemView::SingleSelection);
    currentLayout->addWidget(currentTeacherList);

    contentLayout->addWidget(currentPermissionGroup);

    // 中间：操作按钮
    QVBoxLayout* buttonMiddleLayout = new QVBoxLayout();
    buttonMiddleLayout->addStretch();

    addPermissionBtn = new QPushButton("授权 →");
    addPermissionBtn->setStyleSheet("QPushButton { background-color: #28a745; color: white; padding: 8px 16px; font-weight: bold; border: none; border-radius: 4px; } QPushButton:hover { background-color: #218838; } QPushButton:disabled { background-color: #6c757d; }");
    addPermissionBtn->setEnabled(false);
    buttonMiddleLayout->addWidget(addPermissionBtn);

    buttonMiddleLayout->addSpacing(10);

    removePermissionBtn = new QPushButton("← 撤销");
    removePermissionBtn->setStyleSheet("QPushButton { background-color: #dc3545; color: white; padding: 8px 16px; font-weight: bold; border: none; border-radius: 4px; } QPushButton:hover { background-color: #c82333; } QPushButton:disabled { background-color: #6c757d; }");
    removePermissionBtn->setEnabled(false);
    buttonMiddleLayout->addWidget(removePermissionBtn);

    buttonMiddleLayout->addStretch();
    contentLayout->addLayout(buttonMiddleLayout);

    // 右侧：可添加的教师
    availableTeacherGroup = new QGroupBox("可授权教师");
    availableLayout = new QVBoxLayout(availableTeacherGroup);

    // 学院筛选
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel("学院筛选:");
    collegeFilterCombo = new QComboBox();
    collegeFilterCombo->setMinimumWidth(120);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(collegeFilterCombo);
    filterLayout->addStretch();
    availableLayout->addLayout(filterLayout);

    availableCountLabel = new QLabel("共0位教师");
    availableCountLabel->setStyleSheet("font-size: 12px; color: #6c757d;");
    availableLayout->addWidget(availableCountLabel);

    availableTeacherList = new QListWidget();
    availableTeacherList->setAlternatingRowColors(true);
    availableTeacherList->setSelectionMode(QAbstractItemView::SingleSelection);
    availableLayout->addWidget(availableTeacherList);

    contentLayout->addWidget(availableTeacherGroup);

    mainLayout->addLayout(contentLayout);

    // 底部按钮
    buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet("QPushButton { background-color: #6c757d; color: white; padding: 8px 20px; border: none; border-radius: 4px; } QPushButton:hover { background-color: #5a6268; }");
    buttonLayout->addWidget(closeBtn);

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(addPermissionBtn, &QPushButton::clicked, this, &PermissionDialog::onAddPermissionClicked);
    connect(removePermissionBtn, &QPushButton::clicked, this, &PermissionDialog::onRemovePermissionClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(currentTeacherList, &QListWidget::currentRowChanged, this, &PermissionDialog::onCurrentTeacherChanged);
    connect(availableTeacherList, &QListWidget::currentRowChanged, this, &PermissionDialog::onAvailableTeacherChanged);
    connect(collegeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PermissionDialog::loadAvailableTeachers);
}

void PermissionDialog::loadTeachersWithPermission()
{
    currentTeachers = database->getExamPermissions(examId);

    currentTeacherList->clear();

    for (const Teacher& teacher : currentTeachers) {
        QString displayText;
        // 检查是否是创建者
        bool isCreator = false;
        // 这里需要检查数据库中的is_creator字段，暂时通过teacherId判断
        if (teacher.getId() == currentTeacherId) {
            isCreator = true;
        }

        if (isCreator) {
            displayText = QString("%1 (%2) [创建者]").arg(teacher.getName(), teacher.getCollege());
        } else {
            displayText = QString("%1 (%2)").arg(teacher.getName(), teacher.getCollege());
        }

        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, teacher.getId());

        // 创建者不能被删除，设置不同颜色
        if (isCreator) {
            item->setForeground(QBrush(QColor("#007bff")));
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
        }

        currentTeacherList->addItem(item);
    }

    currentCountLabel->setText(QString("共%1位教师").arg(currentTeachers.size()));
    updateButtonStates();
}

void PermissionDialog::loadAvailableTeachers()
{
    // 获取所有教师（这里需要database提供getAllTeachers方法，暂时用空列表）
    // 实际应该是: QList<Teacher> allTeachers = database->getAllTeachers();
    QList<Teacher> allTeachers; // 暂时为空，需要实现getAllTeachers方法

    // 筛选出没有权限的教师
    availableTeachers.clear();
    QString collegeFilter = collegeFilterCombo->currentText();

    for (const Teacher& teacher : allTeachers) {
        // 跳过已有权限的教师
        bool hasPermission = false;
        for (const Teacher& currentTeacher : currentTeachers) {
            if (teacher.getId() == currentTeacher.getId()) {
                hasPermission = true;
                break;
            }
        }

        if (hasPermission) continue;

        // 学院筛选
        if (collegeFilter != "全部学院" && teacher.getCollege() != collegeFilter) {
            continue;
        }

        availableTeachers.append(teacher);
    }

    // 更新列表显示
    availableTeacherList->clear();

    for (const Teacher& teacher : availableTeachers) {
        QString displayText = QString("%1 (%2)").arg(teacher.getName(), teacher.getCollege());
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, teacher.getId());
        availableTeacherList->addItem(item);
    }

    availableCountLabel->setText(QString("共%1位教师").arg(availableTeachers.size()));

    // 加载学院筛选选项
    if (collegeFilterCombo->count() == 0) {
        QStringList colleges;
        colleges << "全部学院";
        for (const Teacher& teacher : allTeachers) {
            if (!colleges.contains(teacher.getCollege())) {
                colleges.append(teacher.getCollege());
            }
        }
        collegeFilterCombo->addItems(colleges);
    }

    updateButtonStates();
}

void PermissionDialog::updateButtonStates()
{
    // 授权按钮：选中了可添加的教师时启用
    bool hasAvailableSelection = availableTeacherList->currentRow() >= 0;
    addPermissionBtn->setEnabled(hasAvailableSelection);

    // 撤销按钮：选中了非创建者的教师时启用
    bool hasCurrentSelection = currentTeacherList->currentRow() >= 0;
    bool canRemove = false;

    if (hasCurrentSelection) {
        Teacher selected = getSelectedCurrentTeacher();
        // 不能删除创建者的权限
        canRemove = (selected.getId() != currentTeacherId);
    }

    removePermissionBtn->setEnabled(canRemove);
}

Teacher PermissionDialog::getSelectedCurrentTeacher()
{
    int currentRow = currentTeacherList->currentRow();
    if (currentRow >= 0 && currentRow < currentTeachers.size()) {
        return currentTeachers[currentRow];
    }
    return Teacher();
}

Teacher PermissionDialog::getSelectedAvailableTeacher()
{
    int currentRow = availableTeacherList->currentRow();
    if (currentRow >= 0 && currentRow < availableTeachers.size()) {
        return availableTeachers[currentRow];
    }
    return Teacher();
}

void PermissionDialog::onAddPermissionClicked()
{
    Teacher selectedTeacher = getSelectedAvailableTeacher();
    if (selectedTeacher.getId() > 0) {
        if (database->addExamPermission(examId, selectedTeacher.getId(), false)) {
            QMessageBox::information(this, "成功",
                                     QString("已成功授权给 %1 编辑此试卷的权限！").arg(selectedTeacher.getName()));
            loadTeachersWithPermission();
            loadAvailableTeachers();
        } else {
            QMessageBox::warning(this, "失败", "授权失败，请稍后重试！");
        }
    }
}

void PermissionDialog::onRemovePermissionClicked()
{
    Teacher selectedTeacher = getSelectedCurrentTeacher();
    if (selectedTeacher.getId() > 0 && selectedTeacher.getId() != currentTeacherId) {
        int ret = QMessageBox::question(this, "确认撤销",
                                        QString("确定要撤销 %1 的编辑权限吗？").arg(selectedTeacher.getName()),
                                        QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes) {
            if (database->removeExamPermission(examId, selectedTeacher.getId())) {
                QMessageBox::information(this, "成功",
                                         QString("已撤销 %1 的编辑权限！").arg(selectedTeacher.getName()));
                loadTeachersWithPermission();
                loadAvailableTeachers();
            } else {
                QMessageBox::warning(this, "失败", "撤销权限失败，请稍后重试！");
            }
        }
    }
}

void PermissionDialog::onCurrentTeacherChanged()
{
    updateButtonStates();
}

void PermissionDialog::onAvailableTeacherChanged()
{
    updateButtonStates();
}
