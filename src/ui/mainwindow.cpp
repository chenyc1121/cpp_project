#include "mainwindow.h"
#include "../core/game.h"
#include "../core/board.h"
#include "../core/player.h"
#include "../core/tile.h"
#include "../core/effectcard.h"
#include "../core/questionbank.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_boardWidget(new BoardWidget(this)),
      m_playerPanel(new PlayerPanel(this)),
      m_diceWidget(new DiceWidget(this)),
      m_eventLog(new EventLog(this)),
      m_statusLabel(new QLabel("欢迎来到大富翁！点击 游戏→新游戏 开始", this)),
      m_propertyArea(new QScrollArea(this)),
      m_propertyContent(new QWidget(this))
{
    setupUI();
    setupMenuBar();
    resize(960, 800);
    setWindowTitle("大富翁 Monopoly");
}

MainWindow::~MainWindow() {
    endCurrentGame();
}

// ==================== UI 布局 ====================
void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainVLayout = new QVBoxLayout(centralWidget);
    mainVLayout->setSpacing(6);
    mainVLayout->setContentsMargins(8, 8, 8, 8);

    // 上方区域：棋盘 + 右侧面板
    auto* topArea = new QHBoxLayout();
    topArea->setSpacing(8);

    topArea->addWidget(m_boardWidget, 1);

    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(6);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "QLabel { background-color: #FFF9C4; border: 2px solid #F9A825; "
        "border-radius: 6px; padding: 8px; font-size: 14px; font-weight: bold; }");
    m_statusLabel->setWordWrap(true);
    rightLayout->addWidget(m_statusLabel);

    rightLayout->addWidget(m_playerPanel);
    rightLayout->addWidget(m_diceWidget);
    rightLayout->addWidget(m_eventLog);

    rightPanel->setMaximumWidth(280);
    topArea->addWidget(rightPanel);

    mainVLayout->addLayout(topArea, 1);

    // 下方区域：玩家道具展示
    m_propertyArea->setWidgetResizable(true);
    m_propertyArea->setMaximumHeight(130);
    m_propertyArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_propertyArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_propertyArea->setStyleSheet(
        "QScrollArea { border: 2px solid #388E3C; border-radius: 6px; "
        "background-color: #E8F5E9; }");

    m_propertyContent->setStyleSheet("background: transparent;");
    auto* propLayout = new QHBoxLayout(m_propertyContent);
    propLayout->setSpacing(12);
    propLayout->setContentsMargins(8, 4, 8, 4);
    propLayout->addStretch();
    m_propertyContent->setLayout(propLayout);

    m_propertyArea->setWidget(m_propertyContent);
    mainVLayout->addWidget(m_propertyArea);
}

void MainWindow::setupMenuBar() {
    QMenu* gameMenu = menuBar()->addMenu("游戏(&G)");

    QAction* newGameAction = gameMenu->addAction("新游戏(&N)");
    newGameAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);

    QAction* rulesAction = gameMenu->addAction("规则说明(&R)");
    connect(rulesAction, &QAction::triggered, this, &MainWindow::onRules);

    gameMenu->addSeparator();

    QAction* quitAction = gameMenu->addAction("退出(&Q)");
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

