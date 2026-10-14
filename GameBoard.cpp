#include "GameBoard.h"
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QPushButton>
#include <vector>
#include <random>
#include <algorithm>


#include "Character.h"
#include "GameLogic.h"

GameBoard::GameBoard(QWidget *parent) //初始化及定时器设置
    : QWidget(parent),
    m_selectedRow1(-1),
    m_selectedCol1(-1),
    m_selectedRow2(-1),
    m_selectedCol2(-1),
    m_isGameOver(false),
    m_isAnimating(false),
    m_score1(0),
    m_score2(0),
    m_gameMode(GameMode::SinglePlayer),
    m_currentPlayerTurn(1),
    m_remainingTime(120),
    m_comboCount1(0),
    m_comboCount2(0),
    m_isHintActive(false),
    m_hintPair1(-1, -1),
    m_hintPair2(-1, -1),
    m_isPaused(false),
    m_pauseMenuSelection(0)
{
    setFixedSize(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);
    setFocusPolicy(Qt::StrongFocus);

    m_animationTimer = new QTimer(this);
    m_animationTimer->setSingleShot(true);
    connect(m_animationTimer, &QTimer::timeout, this, &GameBoard::onAnimationFinished);

    m_messageClearTimer = new QTimer(this);
    m_messageClearTimer->setSingleShot(true);
    connect(m_messageClearTimer, &QTimer::timeout, this, &GameBoard::clearStatusMessage);

    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &GameBoard::onGameTick);
    m_gameTimer->start(1000);

    m_comboTimer1 = new QTimer(this);
    m_comboTimer1->setSingleShot(true);
    connect(m_comboTimer1, &QTimer::timeout, this, &GameBoard::onComboTimeout1);

    m_comboTimer2 = new QTimer(this);
    m_comboTimer2->setSingleShot(true);
    connect(m_comboTimer2, &QTimer::timeout, this, &GameBoard::onComboTimeout2);

    m_itemSpawnTimer = new QTimer(this);
    connect(m_itemSpawnTimer, &QTimer::timeout, this, &GameBoard::onItemSpawnCheck);
    m_itemSpawnTimer->start(8000); //每8秒检查一次是否生成道具

    m_hintTimer = new QTimer(this);
    m_hintTimer->setSingleShot(true);
    connect(m_hintTimer, &QTimer::timeout, this, &GameBoard::onHintTimeout);

    m_charAnimationTimer = new QTimer(this);
    connect(m_charAnimationTimer, &QTimer::timeout, this, &GameBoard::updateCharacters);
    m_charAnimationTimer->start(16);

    loadResources();
    initGame();
}

void GameBoard::initGame() //初始化游戏地图
{
    const int boxAreaStartRow = 2;
    const int boxAreaStartCol = 2;
    const int boxAreaHeight = 8;
    const int boxAreaWidth = 8;
    int totalTiles = boxAreaWidth * boxAreaHeight;

    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            if (r == 0 || r == BOARD_HEIGHT - 1 || c == 0 || c == BOARD_WIDTH - 1) {
                m_gameMap[r][c] = WALL; // 墙壁
            } else {
                m_gameMap[r][c] = EMPTY;  // 空地
            }
        }
    }

    std::vector<int> tiles;
    std::random_device rand;
    std::mt19937 g(rand());

    int pairs = totalTiles / 2;
    for (int i = 0; i < pairs; ++i) {
        int tileType = (g() % BOX_TYPE_COUNT) + 1;
        tiles.push_back(tileType);
        tiles.push_back(tileType);
    }

    std::shuffle(tiles.begin(), tiles.end(), g);

    int index = 0;
    for (int r = boxAreaStartRow; r < boxAreaStartRow + boxAreaHeight; ++r) {
        for (int c = boxAreaStartCol; c < boxAreaStartCol + boxAreaWidth; ++c) {
            m_gameMap[r][c] = tiles[index++];
        }
    }

}

