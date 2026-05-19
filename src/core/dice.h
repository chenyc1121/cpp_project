#ifndef DICE_H
#define DICE_H

#include <QObject>
#include <QPair>

// ==================== 骰子系统 ====================
// 管理两个骰子的掷骰结果，信号驱动UI更新
class Dice : public QObject {
    Q_OBJECT
public:
    explicit Dice(QObject* parent = nullptr);

    // 掷骰子，返回 (骰子1, 骰子2)
    QPair<int, int> roll();

    // 获取最近一次结果
    int die1() const { return m_die1; }
    int die2() const { return m_die2; }
    int total() const { return m_die1 + m_die2; }
    bool isDouble() const { return m_die1 == m_die2; }

signals:
    // 掷骰完成信号，通知UI更新
    void diceRolled(int die1, int die2);

private:
    int m_die1 = 1;
    int m_die2 = 1;
};

#endif // DICE_H