// ==================== 信号连接 ====================
void MainWindow::connectSignals() {
    if (!m_game) return;

    connect(m_diceWidget, &DiceWidget::rollRequested,
            this, &MainWindow::onRollDice);

    connect(m_game, &Game::eventLogged,
            this, &MainWindow::onLogEvent);
    connect(m_game, &Game::diceRolled,
            this, &MainWindow::onDiceRolled);
    connect(m_game, &Game::playerUpdated,
            this, &MainWindow::onPlayerUpdated);
    connect(m_game, &Game::playerMoved,
            this, &MainWindow::onPlayerMoved);
    connect(m_game, &Game::turnStarted,
            this, &MainWindow::onTurnStarted);
    connect(m_game, &Game::gameOver,
            this, &MainWindow::onGameOver);
    connect(m_game, &Game::promptBuyProperty,
            this, &MainWindow::onPromptBuyProperty);
    connect(m_game, &Game::promptBuildHouse,
            this, &MainWindow::onPromptBuildHouse);
    connect(m_game, &Game::boardUpdated,
            m_boardWidget, [this]() { m_boardWidget->refreshBoard(); });
    connect(m_game, &Game::boardUpdated,
            this, [this]() { updatePropertyDisplay(); });

    // 新增信号
    connect(m_game, &Game::promptQA,
            this, &MainWindow::onPromptQA);
    connect(m_game, &Game::promptComputerLab,
            this, &MainWindow::onPromptComputerLab);
    connect(m_game, &Game::promptShop,
            this, &MainWindow::onPromptShop);
    connect(m_game, &Game::promptShopEntrance,
            this, &MainWindow::onPromptShopEntrance);
    connect(m_game, &Game::promptUseCard,
            this, &MainWindow::onPromptUseCard);
    connect(m_game, &Game::promptUniversalDice,
            this, &MainWindow::onPromptUniversalDice);

    // 虚函数卡
    connect(m_game, &Game::promptVirtualFuncBuy,
            this, &MainWindow::onPromptVirtualFuncBuy);
    connect(m_game, &Game::promptVirtualFuncRent,
            this, &MainWindow::onPromptVirtualFuncRent);
    connect(m_game, &Game::promptVirtualFuncBuild,
            this, &MainWindow::onPromptVirtualFuncBuild);
}


// ==================== 游戏流程 ====================
void MainWindow::startNewGame() {
    endCurrentGame();

    m_game = new Game(this);
    m_boardWidget->setBoard(&m_game->board());

    bool ok;
    int playerCount = QInputDialog::getInt(this, "玩家数量",
                                           "请输入玩家人数 (2-4):", 2, 2, 4, 1, &ok);
    if (!ok) {
        delete m_game;
        m_game = nullptr;
        return;
    }

    QVector<QColor> colors = {QColor("#E53935"), QColor("#1E88E5"),
                              QColor("#43A047"), QColor("#FB8C00")};
    QVector<QString> defaultNames = {"小红", "小蓝", "小绿", "小橙"};

    for (int i = 0; i < playerCount; ++i) {
        QString name = QInputDialog::getText(this, "玩家 " + QString::number(i + 1),
                                             "请输入玩家名称:",
                                             QLineEdit::Normal,
                                             defaultNames[i], &ok);
        if (!ok || name.isEmpty()) {
            name = defaultNames[i];
        }
        m_game->addPlayer(name, colors[i]);
    }

    m_eventLog->clear();
    m_playerPanel->setPlayers(m_game->players());
    m_boardWidget->setPlayers(&m_game->players());
    m_diceWidget->clearDice();
    updatePropertyDisplay();

    connectSignals();
    m_game->startGame();
}

void MainWindow::endCurrentGame() {
    if (m_game) {
        m_game->deleteLater();
        m_game = nullptr;
    }
}

void MainWindow::onNewGame() {
    startNewGame();
}

void MainWindow::onRules() {
    QString rules =
        "=== 大富翁 游戏规则 ===\n\n"
        "1. 轮流掷两颗骰子，按点数前进\n"
        "2. 停在无人拥有的地产上可以购买\n"
        "3. 停在他人地产上需要支付租金\n"
        "4. 拥有同色全部地产后可以建造房屋和旅馆\n"
        "5. 经过或停在起点可获得奖金\n"
        "6. 掷出对子（两颗骰子点数相同）可以再掷一次\n"
        "7. 走到问答格需回答C++选择题，答对大概率得效果卡\n"
        "8. 走到商店可购买效果卡，商店入口可选择是否进入\n"
        "9. 走到上机课需答题且跳过下一回合，答对必得效果卡\n"
        "10. 最后一位未破产的玩家获胜！\n\n"
        "效果卡类型：\n"
        "  • 再丢一次骰子 — 重掷骰子\n"
        "  • 万能骰子 — 自选骰子点数(1-6)\n"
        "  • 虚函数卡 — 占位（待实现）\n"
        "  • 跳过卡 — 跳过当前地块的负面效果\n\n"
        "地产颜色组（成套仅影响租金加成）：\n"
        "  棕色(2块) → 浅蓝(3块) → 粉色(3块) → 橙色(3块)\n"
        "  → 红色(3块) → 黄色 → 绿色 → 深蓝(2块)";
    QMessageBox::information(this, "游戏规则", rules);
}