void GameBoard::loadResources() //加载游戏资源
{
    //加载对应的tileset
    m_tileset.load(":/images/Dungeon_A2.png");
    m_boxTileset1.load(":/images/SF_Outside_B.png");
    m_boxTileset2.load(":/images/CaveBar_B.png");
    m_boxTileset3.load(":/images/fsm_Forest01_E.png");
    m_itemTileset.load(":/images/IconSet.png");

    const int TILE_SPRITE_SIZE = 48;
    //裁剪
    QRect ground(0,3 * TILE_SPRITE_SIZE, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
    QRect wall(12 * TILE_SPRITE_SIZE, 9 * TILE_SPRITE_SIZE, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
    m_groundImage = m_tileset.copy(ground);
    m_wallImage = m_tileset.copy(wall);

    QRect paper(10 * TILE_SPRITE_SIZE, 0, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
    QRect wood(8 * TILE_SPRITE_SIZE, 0, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
    QRect steel(9 * TILE_SPRITE_SIZE, 0, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
    QRect copper(4 * TILE_SPRITE_SIZE, 3 * TILE_SPRITE_SIZE, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
    QRect forest(15 * TILE_SPRITE_SIZE, 2 * TILE_SPRITE_SIZE, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
    m_PaperBoxImage = m_boxTileset1.copy(paper);
    m_WoodBoxImage = m_boxTileset1.copy(wood);
    m_SteelBoxImage = m_boxTileset1.copy(steel);
    m_CopperBoxImage = m_boxTileset2.copy(copper);
    m_ForestBoxImage = m_boxTileset3.copy(forest);

    QRect time(12 * TILE_ITEM_SIZE, 18 * TILE_ITEM_SIZE, TILE_ITEM_SIZE, TILE_ITEM_SIZE);
    QRect shuffle(6 * TILE_ITEM_SIZE, 11 * TILE_ITEM_SIZE, TILE_ITEM_SIZE, TILE_ITEM_SIZE);
    QRect hint(11 * TILE_ITEM_SIZE, 19 * TILE_ITEM_SIZE, TILE_ITEM_SIZE, TILE_ITEM_SIZE);
    m_itemTimeImage = m_itemTileset.copy(time);
    m_itemShuffleImage = m_itemTileset.copy(shuffle);
    m_itemHintImage = m_itemTileset.copy(hint);

    m_char1.loadResources(":/images/Actor1.png");
    m_char2.loadResources(":/images/Actor3.png");

}

void GameBoard::newGame(GameMode mode) //根据传入的游戏模式开始新游戏
{
    m_gameMode = mode;
    m_score1 = 0;
    m_score2 = 0;
    m_remainingTime = 120; // 初始时间120秒
    m_comboCount1 = 0;
    m_comboCount2 = 0;

    m_selectedRow1 = -1; // 清除玩家的方块选择
    m_selectedCol1 = -1;
    m_selectedRow2 = -1;
    m_selectedCol2 = -1;

    m_isGameOver = false;
    m_isAnimating = false;
    m_isPaused = false; // 确保游戏开始时不是暂停状态

    // 重新初始化游戏地图
    initGame();

    // 将角色位置重置到初始点
    m_char1.setPosition(1, 1);
    if (m_gameMode == GameMode::TwoPlayer) {
        m_char2.setPosition(1, BOARD_WIDTH - 2);
    }

    m_char1.resetState(); //重置角色状态
    if (m_gameMode == GameMode::TwoPlayer) {
        m_char2.resetState();
    }

    // 停止所有可能正在运行的计时器
    m_animationTimer->stop();
    m_messageClearTimer->stop();
    m_comboTimer1->stop();
    m_comboTimer2->stop();
    m_itemSpawnTimer->stop();
    m_hintTimer->stop();
    m_gameTimer->stop(); // 先停止，再根据需要启动

    // 重新启动游戏运行所必需的计时器
    m_gameTimer->start(1000);       // 主倒计时开始

    m_itemSpawnTimer->start(8000);

    //重置UI和提示信息
    m_statusMessage = "游戏开始!";
    m_messageClearTimer->start(2000);

    // 清除可能残留的动画或高亮数据
    m_linePath.clear();
    m_eliminationBox1 = QPoint(-1, -1);
    m_eliminationBox2 = QPoint(-1, -1);

    m_isHintActive = false;
    m_hintPair1 = QPoint(-1, -1);
    m_hintPair2 = QPoint(-1, -1);
    m_pauseMenuSelection = 0; // 重置暂停菜单的默认选项

    update();
}

bool GameBoard::saveGame() //保存游戏
{
    QFile saveFile("savegame.dat");
    if (!saveFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法创建存档文件！");
        return false;
    }

    QDataStream out(&saveFile);
    out.setVersion(QDataStream::Qt_5_15);

    // 按照顺序写入所有需要保存的数据
    out << m_score1;
    out << m_score2;
    out << m_gameMode;
    out << m_remainingTime;
    out << m_comboCount1;
    out << m_comboCount2;
    out << m_char1.getRow();
    out << m_char1.getCol();
    out << m_char2.getRow();
    out << m_char2.getCol();
    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            out << m_gameMap[r][c];
        }
    }

    saveFile.close();
    QMessageBox::information(this, "成功", "游戏已保存！");
    return true;
}

bool GameBoard::loadGame() //加载游戏
{
    QFile saveFile("savegame.dat");
    if (!saveFile.exists()) {
        QMessageBox::warning(this, "错误", "未找到存档文件！");
        return false;
    }
    if (!saveFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法读取存档文件！");
        return false;
    }

    QDataStream in(&saveFile);
    in.setVersion(QDataStream::Qt_5_15);

    // 停止所有计时器，准备加载
    m_gameTimer->stop();
    m_itemSpawnTimer->stop();

    // 按照和保存时完全相同的顺序读取数据
    in >> m_score1;
    in >> m_score2;
    in >> m_gameMode;
    in >> m_remainingTime;
    in >> m_comboCount1;
    in >> m_comboCount2;

    int m_Row1, m_Col1, m_Row2, m_Col2;
    in >> m_Row1;
    in >> m_Col1;
    in >> m_Row2;
    in >> m_Col2;
    m_char1.setPosition(m_Row1, m_Col1);
    m_char2.setPosition(m_Row2, m_Col2);

    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            in >> m_gameMap[r][c];
        }
    }

    saveFile.close();

    // 加载完成后，恢复游戏状态
    m_isGameOver = false;
    m_isPaused = false;
    m_statusMessage = "游戏已加载";
    m_messageClearTimer->start(2000);

    // 重新启动计时器
    m_gameTimer->start(1000);
    m_itemSpawnTimer->start(8000);

    update();
    return true;
}

void GameBoard::togglePause() //暂停处理
{
    m_isPaused = !m_isPaused;
    if (m_isPaused) {
        // 暂停所有游戏相关计时器
        m_gameTimer->stop();
        m_itemSpawnTimer->stop();
        if (m_comboTimer1->isActive()) {
            m_comboTimer1->stop();
        }
        if (m_comboTimer2->isActive()) {
            m_comboTimer2->stop();
        }
    } else {
        // 恢复计时器
        m_gameTimer->start();
        m_itemSpawnTimer->start();
        // 如果连击数大于0，说明之前在连击状态，恢复计时器
        if (m_comboCount1 > 0) {
            m_comboTimer1->start();
        }
        if (m_comboCount2 > 0) {
            m_comboTimer2->start();
        }
        m_pauseMenuSelection = 0;
    }
    update();
}

bool GameBoard::checkWinCondition() //检查游戏是否获胜
{
    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            if (m_gameMap[r][c] > 0) {
                // 只要找到一个箱子，就说明游戏尚未胜利
                return false;
            }
        }
    }
    // 如果循环结束都没有找到任何箱子，说明游戏胜利
    return true;
}

void GameBoard::handleSelection(int row, int col, int playerIndex) //依据玩家编号 playerIndex来处理选中的 row行 col列处的箱子
{
    // 一个通用的失败处理 lambda 函数
    auto failSelection = [&]() {
        m_statusMessage = "不可消除!";
        if (playerIndex == 1) {
            m_selectedRow1 = -1;
            m_selectedCol1 = -1;
        } else {
            m_selectedRow2 = -1;
            m_selectedCol2 = -1;
        }
        m_messageClearTimer->start(1000);
    };

    if (m_char1.isAnimating() || (m_gameMode == GameMode::TwoPlayer && m_char2.isAnimating()) || m_isAnimating) {
        return;
    }

    int& m_selectedRow = (playerIndex == 1) ? m_selectedRow1 : m_selectedRow2;
    int& m_selectedCol = (playerIndex == 1) ? m_selectedCol1 : m_selectedCol2;
    int& m_otherRow = (playerIndex == 1) ? m_selectedRow2 : m_selectedRow1;
    int& m_otherCol = (playerIndex == 1) ? m_selectedCol2 : m_selectedCol1;

    if (m_selectedRow == -1) { // 首次选择
        m_selectedRow = row;
        m_selectedCol = col;
    } else { // 第二次选择
        if (row == m_selectedRow && col == m_selectedCol) { // 取消选择
            m_selectedRow = -1;
            m_selectedCol = -1;
        } else if (m_gameMap[row][col] == m_gameMap[m_selectedRow][m_selectedCol]) { // 类型相同
            QPoint start(m_selectedCol, m_selectedRow);
            QPoint end(col, row);

            if (GameLogic::findPath(m_gameMap, start, end, m_linePath)) {
                // 成功找到路径
                if(m_otherRow == row && m_otherCol == col){
                    m_otherRow = -1;
                    m_otherCol = -1;
                }

                if(m_otherRow == m_selectedRow && m_otherCol == m_selectedCol){
                    m_otherRow = -1;
                    m_otherCol = -1;
                }

                m_statusMessage = "消除!";
                m_eliminationBox1 = start;
                m_eliminationBox2 = end;
                m_messageClearTimer->start(1000);
                m_isAnimating = true;
                m_selectedRow = -1;
                m_selectedCol = -1;
                m_animationTimer->start(300); // 开始动画
            } else {
                failSelection();
            }
        } else {
            failSelection();
        }
    }
    update();
}

void GameBoard::handlePauseInput(QKeyEvent *event) //暂停时的键盘操作函数
{
    switch (event->key()) {
    case Qt::Key_W: // 上
        m_pauseMenuSelection = (m_pauseMenuSelection - 1 + 4) % 4; // 循环选择
        break;
    case Qt::Key_S: // 下
        m_pauseMenuSelection = (m_pauseMenuSelection + 1) % 4;
        break;
    case Qt::Key_Return: // 确认
    case Qt::Key_Enter:
        switch (m_pauseMenuSelection) {
        case 0: togglePause(); break;                           // 回到游戏
        case 1: saveGame(); togglePause(); break;               // 存档并返回游戏
        case 2: newGame(m_gameMode); break;                               // 重新开始
        case 3: emit backToMainClicked(); m_isPaused = false; break; // 回到主菜单
        }
        break;
    default:
        break;
    }
    update();
}

void GameBoard::activateShuffle() //Shuffle道具实现与激活
{
    QVector<QPoint> boxPositions;
    QVector<int> boxTypes;

    //记录所有箱子的位置和类型
    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            if (m_gameMap[r][c] > 0) {
                boxPositions.append(QPoint(c, r));
                boxTypes.append(m_gameMap[r][c]);
            }
        }
    }

    //打乱箱子类型列表
    std::random_device rd;
    std::mt19937 s(rd());
    std::shuffle(boxTypes.begin(), boxTypes.end(), s);

    //将打乱后的箱子类型放回原来的位置
    for (int i = 0; i < boxPositions.size(); ++i) {
        m_gameMap[boxPositions[i].y()][boxPositions[i].x()] = boxTypes[i];
    }

    if (m_isHintActive) {
        if (!findAndSetHintPair()) {
            onHintTimeout();
        }
    }

    // 如果之前有选中，打乱后取消选择状态
    m_selectedRow1 = -1;
    m_selectedCol1 = -1;
    m_selectedRow2 = -1;
    m_selectedCol2 = -1;

    m_statusMessage = "箱子已打乱!";
    m_messageClearTimer->start(1500);

    update();
}

