#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QVector>
#include <QColor>
#include "config.h"
#include "effectcard.h"

class PropertyTile;
class Game;

// ==================== 玩家类 ====================
class Player {
public:
    Player(const QString& name, const QColor& color, int id);

    // === 属性访问 ===
    int id() const { return m_id; }
    QString name() const { return m_name; }
    QColor color() const { return m_color; }
    int money() const { return m_money; }
    int position() const { return m_position; }
    bool isBankrupt() const { return m_bankrupt; }

    // === 金钱操作 ===
    void receiveMoney(int amount);
    bool payMoney(int amount, Game* game);
    bool payMoneyTo(int amount, Player* recipient, Game* game);
    bool canAfford(int amount) const { return m_money >= amount; }

    // === 移动 ===
    void setPosition(int pos);
    void moveBy(int steps, Game* game);

    // === 跳过回合 ===
    bool skipNextTurn() const { return m_skipNextTurn; }
    void setSkipNextTurn(bool skip) { m_skipNextTurn = skip; }

    // === 效果卡管理 ===
    const QVector<EffectCard>& effectCards() const { return m_effectCards; }
    int effectCardCount() const { return m_effectCards.size(); }
    int effectCardCount(EffectCardType type) const;
    void addEffectCard(const EffectCard& card);
    void addEffectCard(EffectCardType type);
    bool useEffectCard(EffectCardType type);
    bool hasEffectCard(EffectCardType type) const;

    // === 地产管理 ===
    void addProperty(PropertyTile* tile);
    void removeProperty(PropertyTile* tile);
    const QVector<PropertyTile*>& properties() const { return m_properties; }
    int propertyCount() const { return m_properties.size(); }

    int railroadCount() const;
    int utilityCount() const;
    bool ownsFullGroup(ColorGroup group) const;
    QVector<PropertyTile*> propertiesInGroup(ColorGroup group) const;

    // === 破产 ===
    void declareBankrupt(Game* game);
    void liquidate(Game* game);

    // === 重置 ===
    void reset();

private:
    int m_id;
    QString m_name;
    QColor m_color;
    int m_money = INITIAL_MONEY;
    int m_position = 0;
    bool m_bankrupt = false;
    bool m_skipNextTurn = false;
    QVector<EffectCard> m_effectCards;
    QVector<PropertyTile*> m_properties;
};

#endif // PLAYER_H