// ==================== 掷骰子 ====================
void MainWindow::onRollDice() {
    if (!m_game) return;
    m_game->rollDice();
}

// ==================== 玩家道具展示 ====================
static QColor chipColorForGroup(ColorGroup g) {
    switch (g) {
    case ColorGroup::BROWN:       return QColor("#8D6E3F");
    case ColorGroup::LIGHT_BLUE:  return QColor("#4FC3F7");
    case ColorGroup::PINK:        return QColor("#BA68C8");
    case ColorGroup::ORANGE:      return QColor("#FF9800");
    case ColorGroup::RED:         return QColor("#E53935");
    case ColorGroup::YELLOW:      return QColor("#FBC02D");
    case ColorGroup::GREEN:       return QColor("#2ECC71");
    case ColorGroup::DEEP_BLUE:   return QColor("#1565C0");
    default: return QColor("#9E9E9E");
    }
}

void MainWindow::updatePropertyDisplay() {
    auto* layout = qobject_cast<QBoxLayout*>(m_propertyContent->layout());
    if (!layout) return;

    // 清除现有内容
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    if (!m_game) {
        layout->addStretch();
        return;
    }

    const auto& players = m_game->players();

    for (Player* player : players) {
        auto* playerGroup = new QWidget(m_propertyContent);
        auto* pgLayout = new QVBoxLayout(playerGroup);
        pgLayout->setSpacing(3);
        pgLayout->setContentsMargins(6, 3, 6, 3);

        // 玩家头部：色点 + 名字
        auto* headerRow = new QHBoxLayout();
        headerRow->setSpacing(4);

        auto* colorDot = new QLabel(playerGroup);
        colorDot->setFixedSize(12, 12);
        colorDot->setStyleSheet(
            QString("background-color: %1; border-radius: 6px; border: 1px solid #666;")
            .arg(player->color().name()));
        headerRow->addWidget(colorDot);

        auto* nameLabel = new QLabel(player->name(), playerGroup);
        nameLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
        headerRow->addWidget(nameLabel);

        if (player->isBankrupt()) {
            auto* bkrLabel = new QLabel("(破产)", playerGroup);
            bkrLabel->setStyleSheet("color: red; font-size: 10px;");
            headerRow->addWidget(bkrLabel);
        }

        headerRow->addStretch();
        pgLayout->addLayout(headerRow);

        // 道具芯片行
        auto* chipsRow = new QHBoxLayout();
        chipsRow->setSpacing(3);

        bool hasItems = false;

        for (PropertyTile* pt : player->properties()) {
            hasItems = true;
            QColor c;
            if (pt->type() == TileType::STATICVAL) {
                c = QColor("#FFD700");  // 金色 — 静态成员变量
            } else if (pt->type() == TileType::VIRTUALFUNC) {
                c = QColor("#7C4DFF");  // 紫色 — 虚函数
            } else {
                c = chipColorForGroup(pt->group());
            }
            QString typeLabel;
            if (pt->type() == TileType::STATICVAL) {
                typeLabel = "静态变量组";
            } else if (pt->type() == TileType::VIRTUALFUNC) {
                typeLabel = "虚函数格";
            } else {
                typeLabel = colorGroupName(pt->group());
            }
            auto* chip = new QLabel(pt->name(), playerGroup);
            chip->setStyleSheet(QString(
                "background-color: %1; color: white; border-radius: 3px; "
                "padding: 1px 6px; font-size: 10px; font-weight: bold;")
                .arg(c.name()));
            chip->setToolTip(pt->name() + " (" + typeLabel + ")");
            chipsRow->addWidget(chip);
        }

        for (int i = 0; i < BOARD_SIZE; ++i) {
            Tile* t = m_game->board().tileAt(i);
            if (auto* rt = dynamic_cast<RailroadTile*>(t)) {
                if (rt->owner() == player) {
                    hasItems = true;
                    auto* chip = new QLabel("R:" + rt->name(), playerGroup);
                    chip->setStyleSheet(
                        "background-color: #5D4037; color: white; border-radius: 3px; "
                        "padding: 1px 6px; font-size: 10px;");
                    chipsRow->addWidget(chip);
                }
            }
            if (auto* ut = dynamic_cast<UtilityTile*>(t)) {
                if (ut->owner() == player) {
                    hasItems = true;
                    auto* chip = new QLabel("U:" + ut->name(), playerGroup);
                    chip->setStyleSheet(
                        "background-color: #455A64; color: white; border-radius: 3px; "
                        "padding: 1px 6px; font-size: 10px;");
                    chipsRow->addWidget(chip);
                }
            }
        }

        for (const auto& card : player->effectCards()) {
            hasItems = true;
            auto* chip = new QLabel(card.name, playerGroup);
            chip->setStyleSheet(
                "background-color: #E65100; color: white; border-radius: 3px; "
                "padding: 1px 6px; font-size: 10px;");
            chipsRow->addWidget(chip);
        }

        if (!hasItems) {
            auto* emptyLabel = new QLabel("暂无道具", playerGroup);
            emptyLabel->setStyleSheet("color: #999; font-size: 10px; font-style: italic;");
            chipsRow->addWidget(emptyLabel);
        }

        chipsRow->addStretch();
        pgLayout->addLayout(chipsRow);

        layout->addWidget(playerGroup);
    }

    layout->addStretch();
}

