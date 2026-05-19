#include "player.h"
#include "tile.h"
#include "board.h"
#include "game.h"

Player::Player(const QString& name, const QColor& color, int id)
    : m_id(id), m_name(name), m_color(color)
{
}


// ==================== 金钱操作 ====================
void Player::receiveMoney(int amount) {
    m_money += amount;
}

bool Player::payMoney(int amount, Game* game) {
    if (m_money >= amount) {
        m_money -= amount;
        return true;
    }
    game->logEvent(m_name + " 无法支付 " + QString::number(amount) + " 元！");
    return false;
}

bool Player::payMoneyTo(int amount, Player* recipient, Game* game) {
    if (m_money >= amount) {
        m_money -= amount;
        recipient->receiveMoney(amount);
        return true;
    }
    if (m_money > 0) {
        recipient->receiveMoney(m_money);
        game->logEvent(m_name + " 付了全部余额 " + QString::number(m_money) + " 元");
        m_money = 0;
    }
    game->logEvent(m_name + " 无力支付，宣告破产！");
    declareBankrupt(game);
    return false;
}


// ==================== 移动 ====================
void Player::setPosition(int pos) {
    m_position = pos;
}

void Player::moveBy(int steps, Game* game) {
    int oldPos = m_position;
    m_position = (m_position + steps) % BOARD_SIZE;

    if (m_position < oldPos && steps > 0) {
        game->logEvent(m_name + " 经过起点！");
    }

    game->logEvent(m_name + " 从 " + QString::number(oldPos)
                   + " 移动到 " + QString::number(m_position)
                   + "（" + game->board().tileAt(m_position)->name() + "）");
}


// ==================== 效果卡管理 ====================
int Player::effectCardCount(EffectCardType type) const {
    int count = 0;
    for (const auto& card : m_effectCards) {
        if (card.type == type) count++;
    }
    return count;
}

void Player::addEffectCard(const EffectCard& card) {
    m_effectCards.append(card);
}

void Player::addEffectCard(EffectCardType type) {
    m_effectCards.append(createEffectCard(type));
}

bool Player::useEffectCard(EffectCardType type) {
    for (int i = 0; i < m_effectCards.size(); ++i) {
        if (m_effectCards[i].type == type) {
            m_effectCards.removeAt(i);
            return true;
        }
    }
    return false;
}

bool Player::hasEffectCard(EffectCardType type) const {
    for (const auto& card : m_effectCards) {
        if (card.type == type) return true;
    }
    return false;
}


// ==================== 地产管理 ====================
void Player::addProperty(PropertyTile* tile) {
    if (!m_properties.contains(tile)) {
        m_properties.append(tile);
    }
}

void Player::removeProperty(PropertyTile* tile) {
    m_properties.removeAll(tile);
}

int Player::railroadCount() const {
    int count = 0;
    for (auto* t : m_properties) {
        if (t->type() == TileType::RAILROAD) count++;
    }
    return count;
}

int Player::utilityCount() const {
    int count = 0;
    for (auto* t : m_properties) {
        if (t->type() == TileType::UTILITY) count++;
    }
    return count;
}

bool Player::ownsFullGroup(ColorGroup group) const {
    int needed = colorGroupSize(group);
    int owned = 0;
    for (auto* t : m_properties) {
        if (t->group() == group) owned++;
    }
    return owned >= needed;
}

QVector<PropertyTile*> Player::propertiesInGroup(ColorGroup group) const {
    QVector<PropertyTile*> result;
    for (auto* t : m_properties) {
        if (t->group() == group) {
            result.append(t);
        }
    }
    return result;
}


// ==================== 破产 ====================
void Player::declareBankrupt(Game* game) {
    m_bankrupt = true;
    game->logEvent(m_name + " 已破产！");
    liquidate(game);
}

void Player::liquidate(Game* game) {
    Q_UNUSED(game)
    for (auto* tile : m_properties) {
        tile->reset();
    }
    m_properties.clear();
    m_effectCards.clear();
    m_money = 0;
    m_skipNextTurn = false;
}


// ==================== 重置 ====================
void Player::reset() {
    m_money = INITIAL_MONEY;
    m_position = 0;
    m_bankrupt = false;
    m_skipNextTurn = false;
    m_effectCards.clear();
    m_properties.clear();
}
