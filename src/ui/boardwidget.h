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
    void startGameRequested();  // 点击中央开始按钮

private:
    void drawTile(QPainter& painter, int index, const QRect& rect);
    void drawPlayers(QPainter& painter);
    QColor colorForGroup(int tileIndex) const;
    QRect pieceAreaRect(const QRect& tileRect) const;

    // 按钮点击区域计算
    QRect titleBarButtonRect(const QRect& tileRect) const;
    QRect bodyButtonRect(const QRect& tileRect) const;
    QRect startButtonRect() const;  // 中央开始游戏按钮区域

    Board* m_board = nullptr;
    const QVector<Player*>* m_players = nullptr;
    QRect m_startBtnRect;  // 缓存开始按钮区域用于点击检测

    int cellSize() const;
    double scaleFactor() const;

    static constexpr int CELLS_PER_SIDE = 8;   // 8格/边（含两个角格）
    static constexpr int MARGIN = 8;
    static constexpr int REFERENCE_CELL = 68;  // 基准cell大小，用于缩放UI元素（配合8格/边）
};

#endif // BOARDWIDGET_H
