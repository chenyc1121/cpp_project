#include "boardwidget.h"
#include "../core/board.h"
#include "../core/tile.h"
#include "../core/player.h"
#include "../core/config.h"

#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>
#include <QMessageBox>
#include <QFont>

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(300, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void BoardWidget::setBoard(Board* board) {
    m_board = board;
    update();
}

void BoardWidget::setPlayers(const QVector<Player*>* players) {
    m_players = players;
    update();
}

void BoardWidget::refreshBoard() {
    update();
}

QSize BoardWidget::sizeHint() const {
    return QSize(600, 600);
}

int BoardWidget::cellSize() const {
    int avail = qMin(width(), height()) - 2 * MARGIN;
    return qMax(avail / CELLS_PER_SIDE, 40);
}

double BoardWidget::scaleFactor() const {
    return static_cast<double>(cellSize()) / REFERENCE_CELL;
}

// ==================== 地块坐标计算 ====================
// 8×8网格，4角格+每边6个中间格=28格，每格独立位置无重叠
QRect BoardWidget::tileRect(int tileIndex) const {
    if (tileIndex < 0 || tileIndex >= BOARD_SIZE) return QRect();

    int cell = cellSize();
    int m = MARGIN;
    int last = CELLS_PER_SIDE - 1;  // = 7, 最大坐标索引

    // 角格
    if (tileIndex == 0)  return QRect(m, m + last * cell, cell, cell);           // 左下角（起点）
    if (tileIndex == 7)  return QRect(m + last * cell, m + last * cell, cell, cell); // 右下角（商店）
    if (tileIndex == 14) return QRect(m + last * cell, m, cell, cell);               // 右上角（上机课）
    if (tileIndex == 21) return QRect(m, m, cell, cell);                             // 左上角（商店入口）

    // 底边: 1-6，从左到右
    if (tileIndex <= 6)
        return QRect(m + tileIndex * cell, m + last * cell, cell, cell);

    // 左边: 8-13，从下到上
    if (tileIndex <= 13)
        return QRect(m + last * cell, m + (last - (tileIndex - 7)) * cell, cell, cell);

    // 顶边: 15-20，从右到左
    if (tileIndex <= 20)
        return QRect(m + (last - (tileIndex - 14)) * cell, m, cell, cell);

    // 右边: 22-27，从上到下
    return QRect(m, m + (tileIndex - 21) * cell, cell, cell);
}


// ==================== 颜色映射 ====================
QColor BoardWidget::colorForGroup(int tileIndex) const {
    if (!m_board) return QColor("#f5f5dc");
    Tile* t = m_board->tileAt(tileIndex);
    if (!t) return QColor("#f5f5dc");

    switch (t->group()) {
    case ColorGroup::BROWN:       return QColor("#CD853F");
    case ColorGroup::LIGHT_BLUE:  return QColor("#87CEEB");
    case ColorGroup::PINK:        return QColor("#DDA0DD");
    case ColorGroup::ORANGE:      return QColor("#FFB347");
    case ColorGroup::RED:         return QColor("#E74C3C");
    case ColorGroup::YELLOW:      return QColor("#F1C40F");
    case ColorGroup::GREEN:       return QColor("#2ECC71");
    case ColorGroup::DEEP_BLUE:   return QColor("#2980B9");
    default: return QColor("#f5f5dc");
    }
}


// ==================== 按钮区域计算 ====================
QRect BoardWidget::titleBarButtonRect(const QRect& tileRect, bool isCorner) const {
    double s = static_cast<double>(tileRect.width()) / REFERENCE_CELL;
    int barH = qBound(8, static_cast<int>((isCorner ? 16 : 12) * s), 22);
    int bw = qBound(10, static_cast<int>(14 * s), 20);
    int bh = barH - 2;
    return QRect(tileRect.right() - bw - 2, tileRect.top() + 2, bw, bh);
}

QRect BoardWidget::bodyButtonRect(const QRect& tileRect) const {
    double s = static_cast<double>(tileRect.width()) / REFERENCE_CELL;
    int bw = qBound(10, static_cast<int>(16 * s), 24);
    int bh = qBound(8, static_cast<int>(12 * s), 18);
    return QRect(tileRect.right() - bw - 3, tileRect.bottom() - bh - 3, bw, bh);
}


// ==================== 主绘制 ====================
void BoardWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int cell = cellSize();
    int boardPx = CELLS_PER_SIDE * cell;
    int m = MARGIN;
    double s = scaleFactor();

    painter.fillRect(rect(), QColor("#8F1A10"));

    QRect boardOuter(m, m, boardPx, boardPx);
    painter.fillRect(boardOuter, QColor("#E6C8C8"));
    painter.setPen(QPen(QColor("#8E3838"), 2));
    painter.drawRect(boardOuter);

    QRect center(m + cell, m + cell,
                 (CELLS_PER_SIDE - 2) * cell,
                 (CELLS_PER_SIDE - 2) * cell);
    painter.fillRect(center, QColor("#F5E8E8"));
    painter.setPen(QPen(QColor("#D6A7A7"), 2));
    painter.drawRect(center);

    int titleSize = qBound(12, static_cast<int>(24 * s), 36);
    QFont titleFont("Microsoft YaHei", titleSize, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(QColor("#8F1A10"));
    painter.drawText(center, Qt::AlignHCenter | Qt::AlignVCenter, "大富翁\nMonopoly");

    for (int i = 0; i < BOARD_SIZE; ++i) {
        QRect r = tileRect(i);
        drawTile(painter, i, r);
    }

    drawPlayers(painter);
}


// ==================== 绘制单个地块 ====================
void BoardWidget::drawTile(QPainter& painter, int index, const QRect& rect) {
    if (!m_board) return;
    Tile* tile = m_board->tileAt(index);
    if (!tile) return;

    bool isCorner = (index == 0 || index == 7 || index == 14 || index == 21);
    double s = scaleFactor();

    QColor bg = colorForGroup(index);

    switch (tile->type()) {
    case TileType::START:
        bg = QColor("#FFEB3B");
        break;
    case TileType::SHOP:
        bg = QColor("#FFD54F");
        break;
    case TileType::SHOP_ENTRANCE:
        bg = QColor("#FFCC80");
        break;
    case TileType::COMPUTER_LAB:
        bg = QColor("#80DEEA");
        break;
    case TileType::QA:
        bg = QColor("#CE93D8");
        break;
    case TileType::TAX:
        bg = QColor("#EF9A9A");
        break;
    case TileType::ITERATOR:
        bg = QColor("#80CBC4");
        break;
    default:
        break;
    }

    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(bg);
    painter.drawRoundedRect(rect.adjusted(0, 0, -1, -1), qBound(2, static_cast<int>(3 * s), 4), qBound(2, static_cast<int>(3 * s), 4));

    // 标题栏
    int barH = qBound(8, static_cast<int>((isCorner ? 16 : 12) * s), 22);
    QRect bar = rect.adjusted(1, 1, -1, 0);
    bar.setHeight(barH);

    QColor barColor;
    if (tile->group() != ColorGroup::NONE) {
        barColor = colorForGroup(index);
    } else {
        barColor = bg.darker(130);
    }
    painter.fillRect(bar, barColor);

    int barFontSz = qBound(4, static_cast<int>((isCorner ? 7 : 6) * s), 10);
    QFont barFont("Microsoft YaHei", barFontSz, QFont::Bold);
    painter.setFont(barFont);
    int luminance = (barColor.red() * 299 + barColor.green() * 587 + barColor.blue() * 114) / 1000;
    painter.setPen(luminance > 128 ? Qt::black : Qt::white);
    int btnW = qBound(10, static_cast<int>(14 * s), 20);
    QRect barTextRect = bar.adjusted(2, 0, -(btnW + 3), 0);
    painter.drawText(barTextRect, Qt::AlignVCenter | Qt::AlignLeft, tile->titleBarText());

    // 标题栏按钮 [i]
    QRect tbBtn = titleBarButtonRect(rect, isCorner);
    painter.setPen(QPen(barColor.darker(150), 1));
    painter.setBrush(barColor.lighter(130));
    painter.drawRoundedRect(tbBtn, 2, 2);
    painter.setPen(luminance > 128 ? Qt::black : Qt::white);
    int btnFontSz = qBound(4, static_cast<int>((isCorner ? 8 : 6) * s), 10);
    QFont btnFont("Arial", btnFontSz, QFont::Bold);
    painter.setFont(btnFont);
    painter.drawText(tbBtn, Qt::AlignCenter, "i");

    // 格子主体内容
    int topOffset = barH + qBound(2, static_cast<int>(3 * s), 4);
    QRect textRect = rect.adjusted(2, topOffset, -2, -2);

    int nameFontSz = qBound(4, static_cast<int>((isCorner ? 8 : 7) * s), 11);
    QFont nameFont("Microsoft YaHei", nameFontSz, QFont::Bold);
    painter.setFont(nameFont);
    painter.setPen(Qt::black);
    painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, tile->name());

    // 特殊符号
    int symFontSz = qBound(6, static_cast<int>((isCorner ? 14 : 10) * s), 20);
    QFont symFont("Arial", symFontSz, QFont::Bold);
    painter.setFont(symFont);

    if (tile->type() == TileType::QA) {
        painter.setPen(QColor("#6A1B9A"));
        painter.drawText(rect, Qt::AlignCenter, "Q");
    } else if (tile->type() == TileType::START) {
        painter.setPen(Qt::black);
        painter.drawText(rect, Qt::AlignCenter, "GO");
    } else if (tile->type() == TileType::SHOP) {
        painter.setPen(QColor("#E65100"));
        painter.drawText(rect, Qt::AlignCenter, "¥");
    } else if (tile->type() == TileType::COMPUTER_LAB) {
        painter.setPen(QColor("#006064"));
        painter.drawText(rect, Qt::AlignCenter, "PC");
    } else if (tile->type() == TileType::SHOP_ENTRANCE) {
        painter.setPen(QColor("#BF360C"));
        painter.drawText(rect, Qt::AlignCenter, ">>");
    } else if (tile->type() == TileType::ITERATOR) {
        painter.setPen(QColor("#00695C"));
        painter.drawText(rect, Qt::AlignCenter, "<>");
    }

    // 价格
    auto* pt = dynamic_cast<PropertyTile*>(tile);
    auto* ut = dynamic_cast<UtilityTile*>(tile);
    auto* rt = dynamic_cast<RailroadTile*>(tile);
    auto* it_tile = dynamic_cast<IteratorTile*>(tile);

    if (pt || ut || rt || it_tile) {
        int priceFontSz = qBound(4, static_cast<int>(7 * s), 10);
        QFont priceFont("Arial", priceFontSz);
        painter.setFont(priceFont);
        int price = pt ? pt->price() : (ut ? ut->price() : (rt ? rt->price() : (it_tile ? it_tile->price() : 0)));

        Player* owner = pt ? pt->owner() : (ut ? ut->owner() : (rt ? rt->owner() : (it_tile ? it_tile->owner() : nullptr)));

        if (owner) {
            painter.setPen(owner->color().darker(150));
            int rent = pt ? pt->calculateRent() : (it_tile ? it_tile->calculateRent() : 0);
            painter.drawText(rect.adjusted(1, 0, -1, -2),
                             Qt::AlignBottom | Qt::AlignHCenter,
                             "¥" + QString::number(rent > 0 ? rent : price));
        } else {
            painter.setPen(QColor(0,0,0));
            painter.drawText(rect.adjusted(1, 0, -1, -2),
                             Qt::AlignBottom | Qt::AlignHCenter,
                             "¥" + QString::number(price));
        }
    }

    // 房屋标记
    if (auto* prop = dynamic_cast<PropertyTile*>(tile)) {
        if (prop->houses() > 0 && !prop->hasHotel()) {
            painter.setPen(Qt::darkGreen);
            int hfSz = qBound(4, static_cast<int>(7 * s), 10);
            QFont hf("Arial", hfSz, QFont::Bold);
            painter.setFont(hf);
            int offset = qBound(8, static_cast<int>(14 * s), 20);
            painter.drawText(rect.adjusted(1, 0, -1, -offset),
                             Qt::AlignBottom | Qt::AlignHCenter,
                             "▣" + QString::number(prop->houses()));
        } else if (prop->hasHotel()) {
            painter.setPen(Qt::red);
            int hfSz = qBound(4, static_cast<int>(8 * s), 11);
            QFont hf("Arial", hfSz, QFont::Bold);
            painter.setFont(hf);
            int offset = qBound(8, static_cast<int>(14 * s), 20);
            painter.drawText(rect.adjusted(1, 0, -1, -offset),
                             Qt::AlignBottom | Qt::AlignHCenter, "H");
        }
    }

    // 格子主体按钮 [...]
    QRect bodyBtn = bodyButtonRect(rect);
    painter.setPen(QPen(QColor("#888888"), 1));
    painter.setBrush(QColor("#F5F5F5"));
    painter.drawRoundedRect(bodyBtn, 2, 2);
    painter.setPen(QColor("#666666"));
    int bbFontSz = qBound(4, static_cast<int>(7 * s), 10);
    QFont bbFont("Arial", bbFontSz, QFont::Bold);
    painter.setFont(bbFont);
    painter.drawText(bodyBtn, Qt::AlignCenter, "...");
}


