#include "game.h"
#include "board.h"
#include "tile.h"
#include "player.h"
#include "dice.h"

#include <QTimer>
#include <QRandomGenerator>

// ==================== 构造/析构 ====================
Game::Game(QObject* parent)
    : QObject(parent),
      m_board(new Board(this)),
      m_dice(new Dice(this))
{
    connect(m_dice, &Dice::diceRolled, this, &Game::onDiceRolled);
}

Game::~Game() {
    qDeleteAll(m_players);
    m_players.clear();
}


// ==================== 初始化 ====================
void Game::addPlayer(const QString& name, const QColor& color) {
    if (m_players.size() >= 4) return;
    int id = m_players.size();
    m_players.append(new Player(name, color, id));
}

void Game::startGame() {
    if (m_players.size() < 2) return;

    m_state = GameState::PRE_ROLL;
    m_currentPlayerIndex = 0;
    m_consecutiveDoubles = 0;
    logEvent("=== 游戏开始！共 " + QString::number(m_players.size()) + " 位玩家 ===");
    emit gameStateChanged(m_state);
    emit turnStarted(currentPlayer());
}

void Game::resetGame() {
    m_state = GameState::WAITING_TO_START;
    m_currentPlayerIndex = 0;
    m_consecutiveDoubles = 0;
    m_waitingForDecision = false;
    m_waitingForCardDecision = false;
    m_skipLanding = false;
    m_lastDie1 = m_lastDie2 = m_lastDiceTotal = 0;

    m_board->reset();

    for (auto* p : m_players) {
        p->reset();
    }

    emit gameStateChanged(m_state);
    emit boardUpdated();
    logEvent("游戏已重置，准备开始新一局");
}


// ==================== 访问器 ====================
Player* Game::currentPlayer() const {
    if (m_players.isEmpty()) return nullptr;
    return m_players[m_currentPlayerIndex];
}


// ==================== 回合流程 ====================
void Game::rollDice() {
    if (m_state != GameState::PRE_ROLL) return;

    m_state = GameState::ROLLING;
    emit gameStateChanged(m_state);

    Player* player = currentPlayer();

    // 检查是否因上机课需要跳过本回合
    if (player->skipNextTurn()) {
        player->setSkipNextTurn(false);
        logEvent(player->name() + " 因上机课跳过本回合！");
        emit playerUpdated(player);
        endTurn();
        return;
    }

    // 检查万能骰子
    if (player->hasEffectCard(EffectCardType::UNIVERSAL_DICE)) {
        emit promptUseCard(player, EffectCardType::UNIVERSAL_DICE);
        m_waitingForCardDecision = true;
        m_pendingCardType = EffectCardType::UNIVERSAL_DICE;
        return;
    }

    logEvent(player->name() + " 的回合 —— 掷骰子...");
    m_dice->roll();
}

void Game::onDiceRolled(int die1, int die2) {
    m_lastDie1 = die1;
    m_lastDie2 = die2;
    m_lastDiceTotal = die1 + die2;

    Player* player = currentPlayer();
    bool isDouble = (die1 == die2);

    logEvent(player->name() + QString(" 掷出了 %1 + %2 = %3")
             .arg(die1).arg(die2).arg(die1 + die2));

    if (isDouble) {
        m_consecutiveDoubles++;
        logEvent("掷出对子！");
    } else {
        m_consecutiveDoubles = 0;
    }

    emit diceRolled(die1, die2);

    // 检查再丢一次骰子卡
    if (player->hasEffectCard(EffectCardType::ROLL_AGAIN)) {
        emit promptUseCard(player, EffectCardType::ROLL_AGAIN);
        m_waitingForCardDecision = true;
        m_pendingCardType = EffectCardType::ROLL_AGAIN;
        return;
    }

    m_state = GameState::MOVING;
    emit gameStateChanged(m_state);
    movePlayer();
}

void Game::movePlayer() {
    Player* player = currentPlayer();
    int fromPos = player->position();
    int steps = m_lastDiceTotal;

    player->moveBy(steps, this);

    int toPos = player->position();
    emit playerMoved(player, fromPos, toPos);

    m_state = GameState::LANDING;
    emit gameStateChanged(m_state);

    QTimer::singleShot(500, this, &Game::handleLanding);
}

