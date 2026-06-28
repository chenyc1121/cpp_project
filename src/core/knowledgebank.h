#ifndef KNOWLEDGEBANK_H
#define KNOWLEDGEBANK_H

#include <QString>
#include <QVector>
#include <QRandomGenerator>

struct KnowledgeEntry {
    QString title;
    QString content;
};

class KnowledgeBank {
public:
    static const QVector<KnowledgeEntry>& entries();
    static KnowledgeEntry drawRandom();
};

#endif // KNOWLEDGEBANK_H