void MainWindow::onDiceRolled(int die1, int die2) {
    m_diceWidget->setDice(die1, die2);
}


// ==================== 回合通知 ====================
void MainWindow::onTurnStarted(Player* player) {
    if (!player) return;
    m_statusLabel->setText("当前回合：" + player->name());
    m_statusLabel->setStyleSheet(
        "QLabel { background-color: #C8E6C9; border: 2px solid #388E3C; "
        "border-radius: 6px; padding: 8px; font-size: 14px; font-weight: bold; }");
    m_playerPanel->highlightCurrentPlayer(player);
    m_diceWidget->setRollEnabled(true);
}

void MainWindow::onPlayerMoved(Player* player, int from, int to) {
    Q_UNUSED(from)
    Q_UNUSED(to)
    if (player) {
        m_statusLabel->setText(player->name() + " 正在移动...");
    }
    m_boardWidget->refreshBoard();
    m_playerPanel->updatePlayer(player);
}

void MainWindow::onPlayerUpdated(Player* player) {
    if (player) {
        m_playerPanel->updatePlayer(player);
    }
    m_boardWidget->refreshBoard();
    updatePropertyDisplay();
}


// ==================== 购买/升级提示 ====================
void MainWindow::onPromptBuyProperty(int tileIndex, Player* player) {
    if (!m_game || !player) return;

    Tile* t = m_game->board().tileAt(tileIndex);
    if (!t) return;

    // 虚函数格 + 有虚函数卡 → 弹出选择对话框
    if (auto* vt = dynamic_cast<VirtualfuncTile*>(t)) {
        if (player->hasEffectCard(EffectCardType::VIRTUAL_FUNCTION)) {
            int basePrice = vt->PropertyTile::price();
            int derivedPrice = vt->price();
            onPromptVirtualFuncBuy(player, tileIndex, basePrice, derivedPrice);
            return;
        }
    }

    QString msg;
    int price = 0;
    if (auto* pt = dynamic_cast<PropertyTile*>(t)) {
        price = pt->price();
        msg = player->name() + "，是否购买 " + t->name()
              + "？\n价格：¥" + QString::number(price)
              + "\n当前资金：¥" + QString::number(player->money());
    } else if (auto* ut = dynamic_cast<UtilityTile*>(t)) {
        price = ut->price();
        msg = player->name() + "，是否购买 " + t->name()
              + "？\n价格：¥" + QString::number(price);
    } else if (auto* rt = dynamic_cast<RailroadTile*>(t)) {
        price = rt->price();
        msg = player->name() + "，是否购买 " + t->name()
              + "？\n价格：¥" + QString::number(price);
    }

    if (player->money() < price) {
        m_game->logEvent(player->name() + " 资金不足，无法购买 " + t->name());
        m_game->skipAction();
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "购买地产", msg,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (reply == QMessageBox::Yes) {
        m_game->buyProperty(player, tileIndex);
    } else {
        m_game->logEvent(player->name() + " 放弃购买 " + t->name());
        m_game->skipAction();
    }
}

void MainWindow::onPromptBuildHouse(int tileIndex, Player* player) {
    if (!m_game || !player) return;

    auto* pt = dynamic_cast<PropertyTile*>(m_game->board().tileAt(tileIndex));
    if (!pt) return;

    if (!pt->canBuildHouse(player)) {
        m_game->skipAction();
        return;
    }

    // 虚函数格 + 有虚函数卡 → 弹出选择对话框
    if (auto* vt = dynamic_cast<VirtualfuncTile*>(pt)) {
        if (player->hasEffectCard(EffectCardType::VIRTUAL_FUNCTION)) {
            int baseCost = vt->houseCost();
            int derivedCost = vt->houseCost();  // 建房费相同，但 buildHouse 行为可能不同
            onPromptVirtualFuncBuild(player, tileIndex, baseCost, derivedCost);
            return;
        }
    }

    QString info;
    if (pt->houses() < 4) {
        info = "升级 " + pt->name() + "？\n"
               "当前：" + QString::number(pt->houses()) + " 栋房子\n"
               "建房费用：¥" + QString::number(pt->houseCost())
               + "\n当前资金：¥" + QString::number(player->money());
    } else {
        info = "在 " + pt->name() + " 建造旅馆？\n"
               "费用：¥" + QString::number(pt->houseCost())
               + "\n当前资金：¥" + QString::number(player->money());
    }

    if (player->money() < pt->houseCost()) {
        m_game->logEvent(player->name() + " 资金不足，无法升级 " + pt->name());
        m_game->skipAction();
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "升级地产", info,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_game->buildHouse(player, tileIndex);
    } else {
        m_game->skipAction();
    }
}


// ==================== 问答格 ====================
void MainWindow::onPromptQA(Player* player, int tileIndex) {
    if (!m_game || !player) return;

    Question q = QuestionBank::drawRandom();
    m_game->setCurrentQuestion(q);

    QStringList options;
    options << "A. " + q.optionA
            << "B. " + q.optionB
            << "C. " + q.optionC
            << "D. " + q.optionD;

    bool ok;
    QString choice = QInputDialog::getItem(this,
        "问答格 - " + player->name(),
        q.text + "\n\n" + options.join("\n"),
        options, 0, false, &ok);

    if (ok) {
        int answerIndex = options.indexOf(choice);
        m_game->answerQA(player, tileIndex, answerIndex);
    } else {
        m_game->answerQA(player, tileIndex, -1);
    }
}


// ==================== 上机课 ====================
void MainWindow::onPromptComputerLab(Player* player) {
    if (!m_game || !player) return;

    Question q = QuestionBank::drawRandom();
    m_game->setCurrentQuestion(q);

    QStringList options;
    options << "A. " + q.optionA
            << "B. " + q.optionB
            << "C. " + q.optionC
            << "D. " + q.optionD;

    bool ok;
    QString choice = QInputDialog::getItem(this,
        "上机课 - " + player->name(),
        player->name() + " 进入上机课！\n停止一回合，回答一道C++题：\n\n" + q.text + "\n\n" + options.join("\n"),
        options, 0, false, &ok);

    if (ok) {
        int answerIndex = options.indexOf(choice);
        m_game->answerComputerLab(player, answerIndex);
    } else {
        m_game->answerComputerLab(player, -1);
    }
}


// ==================== 商店 ====================
void MainWindow::onPromptShop(Player* player) {
    if (!m_game || !player) return;

    QStringList items;
    QVector<EffectCardType> types;

    EffectCardType allTypes[] = {
        EffectCardType::ROLL_AGAIN,
        EffectCardType::UNIVERSAL_DICE,
        EffectCardType::VIRTUAL_FUNCTION,
        EffectCardType::SKIP_EFFECT
    };

    for (auto type : allTypes) {
        EffectCard card = createEffectCard(type);
        types.append(type);
        if (player->canAfford(card.price)) {
            items << card.name + " (¥" + QString::number(card.price) + ")";
        } else {
            items << card.name + " (¥" + QString::number(card.price) + ") - 资金不足";
        }
    }

    bool ok;
    QString choice = QInputDialog::getItem(this,
        "商店 - " + player->name(),
        "欢迎光临！请选择要购买的卡片：\n当前资金：¥" + QString::number(player->money()),
        items, 0, false, &ok);

    if (ok) {
        int idx = items.indexOf(choice);
        if (idx >= 0 && idx < types.size()) {
            EffectCardType selectedType = types[idx];
            EffectCard card = createEffectCard(selectedType);
            if (player->canAfford(card.price)) {
                m_game->buyEffectCard(player, selectedType);
            } else {
                m_game->logEvent(player->name() + " 资金不足，无法购买！");
            }
        }
    }

    m_game->skipAction();
}


// ==================== 商店入口 ====================
void MainWindow::onPromptShopEntrance(Player* player) {
    if (!m_game || !player) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "商店入口",
        player->name() + "，前方是商店！是否进入购买效果卡？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_game->goToShop(player);
    } else {
        m_game->declineShopEntrance();
    }
}


