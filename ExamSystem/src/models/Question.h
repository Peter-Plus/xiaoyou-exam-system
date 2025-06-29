#ifndef QUESTION_H
#define QUESTION_H

#include <QString>

class Question
{
public:
    // 构造函数
    Question();
    Question(int questionId, const QString &questionType, const QString &courseName,
             int score, const QString &content, const QString &answer,
             int creatorId, int modifierId, int frequency = 0, bool isPublic = true);

    // getter方法
    int getQuestionId() const;
    QString getQuestionType() const;
    QString getCourseName() const;
    int getScore() const;
    QString getContent() const;
    QString getAnswer() const;
    int getCreatorId() const;
    int getModifierId() const;
    int getFrequency() const;
    bool getIsPublic() const;
    QString getCreatorName() const;
    QString getModifierName() const;
    bool getIsFavorited() const;

    // setter方法
    void setQuestionId(int questionId);
    void setQuestionType(const QString &questionType);
    void setCourseName(const QString &courseName);
    void setScore(int score);
    void setContent(const QString &content);
    void setAnswer(const QString &answer);
    void setCreatorId(int creatorId);
    void setModifierId(int modifierId);
    void setFrequency(int frequency);
    void setIsPublic(bool isPublic);
    void setCreatorName(const QString &creatorName);
    void setModifierName(const QString &modifierName);
    void setIsFavorited(bool isFavorited);

    // 辅助方法
    QString getContentPreview(int maxLength = 50) const; // 获取内容预览
    QString getPublicStatusText() const; // 获取公开状态文字
    bool isValid() const; // 检查基本有效性
    bool operator==(const Question &other) const;

private:
    // 基本属性（对应数据库字段）
    int m_questionId;
    QString m_questionType;
    QString m_courseName;
    int m_score;
    QString m_content;
    QString m_answer;
    int m_creatorId;
    int m_modifierId;
    int m_frequency;
    bool m_isPublic;

    // 扩展属性（用于显示）
    QString m_creatorName;
    QString m_modifierName;
    bool m_isFavorited;
};

#endif // QUESTION_H
