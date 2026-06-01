#include "dicewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFont>

DiceWidget::DiceWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    auto* group = new QGroupBox("骰子", this);
    auto* groupLayout = new QVBoxLayout(group);
    groupLayout->setSpacing(8);

    // 骰子显示行
    auto* diceLayout = new QHBoxLayout();

    // 大号字体显示骰子点数
    QFont diceFont("Arial", 36, QFont::Bold);

    m_die1Label = new QLabel("?", this);
    m_die1Label->setFont(diceFont);
    m_die1Label->setAlignment(Qt::AlignCenter);
    m_die1Label->setFixedSize(80, 80);
    m_die1Label->setStyleSheet(
        "background-color: #000000; color: white; border: 3px solid #333; border-radius: 10px;");
    diceLayout->addWidget(m_die1Label);

    m_die2Label = new QLabel("?", this);
    m_die2Label->setFont(diceFont);
    m_die2Label->setAlignment(Qt::AlignCenter);
    m_die2Label->setFixedSize(80, 80);
    m_die2Label->setStyleSheet(
        "background-color: #000000; color: white; border: 3px solid #333; border-radius: 10px;");
    diceLayout->addWidget(m_die2Label);

    groupLayout->addLayout(diceLayout);

    // 总点数
    m_totalLabel = new QLabel("总点数: --", this);
    m_totalLabel->setAlignment(Qt::AlignCenter);
    QFont totalFont("Arial", 14, QFont::Bold);
    m_totalLabel->setFont(totalFont);
    groupLayout->addWidget(m_totalLabel);

    // 掷骰子按钮
    m_rollBtn = new QPushButton("掷骰子！", this);
    m_rollBtn->setMinimumHeight(40);
    m_rollBtn->setEnabled(false);
    m_rollBtn->setStyleSheet(
        "QPushButton { background-color: #8F1A10; color: white; font-size: 16px; "
        "border-radius: 8px; padding: 8px; }"
        "QPushButton:hover { background-color: #E6C8C8; }"
        "QPushButton:disabled { background-color: #cccccc; }");
    groupLayout->addWidget(m_rollBtn);

    mainLayout->addWidget(group);
    setLayout(mainLayout);

    connect(m_rollBtn, &QPushButton::clicked, this, &DiceWidget::rollRequested);
}

void DiceWidget::setDice(int die1, int die2) {
    m_die1 = die1;
    m_die2 = die2;
    updateDisplay();
}

void DiceWidget::setRollEnabled(bool enabled) {
    m_rollBtn->setEnabled(enabled);
}

void DiceWidget::clearDice() {
    m_die1 = 0;
    m_die2 = 0;
    updateDisplay();
    m_rollBtn->setEnabled(false);
}

void DiceWidget::updateDisplay() {
    if (m_die1 > 0 && m_die2 > 0) {
        m_die1Label->setText(QString::number(m_die1));
        m_die2Label->setText(QString::number(m_die2));
        m_totalLabel->setText("总点数: " + QString::number(m_die1 + m_die2));
        // 掷出对子时高亮显示
        if (m_die1 == m_die2) {
            m_totalLabel->setText("总点数: " + QString::number(m_die1 + m_die2) + "  (对子！)");
            m_totalLabel->setStyleSheet("color: red;");
        } else {
            m_totalLabel->setStyleSheet("");
        }
    } else {
        m_die1Label->setText("?");
        m_die2Label->setText("?");
        m_totalLabel->setText("总点数: --");
        m_totalLabel->setStyleSheet("");
    }
}
