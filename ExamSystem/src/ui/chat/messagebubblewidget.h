#ifndef MESSAGEBUBBLEWIDGET_H
#define MESSAGEBUBBLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QTextDocument>
#include <QTextOption>
#include <QResizeEvent>
#include <QEvent>
#include <QTimer>
#include <QRegularExpression>
#include "../../models/messageinfo.h"

class MessageBubbleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MessageBubbleWidget(const MessageInfo &message, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void updateBubbleLayout();

private:
    void setupLayout();
    void updateMessageLayout();
    int calculateOptimalWidth(const QString &text);
    int getMaxBubbleWidth() const;
    QColor getBubbleColor() const;
    QColor getTextColor() const;
    QString formatMessageTime() const;
    void drawBubble(QPainter &painter, const QRect &bubbleRect);
    QString processTextForWrapping(const QString &text);
    bool needsForceWrapping(const QString &text, int maxWidth);
    QString insertBreakOpportunities(const QString &text, int maxWidth);
    QString breakLongWord(const QString &word, int maxWidth, const QFontMetrics &metrics);

    MessageInfo m_message;
    bool m_isFromMe;

    // UI组件
    QVBoxLayout *m_mainLayout;         // 改为垂直布局
    QWidget *m_headerWidget;           // 新增：头部容器（发送者信息）
    QHBoxLayout *m_headerLayout;       // 新增：头部布局
    QLabel *m_senderLabel;             // 新增：发送者姓名标签
    QLabel *m_timeLabel;               // 时间标签（从气泡内移出）
    QHBoxLayout *m_bubbleWrapperLayout;// 新增：气泡包装布局
    QWidget *m_bubbleContainer;
    QVBoxLayout *m_bubbleLayout;
    QLabel *m_messageLabel;

    // 布局参数
    static const int BUBBLE_MARGIN = 8;
    static const int BUBBLE_PADDING = 12;
    static const int BUBBLE_RADIUS = 12;
    static const int MIN_BUBBLE_WIDTH = 60;
    static const int HEADER_HEIGHT = 20;   // 新增：头部高度

    // 动态计算的参数
    int m_maxBubbleWidth;
    int m_optimalWidth;
};

#endif // MESSAGEBUBBLEWIDGET_H
