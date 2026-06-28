#include "mainwindow.h"
#include "../core/game.h"
#include "../core/board.h"
#include "../core/player.h"
#include "../core/tile.h"
#include "../core/effectcard.h"
#include "../core/questionbank.h"
#include "../core/knowledgebank.h"

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
#include <QComboBox>
#include <QDialogButtonBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QTimer>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_boardWidget(new BoardWidget(this)),
      m_playerPanel(new PlayerPanel(this)),
      m_diceWidget(new DiceWidget(this)),
      m_eventLog(new EventLog(this)),
      m_statusLabel(new QLabel("欢迎来到程序设计实习大富翁！点击 游戏→新游戏 开始", this)),
      m_propertyArea(new QScrollArea(this)),
      m_propertyContent(new QWidget(this))
{
    setupUI();
    setupMenuBar();
    resize(960, 800);
    setWindowTitle("程设大富翁 mOnOPoly");
}

MainWindow::~MainWindow() {
    endCurrentGame();
}

// ==================== UI 布局 ====================
void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #E8E0D8;");
    setCentralWidget(centralWidget);

    auto* mainVLayout = new QVBoxLayout(centralWidget);
    mainVLayout->setSpacing(4);
    mainVLayout->setContentsMargins(4, 4, 4, 4);

    // 上方区域：棋盘 + 右侧面板（按比例自适应）
    auto* topArea = new QHBoxLayout();
    topArea->setSpacing(4);

    topArea->addWidget(m_boardWidget, 7);  // 棋盘占 70%

    auto* rightPanel = new QWidget(this);
    rightPanel->setStyleSheet("background-color: #E8E0D8;");
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(4);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "QLabel { background-color: #FFF8F5; border: 2px solid #8B1A1A; "
        "border-radius: 6px; padding: 8px; font-size: 13px; font-weight: bold; color: #8B1A1A;}");
    m_statusLabel->setWordWrap(true);
    rightLayout->addWidget(m_statusLabel);

    rightLayout->addWidget(m_playerPanel);
    rightLayout->addWidget(m_diceWidget);
    rightLayout->addWidget(m_eventLog);

    topArea->addWidget(rightPanel, 3);  // 右侧面板占 30%
    mainVLayout->addLayout(topArea, 9);  // 上方占 90%

    // 下方道具展示区（自适应高度）
    m_propertyArea->setWidgetResizable(true);
    m_propertyArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_propertyArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_propertyArea->setStyleSheet(
        "QScrollArea { border: 1px solid #8B1A1A; border-radius: 4px; "
        "background-color: #FFF8F5; }"
        "QScrollBar:horizontal { height: 6px; background: #F0E8DD; border-radius: 3px; }"
        "QScrollBar::handle:horizontal { background: #8B1A1A; border-radius: 3px; min-width: 20px; }");

    m_propertyContent->setStyleSheet("background: transparent;");
    auto* propLayout = new QHBoxLayout(m_propertyContent);
    propLayout->setSpacing(12);
    propLayout->setContentsMargins(8, 4, 8, 4);
    propLayout->addStretch();
    m_propertyContent->setLayout(propLayout);
    m_propertyArea->setWidget(m_propertyContent);

    mainVLayout->addWidget(m_propertyArea, 1);  // 下方占 10%

    // 棋盘中央"开始游戏"按钮
    connect(m_boardWidget, &BoardWidget::startGameRequested,
            this, &MainWindow::onNewGame);
}

