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

    // 新增群聊支持方法
    void setGroupChatMode(bool isGroupChat);

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

    // 新增群聊相关方法
    void updateGroupChatLayout();
    void updateGroupChatStyles();
    void updateHeaderVisibility();
    void setupInitialStyles();

    MessageInfo m_message;
    bool m_isFromMe;
    bool m_isGroupChat;  // 新增群聊模式标识

    // UI组件
    QVBoxLayout *m_mainLayout;
    QWidget *m_headerWidget;
    QHBoxLayout *m_headerLayout;
    QLabel *m_senderLabel;
    QLabel *m_timeLabel;
    QHBoxLayout *m_bubbleWrapperLayout;
    QWidget *m_bubbleContainer;
    QVBoxLayout *m_bubbleLayout;
    QLabel *m_messageLabel;

    // 布局参数
    static const int BUBBLE_MARGIN = 8;
    static const int BUBBLE_PADDING = 12;
    static const int BUBBLE_RADIUS = 12;
    static const int MIN_BUBBLE_WIDTH = 60;
    static const int HEADER_HEIGHT = 20;

    // 动态计算的参数
    int m_maxBubbleWidth;
    int m_optimalWidth;
};

#endif // MESSAGEBUBBLEWIDGET_H
