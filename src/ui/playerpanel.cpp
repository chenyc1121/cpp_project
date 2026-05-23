#include "playerpanel.h"
#include "../core/player.h"
#include "../core/tile.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGroupBox>

PlayerPanel::PlayerPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addStretch();
    setLayout(mainLayout);
}

void PlayerPanel::setPlayers(const QVector<Player*>& players) {
    for (auto& row : m_rows) {
        delete row.container;
    }
    m_rows.clear();
    m_players = players;

    auto* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (!mainLayout) return;

    for (int i = 0; i < players.size(); ++i) {
        PlayerRow row;
        row.container = new QFrame(this);
        row.container->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        row.container->setStyleSheet(
            "QFrame { border: 2px solid #ccc; border-radius: 6px; "
            "padding: 4px; margin: 2px; }");

        auto* rowLayout = new QHBoxLayout(row.container);
        rowLayout->setSpacing(6);

        auto* colorDot = new QLabel(this);
        colorDot->setFixedSize(16, 16);
        QString colorName = players[i]->color().name();
        colorDot->setStyleSheet(
            QString("background-color: %1; border-radius: 8px; border: 1px solid #999;")
            .arg(colorName));
        rowLayout->addWidget(colorDot);

        row.nameLabel = new QLabel(players[i]->name(), this);
        row.nameLabel->setMinimumWidth(50);
        row.nameLabel->setStyleSheet("font-weight: bold;");
        rowLayout->addWidget(row.nameLabel);

        row.moneyLabel = new QLabel(this);
        row.moneyLabel->setMinimumWidth(80);
        rowLayout->addWidget(row.moneyLabel);

        row.statusLabel = new QLabel(this);
        row.statusLabel->setMinimumWidth(60);
        rowLayout->addWidget(row.statusLabel);

        row.container->setLayout(rowLayout);
        mainLayout->insertWidget(mainLayout->count() - 1, row.container);
        m_rows.append(row);
    }

    for (int i = 0; i < players.size(); ++i) {
        updatePlayer(players[i]);
    }
}

void PlayerPanel::updatePlayer(Player* player) {
    int idx = m_players.indexOf(player);
    if (idx < 0 || idx >= m_rows.size()) return;

    auto& row = m_rows[idx];

    row.moneyLabel->setText(QString("¥%1").arg(player->money()));
    if (player->money() < 500) {
        row.moneyLabel->setStyleSheet("color: red; font-weight: bold;");
    } else if (player->money() > 30000) {
        row.moneyLabel->setStyleSheet("color: green; font-weight: bold;");
    } else {
        row.moneyLabel->setStyleSheet("");
    }

    QString status;
    if (player->isBankrupt()) {
        status = "破产";
        row.container->setStyleSheet(
            "QFrame { border: 2px solid #ccc; border-radius: 6px; "
            "padding: 4px; margin: 2px; background-color: #eee; }");
    } else {
        int cardCount = player->effectCardCount();
        if (cardCount > 0) {
            status = QString("卡x%1").arg(cardCount);
        } else {
            status = "✓";
        }
    }
    row.statusLabel->setText(status);
}

void PlayerPanel::highlightCurrentPlayer(Player* player) {
    for (int i = 0; i < m_players.size(); ++i) {
        if (i < m_rows.size()) {
            auto& row = m_rows[i];
            if (m_players[i] == player && !player->isBankrupt()) {
                row.container->setStyleSheet(
                    "QFrame { border: 3px solid #FFD700; border-radius: 6px; "
                    "padding: 4px; margin: 2px;}");
            } else if (!m_players[i]->isBankrupt()) {
                row.container->setStyleSheet(
                    "QFrame { border: 2px solid #ccc; border-radius: 6px; "
                    "padding: 4px; margin: 2px; }");
            }
        }
    }
}
