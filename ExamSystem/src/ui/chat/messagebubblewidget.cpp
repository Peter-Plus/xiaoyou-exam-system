#include "messagebubblewidget.h"
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QRegularExpression>

MessageBubbleWidget::MessageBubbleWidget(const MessageInfo &message, QWidget *parent)
    : QWidget(parent)
    , m_message(message)
    , m_isFromMe(message.isSentByMe())
    , m_maxBubbleWidth(300)
    , m_optimalWidth(MIN_BUBBLE_WIDTH)
{
    setupLayout();
    updateMessageLayout();
}

void MessageBubbleWidget::setupLayout()
{
    setObjectName("messageBubbleWidget");

    // 主垂直布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(BUBBLE_MARGIN, BUBBLE_MARGIN/2, BUBBLE_MARGIN, BUBBLE_MARGIN/2);
    m_mainLayout->setSpacing(4);

    // 创建头部容器（显示发送者信息）
    m_headerWidget = new QWidget();
    m_headerWidget->setObjectName("headerWidget");
    m_headerWidget->setFixedHeight(HEADER_HEIGHT);

    m_headerLayout = new QHBoxLayout(m_headerWidget);
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(8);

    // 创建发送者标签
    m_senderLabel = new QLabel();
    m_senderLabel->setObjectName("senderLabel");

    // 创建时间标签
    m_timeLabel = new QLabel();
    m_timeLabel->setObjectName("timeLabel");
    m_timeLabel->setText(formatMessageTime());

    // 设置字体
    QFont headerFont = m_senderLabel->font();
    headerFont.setPointSize(9);
    m_senderLabel->setFont(headerFont);
    m_timeLabel->setFont(headerFont);

    // 根据消息方向设置头部布局
    if (m_isFromMe) {
        // 我发送的消息，右对齐
        m_senderLabel->setText("我");
        m_headerLayout->addStretch();
        m_headerLayout->addWidget(m_timeLabel);
        m_headerLayout->addWidget(m_senderLabel);
    } else {
        // 收到的消息，左对齐
        m_senderLabel->setText(m_message.getSenderName());
        m_headerLayout->addWidget(m_senderLabel);
        m_headerLayout->addWidget(m_timeLabel);
        m_headerLayout->addStretch();
    }

    // 创建气泡包装布局
    m_bubbleWrapperLayout = new QHBoxLayout();
    m_bubbleWrapperLayout->setContentsMargins(0, 0, 0, 0);
    m_bubbleWrapperLayout->setSpacing(0);

    // 创建气泡容器
    m_bubbleContainer = new QWidget();
    m_bubbleContainer->setObjectName("bubbleContainer");

    m_bubbleLayout = new QVBoxLayout(m_bubbleContainer);
    m_bubbleLayout->setContentsMargins(BUBBLE_PADDING, BUBBLE_PADDING, BUBBLE_PADDING, BUBBLE_PADDING);
    m_bubbleLayout->setSpacing(0);

    // 创建消息标签
    m_messageLabel = new QLabel();
    m_messageLabel->setObjectName("messageLabel");
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setTextFormat(Qt::PlainText);
    m_messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_messageLabel->setScaledContents(false);

    // 设置消息字体
    QFont messageFont = m_messageLabel->font();
    messageFont.setPointSize(10);
    m_messageLabel->setFont(messageFont);

    // 添加消息到气泡布局
    m_bubbleLayout->addWidget(m_messageLabel);

    // 根据消息发送方向设置气泡布局
    if (m_isFromMe) {
        // 我发送的消息，右对齐
        m_bubbleWrapperLayout->addStretch();
        m_bubbleWrapperLayout->addWidget(m_bubbleContainer);
    } else {
        // 收到的消息，左对齐
        m_bubbleWrapperLayout->addWidget(m_bubbleContainer);
        m_bubbleWrapperLayout->addStretch();
    }

    // 添加到主布局
    m_mainLayout->addWidget(m_headerWidget);
    m_mainLayout->addLayout(m_bubbleWrapperLayout);

    // 设置样式
    QString headerStyle = QString(
                              "#headerWidget {"
                              "    background-color: transparent;"
                              "}"
                              "#senderLabel {"
                              "    color: %1;"
                              "    font-weight: bold;"
                              "}"
                              "#timeLabel {"
                              "    color: %2;"
                              "}"
                              ).arg(m_isFromMe ? "#666666" : "#666666")
                              .arg("#999999");

    QString bubbleStyle = QString(
                              "#bubbleContainer {"
                              "    background-color: transparent;"
                              "    border: none;"
                              "}"
                              "#messageLabel {"
                              "    background-color: transparent;"
                              "    border: none;"
                              "    color: %1;"
                              "}"
                              ).arg(getTextColor().name());

    setStyleSheet(headerStyle + bubbleStyle);

    // 连接父窗口的resize事件
    if (parent()) {
        parent()->installEventFilter(this);
    }
}

