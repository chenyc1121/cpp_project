#ifndef EFFECTCARD_H
#define EFFECTCARD_H

#include <QString>

enum class EffectCardType {
    ROLL_AGAIN,        // 再丢一次骰子
    UNIVERSAL_DICE,    // 万能骰子
    VIRTUAL_FUNCTION,  // 虚函数卡
    SKIP_EFFECT,       // 跳过卡（跳过当前地块负面效果）
    ITERATOR_CARD      // 迭代器卡（迭代器格之间传送）
};

// 迭代器卡子类型
enum class IteratorSubtype {
    INPUT,           // 读写迭代器 — 仅支持 ++
    BIDIRECTIONAL,   // 双向迭代器 — 支持 ++, --
    RANDOM_ACCESS    // 随机访问迭代器 — 支持 ++, --, +=2, -=2
};

// 迭代器操作
enum class IteratorOp {
    INCREMENT,     // ++
    DECREMENT,     // --
    PLUS_EQ_2,     // +=2
    MINUS_EQ_2     // -=2
};

struct EffectCard {
    EffectCardType type;
    QString name;
    QString description;
    int price;
    IteratorSubtype iterSubtype = IteratorSubtype::INPUT;  // 仅 ITERATOR_CARD 有效
};

// 工厂函数
EffectCard createEffectCard(EffectCardType type);
// 随机返回一种效果卡类型（QA奖励用）
EffectCardType randomEffectCardType();
// 迭代器卡辅助
IteratorSubtype randomIteratorSubtype();
QString iteratorSubtypeName(IteratorSubtype sub);
QString iteratorOpName(IteratorOp op);
bool iteratorSubtypeSupports(IteratorSubtype sub, IteratorOp op);

#endif // EFFECTCARD_H
