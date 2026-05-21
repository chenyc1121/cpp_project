#ifndef BOARD_H
#define BOARD_H

#include <QObject>
#include <QVector>
#include "config.h"

class Tile;
class Player;

// ==================== 棋盘类 ====================
// 管理所有地块的创建、访问和重置
class Board : public QObject {
    Q_OBJECT
public:
    explicit Board(QObject* parent = nullptr);
    ~Board();

    // 访问地块
    Tile* tileAt(int index) const;
    int size() const { return BOARD_SIZE; }

    // 获取所有地产类地块
    QVector<class PropertyTile*> allProperties() const;
    // 获取所有铁路地块
    QVector<class RailroadTile*> allRailroads() const;
    // 获取所有迭代器格
    QVector<class IteratorTile*> allIterators() const;
    // 获取迭代器格索引列表（有序，顺时针方向）
    QVector<int> iteratorIndices() const;

    // 重置棋盘到初始状态
    void reset();

signals:
    void boardChanged();  // 棋盘状态变化时通知UI刷新

private:
    QVector<Tile*> m_tiles;
};

#endif // BOARD_H