void Game::handleLanding() {
    Player* player = currentPlayer();
    Tile* tile = m_board->tileAt(player->position());
    if (!tile) return;

    // 检查是否是商店入口移动后的处理
    if (m_skipLanding) {
        m_skipLanding = false;
        endTurn();
        return;
    }

    // 检查跳过卡（仅对负面地块提示）
    bool canSkip = false;
    switch (tile->type()) {
    case TileType::TAX:
        canSkip = true;
        break;
    case TileType::PROPERTY: {
        auto* pt = dynamic_cast<PropertyTile*>(tile);
        if (pt && pt->owner() && pt->owner() != player && !pt->owner()->isBankrupt())
            canSkip = true;
        break;
    }
    case TileType::UTILITY: {
        auto* ut = dynamic_cast<UtilityTile*>(tile);
        if (ut && ut->owner() && ut->owner() != player && !ut->owner()->isBankrupt())
            canSkip = true;
        break;
    }
    case TileType::RAILROAD: {
        auto* rt = dynamic_cast<RailroadTile*>(tile);
        if (rt && rt->owner() && rt->owner() != player && !rt->owner()->isBankrupt())
            canSkip = true;
        break;
    }
    default:
        break;
    }

    if (canSkip && player->hasEffectCard(EffectCardType::SKIP_EFFECT)) {
        emit promptUseCard(player, EffectCardType::SKIP_EFFECT);
        m_waitingForCardDecision = true;
        m_pendingCardType = EffectCardType::SKIP_EFFECT;
        return;
    }

    tile->landOn(player, this);

    if (!m_waitingForDecision && !m_waitingForCardDecision) {
        endTurn();
    }
}

void Game::endTurn() {
    if (m_state == GameState::GAME_OVER) return;

    Player* player = currentPlayer();

    // 掷出对子可获得额外回合
    bool isDouble = (m_lastDie1 == m_lastDie2);
    if (isDouble && !player->isBankrupt()) {
        logEvent(player->name() + " 掷出对子，再来一回合！");
        m_state = GameState::PRE_ROLL;
        emit gameStateChanged(m_state);
        emit turnStarted(player);
        return;
    }

    if (!checkGameOver()) {
        nextPlayer();
        m_consecutiveDoubles = 0;
        m_state = GameState::PRE_ROLL;
        emit gameStateChanged(m_state);
        emit turnStarted(currentPlayer());
    }
}


// ==================== 下一个玩家 ====================
void Game::nextPlayer() {
    int startIndex = m_currentPlayerIndex;
    do {
        m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_players.size();
    } while (m_players[m_currentPlayerIndex]->isBankrupt()
             && m_currentPlayerIndex != startIndex);
}


// ==================== 玩家操作 ====================
void Game::buyProperty(Player* player, int tileIndex) {
    Tile* t = m_board->tileAt(tileIndex);
    if (!t) return;

    if (auto* pt = dynamic_cast<PropertyTile*>(t)) {
        if (pt->owner() != nullptr) return;
        if (!player->canAfford(pt->price())) {
            logEvent(player->name() + " 钱不够，无法购买！");
            m_waitingForDecision = false;
            endTurn();
            return;
        }
        player->payMoney(pt->price(), this);
        pt->setOwner(player);
        player->addProperty(pt);
        logEvent(player->name() + QString(" 购买了 %1，花费 %2 元")
                  .arg(pt->name()).arg(pt->price()));
        emit playerUpdated(player);
        emit boardUpdated();
    } else if (auto* ut = dynamic_cast<UtilityTile*>(t)) {
        if (ut->owner() != nullptr) return;
        if (!player->canAfford(ut->price())) return;
        player->payMoney(ut->price(), this);
        ut->setOwner(player);
        logEvent(player->name() + QString(" 购买了 %1，花费 %2 元")
                  .arg(ut->name()).arg(ut->price()));
        emit playerUpdated(player);
        emit boardUpdated();
    } else if (auto* rt = dynamic_cast<RailroadTile*>(t)) {
        if (rt->owner() != nullptr) return;
        if (!player->canAfford(rt->price())) return;
        player->payMoney(rt->price(), this);
        rt->setOwner(player);
        logEvent(player->name() + QString(" 购买了 %1，花费 %2 元")
                  .arg(rt->name()).arg(rt->price()));
        emit playerUpdated(player);
        emit boardUpdated();
    }

    m_waitingForDecision = false;
    endTurn();
}

