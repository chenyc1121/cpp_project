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
            "QFrame { border: 1px solid #D6C8B8; border-radius: 8px; "
            "padding: 6px 8px; margin: 2px; "
            "background-color: #FFFFFF; }");

        auto* rowLayout = new QHBoxLayout(row.container);
        rowLayout->setSpacing(8);

        auto* colorDot = new QLabel(this);
        colorDot->setFixedSize(14, 14);
        QString colorName = players[i]->color().name();
        colorDot->setStyleSheet(
            QString("background-color: %1; border-radius: 7px; "
                    "border: 2px solid rgba(0,0,0,0.15);")
            .arg(colorName));
        rowLayout->addWidget(colorDot);

        QString displayName = players[i]->isAI()
            ? players[i]->name() + " · AI"
            : players[i]->name();
        row.nameLabel = new QLabel(displayName, this);
        row.nameLabel->setMinimumWidth(50);
        row.nameLabel->setStyleSheet("font-weight: 600; color: #3D2820; font-size: 13px;");
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
        row.moneyLabel->setStyleSheet("color: #C0392B; font-weight: bold; font-size: 12px;");
    } else if (player->money() > 30000) {
        row.moneyLabel->setStyleSheet("color: #2E7D32; font-weight: bold; font-size: 12px;");
    } else {
        row.moneyLabel->setStyleSheet("color: #4A3028; font-size: 12px;");
    }

    QString status;
    if (player->isBankrupt()) {
        status = "破产";
        row.container->setStyleSheet(
            "QFrame { border: 1px solid #D0C8C0; border-radius: 8px; "
            "padding: 6px 8px; margin: 2px; background-color: #EDE8E2; }");
    } else {
        int cardCount = player->effectCardCount();
        if (cardCount > 0) {
            status = QString("卡 ×%1").arg(cardCount);
        } else {
            status = "—";
        }
    }
    row.statusLabel->setText(status);
    row.statusLabel->setStyleSheet("color: #8B7355; font-size: 12px;");
}

void PlayerPanel::highlightCurrentPlayer(Player* player) {
    for (int i = 0; i < m_players.size(); ++i) {
        if (i < m_rows.size()) {
            auto& row = m_rows[i];
            if (m_players[i] == player && !player->isBankrupt()) {
                row.container->setStyleSheet(
                    "QFrame { border: 2px solid #C8963E; border-radius: 8px; "
                    "padding: 6px 8px; margin: 2px; "
                    "background-color: #FFF8F5; }");
            } else if (!m_players[i]->isBankrupt()) {
                row.container->setStyleSheet(
                    "QFrame { border: 1px solid #D6C8B8; border-radius: 8px; "
                    "padding: 6px 8px; margin: 2px; "
                    "background-color: #FFFFFF; }");
            }
        }
    }
}
