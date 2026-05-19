#include "dice.h"
#include <QRandomGenerator>

Dice::Dice(QObject* parent)
    : QObject(parent)
{
}

QPair<int, int> Dice::roll() {
    // 生成1-6的随机数
    m_die1 = QRandomGenerator::global()->bounded(1, 7);
    m_die2 = QRandomGenerator::global()->bounded(1, 7);
    emit diceRolled(m_die1, m_die2);
    return {m_die1, m_die2};
}
