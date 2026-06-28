#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include "boardwidget.h"
#include "playerpanel.h"
#include "dicewidget.h"
#include "eventlog.h"

class Game;
class Player;
class QLabel;
enum class EffectCardType;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onNewGame();
    void onRules();
    void onRollDice();

    // 地产购买/升级
    void onPromptBuyProperty(int tileIndex, Player* player);
    void onPromptBuildHouse(int tileIndex, Player* player);

    // QA / 商店 / 上机课
    void onPromptQA(Player* player, int tileIndex);
    void onPromptComputerLab(Player* player);
    void onPromptShop(Player* player);
    void onPromptShopEntrance(Player* player);

    // 迭代器卡
    void onPromptIteratorCard(Player* player, int tileIndex);

    // 效果卡
    void onPromptUseCard(Player* player, EffectCardType type);
    void onPromptUniversalDice(Player* player);

    // 虚函数卡
    void onPromptVirtualFuncBuy(Player* player, int tileIndex, int basePrice, int derivedPrice);
    void onPromptVirtualFuncRent(Player* payer, int tileIndex, Player* owner, int baseRent, int derivedRent);
    void onPromptVirtualFuncBuild(Player* player, int tileIndex, int baseCost, int derivedCost);

    // 知识点事件
    void onPromptKnowledge(Player* player, const QString& title, const QString& content);
    void onPromptDebugKnowledge(Player* player);

    // 状态更新
    void onPlayerUpdated(Player* player);
    void onTurnStarted(Player* player);
    void onGameOver(Player* winner);
    void onLogEvent(const QString& msg);
    void onDiceRolled(int die1, int die2);
    void onPlayerMoved(Player* player, int from, int to);

private:
    void setupUI();
    void setupMenuBar();
    void connectSignals();
    void startNewGame();
    void endCurrentGame();
    void updatePropertyDisplay();
    QString playerSummary(Player* p) const;
    QDialog* createBoardDialog(const QString& title);
    void positionOnBoard(QDialog* dlg);

    Game* m_game = nullptr;
    bool m_debugMode = false;
    bool m_debugDialogOpen = false;

    BoardWidget* m_boardWidget;
    PlayerPanel* m_playerPanel;
    DiceWidget* m_diceWidget;
    EventLog* m_eventLog;
    QLabel* m_statusLabel;
    QScrollArea* m_propertyArea;
    QWidget* m_propertyContent;
};

#endif // MAINWINDOW_H