// ==================== 玩家标记 ====================
void BoardWidget::drawPlayers(QPainter& painter) {
    if (!m_players || m_players->isEmpty()) return;

    double s = scaleFactor();
    int dotSize = qBound(8, static_cast<int>(12 * s), 18);
    int spacing = dotSize + 1;

    for (int i = 0; i < m_players->size(); ++i) {
        Player* p = (*m_players)[i];
        if (p->isBankrupt()) continue;

        QRect tileR = tileRect(p->position());
        if (!tileR.isValid()) continue;

        int sameTileCount = 0;
        for (int j = 0; j < i; ++j) {
            if ((*m_players)[j]->position() == p->position()
                && !(*m_players)[j]->isBankrupt()) {
                sameTileCount++;
            }
        }

        int col = sameTileCount % 4;
        int row = sameTileCount / 4;
        int x = tileR.x() + 3 + col * spacing;
        int y = tileR.y() + 3 + row * spacing;

        painter.setBrush(p->color());
        painter.setPen(QPen(Qt::white, 1));
        painter.drawEllipse(QPointF(x + dotSize/2.0, y + dotSize/2.0),
                            dotSize/2.0, dotSize/2.0);

        painter.setPen(Qt::white);
        int idFontSz = qBound(4, static_cast<int>(6 * s), 9);
        QFont idFont("Arial", idFontSz, QFont::Bold);
        painter.setFont(idFont);
        painter.drawText(QRect(x, y, dotSize, dotSize),
                         Qt::AlignCenter, QString::number(p->id() + 1));
    }
}