void MessageBubbleWidget::updateMessageLayout()
{
    m_maxBubbleWidth = getMaxBubbleWidth();

    QString content = m_message.getContent();

    // 处理文本换行
    QString processedContent = processTextForWrapping(content);
    m_messageLabel->setText(processedContent);

    // 计算最佳宽度
    m_optimalWidth = calculateOptimalWidth(processedContent);

    // 设置气泡容器的固定宽度
    m_bubbleContainer->setFixedWidth(m_optimalWidth);

    qDebug() << "消息气泡更新 - 内容长度:" << content.length()
             << "最大宽度:" << m_maxBubbleWidth
             << "最佳宽度:" << m_optimalWidth;
}

int MessageBubbleWidget::calculateOptimalWidth(const QString &text)
{
    if (text.isEmpty()) {
        return MIN_BUBBLE_WIDTH;
    }

    QFontMetrics metrics(m_messageLabel->font());

    // 计算单行文本的实际宽度
    int singleLineWidth = metrics.horizontalAdvance(text);

    // 加上内边距
    int totalPadding = BUBBLE_PADDING * 2;
    int singleLineWidthWithPadding = singleLineWidth + totalPadding;

    // 如果单行宽度小于等于最大宽度，使用单行宽度
    if (singleLineWidthWithPadding <= m_maxBubbleWidth) {
        return qMax(MIN_BUBBLE_WIDTH, singleLineWidthWithPadding);
    }

    // 否则使用最大宽度，让文本自动换行
    return m_maxBubbleWidth;
}

int MessageBubbleWidget::getMaxBubbleWidth() const
{
    // 基于父窗口宽度计算最大气泡宽度
    QWidget *parent = parentWidget();
    if (!parent) {
        return 300; // 默认宽度
    }

    int parentWidth = parent->width();
    if (parentWidth <= 0) {
        return 300;
    }

    // 最大气泡宽度为父窗口宽度的60%，但不超过600px，不少于200px
    int maxWidth = qMax(200, qMin(600, static_cast<int>(parentWidth * 0.6)));

    return maxWidth;
}

QColor MessageBubbleWidget::getBubbleColor() const
{
    if (m_message.isSystemMessage()) {
        return QColor("#f0f0f0");
    }

    if (m_isFromMe) {
        return QColor("#007AFF"); // iOS蓝色
    } else {
        return QColor("#E5E5EA"); // 浅灰色
    }
}

QColor MessageBubbleWidget::getTextColor() const
{
    if (m_message.isSystemMessage()) {
        return QColor("#999999");
    }

    if (m_isFromMe) {
        return QColor("#FFFFFF"); // 白色文字
    } else {
        return QColor("#000000"); // 黑色文字
    }
}

QString MessageBubbleWidget::formatMessageTime() const
{
    QDateTime sendTime = m_message.getSendTime();
    if (!sendTime.isValid()) {
        return "";
    }

    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();
    QDate messageDate = sendTime.date();

    if (messageDate == today) {
        return sendTime.toString("hh:mm");
    } else if (messageDate == today.addDays(-1)) {
        return QString("昨天 %1").arg(sendTime.toString("hh:mm"));
    } else {
        return sendTime.toString("MM-dd hh:mm");
    }
}

void MessageBubbleWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 获取气泡容器的位置和大小
    QRect bubbleRect = m_bubbleContainer->geometry();

    // 绘制气泡背景
    drawBubble(painter, bubbleRect);

    QWidget::paintEvent(event);
}

