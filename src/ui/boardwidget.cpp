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
    if (tileIndex == 21) return QRect(m, m, cell, cell);                             // 左上角（29楼地下室）

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
    if (!m_board) return QColor("#F0EBE3");
    Tile* t = m_board->tileAt(tileIndex);
    if (!t) return QColor("#F0EBE3");

    switch (t->group()) {
    case ColorGroup::BROWN:       return QColor("#B8956A");
    case ColorGroup::LIGHT_BLUE:  return QColor("#82B8D9");
    case ColorGroup::PINK:        return QColor("#C894C8");
    case ColorGroup::ORANGE:      return QColor("#E8A85F");
    case ColorGroup::RED:         return QColor("#D9534F");
    case ColorGroup::YELLOW:      return QColor("#E0B840");
    case ColorGroup::GREEN:       return QColor("#5DAC6E");
    case ColorGroup::DEEP_BLUE:   return QColor("#4A7FB5");
    default: return QColor("#F0EBE3");
    }
}


// ==================== 按钮区域计算 ====================
QRect BoardWidget::titleBarButtonRect(const QRect& tileRect) const {
    double s = static_cast<double>(tileRect.width()) / REFERENCE_CELL;
    int barH = qBound(8, static_cast<int>(16 * s), 22);
    int bw = qBound(10, static_cast<int>(14 * s), 20);
    int bh = barH - 2;
    return QRect(tileRect.right() - bw - 2, tileRect.top() + 2, bw, bh);
}