void GameBoard::activateHint() //Hint道具激活
{
    // 清除可能残留的旧高亮
    m_isHintActive = false;
    m_hintPair1 = QPoint(-1, -1);
    m_hintPair2 = QPoint(-1, -1);

    if (findAndSetHintPair()) {
        m_isHintActive = true;
        m_hintTimer->start(10000); // 10秒后自动关闭
        m_statusMessage = "提示已激活!";
        m_messageClearTimer->start(2000);
    }

    update();
}

bool GameBoard::findAndSetHintPair() //找到一对可消除的箱子
{
    QVector<QPoint> boxes;
    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            if (m_gameMap[r][c] > 0) {
                boxes.append(QPoint(c, r));
            }
        }
    }

    for (int i = 0; i < boxes.size(); ++i) {
        for (int j = i + 1; j < boxes.size(); ++j) {
            QPoint p1 = boxes[i];
            QPoint p2 = boxes[j];
            if (m_gameMap[p1.y()][p1.x()] == m_gameMap[p2.y()][p2.x()]) {
                QVector<QPoint> path;
                if (GameLogic::findPath(m_gameMap, p1, p2, path)) {
                    m_hintPair1 = p1;
                    m_hintPair2 = p2;
                    return true;
                }
            }
        }
    }
    return false;
}

