#include <QtTest/QtTest>

#include "GameBoard.h"

// 前向声明
class GameBoardTest;

class GameBoardTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testHandleSelection_FirstSelection();
    void testHandleSelection_SamePositionCancel();
    void testHandleSelection_DifferentType();
    void testHandleSelection_SameTypeWithValidPath();
    void testHandleSelection_SameTypeWithNoPath();
    void testHandleSelection_SameTypeWithMoreTurn();
    void testHandleSelection_AnimationBlocked();
    void testHandleSelection_CrossPlayerCancel();

private:
    GameBoard* board;
};

void GameBoardTest::init()
{
    board = new GameBoard();
    // 初始化游戏，设置单人模式
    board->newGame(GameMode::SinglePlayer);

    // 重置关键状态
    board->m_selectedRow1 = -1;
    board->m_selectedCol1 = -1;
    board->m_selectedRow2 = -1;
    board->m_selectedCol2 = -1;
    board->m_isAnimating = false;
    board->m_statusMessage.clear();
}

void GameBoardTest::cleanup()
{
    delete board;
}

void GameBoardTest::testHandleSelection_FirstSelection()
{
    // 设置地图：(1,1) 是一个纸箱
    board->m_gameMap[1][1] = BOX_PAPER;

    // 玩家1 选择 (1,1)
    board->handleSelection(1, 1, 1);

    // 验证：应记录选择位置
    QCOMPARE(board->m_selectedRow1, 1);
    QCOMPARE(board->m_selectedCol1, 1);
    QCOMPARE(board->m_statusMessage, QString("")); // 无消息
    QVERIFY(!board->m_isAnimating);
}

void GameBoardTest::testHandleSelection_SamePositionCancel()
{
    board->m_selectedRow1 = 1;
    board->m_selectedCol1 = 1;

    // 再次选择 (1,1) → 应取消
    board->handleSelection(1, 1, 1);

    QCOMPARE(board->m_selectedRow1, -1);
    QCOMPARE(board->m_selectedCol1, -1);
}

void GameBoardTest::testHandleSelection_DifferentType()
{
    // 地图：(1,1)=纸箱, (2,2)=木箱
    board->m_gameMap[1][1] = BOX_PAPER;
    board->m_gameMap[2][2] = BOX_WOOD;

    board->handleSelection(1, 1, 1); // 第一次选纸箱
    board->handleSelection(2, 2, 1); // 第二次选木箱 → 类型不同

    // 验证：选择被清除，提示失败
    QCOMPARE(board->m_selectedRow1, -1);
    QCOMPARE(board->m_statusMessage, QString("不可消除!"));
}

void GameBoardTest::testHandleSelection_SameTypeWithValidPath()
{

    memset(board->m_gameMap, EMPTY, sizeof(board->m_gameMap));
    board->m_gameMap[1][1] = BOX_PAPER;
    board->m_gameMap[2][2] = BOX_PAPER;

    board->handleSelection(1, 1, 1); // 选第一个
    board->handleSelection(2, 2, 1); // 选第二个

    // 验证：成功消除
    QVERIFY(board->m_isAnimating);
    QCOMPARE(board->m_eliminationBox1, QPoint(1,1));
    QCOMPARE(board->m_eliminationBox2, QPoint(2,2));
    QCOMPARE(board->m_selectedRow1, -1);
    QCOMPARE(board->m_statusMessage, QString("消除!"));
}

void GameBoardTest::testHandleSelection_SameTypeWithNoPath()
{
    /*
     * 地图：
     * W W W .
     * W P W .
     * W W P .
     * . . . .
     * W=WALL，阻挡路径
     */
    memset(board->m_gameMap, EMPTY, sizeof(board->m_gameMap));
    board->m_gameMap[1][1] = BOX_PAPER;
    board->m_gameMap[0][0] = WALL;
    board->m_gameMap[0][1] = WALL;
    board->m_gameMap[0][2] = WALL;
    board->m_gameMap[1][0] = WALL;
    board->m_gameMap[1][2] = WALL;
    board->m_gameMap[2][0] = WALL;
    board->m_gameMap[2][1] = WALL;
    board->m_gameMap[2][2] = BOX_PAPER;

    board->handleSelection(1, 1, 1);
    board->handleSelection(2, 2, 1);

    // 验证：未消除，提示失败
    QVERIFY(!board->m_isAnimating);
    QCOMPARE(board->m_selectedRow1, -1);
    QCOMPARE(board->m_statusMessage, QString("不可消除!"));
}

void GameBoardTest::testHandleSelection_SameTypeWithMoreTurn()
{
    /*
     * 地图：
     * W W W .
     * W P . W
     * W W . . W P
     * . . W . . .
     * W=WALL，阻挡路径
     */
    memset(board->m_gameMap, EMPTY, sizeof(board->m_gameMap));
    board->m_gameMap[1][1] = BOX_PAPER;
    board->m_gameMap[0][0] = WALL;
    board->m_gameMap[0][1] = WALL;
    board->m_gameMap[0][2] = WALL;
    board->m_gameMap[1][0] = WALL;
    board->m_gameMap[1][3] = WALL;
    board->m_gameMap[2][0] = WALL;
    board->m_gameMap[2][1] = WALL;
    board->m_gameMap[2][4] = WALL;
    board->m_gameMap[3][2] = WALL;
    board->m_gameMap[2][5] = BOX_PAPER;

    board->handleSelection(1, 1, 1);
    board->handleSelection(2, 5, 1);

    // 验证：未消除，提示失败
    QVERIFY(!board->m_isAnimating);
    QCOMPARE(board->m_selectedRow1, -1);
    QCOMPARE(board->m_statusMessage, QString("不可消除!"));
}

void GameBoardTest::testHandleSelection_AnimationBlocked()
{
    board->m_isAnimating = true; // 模拟动画中

    // 即使地图有箱子，也不应处理
    board->m_gameMap[1][1] = BOX_PAPER;
    board->handleSelection(1, 1, 1);

    // 验证：无反应
    QCOMPARE(board->m_selectedRow1, -1);
    QVERIFY(board->m_isAnimating); // 保持原状态
}

void GameBoardTest::testHandleSelection_CrossPlayerCancel()
{
    // 玩家2 已选中 (2,2)
    board->m_selectedRow2 = 2;
    board->m_selectedCol2 = 2;

    // 地图：(1,1) 和 (2,2) 都是纸箱
    board->m_gameMap[1][1] = BOX_PAPER;
    board->m_gameMap[2][2] = BOX_PAPER;

    board->handleSelection(1, 1, 1); // 玩家1 选 (1,1)
    board->handleSelection(2, 2, 1); // 玩家1 消除 (2,2)

    // 验证：玩家2 的选择被清除
    QCOMPARE(board->m_selectedRow2, -1);
    QCOMPARE(board->m_selectedCol2, -1);
}

QTEST_MAIN(GameBoardTest)

#include "tst_gameboard.moc"