QRect BoardWidget::startButtonRect() const {
    int cell = cellSize();
    int m = MARGIN;
    int boardPx = CELLS_PER_SIDE * cell;
    // 按钮位于中心区域的下半部分
    int centerX = m + boardPx / 2;
    int centerY = m + boardPx / 2;
    double s = scaleFactor();
    int btnW = qBound(140, static_cast<int>(200 * s), 260);
    int btnH = qBound(36, static_cast<int>(48 * s), 62);
    return QRect(centerX - btnW / 2, centerY + qBound(8, static_cast<int>(20 * s), 30), btnW, btnH);
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

    // 背景：木质暖灰
    painter.fillRect(rect(), QColor("#E8E0D8"));

    QRect boardOuter(m, m, boardPx, boardPx);
    // 棋盘底色：木质调米白
    painter.fillRect(boardOuter, QColor("#FDF8F2"));
    painter.setPen(QPen(QColor("#8B1A1A"), 3));
    painter.drawRect(boardOuter);

    QRect center(m + cell, m + cell,
                 (CELLS_PER_SIDE - 2) * cell,
                 (CELLS_PER_SIDE - 2) * cell);
    // 中心区域：纯白
    painter.fillRect(center, QColor("#FFFFFF"));
    painter.setPen(QPen(QColor("#8B1A1A"), 2));
    painter.drawRect(center);

    // ── 中心区域：标题 / 开始按钮 ──
    bool noGame = (m_board == nullptr || m_players == nullptr || m_players->isEmpty());

    int cTop = center.top();
    int cH = center.height();
    int cW = center.width();
    QPoint cCenter = center.center();

    if (noGame) {
        // 上部 45%：标题
        int titleAreaH = cH * 45 / 100;
        int titleFontSz = qBound(8, titleAreaH / 4, 36);
        QFont tFont("Microsoft YaHei", titleFontSz, QFont::Bold);
        painter.setFont(tFont);
        painter.setPen(QColor("#8B1A1A"));
        QRect titleRect(cCenter.x() - cW/2 + 8, cTop + titleAreaH/6,
                        cW - 16, titleAreaH * 2 / 3);
        painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                         "程序设计实习\nmOnOPoly");

        // 装饰线
        int lineY = cTop + titleAreaH * 4 / 5;
        int lineW = qMin(cW * 3 / 5, 200);
        painter.setPen(QPen(QColor("#8B1A1A"), 2));
        painter.drawLine(QPoint(cCenter.x() - lineW/2, lineY),
                         QPoint(cCenter.x() + lineW/2, lineY));

        // 下部：开始按钮（扁平纯色）
        int btnAreaTop = cTop + titleAreaH;
        int btnAreaH = cH - titleAreaH;
        int btnW = qMin(cW * 3 / 4, 260);
        int btnH = qMin(btnAreaH * 7 / 10, 56);
        btnH = qMax(btnH, 28);
        QRect btnRect(cCenter.x() - btnW/2,
                      btnAreaTop + (btnAreaH - btnH)/2,
                      btnW, btnH);
        int btnRadius = qMin(btnH / 4, 14);

        // 按钮主体 — 纯色无渐变无发光
        painter.setBrush(QColor("#B22222"));
        painter.setPen(QPen(QColor("#8B1A1A"), 2));
        painter.drawRoundedRect(btnRect, btnRadius, btnRadius);

        // 按钮文字
        int btnFontSz = qBound(10, btnH / 2, 22);
        QFont btnFont("Microsoft YaHei", btnFontSz, QFont::Bold);
        btnFont.setLetterSpacing(QFont::AbsoluteSpacing, qBound(1, btnW / 40, 5));
        painter.setFont(btnFont);
        painter.setPen(QColor("#FFFFFF"));
        painter.drawText(btnRect, Qt::AlignCenter, "开 始 游 戏");

        m_startBtnRect = btnRect;

    } else {
        // 游戏中：标题居中
        int titleFontSz = qBound(10, cH / 6, 36);
        QFont tFont("Microsoft YaHei", titleFontSz, QFont::Bold);
        painter.setFont(tFont);
        painter.setPen(QColor("#8B1A1A"));
        QRect titleRect(cCenter.x() - cW/2 + 8, cCenter.y() - cH/4,
                        cW - 16, cH/2);
        painter.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                         "程序设计实习\nmOnOPoly");

        // 装饰线
        int lineY = cCenter.y() + cH/8;
        int lineW = qMin(cW * 3 / 5, 200);
        painter.setPen(QPen(QColor("#8B1A1A"), 2));
        painter.drawLine(QPoint(cCenter.x() - lineW/2, lineY),
                         QPoint(cCenter.x() + lineW/2, lineY));
    }

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

    double s = scaleFactor();

    QColor bg = colorForGroup(index);

    switch (tile->type()) {
    case TileType::START:
        bg = QColor("#E8D44D");
        break;
    case TileType::SHOP:
        bg = QColor("#E8C95A");
        break;
    case TileType::SHOP_ENTRANCE:
        bg = QColor("#E8B878");
        break;
    case TileType::COMPUTER_LAB:
        bg = QColor("#7EC8D0");
        break;
    case TileType::QA:
        bg = QColor("#C49DCE");
        break;
    case TileType::TAX:
        bg = QColor("#D99191");
        break;
    case TileType::ITERATOR:
        bg = QColor("#7AB8B0");
        break;
    default:
        break;
    }

    // 地块主体
    int radius = qBound(2, static_cast<int>(4 * s), 6);
    painter.setPen(QPen(QColor("#3D2820"), 1));
    painter.setBrush(bg);
    painter.drawRoundedRect(rect.adjusted(0, 0, -1, -1), radius, radius);

    // 标题栏 — 纯色扁平
    int barH = qBound(8, static_cast<int>(16 * s), 22);
    QRect bar = rect.adjusted(1, 1, -1, 0);
    bar.setHeight(barH);

    QColor barColor;
    if (tile->group() != ColorGroup::NONE) {
        barColor = colorForGroup(index);
    } else {
        barColor = bg.darker(125);
    }
    painter.fillRect(bar, barColor);

    int barFontSz = qBound(4, static_cast<int>(7 * s), 10);
    QFont barFont("Microsoft YaHei", barFontSz, QFont::Bold);
    painter.setFont(barFont);
    int luminance = (barColor.red() * 299 + barColor.green() * 587 + barColor.blue() * 114) / 1000;
    painter.setPen(luminance > 128 ? QColor("#2C1810") : QColor("#FFFFFF"));
    int btnW = qBound(10, static_cast<int>(14 * s), 20);
    QRect barTextRect = bar.adjusted(2, 0, -(btnW + 3), 0);
    painter.drawText(barTextRect, Qt::AlignVCenter | Qt::AlignLeft, tile->titleBarText());

    // 标题栏按钮 [i]
    QRect tbBtn = titleBarButtonRect(rect);
    painter.setPen(QPen(barColor.darker(160), 1));
    painter.setBrush(barColor.lighter(120));
    painter.drawRoundedRect(tbBtn, 3, 3);
    painter.setPen(luminance > 128 ? QColor("#4A3028") : QColor("#FFFFFF"));
    int btnFontSz = qBound(4, static_cast<int>(8 * s), 10);
    QFont btnFont("Arial", btnFontSz, QFont::Bold);
    painter.setFont(btnFont);
    painter.drawText(tbBtn, Qt::AlignCenter, "i");

    // ── 格子主体内容 ──
    int topOffset = barH + qBound(2, static_cast<int>(3 * s), 4);
    QRect bodyRect = rect.adjusted(2, topOffset, -2, -2);

    bool isSpecial = (tile->type() == TileType::QA ||
                      tile->type() == TileType::START ||
                      tile->type() == TileType::SHOP ||
                      tile->type() == TileType::COMPUTER_LAB ||
                      tile->type() == TileType::SHOP_ENTRANCE ||
                      tile->type() == TileType::ITERATOR);

    if (isSpecial) {
        // 特殊格：上部名称 + 下部大图标
        int nameH = bodyRect.height() * 30 / 100;
        QRect nameArea = bodyRect;
        nameArea.setHeight(nameH);
        int nameFontSz = qBound(4, static_cast<int>(7 * s), 11);
        QFont spNameFont("Microsoft YaHei", nameFontSz, QFont::Bold);
        painter.setFont(spNameFont);
        painter.setPen(QColor("#3D2820"));
        painter.drawText(nameArea, Qt::AlignHCenter | Qt::AlignBottom, tile->name());

        // 图标区域
        QRect iconArea(bodyRect.left(), bodyRect.top() + nameH,
                       bodyRect.width(), bodyRect.height() - nameH);
        int symFontSz = qBound(8, iconArea.height() * 65 / 100, 22);
        QFont symFont("Arial", symFontSz, QFont::Bold);
        painter.setFont(symFont);

        switch (tile->type()) {
        case TileType::QA:
            painter.setPen(QColor("#7B3A8A"));
            painter.drawText(iconArea, Qt::AlignCenter, "?");
            break;
        case TileType::START:
            painter.setPen(QColor("#5D2E0C"));
            painter.drawText(iconArea, Qt::AlignCenter, "✦");
            break;
        case TileType::SHOP:
            painter.setPen(QColor("#B85C10"));
            painter.drawText(iconArea, Qt::AlignCenter, "¥");
            break;
        case TileType::COMPUTER_LAB:
            painter.setPen(QColor("#1A6B72"));
            painter.drawText(iconArea, Qt::AlignCenter, "⌨");
            break;
        case TileType::SHOP_ENTRANCE:
            painter.setPen(QColor("#9B3A0A"));
            painter.drawText(iconArea, Qt::AlignCenter, "→");
            break;
        case TileType::ITERATOR:
            painter.setPen(QColor("#1A6B60"));
            painter.drawText(iconArea, Qt::AlignCenter, "↻");
            break;
        default: break;
        }
    } else {
        // 普通格（地产/公共设施/铁路/迭代器格）：上部名称 + 下部价格
        int nameH = bodyRect.height() * 55 / 100;
        QRect nameArea = bodyRect;
        nameArea.setHeight(nameH);
        int nameFontSz = qBound(4, static_cast<int>(7 * s), 11);
        QFont propNameFont("Microsoft YaHei", nameFontSz, QFont::Bold);
        painter.setFont(propNameFont);
        painter.setPen(QColor("#3D2820"));
        painter.drawText(nameArea, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,
                         tile->name());

        // 价格区域
        int priceAreaTop = bodyRect.top() + nameH;
        QRect priceArea(bodyRect.left(), priceAreaTop,
                        bodyRect.width(), bodyRect.height() - nameH - 2);

        // 房屋标记（如果有）
        if (auto* prop = dynamic_cast<PropertyTile*>(tile)) {
            if (prop->houses() > 0 && !prop->hasHotel()) {
                painter.setPen(QColor("#2E7D32"));
                int hfSz = qBound(4, static_cast<int>(7 * s), 10);
                QFont hf("Arial", hfSz, QFont::Bold);
                painter.setFont(hf);
                painter.drawText(QRect(priceArea.left(), priceArea.top(),
                                       priceArea.width(), priceArea.height() / 2),
                                 Qt::AlignHCenter | Qt::AlignBottom,
                                 "▣" + QString::number(prop->houses()));
            } else if (prop->hasHotel()) {
                painter.setPen(QColor("#C0392B"));
                int hfSz = qBound(4, static_cast<int>(8 * s), 11);
                QFont hf("Arial", hfSz, QFont::Bold);
                painter.setFont(hf);
                painter.drawText(QRect(priceArea.left(), priceArea.top(),
                                       priceArea.width(), priceArea.height() / 2),
                                 Qt::AlignHCenter | Qt::AlignBottom, "H");
            }
        }

        // 价格
        int priceFontSz = qBound(4, static_cast<int>(7 * s), 10);
        QFont priceFont("Arial", priceFontSz);
        painter.setFont(priceFont);

        auto* pt = dynamic_cast<PropertyTile*>(tile);
        auto* ut = dynamic_cast<UtilityTile*>(tile);
        auto* rt = dynamic_cast<RailroadTile*>(tile);
        auto* it_tile = dynamic_cast<IteratorTile*>(tile);
        int price = pt ? pt->price() : (ut ? ut->price() : (rt ? rt->price() :
                     (it_tile ? it_tile->price() : 0)));
        Player* owner = pt ? pt->owner() : (ut ? ut->owner() : (rt ? rt->owner() :
                        (it_tile ? it_tile->owner() : nullptr)));

        QRect priceRect(priceArea.left(), priceArea.top() + priceArea.height()/2,
                        priceArea.width(), priceArea.height()/2);
        if (owner) {
            painter.setPen(owner->color().darker(150));
        } else {
            painter.setPen(QColor("#3D2820"));
        }
        painter.drawText(priceRect, Qt::AlignHCenter | Qt::AlignVCenter,
                         "¥" + QString::number(price));
    }

    // 格子主体按钮 [...]
    QRect bodyBtn = bodyButtonRect(rect);
    painter.setPen(QPen(QColor("#A09080"), 1));
    painter.setBrush(QColor("#F5F0EB"));
    painter.drawRoundedRect(bodyBtn, 3, 3);
    painter.setPen(QColor("#7A6A58"));
    int bbFontSz = qBound(4, static_cast<int>(7 * s), 10);
    QFont bbFont("Arial", bbFontSz, QFont::Bold);
    painter.setFont(bbFont);
    painter.drawText(bodyBtn, Qt::AlignCenter, "···");
}