// ==================== 使用效果卡 ====================
void MainWindow::onPromptUseCard(Player* player, EffectCardType type) {
    if (!m_game || !player) return;

    EffectCard card = createEffectCard(type);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "使用效果卡",
        player->name() + "，是否使用 " + card.name + "？\n" + card.description,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    m_game->onCardDecision(type, reply == QMessageBox::Yes);
}


// ==================== 万能骰子 ====================
void MainWindow::onPromptUniversalDice(Player* player) {
    if (!m_game || !player) return;

    QDialog dialog(this);
    dialog.setWindowTitle("万能骰子 - " + player->name());

    QFormLayout* form = new QFormLayout(&dialog);

    QSpinBox* spin1 = new QSpinBox(&dialog);
    spin1->setRange(1, 6);
    QSpinBox* spin2 = new QSpinBox(&dialog);
    spin2->setRange(1, 6);

    form->addRow("骰子1:", spin1);
    form->addRow("骰子2:", spin2);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        m_game->setUniversalDice(spin1->value(), spin2->value());
    } else {
        m_game->onCardDecision(EffectCardType::UNIVERSAL_DICE, false);
    }
}


// ==================== 虚函数卡交互 ====================
void MainWindow::onPromptVirtualFuncBuy(Player* player, int tileIndex,
                                         int basePrice, int derivedPrice) {
    if (!m_game || !player) return;

    Tile* t = m_game->board().tileAt(tileIndex);
    if (!t) return;

    QString msg = player->name() + "，是否购买 " + t->name() + "？\n\n"
                  "🏠 基类价格：¥" + QString::number(basePrice) + "（固定、安全）\n"
                  "🔀 派生类价格：¥" + QString::number(derivedPrice) + "（需消耗虚函数卡）\n\n"
                  "当前资金：¥" + QString::number(player->money());

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("虚函数卡 — 购买地产");
    msgBox.setText(msg);

    QPushButton* baseBtn = msgBox.addButton("基类购买 (¥" + QString::number(basePrice) + ")", QMessageBox::YesRole);
    QPushButton* derivedBtn = nullptr;
    if (player->canAfford(derivedPrice)) {
        derivedBtn = msgBox.addButton("派生类购买 (¥" + QString::number(derivedPrice) + ")", QMessageBox::AcceptRole);
    }
    QPushButton* cancelBtn = msgBox.addButton("取消", QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == derivedBtn) {
        m_game->buyPropertyVirtualFunc(player, tileIndex, true);
    } else if (msgBox.clickedButton() == baseBtn) {
        m_game->buyPropertyVirtualFunc(player, tileIndex, false);
    } else {
        m_game->logEvent(player->name() + " 放弃购买 " + t->name());
        m_game->skipAction();
    }
}

