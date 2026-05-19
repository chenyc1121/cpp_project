#ifndef DICEWIDGET_H
#define DICEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

// ==================== 骰子显示组件 ====================
// 显示两个骰子的点数，包含掷骰子按钮
class DiceWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiceWidget(QWidget* parent = nullptr);

    // 更新骰子显示
    void setDice(int die1, int die2);
    // 启用/禁用掷骰子按钮
    void setRollEnabled(bool enabled);
    // 清空骰子显示
    void clearDice();

signals:
    // 用户点击了掷骰子按钮
    void rollRequested();

private:
    void updateDisplay();

    QLabel* m_die1Label;
    QLabel* m_die2Label;
    QLabel* m_totalLabel;
    QPushButton* m_rollBtn;
    int m_die1 = 0;
    int m_die2 = 0;
};

#endif // DICEWIDGET_H