// ==================== 玩家标记 ====================
void BoardWidget::drawPlayers(QPainter& painter) {
    if (!m_players || m_players->isEmpty()) return;

    double s = scaleFactor();
    int badgeW = qBound(16, static_cast<int>(22 * s), 28);
    int badgeH = qBound(10, static_cast<int>(13 * s), 18);
    int badgeR = qBound(2, static_cast<int>(3 * s), 5);
    int gap = qBound(2, static_cast<int>(2 * s), 3);
    painter.setRenderHint(QPainter::Antialiasing);

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

        int maxPerRow = 2;
        int col = sameTileCount % maxPerRow;
        int row = sameTileCount / maxPerRow;
        int marginR = qBound(16, static_cast<int>(22 * s), 28);
        int marginB = qBound(2, static_cast<int>(3 * s), 5);
        int startX = tileR.right() - marginR - (maxPerRow - 1) * (badgeW + gap);
        int startY = tileR.bottom() - marginB - badgeH - row * (badgeH + gap);
        int x = startX + col * (badgeW + gap);
        int y = startY;
        QRect badgeRect(x, y, badgeW, badgeH);

        // 纯色扁平徽章
        painter.setBrush(p->color());
        painter.setPen(QPen(QColor("#FFFFFF"), 1));
        painter.drawRoundedRect(badgeRect, badgeR, badgeR);

        // 编号 — 白色文字
        painter.setPen(QColor("#FFFFFF"));
        int idFontSz = qBound(5, static_cast<int>(7 * s), 10);
        QFont idFont("Arial", idFontSz, QFont::Bold);
        painter.setFont(idFont);
        painter.drawText(badgeRect, Qt::AlignCenter, QString::number(p->id() + 1));
    }
}


