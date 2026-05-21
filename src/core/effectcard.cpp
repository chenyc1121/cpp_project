#include "effectcard.h"
#include "config.h"
#include <QRandomGenerator>

EffectCard createEffectCard(EffectCardType type) {
    switch (type) {
    case EffectCardType::ROLL_AGAIN:
        return {type, "再丢一次骰子", "使用后可以再掷一次骰子", CARD_PRICE_ROLL_AGAIN};
    case EffectCardType::UNIVERSAL_DICE:
        return {type, "万能骰子", "使用后可以自选骰子点数（1-6）", CARD_PRICE_UNIVERSAL_DICE};
    case EffectCardType::VIRTUAL_FUNCTION:
        return {type, "虚函数卡", "虚函数卡 - 选择基类/派生类行为", CARD_PRICE_VIRTUAL_FUNCTION};
    case EffectCardType::SKIP_EFFECT:
        return {type, "跳过卡", "使用后可跳过当前地块的负面效果（税收、租金等）", CARD_PRICE_SKIP_EFFECT};
    case EffectCardType::ITERATOR_CARD: {
        IteratorSubtype sub = randomIteratorSubtype();
        QString subName = iteratorSubtypeName(sub);
        QString desc = QString("在迭代器格之间传送。当前类型：%1\n"
                               "支持的操作：%2")
            .arg(subName)
            .arg([sub]() {
                QString ops;
                if (iteratorSubtypeSupports(sub, IteratorOp::INCREMENT)) ops += "++ ";
                if (iteratorSubtypeSupports(sub, IteratorOp::DECREMENT)) ops += "-- ";
                if (iteratorSubtypeSupports(sub, IteratorOp::PLUS_EQ_2)) ops += "+=2 ";
                if (iteratorSubtypeSupports(sub, IteratorOp::MINUS_EQ_2)) ops += "-=2";
                return ops.trimmed();
            }());
        return {type, QString("迭代器卡(%1)").arg(subName), desc, CARD_PRICE_ITERATOR, sub};
    }
    }
    return {type, "未知", "未知效果卡", 0};
}

EffectCardType randomEffectCardType() {
    int r = QRandomGenerator::global()->bounded(5);
    switch (r) {
    case 0: return EffectCardType::ROLL_AGAIN;
    case 1: return EffectCardType::UNIVERSAL_DICE;
    case 2: return EffectCardType::VIRTUAL_FUNCTION;
    case 3: return EffectCardType::SKIP_EFFECT;
    case 4: return EffectCardType::ITERATOR_CARD;
    }
    return EffectCardType::ROLL_AGAIN;
}

IteratorSubtype randomIteratorSubtype() {
    int r = QRandomGenerator::global()->bounded(3);
    switch (r) {
    case 0: return IteratorSubtype::INPUT;
    case 1: return IteratorSubtype::BIDIRECTIONAL;
    case 2: return IteratorSubtype::RANDOM_ACCESS;
    }
    return IteratorSubtype::INPUT;
}

QString iteratorSubtypeName(IteratorSubtype sub) {
    switch (sub) {
    case IteratorSubtype::INPUT:          return "读写迭代器";
    case IteratorSubtype::BIDIRECTIONAL:  return "双向迭代器";
    case IteratorSubtype::RANDOM_ACCESS:  return "随机访问迭代器";
    }
    return "未知";
}

QString iteratorOpName(IteratorOp op) {
    switch (op) {
    case IteratorOp::INCREMENT:  return "++";
    case IteratorOp::DECREMENT:  return "--";
    case IteratorOp::PLUS_EQ_2:  return "+=2";
    case IteratorOp::MINUS_EQ_2: return "-=2";
    }
    return "??";
}

bool iteratorSubtypeSupports(IteratorSubtype sub, IteratorOp op) {
    switch (sub) {
    case IteratorSubtype::INPUT:
        return op == IteratorOp::INCREMENT;
    case IteratorSubtype::BIDIRECTIONAL:
        return op == IteratorOp::INCREMENT || op == IteratorOp::DECREMENT;
    case IteratorSubtype::RANDOM_ACCESS:
        return true;  // supports all 4 operations
    }
    return false;
}