void MainWindow::onPromptVirtualFuncRent(Player* player, int tileIndex,
                                          int baseRent, int derivedRent) {
    if (!m_game || !player) return;

    Tile* t = m_game->board().tileAt(tileIndex);
    if (!t) return;

    auto* vt = dynamic_cast<VirtualfuncTile*>(t);
    QString ownerName = vt ? (vt->owner() ? vt->owner()->name() : "?") : "?";

    QString msg = player->name() + "，你停在 " + ownerName + " 的虚函数格 " + t->name() + "！\n\n"
                  "🏠 基类租金：¥" + QString::number(baseRent) + "（固定、安全）\n"
                  "🔀 派生类租金：¥" + QString::number(derivedRent) + "（需消耗虚函数卡）\n\n"
                  "当前资金：¥" + QString::number(player->money());

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("虚函数卡 — 支付租金");
    msgBox.setText(msg);

    QPushButton* baseBtn = msgBox.addButton("基类付租 (¥" + QString::number(baseRent) + ")", QMessageBox::YesRole);
    QPushButton* derivedBtn = nullptr;
    if (player->canAfford(derivedRent) || true) {  // 即使付不起也可以选
        derivedBtn = msgBox.addButton("派生类付租 (¥" + QString::number(derivedRent) + ")", QMessageBox::AcceptRole);
    }
    QPushButton* cancelBtn = msgBox.addButton("跳过（不使用虚函数卡）", QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == derivedBtn) {
        m_game->payRentVirtualFunc(player, tileIndex, true);
    } else if (msgBox.clickedButton() == cancelBtn) {
        // 默认基类付租
        m_game->payRentVirtualFunc(player, tileIndex, false);
    } else {
        m_game->payRentVirtualFunc(player, tileIndex, false);
    }
}

