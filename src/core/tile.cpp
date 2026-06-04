#include "tile.h"
#include "player.h"
#include "game.h"
#include "effectcard.h"

// ==================== 工厂函数 ====================
Tile* createTile(const TileDef& def, int index) {
    switch (def.type) {
    case TileType::START:         return new StartTile(def, index);
    case TileType::PROPERTY:      return new PropertyTile(def, index);
    case TileType::QA:            return new QATile(def, index);
    case TileType::TAX:           return new TaxTile(def, index);
    case TileType::SHOP:          return new ShopTile(def, index);
    case TileType::COMPUTER_LAB:  return new ComputerLabTile(def, index);
    case TileType::SHOP_ENTRANCE: return new ShopEntranceTile(def, index);
    case TileType::UTILITY:       return new UtilityTile(def, index);
    case TileType::RAILROAD:      return new RailroadTile(def, index);
    case TileType::STATICVAL:     return new StaticvalTile(def, index);
    case TileType::VIRTUALFUNC:   return new VirtualfuncTile(def, index);
    case TileType::ITERATOR:      return new IteratorTile(def, index);
    }
    return nullptr;
}


// ==================== Tile 基类 ====================
Tile::Tile(const TileDef& def, int index)
    : m_index(index), m_name(def.name), m_type(def.type),
      m_group(def.group), m_price(def.price),
      m_titleBarText(def.titleBarText), m_infoText(def.infoText),
      m_titleDetail(def.titleDetail ? QString(def.titleDetail) : QString())
{
}

void Tile::passBy(Player*, Game*) {
}


// ==================== 起点 ====================
StartTile::StartTile(const TileDef& def, int index)
    : Tile(def, index) {}

void StartTile::landOn(Player* player, Game* game) {
    player->receiveMoney(START_BONUS * 2);
    game->logEvent(player->name() + QString(" 停在起点，获得奖金 %1 元！").arg(START_BONUS * 2));
    emit game->playerUpdated(player);
}

void StartTile::passBy(Player* player, Game* game) {
    player->receiveMoney(START_BONUS);
    game->logEvent(player->name() + QString(" 经过起点，获得 %1 元").arg(START_BONUS));
    emit game->playerUpdated(player);
}


// ==================== 地产 ====================
PropertyTile::PropertyTile(const TileDef& def, int index)
    : Tile(def, index), m_houseCost(def.houseCost)
{
    m_rentTable[0] = def.baseRent;
    m_rentTable[1] = def.rentWith1House;
    m_rentTable[2] = def.rentWith2House;
    m_rentTable[3] = def.rentWith3House;
    m_rentTable[4] = def.rentWith4House;
    m_rentTable[5] = def.rentWithHotel;
}

void PropertyTile::landOn(Player* player, Game* game) {
    if (m_owner == nullptr) {
        game->setWaitingForDecision(true);
        emit game->promptBuyProperty(m_index, player);
    } else if (m_owner == player) {
        if (canBuildHouse(player)) {
            game->setWaitingForDecision(true);
            emit game->promptBuildHouse(m_index, player);
        }
    } else if (!m_owner->isBankrupt()) {
        int rent = calculateRent();
        QString msg = player->name() + " 停在 " + m_name
                      + "（属于" + m_owner->name() + "），支付租金 " + QString::number(rent) + " 元";
        game->logEvent(msg);
        player->payMoneyTo(rent, m_owner, game);
        emit game->playerUpdated(player);
        emit game->playerUpdated(m_owner);
    }
}

int PropertyTile::calculateRent(int) const {
    int level = 0;
    if (m_hasHotel) {
        level = 5;
    } else {
        level = qBound(0, m_houses, 4);
    }
    int rent = m_rentTable[level];
    /*if (level == 0 && ownsFullGroup()) {
        rent *= 2;
    }*/
    return rent;
}

/*bool PropertyTile::ownsFullGroup() const {
    if (m_owner == nullptr) return false;
    return m_owner->ownsFullGroup(m_group);
}*/

bool PropertyTile::canBuildHouse(const Player* player) const {
    if (m_hasHotel) return false;
    if (m_owner == nullptr) return false;
    if (player == nullptr || m_owner != player) return false;
    return true;
}

void PropertyTile::buildHouse() {
    if (m_houses < 4) {
        m_houses++;
    } else if (m_houses == 4) {
        m_hasHotel = true;
        m_houses = 0;
    }
}

void PropertyTile::removeHouses(int count) {
    if (m_hasHotel) {
        m_hasHotel = false;
        m_houses = 4;
    } else {
        m_houses = qMax(0, m_houses - count);
    }
}