void Game::buildHouse(Player* player, int tileIndex) {
    auto* pt = dynamic_cast<PropertyTile*>(m_board->tileAt(tileIndex));
    if (!pt || pt->owner() != player) return;
    if (!pt->canBuildHouse(player)) return;
    if (!player->canAfford(pt->houseCost())) {
        logEvent(player->name() + " 钱不够，无法建房！");
        m_waitingForDecision = false;
        endTurn();
        return;
    }

    player->payMoney(pt->houseCost(), this);
    pt->buildHouse();
    if (pt->hasHotel()) {
        logEvent(player->name() + " 在 " + pt->name() + " 建造了旅馆！");
    } else {
        logEvent(player->name() + QString(" 在 %1 建造了第 %2 栋房子")
                  .arg(pt->name()).arg(pt->houses()));
    }
    emit playerUpdated(player);
    emit boardUpdated();

    m_waitingForDecision = false;
    endTurn();
}

void Game::skipAction() {
    m_waitingForDecision = false;
    endTurn();
}


// ==================== QA / 上机课 ====================
void Game::answerQA(Player* player, int tileIndex, int chosenAnswer) {
    Q_UNUSED(tileIndex)
    if (chosenAnswer == m_currentQuestion.correctIndex) {
        logEvent(player->name() + " 回答正确！");
        if (QRandomGenerator::global()->bounded(100) < QA_CARD_CHANCE_PERCENT) {
            EffectCardType rewardType = randomEffectCardType();
            player->addEffectCard(rewardType);
            EffectCard card = createEffectCard(rewardType);
            logEvent(player->name() + " 获得效果卡：" + card.name + "！");
        } else {
            logEvent(player->name() + " 可惜，没有抽到效果卡。");
        }
    } else {
        char correctChar = 'A' + m_currentQuestion.correctIndex;
        logEvent(player->name() + QString(" 回答错误。正确答案是 %1").arg(correctChar));
    }
    emit playerUpdated(player);
    m_waitingForDecision = false;
    endTurn();
}

void Game::answerComputerLab(Player* player, int chosenAnswer) {
    // skipNextTurn 已由 ComputerLabTile::landOn 设置
    if (chosenAnswer == m_currentQuestion.correctIndex) {
        EffectCardType rewardType = randomEffectCardType();
        player->addEffectCard(rewardType);
        EffectCard card = createEffectCard(rewardType);
        logEvent(player->name() + " 回答正确！获得效果卡：" + card.name + "！");
        logEvent(player->name() + " 下回合跳过。");
    } else {
        char correctChar = 'A' + m_currentQuestion.correctIndex;
        logEvent(player->name() + QString(" 回答错误。正确答案是 %1。下回合跳过。").arg(correctChar));
    }
    emit playerUpdated(player);
    m_waitingForDecision = false;
    endTurn();
}


// ==================== 效果卡 ====================
void Game::onCardDecision(EffectCardType cardType, bool used) {
    m_waitingForCardDecision = false;
    Player* player = currentPlayer();

    if (!used) {
        // 不使用，继续正常流程
        proceedAfterCardDecision();
        return;
    }

    switch (cardType) {
    case EffectCardType::UNIVERSAL_DICE:
        // 使用万能骰子，弹出选点界面
        emit promptUniversalDice(player);
        return;
    case EffectCardType::ROLL_AGAIN:
        if (player->useEffectCard(EffectCardType::ROLL_AGAIN)) {
            logEvent(player->name() + " 使用了'再丢一次骰子'卡！");
            m_dice->roll();
        }
        return;
    case EffectCardType::SKIP_EFFECT:
        if (player->useEffectCard(EffectCardType::SKIP_EFFECT)) {
            logEvent(player->name() + " 使用了'跳过卡'，跳过了地块效果！");
        }
        endTurn();
        return;
    case EffectCardType::VIRTUAL_FUNCTION:
        if (player->useEffectCard(EffectCardType::VIRTUAL_FUNCTION)) {
            logEvent(player->name() + " 使用了'虚函数卡'（效果待实现）");
        }
        proceedAfterCardDecision();
        return;
    }
}