void MainWindow::onPromptVirtualFuncBuild(Player* player, int tileIndex,
                                           int baseCost, int derivedCost) {
    if (!m_game || !player) return;

    Tile* t = m_game->board().tileAt(tileIndex);
    if (!t) return;

    auto* vt = dynamic_cast<VirtualfuncTile*>(t);
    if (!vt) return;

    QString info;
    if (vt->houses() < 4) {
        info = "升级 " + vt->name() + "？\n当前：" + QString::number(vt->houses()) + " 栋房子\n\n";
    } else {
        info = "在 " + vt->name() + " 建造旅馆？\n\n";
    }

    info += "🏠 基类建房费：¥" + QString::number(baseCost) + "\n"
            "🔀 派生类建房费：¥" + QString::number(derivedCost) + "（需消耗虚函数卡）\n\n"
            "当前资金：¥" + QString::number(player->money());

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("虚函数卡 — 建房");
    msgBox.setText(info);

    QPushButton* baseBtn = msgBox.addButton("基类建造 (¥" + QString::number(baseCost) + ")", QMessageBox::YesRole);
    QPushButton* derivedBtn = nullptr;
    if (player->canAfford(derivedCost)) {
        derivedBtn = msgBox.addButton("派生类建造 (¥" + QString::number(derivedCost) + ")", QMessageBox::AcceptRole);
    }
    QPushButton* cancelBtn = msgBox.addButton("取消", QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == derivedBtn) {
        m_game->buildHouseVirtualFunc(player, tileIndex, true);
    } else if (msgBox.clickedButton() == baseBtn) {
        m_game->buildHouseVirtualFunc(player, tileIndex, false);
    } else {
        m_game->skipAction();
    }
}


// ==================== 游戏结束 ====================
void MainWindow::onGameOver(Player* winner) {
    m_diceWidget->setRollEnabled(false);
    if (winner) {
        m_statusLabel->setText("游戏结束！胜者：" + winner->name());
        m_statusLabel->setStyleSheet(
            "QLabel { background-color: #FFD700; border: 2px solid #FF6F00; "
            "border-radius: 6px; padding: 8px; font-size: 16px; font-weight: bold; }");
        QMessageBox::information(this, "游戏结束",
                                 winner->name() + " 赢得了比赛！\n\n"
                                 + playerSummary(winner));
    }
    m_boardWidget->refreshBoard();
}


// ==================== 事件日志 ====================
void MainWindow::onLogEvent(const QString& msg) {
    m_eventLog->appendMessage(msg);
}


// ==================== 工具 ====================
QString MainWindow::playerSummary(Player* p) const {
    if (!p) return "";
    return QString("玩家：%1\n剩余资金：¥%2\n拥有地产：%3 块\n效果卡：%4 张")
        .arg(p->name())
        .arg(p->money())
        .arg(p->propertyCount())
        .arg(p->effectCardCount());
}
