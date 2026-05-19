#ifndef PLAYERPANEL_H
#define PLAYERPANEL_H

#include <QWidget>
#include <QLabel>
#include <QVector>

class Player;

// ==================== 玩家信息面板 ====================
// 显示所有玩家的列表及其金钱、地产等信息
class PlayerPanel : public QWidget {
    Q_OBJECT
public:
    explicit PlayerPanel(QWidget* parent = nullptr);

    // 设置要显示的玩家列表
    void setPlayers(const QVector<Player*>& players);
    // 更新特定玩家信息
    void updatePlayer(Player* player);
    // 高亮当前活跃玩家
    void highlightCurrentPlayer(Player* player);

private:
    // 单个玩家的信息行
    struct PlayerRow {
        QLabel* nameLabel;
        QLabel* moneyLabel;
        QLabel* propertiesLabel;
        QLabel* statusLabel;
        QFrame* container;
    };

    QVector<PlayerRow> m_rows;
    QVector<Player*> m_players;
};

#endif // PLAYERPANEL_H