void GameBoard::drawText(QPainter &painter, const QString &text, const QRect &rect, const QFont &font, Qt::Alignment alignment) //描边文字生成函数
{
    QPainterPath path;

    // 使用QFontMetrics获取精确的边界
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(text);
    QRect textBoundingRect = QRect(0, 0, textWidth, fm.height());

    // 根据对齐方式计算初始位置
    QPoint startPoint;
    if (alignment & Qt::AlignLeft) {
        startPoint.setX(rect.left());
    } else if (alignment & Qt::AlignRight) {
        startPoint.setX(rect.right() - textWidth);
    } else { // Center
        startPoint.setX(rect.center().x() - textWidth / 2);
    }
    startPoint.setY(rect.center().y() + fm.ascent() / 2);

    path.addText(startPoint, font, text);

    // 设置画笔（描边）和画刷（填充）
    painter.setPen(QPen(Qt::black, 1.5));
    painter.setBrush(Qt::white);


    painter.drawPath(path);
}

void GameBoard::paintEvent(QPaintEvent *event) //绘制函数
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    paintBoard(painter);
    paintAnimations(painter);

    // 绘制角色
    m_char1.draw(painter);
    if (m_gameMode == GameMode::TwoPlayer) {
        m_char2.draw(painter);
    }
    paintUI(painter);

    if (m_isPaused) {
        paintPauseScreen(painter);
    }
}

void GameBoard::paintBoard(QPainter &painter) //地图绘制函数
{
    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            QRect tileRect(c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE);

            // 先绘制该格子的地面
            painter.drawPixmap(tileRect, m_groundImage);

            // 再绘制覆盖物（墙/箱子/道具）
            TileType tile = static_cast<TileType>(m_gameMap[r][c]);
            switch (tile) {
            case TileType::WALL:        painter.drawPixmap(tileRect, m_wallImage); break;
            case TileType::BOX_PAPER:   painter.drawPixmap(tileRect, m_PaperBoxImage); break;
            case TileType::BOX_WOOD:    painter.drawPixmap(tileRect, m_WoodBoxImage); break;
            case TileType::BOX_STEEL:   painter.drawPixmap(tileRect, m_SteelBoxImage); break;
            case TileType::BOX_COPPER:  painter.drawPixmap(tileRect, m_CopperBoxImage); break;
            case TileType::BOX_FOREST:  painter.drawPixmap(tileRect, m_ForestBoxImage); break;
            case TileType::ITEM_TIME:   painter.drawPixmap(tileRect, m_itemTimeImage); break;
            case TileType::ITEM_SHUFFLE:painter.drawPixmap(tileRect, m_itemShuffleImage); break;
            case TileType::ITEM_HINT:   painter.drawPixmap(tileRect, m_itemHintImage); break;
            // 空地无需额外绘制
            case TileType::EMPTY:       break;
            default: break;
            }
        }
    }
}