void MessageBubbleWidget::drawBubble(QPainter &painter, const QRect &bubbleRect)
{
    QPainterPath path;
    QColor bubbleColor = getBubbleColor();

    // 创建圆角矩形路径
    path.addRoundedRect(bubbleRect, BUBBLE_RADIUS, BUBBLE_RADIUS);

    // 绘制气泡背景
    painter.fillPath(path, bubbleColor);

    // 可选：绘制边框
    if (!m_isFromMe) {
        QPen pen(QColor("#D1D1D6"), 1);
        painter.setPen(pen);
        painter.drawPath(path);
    }
}

void MessageBubbleWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBubbleLayout();
}

void MessageBubbleWidget::updateBubbleLayout()
{
    // 当窗口大小变化时，重新计算气泡布局
    int newMaxWidth = getMaxBubbleWidth();

    if (newMaxWidth != m_maxBubbleWidth) {
        m_maxBubbleWidth = newMaxWidth;
        updateMessageLayout();
        update(); // 触发重绘
    }
}

bool MessageBubbleWidget::eventFilter(QObject *obj, QEvent *event)
{
    // 监听父窗口的resize事件
    if (obj == parent() && event->type() == QEvent::Resize) {
        QTimer::singleShot(0, this, &MessageBubbleWidget::updateBubbleLayout);
    }
    return QWidget::eventFilter(obj, event);
}

QString MessageBubbleWidget::processTextForWrapping(const QString &text)
{
    if (text.isEmpty()) {
        return text;
    }

    // 获取当前可用的最大文本宽度（减去内边距）
    int maxTextWidth = m_maxBubbleWidth - (BUBBLE_PADDING * 2);

    // 检查是否需要强制换行
    if (needsForceWrapping(text, maxTextWidth)) {
        return insertBreakOpportunities(text, maxTextWidth);
    }

    return text;
}

bool MessageBubbleWidget::needsForceWrapping(const QString &text, int maxWidth)
{
    QFontMetrics metrics(m_messageLabel->font());

    // 按空格和常见分隔符分割文本
    QRegularExpression regex("[\\s\\n\\r\\t]+");
    QStringList words = text.split(regex, Qt::SkipEmptyParts);

    for (const QString &word : words) {
        int wordWidth = metrics.horizontalAdvance(word);
        if (wordWidth > maxWidth) {
            return true; // 发现超长单词，需要强制换行
        }
    }

    return false;
}

QString MessageBubbleWidget::insertBreakOpportunities(const QString &text, int maxWidth)
{
    QFontMetrics metrics(m_messageLabel->font());
    QString result;

    // 按空格分割文本，保持正常的单词边界
    QStringList words = text.split(' ', Qt::KeepEmptyParts);

    for (int i = 0; i < words.size(); ++i) {
        QString word = words[i];

        if (i > 0) {
            result += " "; // 添加空格分隔符
        }

        // 检查单词是否超长
        int wordWidth = metrics.horizontalAdvance(word);
        if (wordWidth <= maxWidth) {
            // 正常长度的单词，直接添加
            result += word;
        } else {
            // 超长单词，需要在合适位置插入换行机会
            result += breakLongWord(word, maxWidth, metrics);
        }
    }

    return result;
}

QString MessageBubbleWidget::breakLongWord(const QString &word, int maxWidth, const QFontMetrics &metrics)
{
    if (word.isEmpty()) {
        return word;
    }

    QString result;
    QString currentPart;

    for (int i = 0; i < word.length(); ++i) {
        QChar ch = word[i];
        QString testPart = currentPart + ch;

        int testWidth = metrics.horizontalAdvance(testPart);

        if (testWidth <= maxWidth) {
            // 当前字符可以添加到当前部分
            currentPart += ch;
        } else {
            // 当前字符会超出宽度限制
            if (!currentPart.isEmpty()) {
                // 添加当前部分和软换行符
                result += currentPart;
                result += "\u200B"; // 零宽度空格，提供换行机会
                currentPart = QString(ch);
            } else {
                // 单个字符就超宽（极少见情况），强制添加
                result += ch;
                result += "\u200B";
            }
        }
    }

    // 添加最后一部分
    if (!currentPart.isEmpty()) {
        result += currentPart;
    }

    return result;
}
