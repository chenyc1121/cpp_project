#ifndef EFFECTCARD_H
#define EFFECTCARD_H

#include <QString>

enum class EffectCardType {
    ROLL_AGAIN,        // 再丢一次骰子
    UNIVERSAL_DICE,    // 万能骰子
    VIRTUAL_FUNCTION,  // 虚函数卡（占位接口）
    SKIP_EFFECT        // 跳过卡（跳过当前地块负面效果）
};

struct EffectCard {
    EffectCardType type;
    QString name;
    QString description;
    int price;
};

// 工厂函数：根据类型创建效果卡
EffectCard createEffectCard(EffectCardType type);
// 随机返回一种效果卡类型（QA奖励用）
EffectCardType randomEffectCardType();

#endif // EFFECTCARD_H
