#include "dicewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFont>

DiceWidget::DiceWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* group = new QGroupBox("骰 子", this);
    group->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 13px; color: #8B1A1A; "
        "border: 2px solid #8B1A1A; border-radius: 8px; margin-top: 12px; padding-top: 16px; "
        "background-color: #FFFFFF; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 16px; padding: 0 6px; "
        "color: #8B1A1A; }");
    auto* groupLayout = new QVBoxLayout(group);
    groupLayout->setSpacing(10);

    // 骰子显示行
    auto* diceLayout = new QHBoxLayout();
    diceLayout->setSpacing(12);

    QString diceStyle =
        "QLabel { "
        "background-color: #FFF8F5; "
        "color: #8B1A1A; "
        "border: 2px solid #8B1A1A; "
        "border-radius: 10px; "
        "font-size: 36px; font-weight: bold; }";

    QFont diceFont("Georgia", 36, QFont::Bold);

    m_die1Label = new QLabel("?", this);
    m_die1Label->setFont(diceFont);
    m_die1Label->setAlignment(Qt::AlignCenter);
    m_die1Label->setFixedSize(76, 76);
    m_die1Label->setStyleSheet(diceStyle);
    diceLayout->addWidget(m_die1Label);

    m_die2Label = new QLabel("?", this);
    m_die2Label->setFont(diceFont);
    m_die2Label->setAlignment(Qt::AlignCenter);
    m_die2Label->setFixedSize(76, 76);
    m_die2Label->setStyleSheet(diceStyle);
    diceLayout->addWidget(m_die2Label);

    groupLayout->addLayout(diceLayout);

    // 总点数
    m_totalLabel = new QLabel("总点数: --", this);
    m_totalLabel->setAlignment(Qt::AlignCenter);
    QFont totalFont("Microsoft YaHei", 13, QFont::Medium);
    m_totalLabel->setFont(totalFont);
    m_totalLabel->setStyleSheet("color: #8B1A1A; padding: 2px;");
    groupLayout->addWidget(m_totalLabel);

    // 掷骰子按钮
    m_rollBtn = new QPushButton("掷 骰 子", this);
    m_rollBtn->setMinimumHeight(42);
    m_rollBtn->setEnabled(false);
    m_rollBtn->setCursor(Qt::PointingHandCursor);
    m_rollBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #B22222;"
        "  color: #FFFFFF;"
        "  font-size: 15px; font-weight: bold;"
        "  border: 2px solid #8B1A1A;"
        "  border-radius: 8px;"
        "  padding: 8px 20px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #C62828;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #8B1A1A;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #E0D8D0;"
        "  color: #A09890;"
        "  border: 1px solid #C0B8B0;"
        "}");
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
        if (m_die1 == m_die2) {
            m_totalLabel->setText("总点数: " + QString::number(m_die1 + m_die2) + " · 对子！");
            m_totalLabel->setStyleSheet("color: #B22222; font-weight: bold; padding: 2px;");
            QString doubleStyle =
                "QLabel { "
                "background-color: #FFF0F0; "
                "color: #B22222; "
                "border: 3px solid #B22222; "
                "border-radius: 10px; "
                "font-size: 36px; font-weight: bold; }";
            m_die1Label->setStyleSheet(doubleStyle);
            m_die2Label->setStyleSheet(doubleStyle);
        } else {
            m_totalLabel->setText("总点数: " + QString::number(m_die1 + m_die2));
            m_totalLabel->setStyleSheet("color: #8B1A1A; padding: 2px;");
            QString normalStyle =
                "QLabel { "
                "background-color: #FFF8F5; "
                "color: #8B1A1A; "
                "border: 2px solid #8B1A1A; "
                "border-radius: 10px; "
                "font-size: 36px; font-weight: bold; }";
            m_die1Label->setStyleSheet(normalStyle);
            m_die2Label->setStyleSheet(normalStyle);
        }
    } else {
        m_die1Label->setText("?");
        m_die2Label->setText("?");
        m_totalLabel->setText("总点数: --");
        m_totalLabel->setStyleSheet("color: #A09890; padding: 2px;");
    }
}
