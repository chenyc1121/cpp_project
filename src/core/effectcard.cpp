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
        return {type, "虚函数卡", "虚函数卡 - 待实现", CARD_PRICE_VIRTUAL_FUNCTION};
    case EffectCardType::SKIP_EFFECT:
        return {type, "跳过卡", "使用后可跳过当前地块的负面效果（税收、租金等）", CARD_PRICE_SKIP_EFFECT};
    }
    return {type, "未知", "未知效果卡", 0};
}

EffectCardType randomEffectCardType() {
    int r = QRandomGenerator::global()->bounded(4);
    switch (r) {
    case 0: return EffectCardType::ROLL_AGAIN;
    case 1: return EffectCardType::UNIVERSAL_DICE;
    case 2: return EffectCardType::VIRTUAL_FUNCTION;
    case 3: return EffectCardType::SKIP_EFFECT;
    }
    return EffectCardType::ROLL_AGAIN;
}