void GameBoard::paintAnimations(QPainter &painter) //动画绘制函数
{
    // 绘制选中高亮
    auto drawSelectionHighlight = [&](int row, int col, const QColor &color) {
        if (row != -1 && col != -1) {
            QPen oldPen = painter.pen();
            painter.setPen(QPen(color, 4)); // 设置线宽为4，颜色根据传入参数变化
            painter.setBrush(Qt::NoBrush); // 无填充
            painter.drawRect(col * TILE_SIZE + 2, row * TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4);
            painter.setPen(oldPen);
        }
    };

    bool isSharedSelection = (m_selectedRow1 == m_selectedRow2 && m_selectedCol1 == m_selectedCol2) && (m_selectedRow1 != -1); // 确保不是无效选择

    if (isSharedSelection) {
        // 绘制双人共同选中的特殊效果
        int row = m_selectedRow1;
        int col = m_selectedCol1;
        drawSelectionHighlight(row, col, QColor(255, 0, 180, 220)); //紫色
    }
    else {
        // 正常分别绘制
        drawSelectionHighlight(m_selectedRow1, m_selectedCol1, QColor(0, 255, 0, 200)); // 绿色
        drawSelectionHighlight(m_selectedRow2, m_selectedCol2, QColor(255, 0, 0, 200)); // 红色
    }

    // 绘制提示高亮
    if (m_isHintActive && m_hintPair1.x() != -1) {
        QPen oldPen = painter.pen();
        painter.setPen(QPen(QColor(0, 255, 255, 200), 4));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(m_hintPair1.x() *TILE_SIZE + 2, m_hintPair1.y() *TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4);
        painter.drawRect(m_hintPair2.x() *TILE_SIZE + 2, m_hintPair2.y() *TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4);
        painter.setPen(oldPen);
    }

    // 绘制消除动画线条
    if (m_isAnimating && m_linePath.size() > 1) {
        QPen oldPen = painter.pen();
        QPen linePen(Qt::yellow, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(linePen);

        for (int i = 0; i < m_linePath.size() - 1; ++i) {
            QPoint p1 = m_linePath[i] * TILE_SIZE + QPoint(TILE_SIZE / 2, TILE_SIZE / 2);
            QPoint p2 = m_linePath[i+1] * TILE_SIZE + QPoint(TILE_SIZE / 2, TILE_SIZE / 2);
            painter.drawLine(p1, p2);
        }
        painter.setPen(oldPen);
    }

}

void GameBoard::paintUI(QPainter &painter) //UI界面绘制函数
{
    QFont uiFont;
    uiFont.setPointSize(14);
    uiFont.setBold(true);

    //绘制分数 (左上角)
    if (m_gameMode == GameMode::SinglePlayer) {
        // 单人模式UI
        QRect scoreRect(5, 5, width() / 4, 30);
        drawText(painter, QString("分数: %1").arg(m_score1), scoreRect, uiFont, Qt::AlignLeft);
    } else {
        // 双人模式UI
        QRect score1Rect(5, 5, width() / 4, 30);
        drawText(painter, QString("P1: %1").arg(m_score1), score1Rect, uiFont, Qt::AlignLeft);

        QRect score2Rect(width() / 2, 5, width() / 4, 30);
        drawText(painter, QString("P2: %1").arg(m_score2), score2Rect, uiFont, Qt::AlignCenter);
    }

    //绘制倒计时 (右上角)
    QRect timeRect(width() * 3 / 4 - 5, 5, width() / 4, 30);
    int minutes = m_remainingTime / 60;
    int seconds = m_remainingTime % 60;
    QString timeText = QString("时间: %1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    drawText(painter, timeText, timeRect, uiFont, Qt::AlignRight);


    //绘制状态消息 (顶部中央)
    if (!m_statusMessage.isEmpty()) {
        int msgHeight = this->height() / 4;
        QRect textRect(0, 0, this->width(), msgHeight);

        QFont statusFont;
        statusFont.setPointSize(16);
        statusFont.setBold(true);
        drawText(painter, m_statusMessage, textRect, statusFont, Qt::AlignCenter);
    }
}

void GameBoard::paintPauseScreen(QPainter &painter) //暂停界面绘制函数
{
    if (m_isPaused) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制半透明黑色背景
        painter.fillRect(this->rect(), QColor(0, 0, 0, 150));

        // 绘制菜单项
        QFont font("Arial", 20, QFont::Bold);
        painter.setFont(font);

        QStringList menuItems = {"回到游戏", "存档", "重新开始", "回到主菜单"};

        int itemHeight = 60;
        int startY = (height() - menuItems.size() * itemHeight) / 2;

        for (int i = 0; i < menuItems.size(); ++i) {
            QRect itemRect(0, startY + i * itemHeight, width(), itemHeight);
            if (i == m_pauseMenuSelection) {
                painter.setPen(Qt::yellow); // 高亮当前选项
            } else {
                painter.setPen(Qt::white);
            }
            painter.drawText(itemRect, Qt::AlignCenter, menuItems[i]);
        }
    }
}

void GameBoard::keyPressEvent(QKeyEvent *event) //键盘输入处理
{
    // ESC 和暂停处理
    if (event->key() == Qt::Key_Escape) {
        togglePause();
        return;
    }
    if (m_isPaused) {
        handlePauseInput(event);
        return;
    }

    // 获取按键对应的玩家和方向
    PlayerInfo info = getPlayerByKeyEvent(event);
    if (!info.isValid) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:    m_keyWPressed = true;    break;
    case Qt::Key_S:    m_keySPressed = true;    break;
    case Qt::Key_A:    m_keyAPressed = true;    break;
    case Qt::Key_D:    m_keyDPressed = true;    break;
    case Qt::Key_Up:   m_keyUpPressed = true;   break;
    case Qt::Key_Down: m_keyDownPressed = true; break;
    case Qt::Key_Left: m_keyLeftPressed = true; break;
    case Qt::Key_Right:m_keyRightPressed = true;break;
    }

    Character& character = *info.character;
    character.setDirection(info.direction);

    update();
}