void MainWindow::setupMenuBar() {
    QMenu* gameMenu = menuBar()->addMenu("游戏(&G)");

    QAction* newGameAction = gameMenu->addAction("新游戏(&N)");
    newGameAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);

    QAction* debugAction = gameMenu->addAction("DEBUG 模式(&D)");
    debugAction->setCheckable(true);
    debugAction->setChecked(false);
    connect(debugAction, &QAction::toggled, this, [this](bool checked) {
        m_debugMode = checked;
        if (m_game) m_game->setDebugMode(checked);
        m_statusLabel->setText(checked
            ? "DEBUG 模式已开启 —— 手动骰子 + 知识点控制"
            : "DEBUG 模式已关闭");
    });

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

    // 迭代器卡
    connect(m_game, &Game::promptIteratorCard,
            this, &MainWindow::onPromptIteratorCard);

    // 知识点事件
    connect(m_game, &Game::promptKnowledge,
            this, &MainWindow::onPromptKnowledge);
    connect(m_game, &Game::promptDebugKnowledge,
            this, &MainWindow::onPromptDebugKnowledge);

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

    QVector<QColor> colors = {QColor("#E53935"), QColor("#1E88E5"),
                              QColor("#43A047"), QColor("#FB8C00")};
    QVector<QString> defaultNames = {"小红", "小蓝", "小绿", "小橙"};

    // ── 游戏设置对话框 ──
    QDialog dlg(this);
    dlg.setWindowTitle("新游戏设置");
    dlg.setMinimumWidth(420);
    dlg.setStyleSheet(
        "QDialog { background-color: #FFFFFF; }"
        "QLabel { color: #3D2820; font-size: 13px; }"
        "QLineEdit { font-size: 13px; padding: 5px 8px; border: 1px solid #8B1A1A; "
        "border-radius: 4px; background: #FFFFFF; }"
        "QComboBox { font-size: 13px; padding: 4px 8px; border: 1px solid #8B1A1A; "
        "border-radius: 4px; background: #FFFFFF; }");

    auto* mainLayout = new QVBoxLayout(&dlg);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(24, 20, 24, 20);

    // 标题
    auto* titleLabel = new QLabel("程设大富翁 · 新游戏设置", &dlg);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #8B1A1A; padding-bottom: 4px;");
    mainLayout->addWidget(titleLabel);

    // 装饰线
    auto* divider = new QLabel(&dlg);
    divider->setFixedHeight(1);
    divider->setStyleSheet("background-color: #8B1A1A;");
    mainLayout->addWidget(divider);

    // 玩家数量选择
    auto* countRow = new QHBoxLayout();
    auto* countLabel = new QLabel("玩家数量", &dlg);
    countLabel->setStyleSheet("color: #8B1A1A; font-size: 14px; font-weight: bold;");
    countRow->addWidget(countLabel);
    countRow->addStretch();

    QButtonGroup* countGroup = new QButtonGroup(&dlg);
    int playerCount = 2;
    QString countBtnStyle =
        "QPushButton { background-color: #F5EDE5; color: #8B1A1A; border: 1px solid #8B1A1A; "
        "border-radius: 5px; padding: 5px 18px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #F0E0D8; }"
        "QPushButton:checked { background-color: #B22222; color: #FFFFFF; "
        "border: 1px solid #8B1A1A; }";
    for (int n = 2; n <= 4; ++n) {
        auto* btn = new QPushButton(QString::number(n), &dlg);
        btn->setCheckable(true);
        btn->setChecked(n == 2);
        btn->setStyleSheet(countBtnStyle);
        btn->setCursor(Qt::PointingHandCursor);
        countGroup->addButton(btn, n);
        countRow->addWidget(btn);
    }
    mainLayout->addLayout(countRow);

    // 玩家卡片容器
    QScrollArea* scrollArea = new QScrollArea(&dlg);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setMaximumHeight(340);
    scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { width: 6px; background: #F0E8DD; border-radius: 3px; }"
        "QScrollBar::handle:vertical { background: #8B1A1A; border-radius: 3px; }");

    QWidget* cardsWidget = new QWidget(&dlg);
    cardsWidget->setStyleSheet("background: transparent;");
    QVBoxLayout* cardsLayout = new QVBoxLayout(cardsWidget);
    cardsLayout->setSpacing(8);
    cardsLayout->setContentsMargins(0, 0, 0, 0);
    scrollArea->setWidget(cardsWidget);
    mainLayout->addWidget(scrollArea, 1);

    // 底部按钮
    auto* buttonRow = new QHBoxLayout();
    buttonRow->addStretch();

    QPushButton* cancelBtn = new QPushButton("取 消", &dlg);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #E8E0D8; color: #5A4A3A; padding: 9px 28px; "
        "border: 1px solid #C0B0A0; border-radius: 6px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #DDD5CB; }");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    buttonRow->addWidget(cancelBtn);

    QPushButton* startBtn = new QPushButton("开始游戏", &dlg);
    startBtn->setStyleSheet(
        "QPushButton { background-color: #B22222; color: #FFFFFF; padding: 9px 32px; "
        "border: 1px solid #8B1A1A; border-radius: 6px; font-size: 14px; "
        "font-weight: bold; letter-spacing: 2px; }"
        "QPushButton:hover { background-color: #C62828; }"
        "QPushButton:pressed { background-color: #8B1A1A; }");
    startBtn->setCursor(Qt::PointingHandCursor);
    buttonRow->addWidget(startBtn);
    mainLayout->addLayout(buttonRow);

    // 存储玩家配置
    struct PlayerConfigWidgets {
        QLineEdit* nameEdit;
        QPushButton* humanBtn;
        QPushButton* aiBtn;
        QWidget* difficultyRow;
        QComboBox* diffCombo;
    };
    QVector<PlayerConfigWidgets> configWidgets;

    auto rebuildCards = [&]() {
        for (auto& w : configWidgets) { /* children deleted with parent */ }
        configWidgets.clear();
        QLayoutItem* item;
        while ((item = cardsLayout->takeAt(0)) != nullptr) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }

        QString cardStyle =
            "QFrame { background-color: #FFF8F5; border: 1px solid #D6C8B8; "
            "border-radius: 8px; padding: 4px; }";
        QString toggleStyle =
            "QPushButton { background-color: #F5EDE5; color: #8B1A1A; "
            "border: 1px solid #C0B0A0; border-radius: 4px; padding: 3px 14px; font-size: 12px; }"
            "QPushButton:hover { background-color: #F0E0D8; }"
            "QPushButton:checked { background-color: #B22222; color: #FFFFFF; "
            "border: 1px solid #8B1A1A; }";

        for (int i = 0; i < playerCount; ++i) {
            PlayerConfigWidgets pcw;

            QFrame* card = new QFrame(cardsWidget);
            card->setStyleSheet(cardStyle);
            auto* cardLayout = new QVBoxLayout(card);
            cardLayout->setSpacing(6);
            cardLayout->setContentsMargins(12, 8, 12, 8);

            // 标题行
            auto* headerRow = new QHBoxLayout();
            auto* dot = new QLabel(card);
            dot->setFixedSize(10, 10);
            dot->setStyleSheet(QString(
                "background-color: %1; border-radius: 5px; "
                "border: 1px solid rgba(0,0,0,0.15);").arg(colors[i].name()));
            headerRow->addWidget(dot);
            auto* headerLabel = new QLabel("玩家 " + QString::number(i + 1), card);
            headerLabel->setStyleSheet("color: #8B1A1A; font-size: 12px; font-weight: bold;");
            headerRow->addWidget(headerLabel);
            headerRow->addStretch();
            cardLayout->addLayout(headerRow);

            // 名称行
            auto* nameRow = new QHBoxLayout();
            auto* nameLabel = new QLabel("名称", card);
            nameLabel->setFixedWidth(32);
            nameLabel->setStyleSheet("color: #8B1A1A; font-size: 12px;");
            nameRow->addWidget(nameLabel);
            pcw.nameEdit = new QLineEdit(defaultNames[i], card);
            pcw.nameEdit->setStyleSheet(
                "QLineEdit { background: #FFFFFF; color: #3D2820; border: 1px solid #8B1A1A; "
                "border-radius: 4px; padding: 4px 8px; font-size: 13px; }"
                "QLineEdit:focus { border: 2px solid #B22222; }");
            nameRow->addWidget(pcw.nameEdit);
            cardLayout->addLayout(nameRow);

            // 类型行
            auto* typeRow = new QHBoxLayout();
            auto* typeLabel = new QLabel("类型", card);
            typeLabel->setFixedWidth(32);
            typeLabel->setStyleSheet("color: #8B1A1A; font-size: 12px;");
            typeRow->addWidget(typeLabel);

            QButtonGroup* typeGroup = new QButtonGroup(card);
            pcw.humanBtn = new QPushButton("人类", card);
            pcw.humanBtn->setCheckable(true);
            pcw.humanBtn->setChecked(true);
            pcw.humanBtn->setStyleSheet(toggleStyle);
            pcw.humanBtn->setCursor(Qt::PointingHandCursor);
            typeGroup->addButton(pcw.humanBtn);
            typeRow->addWidget(pcw.humanBtn);

            pcw.aiBtn = new QPushButton("AI", card);
            pcw.aiBtn->setCheckable(true);
            pcw.aiBtn->setStyleSheet(toggleStyle);
            pcw.aiBtn->setCursor(Qt::PointingHandCursor);
            typeGroup->addButton(pcw.aiBtn);
            typeRow->addWidget(pcw.aiBtn);
            typeRow->addStretch();
            cardLayout->addLayout(typeRow);

            // 难度行（默认隐藏）
            pcw.difficultyRow = new QWidget(card);
            auto* diffRowLayout = new QHBoxLayout(pcw.difficultyRow);
            diffRowLayout->setContentsMargins(0, 0, 0, 0);
            auto* diffLabel = new QLabel("难度", card);
            diffLabel->setFixedWidth(32);
            diffLabel->setStyleSheet("color: #8B1A1A; font-size: 12px;");
            diffRowLayout->addWidget(diffLabel);
            pcw.diffCombo = new QComboBox(card);
            pcw.diffCombo->addItem("简单 · ¥10,000");
            pcw.diffCombo->addItem("普通 · ¥15,000");
            pcw.diffCombo->addItem("困难 · ¥20,000");
            pcw.diffCombo->setCurrentIndex(1);
            pcw.diffCombo->setStyleSheet(
                "QComboBox { background: #FFFFFF; color: #8B1A1A; border: 1px solid #8B1A1A; "
                "border-radius: 4px; padding: 3px 10px; font-size: 12px; }"
                "QComboBox:hover { border: 2px solid #B22222; }"
                "QComboBox::drop-down { border: none; width: 18px; }"
                "QComboBox QAbstractItemView { background: #FFFFFF; color: #3D2820; "
                "border: 1px solid #8B1A1A; selection-background-color: #F5EDE5; }");
            pcw.diffCombo->setCursor(Qt::PointingHandCursor);
            diffRowLayout->addWidget(pcw.diffCombo);
            diffRowLayout->addStretch();
            pcw.difficultyRow->setVisible(false);
            cardLayout->addWidget(pcw.difficultyRow);

            auto updateDiff = [pcw]() { pcw.difficultyRow->setVisible(pcw.aiBtn->isChecked()); };
            QObject::connect(pcw.humanBtn, &QPushButton::clicked, cardsWidget, updateDiff, Qt::QueuedConnection);
            QObject::connect(pcw.aiBtn, &QPushButton::clicked, cardsWidget, updateDiff, Qt::QueuedConnection);

            cardsLayout->addWidget(card);
            configWidgets.append(pcw);
        }
        cardsLayout->addStretch();
    };

    rebuildCards();

    QObject::connect(countGroup, &QButtonGroup::idClicked, &dlg, [&](int id) {
        playerCount = id;
        rebuildCards();
        dlg.adjustSize();
    });

    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(startBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.adjustSize();
    QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    dlg.move(center.x() - dlg.width() / 2, center.y() - dlg.height() / 2);

    if (dlg.exec() != QDialog::Accepted) {
        delete m_game;
        m_game = nullptr;
        return;
    }

    // 创建玩家
    for (int i = 0; i < playerCount; ++i) {
        auto& w = configWidgets[i];
        QString name = w.nameEdit->text().trimmed();
        if (name.isEmpty()) name = defaultNames[i];
        bool isAI = w.aiBtn->isChecked();
        AIDifficulty aiDiff = AIDifficulty::NORMAL;
        if (isAI) {
            switch (w.diffCombo->currentIndex()) {
            case 0: aiDiff = AIDifficulty::EASY; break;
            case 1: aiDiff = AIDifficulty::NORMAL; break;
            case 2: aiDiff = AIDifficulty::HARD; break;
            }
        }
        m_game->addPlayer(name, colors[i], isAI, aiDiff);
    }

    m_eventLog->clear();
    m_playerPanel->setPlayers(m_game->players());
    m_boardWidget->setPlayers(&m_game->players());
    m_diceWidget->clearDice();
    updatePropertyDisplay();

    connectSignals();
    m_game->setDebugMode(m_debugMode);
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
        "【基本规则】\n"
        "1. 2~4名玩家轮流掷两颗骰子，按点数前进\n"
        "2. 初始资金 ¥20,000，经过起点 +¥5,000（停留 +¥10,000）\n"
        "3. 停在无主地产/设施上可购买，停在他人地产上需支付租金\n"
        "4. 拥有同色全部地产后，踩中自己地产时可建造房屋和旅馆\n"
        "5. 掷出对子（两颗骰子点数相同）获得额外一个回合\n"
        "6. 付不起租金时宣告破产，最后一位未破产的玩家获胜\n\n"
        "【功能格】\n"
        "  • 问答格(?) — 回答C++选择题，答对概率获得效果卡\n"
        "  • 商店(¥) — 使用金币购买效果卡\n"
        "  • 29楼地下室(>>) — 可选择是否传送到商店\n"
        "  • 上机课(PC) — 答题+跳过下回合，答对必得效果卡\n"
        "  • 税收格 — 强制消费（农园¥2,000 / 燕南¥3,000）\n\n"
        "【效果卡】可在商店购买或通过答题获取：\n"
        "  • 再丢一次 — 掷骰后可再掷一次\n"
        "  • 万能骰子 — 自选两个骰子点数(1-6)\n"
        "  • 虚函数卡 — 配合虚函数格使用\n"
        "  • 跳过卡   — 跳过当前格的负面效果\n"
        "  • 迭代器卡 — 配合迭代器格使用\n\n"
        "【特殊地块】\n"
        "棋盘上有虚函数格、静态成员变量格、迭代器格等特殊地块，\n"
        "其价格、租金计算机制均模拟对应的C++概念。\n"
        "点击格子上方的 [i] 按钮阅读伪代码，理解其运作方式。\n"
        "善用效果卡与地块机制的配合，是取胜的关键！";
    QMessageBox::information(this, "游戏规则", rules);
}


