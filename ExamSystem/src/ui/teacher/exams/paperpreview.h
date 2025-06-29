#ifndef PAPERPREVIEW_H
#define PAPERPREVIEW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QFont>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QTextDocument>
#include "database.h"
#include "exam.h"
#include "question.h"
#include <QSqlQuery>
#include <QSqlError>

class PaperPreview : public QDialog
{
    Q_OBJECT

public:
    explicit PaperPreview(int examId, Database* database, QWidget *parent = nullptr);

private slots:
    void onPrintClicked();
    void onCloseClicked();

private:
    void setupUI();
    void loadExamData();
    void generatePreviewContent();
    QString formatQuestion(const Question& question, int questionNumber);
    QString getQuestionContent(const Question& question);

private:
    int examId;
    Database* database;
    Exam currentExam;
    QList<Question> examQuestions;
    QString generatePaperContent();
    // UI组件
    QVBoxLayout* mainLayout;
    QHBoxLayout* buttonLayout;
    QScrollArea* scrollArea;
    QWidget* contentWidget;
    QVBoxLayout* contentLayout;
    QLabel* titleLabel;
    QLabel* infoLabel;
    QTextEdit* previewTextEdit;
    QPushButton* printBtn;
    QPushButton* closeBtn;

};

#endif // PAPERPREVIEW_H