// ==================== 鼠标点击 ====================
void BoardWidget::mousePressEvent(QMouseEvent* event) {
    if (!m_board) return;

    for (int i = 0; i < BOARD_SIZE; ++i) {
        QRect tr = tileRect(i);
        if (!tr.contains(event->pos())) continue;

        Tile* t = m_board->tileAt(i);
        if (!t) return;

        bool isCorner = (i == 0 || i == 7 || i == 14 || i == 21);

        if (titleBarButtonRect(tr, isCorner).contains(event->pos())) {
            QMessageBox::information(this, t->name() + " — 标题栏",
                                     t->titleBarText());
            return;
        }

        if (bodyButtonRect(tr).contains(event->pos())) {
            QMessageBox::information(this, t->name() + " — 详情",
                                     t->infoText());
            return;
        }

        emit tileClicked(i);

        QString info = "【" + t->name() + "】";
        if (auto* pt = dynamic_cast<PropertyTile*>(t)) {
            info += "\n类型: 地产 (" + ::colorGroupName(pt->group()) + ")";
            info += "\n价格: ¥" + QString::number(pt->price());
            if (pt->owner()) {
                info += "\n所有者: " + pt->owner()->name();
                info += "\n当前租金: ¥" + QString::number(pt->calculateRent());
                if (pt->houses() > 0)
                    info += "\n房屋: " + QString::number(pt->houses()) + " 栋";
                if (pt->hasHotel())
                    info += "\n已建旅馆!";
            }
        } else if (auto* ut = dynamic_cast<UtilityTile*>(t)) {
            info += "\n类型: 公共设施";
            info += "\n价格: ¥" + QString::number(ut->price());
            if (ut->owner())
                info += "\n所有者: " + ut->owner()->name();
        } else if (auto* rt = dynamic_cast<RailroadTile*>(t)) {
            info += "\n类型: 铁路车站";
            info += "\n价格: ¥" + QString::number(rt->price());
            if (rt->owner())
                info += "\n所有者: " + rt->owner()->name();
        } else if (auto* it_tile = dynamic_cast<IteratorTile*>(t)) {
            info += "\n类型: 迭代器格";
            info += "\n价格: ¥" + QString::number(it_tile->price());
            if (it_tile->owner())
                info += "\n所有者: " + it_tile->owner()->name();
            info += "\n\n拥有迭代器卡时可在此格选择操作传送至其他迭代器格。";
        } else if (t->type() == TileType::QA) {
            info += "\n类型: 问答格";
            info += "\n回答C++选择题，答对有概率获得效果卡！";
        } else if (t->type() == TileType::SHOP) {
            info += "\n类型: 商店";
            info += "\n可以购买各类效果卡";
        } else if (t->type() == TileType::COMPUTER_LAB) {
            info += "\n类型: 上机课";
            info += "\n答对C++题可获得效果卡，但需跳过下回合";
        } else if (t->type() == TileType::SHOP_ENTRANCE) {
            info += "\n类型: 商店入口";
            info += "\n可选择是否进入商店购物";
        }
        QToolTip::showText(event->globalPosition().toPoint(), info, this);
        return;
    }
}