void GameBoard::keyReleaseEvent(QKeyEvent *event)
{
    if (m_isGameOver || m_isPaused) {
        return;
    }

    PlayerInfo info = getPlayerByKeyEvent(event);
    if (!info.isValid) {
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:    m_keyWPressed = false;    break;
    case Qt::Key_S:    m_keySPressed = false;    break;
    case Qt::Key_A:    m_keyAPressed = false;    break;
    case Qt::Key_D:    m_keyDPressed = false;    break;
    case Qt::Key_Up:   m_keyUpPressed = false;   break;
    case Qt::Key_Down: m_keyDownPressed = false; break;
    case Qt::Key_Left: m_keyLeftPressed = false; break;
    case Qt::Key_Right:m_keyRightPressed = false;break;
    }

    Character& character = *info.character;
    Direction dir = info.direction;

    if (character.getDirection() == dir) {
        // 如果角色正在移动，不清除方向
        if (!character.isMoving() && !character.isAnimating()) {
            character.setDirection(NONE);
        }
        //  否则，保持方向（即使按键松开，也要保持面向）
    }

}

bool GameBoard::isCharacterInputAllowed(const Character& character) const
{
    return !character.isAnimating() && !m_isGameOver && !m_isAnimating;
}

PlayerInfo GameBoard::getPlayerByKeyEvent(QKeyEvent* event) //获取操作玩家及其朝向
{
    PlayerInfo info;

    switch (event->key()) {
    case Qt::Key_W:
        info = { &m_char1, 1, true, UP };
        break;
    case Qt::Key_S:
        info = { &m_char1, 1, true, DOWN };
        break;
    case Qt::Key_A:
        info = { &m_char1, 1, true, LEFT };
        break;
    case Qt::Key_D:
        info = { &m_char1, 1, true, RIGHT };
        break;

    case Qt::Key_Up:
        if (m_gameMode == GameMode::TwoPlayer) {
            info = { &m_char2, 2, true, UP };
        }
        break;
    case Qt::Key_Down:
        if (m_gameMode == GameMode::TwoPlayer) {
            info = { &m_char2, 2, true, DOWN };
        }
        break;
    case Qt::Key_Left:
        if (m_gameMode == GameMode::TwoPlayer) {
            info = { &m_char2, 2, true, LEFT };
        }
        break;
    case Qt::Key_Right:
        if (m_gameMode == GameMode::TwoPlayer) {
            info = { &m_char2, 2, true, RIGHT };
        }
        break;

    default:
        info.isValid = false;
        break;
    }

    return info;
}