// ==================== 掷骰子 ====================
void MainWindow::onRollDice() {
    if (!m_game) return;

    if (m_debugMode && m_game->state() == GameState::PRE_ROLL) {
        if (m_debugDialogOpen) return;

        QDialog* dialog = new QDialog(this, Qt::WindowStaysOnTopHint);
        dialog->setWindowTitle("DEBUG · 手动骰子");
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setMinimumWidth(280);
        dialog->setStyleSheet(
            "QDialog { background-color: #FFF8F5; border: 3px solid #8B1A1A; border-radius: 8px; }"
            "QLabel { color: #8B1A1A; font-size: 14px; font-weight: bold; }");

        auto* dlgLayout = new QVBoxLayout(dialog);
        dlgLayout->setSpacing(14);
        dlgLayout->setContentsMargins(24, 20, 24, 20);

        auto* titleLabel = new QLabel("手动设置骰子点数", dialog);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 16px;");
        dlgLayout->addWidget(titleLabel);

        auto* diceLayout = new QHBoxLayout();
        diceLayout->setSpacing(16);

        auto makeDice = [&](const QString& label) {
            auto* group = new QVBoxLayout();
            auto* lbl = new QLabel(label, dialog);
            lbl->setAlignment(Qt::AlignCenter);
            lbl->setStyleSheet("font-size: 12px; font-weight: normal; color: #8B7355;");
            group->addWidget(lbl);
            auto* spin = new QSpinBox(dialog);
            spin->setRange(1, 6);
            spin->setValue(1);
            spin->setAlignment(Qt::AlignCenter);
            spin->setFixedSize(64, 48);
            spin->setStyleSheet(
                "QSpinBox { font-size: 24px; font-weight: bold; color: #8B1A1A; "
                "background-color: #FFFFFF; border: 2px solid #8B1A1A; border-radius: 8px; padding: 4px; }"
                "QSpinBox:hover { border-color: #B22222; }"
                "QSpinBox::up-button, QSpinBox::down-button { width: 20px; }");
            group->addWidget(spin);
            return std::make_pair(group, spin);
        };

        auto [d1Layout, spin1] = makeDice("骰子 1");
        auto [d2Layout, spin2] = makeDice("骰子 2");
        diceLayout->addLayout(d1Layout);
        diceLayout->addLayout(d2Layout);
        dlgLayout->addLayout(diceLayout);

        auto* btnRow = new QHBoxLayout();
        btnRow->addStretch();
        QPushButton* okBtn = new QPushButton("掷骰子", dialog);
        okBtn->setStyleSheet(
            "QPushButton { background-color: #B22222; color: #FFFFFF; padding: 9px 28px; "
            "border: 1px solid #8B1A1A; border-radius: 6px; font-size: 14px; font-weight: bold; }"
            "QPushButton:hover { background-color: #C62828; }");
        okBtn->setCursor(Qt::PointingHandCursor);
        btnRow->addWidget(okBtn);

        QPushButton* cancelBtn = new QPushButton("取消", dialog);
        cancelBtn->setStyleSheet(
            "QPushButton { background-color: #E8E0D8; color: #5A4A3A; padding: 9px 28px; "
            "border: 1px solid #C0B0A0; border-radius: 6px; font-size: 14px; font-weight: bold; }"
            "QPushButton:hover { background-color: #DDD5CB; }");
        cancelBtn->setCursor(Qt::PointingHandCursor);
        btnRow->addWidget(cancelBtn);
        btnRow->addStretch();
        dlgLayout->addLayout(btnRow);

        connect(okBtn, &QPushButton::clicked, dialog, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);

        connect(dialog, &QDialog::finished, this,
                [this, spin1, spin2](int result) {
                    m_debugDialogOpen = false;
                    if (result == QDialog::Accepted && m_game) {
                        m_game->debugRollDice(spin1->value(), spin2->value());
                    } else if (m_game && m_game->state() == GameState::PRE_ROLL) {
                        m_diceWidget->setRollEnabled(true);
                    }
                });

        m_debugDialogOpen = true;
        m_diceWidget->setRollEnabled(false);
        dialog->show();
        dialog->adjustSize();
        QPoint boardCenter = m_boardWidget->mapToGlobal(
            QPoint(m_boardWidget->width() / 2, m_boardWidget->height() / 2));
        dialog->move(boardCenter.x() - dialog->width() / 2,
                     boardCenter.y() - dialog->height() / 2);
        return;
    }

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
        nameLabel->setStyleSheet("font-weight: bold; font-size: 12px;color: #000000");
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
            if (auto* it = dynamic_cast<IteratorTile*>(t)) {
                if (it->owner() == player) {
                    hasItems = true;
                    auto* chip = new QLabel("迭代器:" + it->name(), playerGroup);
                    chip->setStyleSheet(
                        "background-color: #00897B; color: white; border-radius: 3px; "
                        "padding: 1px 6px; font-size: 10px;");
                    chipsRow->addWidget(chip);
                }
            }
        }

        for (const auto& card : player->effectCards()) {
            hasItems = true;
            QColor cardColor = QColor("#E65100");  // default orange
            if (card.type == EffectCardType::ITERATOR_CARD) {
                cardColor = QColor("#00695C");  // teal for iterator cards
            }
            auto* chip = new QLabel(card.name, playerGroup);
            chip->setStyleSheet(
                QString("background-color: %1; color: white; border-radius: 3px; "
                        "padding: 1px 6px; font-size: 10px;")
                .arg(cardColor.name()));
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
    QString label = player->isAI()
        ? "当前回合：" + player->name() + " (AI)"
        : "当前回合：" + player->name();
    m_statusLabel->setText(label);
    m_statusLabel->setStyleSheet(
        "QLabel { background-color: #FFF0F0; border: 2px solid #B22222; "
        "border-radius: 6px; padding: 10px; font-size: 14px; font-weight: bold; color: #8B1A1A;}");
    m_playerPanel->highlightCurrentPlayer(player);
    m_diceWidget->setRollEnabled(true);

    // AI 自动掷骰
    if (player->isAI()) {
        m_diceWidget->setRollEnabled(false);
        QTimer::singleShot(800, this, [this]() {
            if (m_game && m_game->currentPlayer()
                && m_game->currentPlayer()->isAI()
                && m_game->state() == GameState::PRE_ROLL) {
                if (m_debugMode) {
                    // DEBUG 模式下 AI 也随机选点
                    int d1 = QRandomGenerator::global()->bounded(1, 7);
                    int d2 = QRandomGenerator::global()->bounded(1, 7);
                    m_game->debugRollDice(d1, d2);
                } else {
                    m_game->rollDice();
                }
            }
        });
    }
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


// ==================== 非模态弹窗辅助 ====================
QDialog* MainWindow::createBoardDialog(const QString& title) {
    m_diceWidget->setRollEnabled(false);
    QDialog* dlg = new QDialog(this);
    dlg->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
    dlg->setWindowTitle(title);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet("QDialog { border: 3px solid #8B1A1A; border-radius: 8px; background: #FFFFFF; }"
        "QLabel { color: #3D2820; font-size: 13px; }"
        "QRadioButton { color: #3D2820; font-size: 13px; }"
        "QComboBox { border: 1px solid #8B1A1A; border-radius: 4px; }");
    return dlg;
}

void MainWindow::positionOnBoard(QDialog* dlg) {
    dlg->adjustSize();
    QPoint boardCenter = m_boardWidget->mapToGlobal(
        QPoint(m_boardWidget->width() / 2, m_boardWidget->height() / 2));
    dlg->move(boardCenter.x() - dlg->width() / 2,
              boardCenter.y() - dlg->height() / 2);
}

// ==================== 购买/升级提示 ====================
void MainWindow::onPromptBuyProperty(int tileIndex, Player* player) {
    if (!m_game || !player) return;

    Tile* t = m_game->board().tileAt(tileIndex);
    if (!t) return;

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
    } else if (auto* it = dynamic_cast<IteratorTile*>(t)) {
        price = it->price();
        msg = player->name() + "，是否购买 " + t->name()
              + "（迭代器格）？\n价格：¥" + QString::number(price)
              + "\n当前资金：¥" + QString::number(player->money());
    }

    if (player->money() < price) {
        m_game->logEvent(player->name() + " 资金不足，无法购买 " + t->name());
        m_game->skipAction();
        return;
    }

    QDialog* dlg = createBoardDialog("购买地产");
    auto* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(msg, dlg));

    auto* btnLayout = new QHBoxLayout();
    auto* yesBtn = new QPushButton("购买 (¥" + QString::number(price) + ")", dlg);
    auto* noBtn  = new QPushButton("放弃", dlg);
    yesBtn->setStyleSheet("QPushButton {background-color:#B22222;color:#FFFFFF;padding:8px 16px;border:1px solid #8B1A1A;border-radius:5px;}");
    noBtn->setStyleSheet("QPushButton {background-color:#E8E0D8;color:#5A4A3A;padding:8px 16px;border:1px solid #C0B0A0;border-radius:5px;}");
    btnLayout->addStretch();
    btnLayout->addWidget(yesBtn);
    btnLayout->addWidget(noBtn);
    layout->addLayout(btnLayout);

    connect(yesBtn, &QPushButton::clicked, this, [this, dlg, player, tileIndex]() {
        dlg->accept();
        m_game->buyProperty(player, tileIndex);
    });
    connect(noBtn, &QPushButton::clicked, this, [this, dlg, player, tileIndex]() {
        dlg->reject();
        m_game->logEvent(player->name() + " 放弃购买 " +
                         m_game->board().tileAt(tileIndex)->name());
        m_game->skipAction();
    });

    dlg->show();
    positionOnBoard(dlg);
}

