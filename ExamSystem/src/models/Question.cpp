#include "question.h"

// 构造函数
Question::Question(): m_questionId(0), m_score(0), m_creatorId(0), m_modifierId(0), m_frequency(0), m_isPublic(true), m_isFavorited(false){}

Question::Question(int questionId, const QString &questionType, const QString &courseName,
                   int score, const QString &content, const QString &answer,
                   int creatorId, int modifierId, int frequency, bool isPublic)
    : m_questionId(questionId), m_questionType(questionType), m_courseName(courseName)
    , m_score(score), m_content(content), m_answer(answer), m_creatorId(creatorId)
    , m_modifierId(modifierId), m_frequency(frequency), m_isPublic(isPublic)
    , m_isFavorited(false){}

//Getter方法
int Question::getQuestionId() const { return m_questionId; }
QString Question::getQuestionType() const { return m_questionType; }
QString Question::getCourseName() const { return m_courseName; }
int Question::getScore() const { return m_score; }
QString Question::getContent() const { return m_content; }
QString Question::getAnswer() const { return m_answer; }
int Question::getCreatorId() const { return m_creatorId; }
int Question::getModifierId() const { return m_modifierId; }
int Question::getFrequency() const { return m_frequency; }
bool Question::getIsPublic() const { return m_isPublic; }
QString Question::getCreatorName() const { return m_creatorName; }
QString Question::getModifierName() const { return m_modifierName; }
bool Question::getIsFavorited() const { return m_isFavorited; }

//Setter方法
void Question::setQuestionId(int questionId) { m_questionId = questionId; }
void Question::setQuestionType(const QString &questionType) { m_questionType = questionType; }
void Question::setCourseName(const QString &courseName) { m_courseName = courseName; }
void Question::setScore(int score) { m_score = score; }
void Question::setContent(const QString &content) { m_content = content; }
void Question::setAnswer(const QString &answer) { m_answer = answer; }
void Question::setCreatorId(int creatorId) { m_creatorId = creatorId; }
void Question::setModifierId(int modifierId) { m_modifierId = modifierId; }
void Question::setFrequency(int frequency) { m_frequency = frequency; }
void Question::setIsPublic(bool isPublic) { m_isPublic = isPublic; }
void Question::setCreatorName(const QString &creatorName) { m_creatorName = creatorName; }
void Question::setModifierName(const QString &modifierName) { m_modifierName = modifierName; }
void Question::setIsFavorited(bool isFavorited) { m_isFavorited = isFavorited; }

//预览内容显示
QString Question::getContentPreview(int maxLength) const
{
    if (m_content.length() <= maxLength) {
        return m_content;
    }
    return m_content.left(maxLength) + "...";
}

//是否公开显示
QString Question::getPublicStatusText() const
{
    return m_isPublic ? "公开" : "私有";
}

//验证题目有效
bool Question::isValid() const
{
    return !m_questionType.isEmpty() &&
           !m_courseName.isEmpty() &&
           !m_content.isEmpty() &&
           !m_answer.isEmpty() &&
           m_score > 0;
}

//比较
bool Question::operator==(const Question &other) const
{
    return m_questionId == other.m_questionId;
}