// ==================== 鼠标点击 ====================
void BoardWidget::mousePressEvent(QMouseEvent* event) {
    // 检查是否点击了中央开始按钮
    if (!m_board || (m_players && m_players->isEmpty())) {
        if (m_startBtnRect.isValid() && m_startBtnRect.contains(event->pos())) {
            emit startGameRequested();
            return;
        }
    }

    if (!m_board) return;

    for (int i = 0; i < BOARD_SIZE; ++i) {
        QRect tr = tileRect(i);
        if (!tr.contains(event->pos())) continue;

        Tile* t = m_board->tileAt(i);
        if (!t) return;

        bool isCorner = (i == 0 || i == 7 || i == 14 || i == 21);

        if (titleBarButtonRect(tr).contains(event->pos())) {
            QMessageBox::information(this, t->name() + " — 说明",
                                     t->titleDetail());
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
            info += "\n建房费: ¥" + QString::number(pt->houseCost()) + "/栋";
            info += "\n\n基础租金表：";
            info += "\n  空地: ¥" + QString::number(pt->rentAtLevel(0));
            info += "\n  1栋:  ¥" + QString::number(pt->rentAtLevel(1));
            info += "\n  2栋:  ¥" + QString::number(pt->rentAtLevel(2));
            info += "\n  3栋:  ¥" + QString::number(pt->rentAtLevel(3));
            info += "\n  4栋:  ¥" + QString::number(pt->rentAtLevel(4));
            info += "\n  旅馆: ¥" + QString::number(pt->rentAtLevel(5));
            if (pt->owner()) {
                info += "\n\n所有者: " + pt->owner()->name();
                if (pt->houses() > 0)
                    info += "\n房屋: " + QString::number(pt->houses()) + " 栋";
                if (pt->hasHotel())
                    info += "\n已建旅馆!";
            }
        } else if (auto* ut = dynamic_cast<UtilityTile*>(t)) {
            info += "\n类型: 公共设施";
            info += "\n价格: ¥" + QString::number(ut->price());
            info += "\n\n基础租金规则：";
            info += "\n  拥有1个公共设施: 骰子点数 × 4";
            info += "\n  拥有2个公共设施: 骰子点数 × 10";
            if (ut->owner())
                info += "\n\n所有者: " + ut->owner()->name();
        } else if (auto* rt = dynamic_cast<RailroadTile*>(t)) {
            info += "\n类型: 铁路车站";
            info += "\n价格: ¥" + QString::number(rt->price());
            info += "\n\n基础租金表：";
            info += "\n  1个车站: ¥250";
            info += "\n  2个车站: ¥500";
            info += "\n  3个车站: ¥1000";
            info += "\n  4个车站: ¥2000";
            if (rt->owner())
                info += "\n\n所有者: " + rt->owner()->name();
        } else if (auto* it_tile = dynamic_cast<IteratorTile*>(t)) {
            info += "\n类型: 迭代器格";
            info += "\n价格: ¥" + QString::number(it_tile->price());
            int base = it_tile->baseRent();
            info += "\n\n基础租金表：";
            info += "\n  1个迭代器格: ¥" + QString::number(base);
            info += "\n  2个迭代器格: ¥" + QString::number(base * 2);
            info += "\n  3个迭代器格: ¥" + QString::number(base * 4);
            info += "\n  4个迭代器格: ¥" + QString::number(base * 8);
            if (it_tile->owner())
                info += "\n\n所有者: " + it_tile->owner()->name();
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
            info += "\n类型: 29楼地下室";
            info += "\n可选择是否进入商店购物";
        }
        QToolTip::showText(event->globalPosition().toPoint(), info, this);
        return;
    }
}