void PropertyTile::reset() {
    m_owner = nullptr;
    m_houses = 0;
    m_hasHotel = false;
}


// ==================== 虚函数格 ====================
VirtualfuncTile::VirtualfuncTile(const TileDef& def, int index)
    : PropertyTile(def, index),
      m_ratio(def.ratio),
      m_buyRatio(def.buy_ratio),
      m_rentRatio(def.rent_ratio),
      m_buyDecay(def.buy_decay),
      m_rentDecay(def.rent_decay),
      m_rentIsPureVirtual(def.rentIsPureVirtual),
      m_rentIsNonVirtual(def.rentIsNonVirtual)
{
}

void VirtualfuncTile::landOn(Player* player, Game* game) {
    if (m_owner == nullptr) {
        // 无主：提示购买
        game->setWaitingForDecision(true);
        emit game->promptBuyProperty(m_index, player);
    } else if (m_owner == player) {
        // 自己踩中：提示建房
        if (canBuildHouse(player)) {
            game->setWaitingForDecision(true);
            emit game->promptBuildHouse(m_index, player);
        }
    } else if (!m_owner->isBankrupt()) {
        // 别人踩中地主的地 —— 地主决定是否用虚函数卡
        Player* payer = player;
        Player* owner = m_owner;

        // Case 1: 纯虚函数租金 + 地主无卡 → 无法收租
        if (m_rentIsPureVirtual && !owner->hasEffectCard(EffectCardType::VIRTUAL_FUNCTION)) {
            game->logEvent(payer->name() + " 停在 " + m_name
                           + "（属于" + owner->name() + "），"
                           + "该格租金为纯虚函数，无法收取租金");
            game->handlePureVirtualNoRent(payer, m_index);
            return;
        }

        // Case 2: 地主有卡 → 弹窗让地主选择基类/派生类
        if (owner->hasEffectCard(EffectCardType::VIRTUAL_FUNCTION)) {
            int baseRent = PropertyTile::calculateRent();
            int derivedRent = VirtualfuncTile::calculateRent();
            game->logEvent(payer->name() + " 停在虚函数格 " + m_name
                           + "（属于" + owner->name() + "），"
                           + owner->name() + " 可选择基类/派生类租金");
            game->setWaitingForDecision(true);
            emit game->promptVirtualFuncRent(payer, m_index, owner, baseRent, derivedRent);
            return;
        }

        // Case 3: 地主无卡 → 收基类租金
        int rent = PropertyTile::calculateRent();
        game->logEvent(payer->name() + " 停在 " + m_name
                       + "（属于" + owner->name() + "），支付租金 "
                       + QString::number(rent) + " 元");
        payer->payMoneyTo(rent, owner, game);
        emit game->playerUpdated(payer);
        emit game->playerUpdated(owner);
    }
}

int VirtualfuncTile::price() const {
    return m_price * m_buyRatio / 100 - m_buyDecay;
}

int VirtualfuncTile::calculateRent(int) const {
    int level = 0;
    if (m_hasHotel) {
        level = 5;
    } else {
        level = qBound(0, m_houses, 4);
    }
    int rent = m_rentTable[level] * m_rentRatio / 100 - m_rentDecay;
    if (m_owner) {
        rent += m_owner->money() * m_ratio / 100;
    }
    if (rent < 0) rent = 0;
    return rent;
}

void VirtualfuncTile::buildHouse() {
    PropertyTile::buildHouse();
}
// ==================== 静态成员变量格 ====================
StaticvalTile::StaticvalTile(const TileDef& def, int index)
    : PropertyTile(def, index)
{
}

void StaticvalTile::landOn(Player* player, Game* game) {
    PropertyTile::landOn(player, game);
}

int StaticvalTile::calculateRent(int) const {
    int rent = PropertyTile::calculateRent();
    if (ownsFullGroup()) {
        rent += m_rentTable[0];  // 成套后增加空地基础租金
    }
    return rent;
}

bool StaticvalTile::ownsFullGroup() const {
    if (m_owner == nullptr) return false;
    return m_owner->ownsFullGroup(m_group);
}

// ==================== 问答格 ====================
QATile::QATile(const TileDef& def, int index)
    : Tile(def, index) {}

void QATile::landOn(Player* player, Game* game) {
    game->logEvent(player->name() + " 停在问答格！");
    game->setWaitingForDecision(true);
    emit game->promptQA(player, m_index);
}


// ==================== 税收 ====================
TaxTile::TaxTile(const TileDef& def, int index)
    : Tile(def, index) {}

void TaxTile::landOn(Player* player, Game* game) {
    game->logEvent(player->name() + " 花掉了" + QString::number(m_price) + " 元，吃成了大卫戴！");
    player->payMoney(m_price, game);
    emit game->playerUpdated(player);
}