void MainWindow::onPromptBuildHouse(int tileIndex, Player* player) {
    if (!m_game || !player) return;

    auto* pt = dynamic_cast<PropertyTile*>(m_game->board().tileAt(tileIndex));
    if (!pt) return;

    if (!pt->canBuildHouse(player)) {
        m_game->skipAction();
        return;
    }

    if (auto* vt = dynamic_cast<VirtualfuncTile*>(pt)) {
        if (player->hasEffectCard(EffectCardType::VIRTUAL_FUNCTION)) {
            int baseCost = vt->houseCost();
            int derivedCost = vt->houseCost();
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

    QDialog* dlg = createBoardDialog("升级地产");
    auto* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(info, dlg));

    auto* btnLayout = new QHBoxLayout();
    auto* yesBtn = new QPushButton("升级", dlg);
    auto* noBtn  = new QPushButton("放弃", dlg);
    yesBtn->setStyleSheet("QPushButton {background-color:#B22222;color:#FFFFFF;padding:8px 16px;border:1px solid #8B1A1A;border-radius:5px;}");
    noBtn->setStyleSheet("QPushButton {background-color:#E8E0D8;color:#5A4A3A;padding:8px 16px;border:1px solid #C0B0A0;border-radius:5px;}");
    btnLayout->addStretch();
    btnLayout->addWidget(yesBtn);
    btnLayout->addWidget(noBtn);
    layout->addLayout(btnLayout);

    connect(yesBtn, &QPushButton::clicked, this, [this, dlg, player, tileIndex]() {
        dlg->accept();
        m_game->buildHouse(player, tileIndex);
    });
    connect(noBtn, &QPushButton::clicked, this, [this, dlg]() {
        dlg->reject();
        m_game->skipAction();
    });

    dlg->show();
    positionOnBoard(dlg);
}


// ==================== 问答格 ====================
void MainWindow::onPromptQA(Player* player, int tileIndex) {
    if (!m_game || !player) return;
    Q_UNUSED(tileIndex)

    Question q = QuestionBank::drawRandom();
    m_game->setCurrentQuestion(q);
    QDialog* dlg = createBoardDialog(player->name() + " 来到了问答格");
    dlg->setMinimumWidth(350);
    auto* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(q.text, dlg));
    QString options[4] = {"A " + q.optionA, "B " + q.optionB,
                          "C " + q.optionC, "D " + q.optionD};
    QButtonGroup* grp = new QButtonGroup(dlg);
    for (int i = 0; i < 4; i++) {
        auto* radio = new QRadioButton(options[i], dlg);
        grp->addButton(radio, i);
        layout->addWidget(radio);
    }
    auto* buttonLayout = new QHBoxLayout();
    auto* answerButton = new QPushButton("确定", dlg);
    auto* cancelButton = new QPushButton("离开", dlg);
    answerButton->setStyleSheet("QPushButton {background-color:#8F1A10;color:white;padding:8px 16px;border:none;border-radius:4px}");
    cancelButton->setStyleSheet("QPushButton {background-color:#cccccc;color:white;padding:8px 16px;border:none;border-radius:4px}");
    buttonLayout->addStretch();
    buttonLayout->addWidget(answerButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(answerButton, &QPushButton::clicked, this, [this, dlg, player, grp]() {
        m_game->answerQA(player, player->position(), grp->checkedId());
        dlg->accept();
    });
    connect(cancelButton, &QPushButton::clicked, this, [this, dlg]() {
        dlg->reject();
        m_game->skipAction();
    });

    dlg->show();
    positionOnBoard(dlg);
}


// ==================== 上机课 ====================
void MainWindow::onPromptComputerLab(Player* player) {
    if (!m_game || !player) return;

    Question q = QuestionBank::drawRandom();
    m_game->setCurrentQuestion(q);
    QDialog* dlg = createBoardDialog(player->name() + " 前来上机");
    dlg->setMinimumWidth(350);
    auto* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(q.text, dlg));
    QString options[4] = {"A " + q.optionA, "B " + q.optionB,
                          "C " + q.optionC, "D " + q.optionD};
    QButtonGroup* grp = new QButtonGroup(dlg);
    for (int i = 0; i < 4; i++) {
        auto* radio = new QRadioButton(options[i], dlg);
        grp->addButton(radio, i);
        layout->addWidget(radio);
    }
    auto* buttonLayout = new QHBoxLayout();
    auto* answerButton = new QPushButton("确定", dlg);
    auto* cancelButton = new QPushButton("离开", dlg);
    answerButton->setStyleSheet("QPushButton {background-color:#8F1A10;color:white;padding:8px 16px;border:none;border-radius:4px}");
    cancelButton->setStyleSheet("QPushButton {background-color:#cccccc;color:white;padding:8px 16px;border:none;border-radius:4px}");
    buttonLayout->addStretch();
    buttonLayout->addWidget(answerButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(answerButton, &QPushButton::clicked, this, [this, dlg, player, grp]() {
        m_game->answerComputerLab(player, grp->checkedId());
        dlg->accept();
    });
    connect(cancelButton, &QPushButton::clicked, this, [this, dlg]() {
        dlg->reject();
        m_game->skipAction();
    });

    dlg->show();
    positionOnBoard(dlg);
}


// ==================== 商店 ====================
void MainWindow::onPromptShop(Player* player) {
    if (!m_game || !player) return;

    QStringList items;
    QVector<EffectCardType> types;
    QVector<EffectCard> displayCards;  // store display cards for reuse

    EffectCardType allTypes[] = {
        EffectCardType::ROLL_AGAIN,
        EffectCardType::UNIVERSAL_DICE,
        EffectCardType::VIRTUAL_FUNCTION,
        EffectCardType::SKIP_EFFECT,
        EffectCardType::ITERATOR_CARD
    };

    for (auto type : allTypes) {
        EffectCard card = createEffectCard(type);
        displayCards.append(card);
        types.append(type);
        if (player->canAfford(card.price)) {
            items << card.name + " (¥" + QString::number(card.price) + ")";
        } else {
            items << card.name + " (¥" + QString::number(card.price) + ") - 资金不足";
        }
    }

    QDialog* dlg = createBoardDialog(player->name() + " 来到了麦叔的铺子");
    dlg->setMinimumWidth(350);
    QVBoxLayout* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel("欢迎光临！看看要买点什么呢？", dlg));
    layout->addWidget(new QLabel("当前资金：¥" + QString::number(player->money()), dlg));
    QButtonGroup* grp = new QButtonGroup(dlg);
    for (int i = 0; i < displayCards.size(); i++) {
        const EffectCard& card = displayCards[i];
        QRadioButton* radio = new QRadioButton(dlg);
        QString text = card.name + " (¥" + QString::number(card.price) + ")";
        if (!player->canAfford(card.price)) {
            text += " - 资金不足";
            radio->setEnabled(false);
            radio->setStyleSheet("color:#cccccc");
        }
        radio->setText(text);
        grp->addButton(radio, i);
        layout->addWidget(radio);
    }
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* buyButton = new QPushButton("购买", dlg);
    QPushButton* cancelButton = new QPushButton("离开", dlg);
    buyButton->setStyleSheet("QPushButton {background-color:#B22222;color:#FFFFFF;padding:8px 16px;border:1px solid #8B1A1A;border-radius:5px;}");
    cancelButton->setStyleSheet("QPushButton {background-color:#E8E0D8;color:#5A4A3A;padding:8px 16px;border:1px solid #C0B0A0;border-radius:5px;}");
    buttonLayout->addStretch();
    buttonLayout->addWidget(buyButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(buyButton, &QPushButton::clicked, this, [this, dlg, player, grp, displayCards]() {
        int idx = grp->checkedId();
        if (idx >= 0 && idx < displayCards.size()) {
            const EffectCard& selectedCard = displayCards[idx];
            if (player->canAfford(selectedCard.price)) {
                m_game->buyEffectCard(player, selectedCard);
                dlg->accept();
            } else {
                m_game->logEvent(player->name() + " 资金不足，无法购买！");
            }
        }
    });
    connect(cancelButton, &QPushButton::clicked, this, [this, dlg]() {
        dlg->reject();
        m_game->skipAction();
    });

    dlg->show();
    positionOnBoard(dlg);
}


// ==================== 29楼地下室 ====================
void MainWindow::onPromptShopEntrance(Player* player) {
    if (!m_game || !player) return;

    QDialog* dlg = createBoardDialog("29楼地下室");
    auto* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(
        player->name() + "，前方是商店！是否进入购买效果卡？", dlg));

    auto* btnLayout = new QHBoxLayout();
    auto* yesBtn = new QPushButton("进入", dlg);
    auto* noBtn  = new QPushButton("跳过", dlg);
    yesBtn->setStyleSheet("QPushButton {background-color:#B22222;color:#FFFFFF;padding:8px 16px;border:1px solid #8B1A1A;border-radius:5px;}");
    noBtn->setStyleSheet("QPushButton {background-color:#E8E0D8;color:#5A4A3A;padding:8px 16px;border:1px solid #C0B0A0;border-radius:5px;}");
    btnLayout->addStretch();
    btnLayout->addWidget(yesBtn);
    btnLayout->addWidget(noBtn);
    layout->addLayout(btnLayout);

    connect(yesBtn, &QPushButton::clicked, this, [this, dlg, player]() {
        dlg->accept();
        m_game->goToShop(player);
    });
    connect(noBtn, &QPushButton::clicked, this, [this, dlg]() {
        dlg->reject();
        m_game->declineShopEntrance();
    });

    dlg->show();
    positionOnBoard(dlg);
}