void GameBoard::processQueuedMoveForCharacter(Character& character, int playerIndex)
{
    // 只有当角色静止时才检查是否该移动
    if (character.isAnimating() || character.isMoving()) {
        return;
    }

    Direction dir = character.getDirection();
    if (dir == NONE) return;

    bool keyHeld = false;
    if (playerIndex == 1) {
        switch (dir) {
        case UP:    keyHeld = m_keyWPressed;    break;
        case DOWN:  keyHeld = m_keySPressed;    break;
        case LEFT:  keyHeld = m_keyAPressed;    break;
        case RIGHT: keyHeld = m_keyDPressed;    break;
        case NONE: break;
        }
    } else if (playerIndex == 2 && m_gameMode == GameMode::TwoPlayer) {
        switch (dir) {
        case UP:    keyHeld = m_keyUpPressed;    break;
        case DOWN:  keyHeld = m_keyDownPressed;  break;
        case LEFT:  keyHeld = m_keyLeftPressed;  break;
        case RIGHT: keyHeld = m_keyRightPressed; break;
        case NONE: break;
        }
    }

    if (!keyHeld) {
        return; // 按键已松开，不继续移动
    }

    int curRow = character.getRow();
    int curCol = character.getCol();
    int targetRow = curRow;
    int targetCol = curCol;

    switch (dir) {
    case UP:    targetRow--; break;
    case DOWN:  targetRow++; break;
    case LEFT:  targetCol--; break;
    case RIGHT: targetCol++; break;
    case NONE:  return;
    }

    // 边界检查
    if (targetRow < 0 || targetRow >= BOARD_HEIGHT ||
        targetCol < 0 || targetCol >= BOARD_WIDTH) {
        return;
    }

    // 双人模式：检查是否被另一角色挡住
    if (m_gameMode == GameMode::TwoPlayer) {
        Character& otherChar = (&character == &m_char1) ? m_char2 : m_char1;
        if (otherChar.getRow() == targetRow && otherChar.getCol() == targetCol) {
            return; // 被挡住，不能移动
        }
    }

    int targetTile = m_gameMap[targetRow][targetCol];

    // 空地
    if (targetTile == EMPTY) {
        character.move();
        return;
    }

    //道具
    if (targetTile == ITEM_TIME || targetTile == ITEM_SHUFFLE || targetTile == ITEM_HINT) {
        character.move();
        applyItemEffect(targetTile);
        m_gameMap[targetRow][targetCol] = EMPTY;
        return;
    }

    // 箱子 → 选择
    if (targetTile > 0) {
        QPair<int, int> pos(targetRow, targetCol);

        if (!m_recentlySelectedBoxes.contains(pos)) {
            m_currentPlayerTurn = playerIndex;
            handleSelection(targetRow, targetCol, playerIndex);
            m_recentlySelectedBoxes.insert(pos);

            QTimer::singleShot(400, [pos, this]() {
                m_recentlySelectedBoxes.remove(pos);
            });
        }
        return;
    }

}

void GameBoard::updateCharacters()
{
    // 更新动画帧和位置
    m_char1.updatePosition();
    m_char1.updateAnimation();

    if (m_gameMode == GameMode::TwoPlayer) {
        m_char2.updatePosition();
        m_char2.updateAnimation();
    }

    // 处理缓存方向并尝试继续移动
    processQueuedMoveForCharacter(m_char1, 1);
    if (m_gameMode == GameMode::TwoPlayer) {
        processQueuedMoveForCharacter(m_char2, 2);
    }

    update(); // 触发重绘
}

void GameBoard::onAnimationFinished() //消除及连击逻辑
{

    m_isAnimating = false;
    m_linePath.clear();

    m_gameMap[m_eliminationBox1.y()][m_eliminationBox1.x()] = 0;
    m_gameMap[m_eliminationBox2.y()][m_eliminationBox2.x()] = 0;

    int baseScore = 10;                     // 每次消除的基础分


    int &comboRef = (m_currentPlayerTurn == 1) ? m_comboCount1 : m_comboCount2;


    if (m_currentPlayerTurn == 1) {
        m_score1 += baseScore + 5 * comboRef;
    } else if (m_currentPlayerTurn == 2) {
        m_score2 += baseScore + 5 * comboRef;
    }

    if (comboRef > 0) {
        // 如果在连击状态下，显示连击提示
        m_statusMessage = QString("Combo x%1! +%2").arg(comboRef + 1).arg(baseScore + 5 * comboRef);
        m_messageClearTimer->start(1000);
    }

    comboRef++;
    if (m_currentPlayerTurn == 1) {
        m_comboTimer1->start(3000);
    } else if (m_currentPlayerTurn == 2) {
        m_comboTimer2->start(3000);
    }            // 重新开始一个3秒的计时窗口，等待下一次消除

    if (m_isHintActive) {
        if (!findAndSetHintPair()) {
            onHintTimeout();
        }
    }

    if (checkWinCondition()) {
        m_isGameOver = true;
        if(m_gameMode == GameMode::TwoPlayer){
            if(m_score1 > m_score2){
                m_statusMessage = "玩家一胜利！";
            }
            if(m_score1 < m_score2){
                m_statusMessage = "玩家二胜利！";
            }
            if(m_score1 == m_score2){
                m_statusMessage = "平局！";
            }
        }else{
        m_statusMessage = "恭喜你，游戏胜利！";
        }
        m_gameTimer->stop();
        m_itemSpawnTimer->stop();

        showGameOverDialog();
        }
    else if (!GameLogic::isMapSolvable(m_gameMap)) {
        //如果没赢，就检查是否还有可消除的解
        m_isGameOver = true;
        if(m_gameMode == GameMode::TwoPlayer){
            if(m_score1 > m_score2){
                m_statusMessage = "无解，玩家一胜利！";
            }
            if(m_score1 < m_score2){
                m_statusMessage = "无解，玩家二胜利！";
            }
            if(m_score1 == m_score2){
                m_statusMessage = "无解，平局！";
            }
        }else{
        m_statusMessage = "无解! 游戏结束!";
        }
        m_gameTimer->stop();
        m_itemSpawnTimer->stop();
        m_comboTimer1->stop();
        m_comboTimer2->stop();

        showGameOverDialog();
    }
    update();
}