void Game::proceedAfterCardDecision() {
    Player* player = currentPlayer();

    // 默认：继续移动流程
    m_state = GameState::MOVING;
    emit gameStateChanged(m_state);
    movePlayer();
}

void Game::setUniversalDice(int die1, int die2) {
    Player* player = currentPlayer();
    if (!player->useEffectCard(EffectCardType::UNIVERSAL_DICE)) return;

    logEvent(player->name() + QString(" 使用了万能骰子：%1 + %2 = %3")
             .arg(die1).arg(die2).arg(die1 + die2));

    m_lastDie1 = die1;
    m_lastDie2 = die2;
    m_lastDiceTotal = die1 + die2;

    if (die1 == die2) {
        m_consecutiveDoubles++;
        logEvent("掷出对子！");
    }

    emit diceRolled(die1, die2);

    m_state = GameState::MOVING;
    emit gameStateChanged(m_state);
    movePlayer();
}


// ==================== 商店 ====================
int Game::getCardPrice(EffectCardType type) const {
    switch (type) {
    case EffectCardType::ROLL_AGAIN:       return CARD_PRICE_ROLL_AGAIN;
    case EffectCardType::UNIVERSAL_DICE:   return CARD_PRICE_UNIVERSAL_DICE;
    case EffectCardType::VIRTUAL_FUNCTION: return CARD_PRICE_VIRTUAL_FUNCTION;
    case EffectCardType::SKIP_EFFECT:      return CARD_PRICE_SKIP_EFFECT;
    }
    return 0;
}

void Game::buyEffectCard(Player* player, EffectCardType type) {
    int price = getCardPrice(type);
    if (!player->canAfford(price)) {
        logEvent(player->name() + " 资金不足，无法购买！");
        return;
    }
    player->payMoney(price, this);
    player->addEffectCard(type);
    EffectCard card = createEffectCard(type);
    logEvent(player->name() + " 购买了 " + card.name + "！");
    emit playerUpdated(player);
}

void Game::goToShop(Player* player) {
    logEvent(player->name() + " 选择进入商店！");
    player->setPosition(7);
    emit playerMoved(player, player->position(), 7);
    emit playerUpdated(player);

    // 在商店触发购物
    Tile* shopTile = m_board->tileAt(7);
    if (shopTile) {
        m_skipLanding = true;  // 防止 handleLanding 再次处理
        shopTile->landOn(player, this);
    }
}

void Game::declineShopEntrance() {
    Player* player = currentPlayer();
    logEvent(player->name() + " 选择不进入商店。");
    if (!m_waitingForDecision) {
        endTurn();
    }
}


// ==================== 事件日志 ====================
void Game::logEvent(const QString& message) {
    emit eventLogged(message);
}


// ==================== 胜利判断 ====================
bool Game::checkGameOver() {
    QVector<Player*> alive;
    for (auto* p : m_players) {
        if (!p->isBankrupt()) alive.append(p);
    }
    if (alive.size() <= 1) {
        m_state = GameState::GAME_OVER;
        Player* winner = alive.isEmpty() ? nullptr : alive.first();
        if (winner) {
            logEvent("=== 游戏结束！" + winner->name() + " 获胜！ ===");
        }
        emit gameOver(winner);
        emit gameStateChanged(m_state);
        return true;
    }
    return false;
}

Player* Game::getWinner() const {
    for (auto* p : m_players) {
        if (!p->isBankrupt()) return p;
    }
    return nullptr;
}
