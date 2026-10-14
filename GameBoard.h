#pragma once

#include <QWidget>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QVector>
#include <QPixmap>
#include <QPoint>
#include <QTimer>
#include <QRandomGenerator>

#include "Character.h"

const int BOARD_WIDTH = 12;
const int BOARD_HEIGHT = 12;//棋盘的长宽
const int TILE_SIZE = 48; // 每个图块的像素大小
const int TILE_ITEM_SIZE=32;
const int BOX_TYPE_COUNT = 5;//箱子的种数

enum TileType {
    EMPTY = 0,
    WALL = -1,
    ITEM_TIME = -2,
    ITEM_SHUFFLE = -3,
    ITEM_HINT = -4,

    BOX_PAPER = 1,
    BOX_WOOD = 2,
    BOX_STEEL = 3,
    BOX_COPPER = 4,
    BOX_FOREST = 5
};

enum class GameMode {
    SinglePlayer,
    TwoPlayer
};

struct PlayerInfo {
    Character* character = nullptr;
    int index = 0;        // 1 或 2
    bool isValid = false;
    Direction direction = UP;
};

class GameBoardTest;

class GameBoard : public QWidget
{
    Q_OBJECT
    friend class GameBoardTest;

public:
    explicit GameBoard(QWidget *parent = nullptr);
    void newGame(GameMode mode = GameMode::SinglePlayer );
    bool saveGame();
    bool loadGame();

signals:
    void backToMainClicked();
    void gameFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void onAnimationFinished();
    void clearStatusMessage();
    void onGameTick();
    void onComboTimeout1();
    void onComboTimeout2();
    void onItemSpawnCheck();
    void onHintTimeout();
    void showGameOverDialog();
    void updateCharacters();

private:
    void initGame();
    void loadResources();
    void togglePause();
    bool checkWinCondition();

    void handlePauseInput(QKeyEvent *event);
    void handleSelection(int row, int col, int playerIndex);
    void processElimination(int playerIndex);
    void processQueuedMoveForCharacter(Character& character, int playerIndex);
    PlayerInfo getPlayerByKeyEvent(QKeyEvent*);
    bool isCharacterInputAllowed(const Character& character) const;


    void activateShuffle();
    void activateHint();
    bool findAndSetHintPair();
    void applyItemEffect(int itemValue);

    void drawText(QPainter &painter, const QString &text, const QRect &rect, const QFont &font, Qt::Alignment alignment);
    void paintBoard(QPainter &painter);
    void paintAnimations(QPainter &painter);
    void paintUI(QPainter &painter);
    void paintPauseScreen(QPainter &painter);


    int m_gameMap[BOARD_HEIGHT][BOARD_WIDTH];
    Character m_char1;
    Character m_char2;
    bool m_isGameOver;
    GameMode m_gameMode;

    bool m_keyWPressed = false;
    bool m_keySPressed = false;
    bool m_keyAPressed = false;
    bool m_keyDPressed = false;
    bool m_keyUpPressed = false;
    bool m_keyDownPressed = false;
    bool m_keyLeftPressed = false;
    bool m_keyRightPressed = false;

    //第一个选中的箱子位置
    int m_selectedRow1, m_selectedCol1;
    int m_selectedRow2, m_selectedCol2;
    int m_currentPlayerTurn;
    QSet<QPair<int, int>> m_recentlySelectedBoxes;
    //计分与计时
    int m_score1, m_score2;
    int m_remainingTime;
    int m_comboCount1, m_comboCount2;
    //贴图
    QPixmap m_tileset;
    QPixmap m_boxTileset1;
    QPixmap m_boxTileset2;
    QPixmap m_boxTileset3;
    QPixmap m_itemTileset;
    QPixmap m_groundImage, m_wallImage;
    QPixmap m_PaperBoxImage, m_WoodBoxImage, m_SteelBoxImage, m_CopperBoxImage, m_ForestBoxImage;
    QPixmap m_itemTimeImage, m_itemShuffleImage, m_itemHintImage;
    //动画
    bool m_isAnimating;
    QString m_statusMessage;
    QVector<QPoint> m_linePath;
    QPoint m_eliminationBox1, m_eliminationBox2;
    //道具提示
    bool m_isHintActive;
    QPoint m_hintPair1, m_hintPair2;
    //暂停状态
    bool m_isPaused;
    int m_pauseMenuSelection;// 0: 回到游戏, 1: 存档, 2: 重新开始, 3: 回到主菜单
    //计时器
    QTimer *m_animationTimer;
    QTimer *m_messageClearTimer;
    QTimer *m_comboTimer1;
    QTimer *m_comboTimer2;
    QTimer *m_gameTimer;
    QTimer *m_itemSpawnTimer;
    QTimer *m_hintTimer;
    QTimer *m_charAnimationTimer;
};