void GameBoard::clearStatusMessage() //清除信息
{
    if (!m_isGameOver) {
        m_statusMessage.clear();
        update();
    }
}

void GameBoard::onGameTick() //倒计时结束
{
    if (m_isGameOver) {
        return;
    }

    m_remainingTime--;

    if (m_remainingTime <= 0) {
        m_remainingTime = 0;
        m_isGameOver = true;
        if(m_gameMode == GameMode::TwoPlayer){
            if(m_score1 > m_score2){
                m_statusMessage = "时间到，玩家一胜利！";
            }
            if(m_score1 < m_score2){
                m_statusMessage = "时间到，玩家二胜利！";
            }
            if(m_score1 == m_score2){
                m_statusMessage = "时间到，平局！";
            }
        }else{
        m_statusMessage = "时间到！游戏结束！";
            }
        m_gameTimer->stop();
        m_itemSpawnTimer->stop();
        m_comboTimer1->stop(); // 停止所有游戏相关计时
        m_comboTimer2->stop();

        showGameOverDialog();
    }

    update();
}

void GameBoard::onComboTimeout1() //玩家一连击清零
{
    m_comboCount1 = 0;
}

void GameBoard::onComboTimeout2() //玩家二连击清零
{
    m_comboCount2 = 0;
}


void GameBoard::onHintTimeout() //提示结束
{
    m_isHintActive = false;
    m_hintPair1 = QPoint(-1, -1);
    m_hintPair2 = QPoint(-1, -1);
    update(); // 刷新界面，清除高亮
}

void GameBoard::onItemSpawnCheck() //道具生成
{
    if (m_isGameOver) {
        m_itemSpawnTimer->stop();
        return;
    }

    if (QRandomGenerator::global()->bounded(100) < 30) {
        //找出所有可以放置道具的空地
        QVector<QPoint> emptyTiles;
        for (int r = 0; r < BOARD_HEIGHT; ++r) {
            for (int c = 0; c < BOARD_WIDTH; ++c) {
                if (m_gameMap[r][c] == TileType::EMPTY) {
                    emptyTiles.append(QPoint(c, r));
                }
            }
        }

        //如果有空地，就随机选一个位置
        if (!emptyTiles.isEmpty()) {
            QPoint spawnPos = emptyTiles.at(QRandomGenerator::global()->bounded(emptyTiles.size()));

            //随机决定生成哪种道具
            int itemTypeRoll = QRandomGenerator::global()->bounded(3);
            TileType newItem;
            switch (itemTypeRoll) {
            case 0: newItem = TileType::ITEM_TIME; break;
            case 1: newItem = TileType::ITEM_SHUFFLE; break;
            case 2: newItem = TileType::ITEM_HINT; break;
            }

            //在地图上放置道具
            m_gameMap[spawnPos.y()][spawnPos.x()] = newItem;

            update();
        }
    }
}

void GameBoard::applyItemEffect(int itemValue)
{
    switch (itemValue) {
    case TileType::ITEM_TIME:
        m_remainingTime += 30;
        m_statusMessage = "获得额外时间！+30秒";
        m_messageClearTimer->start(1000);
        break;
    case TileType::ITEM_SHUFFLE:
        activateShuffle();
        m_statusMessage = "箱子已打乱！";
        m_messageClearTimer->start(1000);
        break;
    case TileType::ITEM_HINT:
        activateHint();
        m_statusMessage = "获得提示！";
        m_messageClearTimer->start(1000);
        break;
    default:
        break;
    }
}

void GameBoard::showGameOverDialog() //游戏结束窗口
{
    QMessageBox gameOverBox(this);
    gameOverBox.setWindowTitle("游戏结束");
    gameOverBox.setText(m_statusMessage);

    // 创建三个自定义按钮
    QPushButton *restartButton = gameOverBox.addButton("重新开始", QMessageBox::ActionRole);
    QPushButton *menuButton    = gameOverBox.addButton("返回主菜单", QMessageBox::ActionRole);
    QPushButton *quitButton    = gameOverBox.addButton("退出游戏", QMessageBox::RejectRole);

    gameOverBox.exec();

    if (gameOverBox.clickedButton() == restartButton) {
        newGame(m_gameMode); // 重新开始当前模式
    }
    else if (gameOverBox.clickedButton() == menuButton) {
        emit gameFinished();
    }
    else if (gameOverBox.clickedButton() == quitButton) {
        QApplication::quit();
    }
}


