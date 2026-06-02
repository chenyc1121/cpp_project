#ifndef TILE_H
#define TILE_H

#include <QString>
#include <QVector>
#include "config.h"

class Player;
class Game;

// ==================== 地块基类 ====================
class Tile {
public:
    explicit Tile(const TileDef& def, int index);
    virtual ~Tile() = default;

    // 玩家停留在此地时触发
    virtual void landOn(Player* player, Game* game) = 0;
    // 玩家经过此地时触发（大多数地块无效果）
    virtual void passBy(Player* player, Game* game);

    // 基本属性
    int index() const { return m_index; }
    QString name() const { return m_name; }
    TileType type() const { return m_type; }
    ColorGroup group() const { return m_group; }
    virtual int price() const { return m_price; }
    QString titleBarText() const { return m_titleBarText; }
    QString infoText() const { return m_infoText; }
    QString titleDetail() const { return m_titleDetail.isEmpty() ? m_titleBarText : m_titleDetail; }

protected:
    int m_index;            // 在地图上的索引 (0-27)
    QString m_name;         // 地块名称
    TileType m_type;        // 地块类型
    ColorGroup m_group;     // 颜色组
    int m_price;            // 购买价格
    QString m_titleBarText; // 标题栏显示文字
    QString m_infoText;     // 详情按钮弹窗文字
    QString m_titleDetail;  // [i] 按钮弹窗文字（空则回退到 titleBarText）
};


// ==================== 起点 ====================
class StartTile : public Tile {
public:
    explicit StartTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;
    void passBy(Player* player, Game* game) override;
};


// ==================== 可购买地产（街道/区域） ====================
class PropertyTile : public Tile {
public:
    explicit PropertyTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;

    // 地产特有属性
    Player* owner() const { return m_owner; }
    void setOwner(Player* p) { m_owner = p; }
    int houses() const { return m_houses; }
    bool hasHotel() const { return m_hasHotel; }
    int houseCost() const { return m_houseCost; }
    int baseRent() const { return m_rentTable[0]; }
    int rentAtLevel(int level) const { return m_rentTable[level]; }  // 0=空地, 1-4=1-4栋, 5=旅馆
    int mortgageValue() const { return m_price / 2; }

    // 建造/升级
    bool canBuildHouse(const Player* player) const;
    virtual void buildHouse();
    void removeHouses(int count = 1);

    // 计算当前租金（根据房屋数）
    virtual int calculateRent(int diceValue = 0) const;

    // 重置（用于新游戏）
    void reset();

protected:
    Player* m_owner = nullptr;
    int m_houses = 0;
    bool m_hasHotel = false;
    int m_houseCost;

    // 租金表（索引0=空地, 1-4=1-4栋房子, 5=旅馆）
    int m_rentTable[6];
};


//==================== 静态成员变量格 ====================
class StaticvalTile : public PropertyTile {
public:
    explicit StaticvalTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;

    int calculateRent(int diceValue = 0) const override;
    bool ownsFullGroup() const;
};


//==================== 虚函数格 ====================
class VirtualfuncTile : public PropertyTile {
public:
    explicit VirtualfuncTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;

    int price() const override;
    int calculateRent(int diceValue = 0) const override;
    void buildHouse() override;

    int buyRatio() const { return m_buyRatio; }
    int rentRatio() const { return m_rentRatio; }
    int ratio() const { return m_ratio; }
    int buyDecay() const { return m_buyDecay; }
    int rentDecay() const { return m_rentDecay; }
    bool rentIsPureVirtual() const { return m_rentIsPureVirtual; }
    bool rentIsNonVirtual() const { return m_rentIsNonVirtual; }

private:
    int m_ratio;      // 收租时基于owner资产的比例
    int m_buyRatio;   // 购入价格比例（百分比）
    int m_rentRatio;  // 收租比例（百分比）
    int m_buyDecay;   // 买入价格减免
    int m_rentDecay;  // 收租减免
    bool m_rentIsPureVirtual = false;  // rent_price() 纯虚函数
    bool m_rentIsNonVirtual  = false;  // rent_price() 非虚函数
};

// ==================== 问答格 ====================
class QATile : public Tile {
public:
    explicit QATile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;
};


// ==================== 税收地块 ====================
class TaxTile : public Tile {
public:
    explicit TaxTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;
};


// ==================== 商店地块 ====================
class ShopTile : public Tile {
public:
    explicit ShopTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;
};


// ==================== 上机课地块（替代免费停留） ====================
class ComputerLabTile : public Tile {
public:
    explicit ComputerLabTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;
};


// ==================== 商店入口地块（替代送往监狱） ====================
class ShopEntranceTile : public Tile {
public:
    explicit ShopEntranceTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;
};


// ==================== 迭代器格 ====================
class IteratorTile : public Tile {
public:
    explicit IteratorTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;

    Player* owner() const { return m_owner; }
    void setOwner(Player* p) { m_owner = p; }
    int calculateRent() const;
    int baseRent() const { return m_baseRent; }

private:
    Player* m_owner = nullptr;
    int m_baseRent;
};


// ==================== 公共设施（水厂/电厂） ====================
class UtilityTile : public Tile {
public:
    explicit UtilityTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;

    Player* owner() const { return m_owner; }
    void setOwner(Player* p) { m_owner = p; }
    int calculateRent(int diceValue) const;

private:
    Player* m_owner = nullptr;
};


// ==================== 铁路/车站 ====================
class RailroadTile : public Tile {
public:
    explicit RailroadTile(const TileDef& def, int index);
    void landOn(Player* player, Game* game) override;

    Player* owner() const { return m_owner; }
    void setOwner(Player* p) { m_owner = p; }
    int calculateRent() const;

private:
    Player* m_owner = nullptr;
};

#endif // TILE_H
