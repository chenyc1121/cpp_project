#include "board.h"
#include "tile.h"

// 前向声明 tile.cpp 中的工厂函数
Tile* createTile(const TileDef& def, int index);

Board::Board(QObject* parent)
    : QObject(parent)
{
    // 根据 BOARD_LAYOUT 配置创建所有地块
    for (int i = 0; i < BOARD_SIZE; ++i) {
        m_tiles.append(createTile(BOARD_LAYOUT[i], i));
    }
}

Board::~Board() {
    qDeleteAll(m_tiles);
    m_tiles.clear();
}

Tile* Board::tileAt(int index) const {
    if (index < 0 || index >= m_tiles.size()) return nullptr;
    return m_tiles[index];
}

QVector<PropertyTile*> Board::allProperties() const {
    QVector<PropertyTile*> props;
    for (auto* t : m_tiles) {
        if (t->type() == TileType::PROPERTY) {
            props.append(static_cast<PropertyTile*>(t));
        }
    }
    return props;
}

QVector<RailroadTile*> Board::allRailroads() const {
    QVector<RailroadTile*> rails;
    for (auto* t : m_tiles) {
        if (t->type() == TileType::RAILROAD) {
            rails.append(static_cast<RailroadTile*>(t));
        }
    }
    return rails;
}

void Board::reset() {
    for (auto* t : m_tiles) {
        if (auto* pt = dynamic_cast<PropertyTile*>(t)) {
            pt->reset();
        }
        if (auto* ut = dynamic_cast<UtilityTile*>(t)) {
            ut->setOwner(nullptr);
        }
        if (auto* rt = dynamic_cast<RailroadTile*>(t)) {
            rt->setOwner(nullptr);
        }
    }
    emit boardChanged();
}