// ==================== 使用效果卡 ====================
void MainWindow::onPromptUseCard(Player* player, EffectCardType type) {
    if (!m_game || !player) return;

    EffectCard card = createEffectCard(type);

    QDialog* dlg = createBoardDialog("使用效果卡");
    auto* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(
        player->name() + "，是否使用 " + card.name + "？\n" + card.description, dlg));

    auto* btnLayout = new QHBoxLayout();
    auto* yesBtn = new QPushButton("使用", dlg);
    auto* noBtn  = new QPushButton("不用", dlg);
    yesBtn->setStyleSheet("QPushButton {background-color:#B22222;color:#FFFFFF;padding:8px 16px;border:1px solid #8B1A1A;border-radius:5px;}");
    noBtn->setStyleSheet("QPushButton {background-color:#E8E0D8;color:#5A4A3A;padding:8px 16px;border:1px solid #C0B0A0;border-radius:5px;}");
    btnLayout->addStretch();
    btnLayout->addWidget(yesBtn);
    btnLayout->addWidget(noBtn);
    layout->addLayout(btnLayout);

    connect(yesBtn, &QPushButton::clicked, this, [this, dlg, type]() {
        dlg->accept();
        m_game->onCardDecision(type, true);
    });
    connect(noBtn, &QPushButton::clicked, this, [this, dlg, type]() {
        dlg->reject();
        m_game->onCardDecision(type, false);
    });

    dlg->show();
    positionOnBoard(dlg);
}


// ==================== 万能骰子 ====================
void MainWindow::onPromptUniversalDice(Player* player) {
    if (!m_game || !player) return;

    QDialog* dlg = createBoardDialog("万能骰子 - " + player->name());

    QFormLayout* form = new QFormLayout(dlg);

    QSpinBox* spin1 = new QSpinBox(dlg);
    spin1->setRange(1, 6);
    QSpinBox* spin2 = new QSpinBox(dlg);
    spin2->setRange(1, 6);

    form->addRow("骰子1:", spin1);
    form->addRow("骰子2:", spin2);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this, dlg, spin1, spin2]() {
        dlg->accept();
        m_game->setUniversalDice(spin1->value(), spin2->value());
    });
    connect(buttons, &QDialogButtonBox::rejected, this, [this, dlg]() {
        dlg->reject();
        m_game->onCardDecision(EffectCardType::UNIVERSAL_DICE, false);
    });

    dlg->show();
    positionOnBoard(dlg);
}


