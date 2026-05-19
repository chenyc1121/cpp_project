#ifndef QUESTIONBANK_H
#define QUESTIONBANK_H

#include <QString>
#include <QVector>

struct Question {
    QString text;
    QString optionA;
    QString optionB;
    QString optionC;
    QString optionD;
    int correctIndex;  // 0=A, 1=B, 2=C, 3=D
};

class QuestionBank {
public:
    static Question drawRandom();
    static int size();
};

#endif // QUESTIONBANK_H
