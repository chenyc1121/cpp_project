#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>
#include <QVector>

class Board;
class Player;
class Tile;

// ==================== 棋盘绘制组件 ====================
// 用 QPainter 绘制经典大富翁环形棋盘
// 28格均匀分布在四条边上，每边7格（含角格），角格同时属于两条边
class BoardWidget : public QWidget {
    Q_OBJECT
public:
    explicit BoardWidget(QWidget* parent = nullptr);

    void setBoard(Board* board);
    void setPlayers(const QVector<Player*>* players);
    void refreshBoard();

    // 获取地块对应的绘制矩形（用于点击检测）
    QRect tileRect(int tileIndex) const;

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void tileClicked(int tileIndex);

private:
    void drawTile(QPainter& painter, int index, const QRect& rect);
    void drawPlayers(QPainter& painter);
    QColor colorForGroup(int tileIndex) const;

    // 按钮点击区域计算
    QRect titleBarButtonRect(const QRect& tileRect, bool isCorner) const;
    QRect bodyButtonRect(const QRect& tileRect) const;

    Board* m_board = nullptr;
    const QVector<Player*>* m_players = nullptr;

    int cellSize() const;
    double scaleFactor() const;

    static constexpr int CELLS_PER_SIDE = 7;
    static constexpr int MARGIN = 10;
    static constexpr int REFERENCE_CELL = 78;  // 基准cell大小，用于缩放UI元素
};

#endif // BOARDWIDGET_H
