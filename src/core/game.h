#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QVector>
#include <QPair>
#include "config.h"
#include "effectcard.h"
#include "questionbank.h"
#include <QVector>

class Player;
class Tile;
class Board;
class Dice;

enum class GameState {
    WAITING_TO_START,
    PRE_ROLL,
    ROLLING,
    MOVING,
    LANDING,
    TURN_END,
    GAME_OVER
};

class Game : public QObject {
    Q_OBJECT
public:
    explicit Game(QObject* parent = nullptr);
    ~Game();

    // === 初始化 ===
    void addPlayer(const QString& name, const QColor& color);
    void startGame();
    void resetGame();

    // === 访问器 ===
    Board& board() const { return *m_board; }
    const QVector<Player*>& players() const { return m_players; }
    Player* currentPlayer() const;
    int currentPlayerIndex() const { return m_currentPlayerIndex; }
    GameState state() const { return m_state; }
    Dice* dice() const { return m_dice; }

    int lastDiceTotal() const { return m_lastDiceTotal; }
    int lastDie1() const { return m_lastDie1; }
    int lastDie2() const { return m_lastDie2; }

    void setWaitingForDecision(bool w) { m_waitingForDecision = w; }

    // === 回合流程 ===
    void rollDice();
    void debugRollDice(int die1, int die2);
    void movePlayer();
    void handleLanding();
    void endTurn();

    // === 玩家操作 ===
    void buyProperty(Player* player, int tileIndex);
    void buildHouse(Player* player, int tileIndex);
    void skipAction();

    // === QA / 上机课 ===
    void setCurrentQuestion(const Question& q) { m_currentQuestion = q; }
    void answerQA(Player* player, int tileIndex, int chosenAnswer);
    void answerComputerLab(Player* player, int chosenAnswer);

    // === 效果卡 ===
    void onCardDecision(EffectCardType cardType, bool used);
    void setUniversalDice(int die1, int die2);

    // === 虚函数卡 ===
    void buyPropertyVirtualFunc(Player* player, int tileIndex, bool useDerived);
    void buildHouseVirtualFunc(Player* player, int tileIndex, bool useDerived);
    void payRentVirtualFunc(Player* payer, int tileIndex, bool useDerived);
    void handlePureVirtualNoRent(Player* payer, int tileIndex);

    // === 迭代器卡 ===
    void useIteratorCard(Player* player, int fromTileIndex,
                         IteratorSubtype sub, IteratorOp op);
    void declineIteratorCard();

    // === 商店 ===
    void buyEffectCard(Player* player, EffectCardType type);
    void buyEffectCard(Player* player, const EffectCard& card);
    void goToShop(Player* player);
    void declineShopEntrance();

    // === 事件日志 ===
    void logEvent(const QString& message);

    // === 胜利判断 ===
    Player* getWinner() const;

signals:
    void gameStateChanged(GameState newState);
    void eventLogged(const QString& message);
    void diceRolled(int die1, int die2);
    void playerMoved(Player* player, int fromPos, int toPos);
    void playerUpdated(Player* player);
    void boardUpdated();
    void promptBuyProperty(int tileIndex, Player* player);
    void promptBuildHouse(int tileIndex, Player* player);
    void gameOver(Player* winner);
    void turnStarted(Player* player);

    // 新增信号
    void promptQA(Player* player, int tileIndex);
    void promptComputerLab(Player* player);
    void promptShop(Player* player);
    void promptShopEntrance(Player* player);
    void promptUseCard(Player* player, EffectCardType type);
    void promptUniversalDice(Player* player);

    // 迭代器卡相关信号
    void promptIteratorCard(Player* player, int tileIndex);

    // 虚函数卡相关信号
    void promptVirtualFuncBuy(Player* player, int tileIndex, int basePrice, int derivedPrice);
    void promptVirtualFuncRent(Player* payer, int tileIndex, Player* owner, int baseRent, int derivedRent);
    void promptVirtualFuncBuild(Player* player, int tileIndex, int baseCost, int derivedCost);

private slots:
    void onDiceRolled(int die1, int die2);

private:
    void nextPlayer();
    bool checkGameOver();
    void proceedAfterCardDecision();
    int getCardPrice(EffectCardType type) const;
    int computeIteratorTarget(int fromIndex, IteratorOp op) const;
    QVector<int> m_iteratorIndices;  // 缓存迭代器格索引（有序）

    Board* m_board;
    Dice* m_dice;
    QVector<Player*> m_players;

    GameState m_state = GameState::WAITING_TO_START;
    int m_currentPlayerIndex = 0;
    int m_consecutiveDoubles = 0;

    int m_lastDie1 = 0;
    int m_lastDie2 = 0;
    int m_lastDiceTotal = 0;

    bool m_waitingForDecision = false;
    bool m_waitingForCardDecision = false;
    bool m_skipLanding = false;
    EffectCardType m_pendingCardType = EffectCardType::ROLL_AGAIN;
    Question m_currentQuestion;
};

#endif // GAME_H