// ==================== 虚函数卡交互 ====================
void MainWindow::onPromptVirtualFuncBuy(Player* player, int tileIndex,
                                         int basePrice, int derivedPrice) {
    Q_UNUSED(basePrice)
    Q_UNUSED(derivedPrice)
    if (!m_game || !player) return;

    Tile* t = m_game->board().tileAt(tileIndex);
    if (!t) return;

    QString msg = player->name() + "，是否购买 " + t->name() + "？\n"
                  "（可使用虚函数卡选择购买方式）\n\n"
                  "当前资金：¥" + QString::number(player->money());

    QDialog* dlg = createBoardDialog("虚函数卡 — 购买地产");
    auto* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(msg, dlg));

    auto* btnLayout = new QHBoxLayout();
    QPushButton* useCardBtn = new QPushButton("使用虚函数卡", dlg);
    useCardBtn->setStyleSheet("QPushButton {background-color:#8B1A1A;color:#FFFFFF;padding:8px 12px;border:1px solid #6B1A1A;border-radius:5px;}");
    btnLayout->addWidget(useCardBtn);
    QPushButton* noCardBtn = new QPushButton("不用卡", dlg);
    noCardBtn->setStyleSheet("QPushButton {background-color:#8F1A10;color:white;padding:8px 12px;border:none;border-radius:4px;}");
    btnLayout->addWidget(noCardBtn);
    QPushButton* cancelBtn = new QPushButton("放弃", dlg);
    cancelBtn->setStyleSheet("QPushButton {background-color:#cccccc;color:white;padding:8px 12px;border:none;border-radius:4px;}");
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(useCardBtn, &QPushButton::clicked, this, [this, dlg, player, tileIndex]() {
        dlg->accept();
        m_game->buyPropertyVirtualFunc(player, tileIndex, true);
    });
    connect(noCardBtn, &QPushButton::clicked, this, [this, dlg, player, tileIndex]() {
        dlg->accept();
        m_game->buyPropertyVirtualFunc(player, tileIndex, false);
    });
    connect(cancelBtn, &QPushButton::clicked, this, [this, dlg, player, t]() {
        dlg->reject();
        m_game->logEvent(player->name() + " 放弃购买 " + t->name());
        m_game->skipAction();
    });

    dlg->show();
    positionOnBoard(dlg);
}

void MainWindow::onPromptVirtualFuncRent(Player* payer, int tileIndex,
                                          Player* owner, int baseRent, int derivedRent) {
    Q_UNUSED(baseRent)
    Q_UNUSED(derivedRent)
    if (!m_game || !payer || !owner) return;

    Tile* t = m_game->board().tileAt(tileIndex);
    if (!t) return;

    auto* vt = dynamic_cast<VirtualfuncTile*>(t);
    if (!vt) return;

    bool isPure = vt->rentIsPureVirtual();
    bool isNonVirtual = vt->rentIsNonVirtual();

    QDialog* dlg = createBoardDialog("虚函数卡 — 收租选择");
    auto* layout = new QVBoxLayout(dlg);
    auto* btnLayout = new QHBoxLayout();

    if (isPure) {
        // Pure: 纯虚函数，基类租金不可用，仅派生类
        QString msg = owner->name() + "，玩家 " + payer->name()
                      + " 踩中了你的虚函数格 " + t->name() + "！\n\n"
                      "⚠️ 该格租金为纯虚函数，必须使用虚函数卡才能收租。\n\n"
                      "当前资金：¥" + QString::number(owner->money());

        QLabel* label = new QLabel(msg, dlg);
        label->setWordWrap(true);
        layout->addWidget(label);

        QPushButton* useCardBtn = new QPushButton("使用虚函数卡", dlg);
        useCardBtn->setStyleSheet(
            "QPushButton {background-color:#8B1A1A;color:#FFFFFF;padding:8px 12px;border:1px solid #6B1A1A;border-radius:5px;}");
        btnLayout->addWidget(useCardBtn);

        QPushButton* cancelBtn = new QPushButton("放弃收租", dlg);
        cancelBtn->setStyleSheet(
            "QPushButton {background-color:#cccccc;color:white;padding:8px 12px;border:none;border-radius:4px;}");
        btnLayout->addWidget(cancelBtn);

        connect(useCardBtn, &QPushButton::clicked, this, [this, dlg, payer, tileIndex]() {
            dlg->accept();
            m_game->payRentVirtualFunc(payer, tileIndex, true);
        });
        connect(cancelBtn, &QPushButton::clicked, this, [this, dlg, payer, tileIndex]() {
            dlg->reject();
            m_game->handlePureVirtualNoRent(payer, tileIndex);
        });
    } else if (isNonVirtual) {
        // Mix1: 租金为非虚函数，虚函数卡无效（教学陷阱）
        QString msg = owner->name() + "，玩家 " + payer->name()
                      + " 踩中了你的虚函数格 " + t->name() + "！\n\n"
                      "（可使用虚函数卡选择收租方式）\n\n"
                      "当前资金：¥" + QString::number(owner->money());

        QLabel* label = new QLabel(msg, dlg);
        label->setWordWrap(true);
        layout->addWidget(label);

        QPushButton* useCardBtn = new QPushButton("使用虚函数卡", dlg);
        useCardBtn->setStyleSheet(
            "QPushButton {background-color:#8B1A1A;color:#FFFFFF;padding:8px 12px;border:1px solid #6B1A1A;border-radius:5px;}");
        btnLayout->addWidget(useCardBtn);

        QPushButton* noCardBtn = new QPushButton("不用卡", dlg);
        noCardBtn->setStyleSheet(
            "QPushButton {background-color:#8F1A10;color:white;padding:8px 12px;border:none;border-radius:4px;}");
        btnLayout->addWidget(noCardBtn);

        QPushButton* cancelBtn = new QPushButton("放弃", dlg);
        cancelBtn->setStyleSheet(
            "QPushButton {background-color:#cccccc;color:white;padding:8px 12px;border:none;border-radius:4px;}");
        btnLayout->addWidget(cancelBtn);

        connect(useCardBtn, &QPushButton::clicked, this, [this, dlg, payer, tileIndex]() {
            dlg->accept();
            m_game->payRentVirtualFunc(payer, tileIndex, true);
        });
        connect(noCardBtn, &QPushButton::clicked, this, [this, dlg, payer, tileIndex]() {
            dlg->accept();
            m_game->payRentVirtualFunc(payer, tileIndex, false);
        });
        connect(cancelBtn, &QPushButton::clicked, this, [this, dlg]() {
            dlg->reject();
            m_game->skipAction();
        });
    } else {
        // 正常虚函数格 (Buy/Rent/Mix2)
        QString msg = owner->name() + "，玩家 " + payer->name()
                      + " 踩中了你的虚函数格 " + t->name() + "！\n\n"
                      "（可使用虚函数卡选择收租方式）\n\n"
                      "当前资金：¥" + QString::number(owner->money());

        QLabel* label = new QLabel(msg, dlg);
        label->setWordWrap(true);
        layout->addWidget(label);

        QPushButton* useCardBtn = new QPushButton("使用虚函数卡", dlg);
        useCardBtn->setStyleSheet(
            "QPushButton {background-color:#8B1A1A;color:#FFFFFF;padding:8px 12px;border:1px solid #6B1A1A;border-radius:5px;}");
        btnLayout->addWidget(useCardBtn);

        QPushButton* noCardBtn = new QPushButton("不用卡", dlg);
        noCardBtn->setStyleSheet(
            "QPushButton {background-color:#8F1A10;color:white;padding:8px 12px;border:none;border-radius:4px;}");
        btnLayout->addWidget(noCardBtn);

        QPushButton* cancelBtn = new QPushButton("放弃", dlg);
        cancelBtn->setStyleSheet(
            "QPushButton {background-color:#cccccc;color:white;padding:8px 12px;border:none;border-radius:4px;}");
        btnLayout->addWidget(cancelBtn);

        connect(useCardBtn, &QPushButton::clicked, this, [this, dlg, payer, tileIndex]() {
            dlg->accept();
            m_game->payRentVirtualFunc(payer, tileIndex, true);
        });
        connect(noCardBtn, &QPushButton::clicked, this, [this, dlg, payer, tileIndex]() {
            dlg->accept();
            m_game->payRentVirtualFunc(payer, tileIndex, false);
        });
        connect(cancelBtn, &QPushButton::clicked, this, [this, dlg]() {
            dlg->reject();
            m_game->skipAction();
        });
    }

    layout->addLayout(btnLayout);
    dlg->show();
    positionOnBoard(dlg);
}