// ==================== 商店 ====================
ShopTile::ShopTile(const TileDef& def, int index)
    : Tile(def, index) {}

void ShopTile::landOn(Player* player, Game* game) {
    game->logEvent(player->name() + " 来到商店！");
    game->setWaitingForDecision(true);
    emit game->promptShop(player);
}


// ==================== 上机课 ====================
ComputerLabTile::ComputerLabTile(const TileDef& def, int index)
    : Tile(def, index) {}

void ComputerLabTile::landOn(Player* player, Game* game) {
    game->logEvent(player->name() + " 进入上机课（计算机实验课），需回答一道题，下回合跳过！");
    player->setSkipNextTurn(true);
    game->setWaitingForDecision(true);
    emit game->promptComputerLab(player);
}


// ==================== 商店入口 ====================
ShopEntranceTile::ShopEntranceTile(const TileDef& def, int index)
    : Tile(def, index) {}

void ShopEntranceTile::landOn(Player* player, Game* game) {
    game->logEvent(player->name() + " 经过29楼地下室");
    game->setWaitingForDecision(true);
    emit game->promptShopEntrance(player);
}


// ==================== 迭代器格 ====================
IteratorTile::IteratorTile(const TileDef& def, int index)
    : Tile(def, index), m_baseRent(def.baseRent)
{
}

void IteratorTile::landOn(Player* player, Game* game) {
    // 检查迭代器卡 — 玩家可选择使用卡片传送
    if (player->hasEffectCard(EffectCardType::ITERATOR_CARD)) {
        game->logEvent(player->name() + " 到达迭代器格 " + m_name + "，可使用迭代器卡传送");
        game->setWaitingForDecision(true);
        emit game->promptIteratorCard(player, m_index);
        return;
    }

    // 无迭代器卡，正常流程
    if (m_owner == nullptr) {
        game->setWaitingForDecision(true);
        emit game->promptBuyProperty(m_index, player);
    } else if (m_owner != player && !m_owner->isBankrupt()) {
        int rent = calculateRent();
        game->logEvent(player->name() + " 到达迭代器格" + m_name
                       + "（属于" + m_owner->name() + "），支付租金 " + QString::number(rent) + " 元");
        player->payMoneyTo(rent, m_owner, game);
        emit game->playerUpdated(player);
        emit game->playerUpdated(m_owner);
    }
}

int IteratorTile::calculateRent() const {
    if (m_owner == nullptr) return 0;
    int count = m_owner->iteratorTileCount();
    if (count <= 0) return m_baseRent;
    return m_baseRent * (1 << (count - 1));
}


// ==================== 公共设施 ====================
UtilityTile::UtilityTile(const TileDef& def, int index)
    : Tile(def, index) {}

void UtilityTile::landOn(Player* player, Game* game) {
    if (m_owner == nullptr) {
        game->setWaitingForDecision(true);
        emit game->promptBuyProperty(m_index, player);
    } else if (m_owner != player && !m_owner->isBankrupt()) {
        int diceTotal = game->lastDiceTotal();
        int rent = calculateRent(diceTotal);
        game->logEvent(player->name() + " 使用" + m_name
                       + "（属于" + m_owner->name() + "），支付租金 " + QString::number(rent) + " 元");
        player->payMoneyTo(rent, m_owner, game);
        emit game->playerUpdated(player);
        emit game->playerUpdated(m_owner);
    }
}

int UtilityTile::calculateRent(int diceValue) const {
    int multiplier = 4;
    if (m_owner && m_owner->utilityCount() >= 2) {
        multiplier = 10;
    }
    return diceValue * multiplier;
}


// ==================== 铁路/车站 ====================
RailroadTile::RailroadTile(const TileDef& def, int index)
    : Tile(def, index) {}

void RailroadTile::landOn(Player* player, Game* game) {
    if (m_owner == nullptr) {
        game->setWaitingForDecision(true);
        emit game->promptBuyProperty(m_index, player);
    } else if (m_owner != player && !m_owner->isBankrupt()) {
        int rent = calculateRent();
        game->logEvent(player->name() + " 乘坐" + m_name
                       + "（属于" + m_owner->name() + "），支付车费 " + QString::number(rent) + " 元");
        player->payMoneyTo(rent, m_owner, game);
        emit game->playerUpdated(player);
        emit game->playerUpdated(m_owner);
    }
}

int RailroadTile::calculateRent() const {
    if (m_owner == nullptr) return 250;
    int count = m_owner->railroadCount();
    return 250 * (1 << (count - 1));
}