void MainWindow::onPromptVirtualFuncBuild(Player* player, int tileIndex,
                                           int baseCost, int derivedCost) {
    Q_UNUSED(baseCost)
    Q_UNUSED(derivedCost)
    if (!m_game || !player) return;

    Tile* t = m_game->board().tileAt(tileIndex);
    if (!t) return;

    auto* vt = dynamic_cast<VirtualfuncTile*>(t);
    if (!vt) return;

    QString info;
    if (vt->houses() < 4) {
        info = "升级 " + vt->name() + "？\n当前：" + QString::number(vt->houses()) + " 栋房子\n";
    } else {
        info = "在 " + vt->name() + " 建造旅馆？\n";
    }
    info += "（可使用虚函数卡选择建房方式）\n\n"
            "当前资金：¥" + QString::number(player->money());

    QDialog* dlg = createBoardDialog("虚函数卡 — 建房");
    auto* layout = new QVBoxLayout(dlg);
    layout->addWidget(new QLabel(info, dlg));

    auto* btnLayout = new QHBoxLayout();
    QPushButton* useCardBtn = new QPushButton("使用虚函数卡", dlg);
    useCardBtn->setStyleSheet("QPushButton {background-color:#8B1A1A;color:#FFFFFF;padding:8px 12px;border:1px solid #6B1A1A;border-radius:5px;}");
    btnLayout->addWidget(useCardBtn);
    QPushButton* noCardBtn = new QPushButton("不用卡", dlg);
    noCardBtn->setStyleSheet("QPushButton {background-color:#8F1A10;color:white;padding:8px 12px;border:none;border-radius:4px;}");
    btnLayout->addWidget(noCardBtn);
    QPushButton* cancelBtn = new QPushButton("放弃", dlg);
    cancelBtn->setStyleSheet("QPushButton {background-color:#cccccc;color:white;padding:8px 12px;border:none;border-radius:4px;}");
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(useCardBtn, &QPushButton::clicked, this, [this, dlg, player, tileIndex]() {
        dlg->accept();
        m_game->buildHouseVirtualFunc(player, tileIndex, true);
    });
    connect(noCardBtn, &QPushButton::clicked, this, [this, dlg, player, tileIndex]() {
        dlg->accept();
        m_game->buildHouseVirtualFunc(player, tileIndex, false);
    });
    connect(cancelBtn, &QPushButton::clicked, this, [this, dlg]() {
        dlg->reject();
        m_game->skipAction();
    });

    dlg->show();
    positionOnBoard(dlg);
}


// ==================== 迭代器卡 ====================
void MainWindow::onPromptIteratorCard(Player* player, int tileIndex) {
    if (!m_game || !player) return;

    // 收集玩家拥有的迭代器卡
    QVector<int> cardIndices;
    for (int i = 0; i < player->effectCards().size(); ++i) {
        if (player->effectCards()[i].type == EffectCardType::ITERATOR_CARD) {
            cardIndices.append(i);
        }
    }

    if (cardIndices.isEmpty()) {
        // 没有迭代器卡，回退到正常流程
        m_game->declineIteratorCard();
        return;
    }

    // 构建卡片选择列表（仅显示卡名，不暴露支持的操作）
    QStringList cardNames;
    for (int idx : cardIndices) {
        cardNames << player->effectCards()[idx].name;
    }

    // Step 1: choose card
    QDialog* cardDlg = createBoardDialog("迭代器卡 — " + player->name());
    auto* cLayout = new QVBoxLayout(cardDlg);
    cLayout->addWidget(new QLabel(
        player->name() + "，选择要使用的迭代器卡：", cardDlg));
    QComboBox* cardCombo = new QComboBox(cardDlg);
    cardCombo->addItems(cardNames);
    cLayout->addWidget(cardCombo);
    auto* cBtnLayout = new QHBoxLayout();
    QPushButton* nextBtn = new QPushButton("下一步", cardDlg);
    QPushButton* cCancelBtn = new QPushButton("取消", cardDlg);
    nextBtn->setStyleSheet("QPushButton {background-color:#B22222;color:#FFFFFF;padding:8px 16px;border:1px solid #8B1A1A;border-radius:5px;}");
    cCancelBtn->setStyleSheet("QPushButton {background-color:#E8E0D8;color:#5A4A3A;padding:8px 16px;border:1px solid #C0B0A0;border-radius:5px;}");
    cBtnLayout->addStretch();
    cBtnLayout->addWidget(nextBtn);
    cBtnLayout->addWidget(cCancelBtn);
    cLayout->addLayout(cBtnLayout);

    connect(cCancelBtn, &QPushButton::clicked, this, [this, cardDlg]() {
        cardDlg->reject();
        m_game->declineIteratorCard();
    });

    connect(nextBtn, &QPushButton::clicked, this, [this, cardDlg, player, tileIndex,
              cardIndices, cardCombo]() {
        int idx = cardCombo->currentIndex();
        if (idx < 0 || idx >= cardIndices.size()) {
            cardDlg->reject();
            m_game->declineIteratorCard();
            return;
        }
        int cardIdx = cardIndices[idx];
        IteratorSubtype sub = player->effectCards()[cardIdx].iterSubtype;
        QString subName = iteratorSubtypeName(sub);

        cardDlg->accept();

        // Step 2: choose operation（仅显示操作符，不解释含义）
        QDialog* opDlg = createBoardDialog("迭代器操作 — " + player->name());
        auto* oLayout = new QVBoxLayout(opDlg);
        oLayout->addWidget(new QLabel(
            player->name() + " 使用 " + subName + "\n请选择操作：", opDlg));
        QStringList ops = {"++", "--", "+=2", "-=2"};
        QComboBox* opCombo = new QComboBox(opDlg);
        opCombo->addItems(ops);
        oLayout->addWidget(opCombo);
        auto* oBtnLayout = new QHBoxLayout();
        QPushButton* okBtn = new QPushButton("确定", opDlg);
        QPushButton* oCancelBtn = new QPushButton("取消", opDlg);
        okBtn->setStyleSheet("QPushButton {background-color:#B22222;color:#FFFFFF;padding:8px 16px;border:1px solid #8B1A1A;border-radius:5px;}");
        oCancelBtn->setStyleSheet("QPushButton {background-color:#E8E0D8;color:#5A4A3A;padding:8px 16px;border:1px solid #C0B0A0;border-radius:5px;}");
        oBtnLayout->addStretch();
        oBtnLayout->addWidget(okBtn);
        oBtnLayout->addWidget(oCancelBtn);
        oLayout->addLayout(oBtnLayout);

        connect(oCancelBtn, &QPushButton::clicked, this, [this, opDlg]() {
            opDlg->reject();
            m_game->declineIteratorCard();
        });
        connect(okBtn, &QPushButton::clicked, this, [this, opDlg, player, tileIndex,
                sub, opCombo]() {
            opDlg->accept();
            IteratorOp op;
            switch (opCombo->currentIndex()) {
            case 0: op = IteratorOp::INCREMENT; break;
            case 1: op = IteratorOp::DECREMENT; break;
            case 2: op = IteratorOp::PLUS_EQ_2; break;
            case 3: op = IteratorOp::MINUS_EQ_2; break;
            default: op = IteratorOp::INCREMENT; break;
            }
            m_game->useIteratorCard(player, tileIndex, sub, op);
        });

        opDlg->show();
        positionOnBoard(opDlg);
    });

    cardDlg->show();
    positionOnBoard(cardDlg);
}


// ==================== 游戏结束 ====================
void MainWindow::onGameOver(Player* winner) {
    m_diceWidget->setRollEnabled(false);
    if (winner) {
        m_statusLabel->setText("游戏结束 · 胜者：" + winner->name());
        m_statusLabel->setStyleSheet(
            "QLabel { background-color: #FFF0F0; border: 2px solid #B22222; "
            "border-radius: 6px; padding: 10px; font-size: 16px; font-weight: bold; color: #8B1A1A; }");
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

// 简易 Markdown → HTML 转换
static QString md2html(const QString& md) {
    QString html = md;
    // 还原 C++ 字符串中的转义换行
    html.replace("\\n", "\n");
    // 代码块处理 —— 手动提取
    QRegularExpression cbRe("```[^`]*```");
    QStringList codeBlocks;
    int pos = 0;
    while (true) {
        auto match = cbRe.match(html, pos);
        if (!match.hasMatch()) break;
        codeBlocks.append(match.captured());
        pos = match.capturedEnd();
    }
    for (int i = 0; i < codeBlocks.size(); ++i) {
        html.replace(codeBlocks[i], QString("\v%1\v").arg(i));
    }
    // 行内代码 `...`
    html.replace(QRegularExpression("`([^`]+)`"), "<code style='background:#F0E8DD;'>\\1</code>");
    // 粗体 **...**
    html.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<b>\\1</b>");
    // 标题
    html.replace(QRegularExpression("^### (.+)$", QRegularExpression::MultilineOption),
                 "<h3>\\1</h3>");
    html.replace(QRegularExpression("^## (.+)$", QRegularExpression::MultilineOption),
                 "<h2>\\1</h2>");
    html.replace(QRegularExpression("^# (.+)$", QRegularExpression::MultilineOption),
                 "<h1>\\1</h1>");
    // 水平线和表格
    html.replace(QRegularExpression("^---$", QRegularExpression::MultilineOption), "<hr>");
    html.replace(QRegularExpression("\\|[-: ]+\\|[-: |]+\\|"), "");
    html.replace(QRegularExpression("^\\|(.+)\\|$", QRegularExpression::MultilineOption),
                 "<p style='font-family:monospace;margin:2px 0;'>\\1</p>");
    html.replace(QRegularExpression("!\\[.*?\\]\\(.*?\\)"), "");
    // 换行
    html.replace("\n\n", "<br><br>");
    html.replace("\n", "<br>");
    // 恢复代码块
    for (int i = 0; i < codeBlocks.size(); ++i) {
        QString cb = codeBlocks[i];
        cb.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
        cb.replace(QRegularExpression("^```\\w*\\n?"), "<pre style='background:#F0E8DD;padding:8px;border-radius:4px;'>");
        cb.replace(QRegularExpression("```$"), "</pre>");
        html.replace(QString("\v%1\v").arg(i), cb);
    }
    // 转义裸露的 &
    html.replace("&", "&amp;");
    // 还原代码块中的转义
    html.replace("&amp;lt;", "&lt;").replace("&amp;gt;", "&gt;");
    return html;
}

void MainWindow::onPromptKnowledge(Player* player, const QString& title,
                                    const QString& content) {
    if (!m_game || !player) return;

    QDialog dlg(this);
    dlg.setWindowTitle("知识点");
    dlg.setMinimumWidth(460);
    dlg.setMinimumHeight(300);
    dlg.setStyleSheet(
        "QDialog { background-color: #FFF8F5; border: 3px solid #8B1A1A; border-radius: 8px; }");

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 16, 20, 16);

    // 标题（使用 QLabel + HTML 渲染）
    auto* titleLabel = new QLabel(&dlg);
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setText(md2html(title));
    titleLabel->setWordWrap(true);
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #8B1A1A; padding-bottom: 2px;");
    layout->addWidget(titleLabel);

    // 分隔线
    auto* div = new QLabel(&dlg);
    div->setFixedHeight(1);
    div->setStyleSheet("background-color: #8B1A1A;");
    layout->addWidget(div);

    // 内容（使用 QTextEdit 只读渲染 HTML）
    auto* contentEdit = new QTextEdit(&dlg);
    contentEdit->setReadOnly(true);
    contentEdit->setHtml(md2html(content));
    contentEdit->setStyleSheet(
        "QTextEdit { font-size: 13px; color: #3D2820; background-color: #FFFFFF; "
        "border: 1px solid #E0D5C8; border-radius: 4px; padding: 8px; "
        "font-family: 'Microsoft YaHei', Arial; }"
        "QScrollBar:vertical { width: 6px; background: #F0E8DD; border-radius: 3px; }"
        "QScrollBar::handle:vertical { background: #8B1A1A; border-radius: 3px; }");
    layout->addWidget(contentEdit, 1);

    // 提示文字
    auto* hintLabel = new QLabel("阅读完成后可获得一张随机效果卡作为奖励", &dlg);
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet("color: #8B7355; font-size: 12px; font-style: italic;");
    layout->addWidget(hintLabel);

    // 按钮
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton* okBtn = new QPushButton("完成阅读，领取奖励", &dlg);
    okBtn->setStyleSheet(
        "QPushButton { background-color: #B22222; color: #FFFFFF; padding: 10px 28px; "
        "border: 1px solid #8B1A1A; border-radius: 6px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #C62828; }");
    okBtn->setCursor(Qt::PointingHandCursor);
    btnLayout->addWidget(okBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    // 居中显示
    dlg.adjustSize();
    QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    dlg.move(center.x() - dlg.width() / 2, center.y() - dlg.height() / 2);

    dlg.exec();

    // 奖励随机效果卡
    EffectCardType rewardType = randomEffectCardType();
    player->addEffectCard(rewardType);
    EffectCard card = createEffectCard(rewardType);
    m_game->logEvent(player->name() + " 完成阅读，获得效果卡：" + card.name + "！");
    emit m_game->playerUpdated(player);

    m_game->endTurn();
}

void MainWindow::onPromptDebugKnowledge(Player* player) {
    if (!m_game || !player) return;

    QDialog dlg(this);
    dlg.setWindowTitle("DEBUG — 知识点事件");
    dlg.setMinimumWidth(320);
    dlg.setStyleSheet(
        "QDialog { background-color: #FFF8F5; border: 3px solid #8B1A1A; border-radius: 8px; }");

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(14);
    layout->setContentsMargins(24, 20, 24, 20);

    auto* label = new QLabel("DEBUG 模式：是否触发知识点事件？", &dlg);
    label->setWordWrap(true);
    label->setStyleSheet("font-size: 15px; font-weight: bold; color: #8B1A1A;");
    layout->addWidget(label);

    auto* hint = new QLabel("选\"是\"展示随机知识点并奖励效果卡\n选\"否\"跳过，直接结束回合", &dlg);
    hint->setStyleSheet("color: #8B7355; font-size: 12px;");
    layout->addWidget(hint);

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    QPushButton* yesBtn = new QPushButton("是，触发知识点", &dlg);
    yesBtn->setStyleSheet(
        "QPushButton { background-color: #B22222; color: #FFFFFF; padding: 9px 24px; "
        "border: 1px solid #8B1A1A; border-radius: 5px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #C62828; }");
    yesBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addWidget(yesBtn);

    QPushButton* noBtn = new QPushButton("否，跳过", &dlg);
    noBtn->setStyleSheet(
        "QPushButton { background-color: #E8E0D8; color: #5A4A3A; padding: 9px 24px; "
        "border: 1px solid #C0B0A0; border-radius: 5px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #DDD5CB; }");
    noBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addWidget(noBtn);
    btnRow->addStretch();
    layout->addLayout(btnRow);

    QObject::connect(yesBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(noBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    dlg.adjustSize();
    QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    dlg.move(center.x() - dlg.width() / 2, center.y() - dlg.height() / 2);

    if (dlg.exec() == QDialog::Accepted) {
        // 触发知识点
        KnowledgeEntry ke = KnowledgeBank::drawRandom();
        if (!ke.title.isEmpty()) {
            // 展示知识点
            m_game->logEvent(player->name() + " [DEBUG] 触发了知识点事件！");
            emit m_game->promptKnowledge(player, ke.title, ke.content);
            return;  // promptKnowledge handler will call endTurn
        }
    }

    // 跳过
    m_game->logEvent(player->name() + " [DEBUG] 跳过了知识点事件");
    m_game->endTurn();
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
