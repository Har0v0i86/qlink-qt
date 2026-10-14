#include "Character.h"
#include "GameBoard.h"
#include <qdebug.h>

QTime Character::s_globalTime = QTime::currentTime();

Character::Character(const QString &tilesetPath):
    m_charRow(1),
    m_charCol(1),
    m_x(1 * TILE_SIZE),     // 初始像素位置
    m_y(1 * TILE_SIZE),
    m_direction(DOWN),
    m_facingDirection(DOWN),
    m_currentFrame(0),
    m_frameCount(3),
    m_isMoving(false),
    m_isAnimating(false),
    m_moveSpeedPixels(4),
    m_moveAnimationDuration(240),
    m_lastFrameTime(0),
    m_frameInterval(80),
    m_moveTimer(new QTimer(this)),
    m_tilesetPath(":/images/Actor1.png")
{
    m_moveTimer->setSingleShot(true);
    connect(m_moveTimer, &QTimer::timeout, this, [this]() {
        m_isMoving = false;
    });
    loadResources();
}

void Character::loadResources() // 使用成员变量中的路径
{
    loadResources(m_tilesetPath);
}

void Character::loadResources(const QString &tilesetPath) //使用指定路径的加载函数
{
    const int TILE_SPRITE_SIZE = 48;
    const int FRAME_COUNT = 3;
    m_charTileset.load(tilesetPath);

    m_framesDown.clear();
    m_framesLeft.clear();
    m_framesRight.clear();
    m_framesUp.clear();

    for (int i = 0; i < FRAME_COUNT; ++i) {

        QRect down(i * TILE_SPRITE_SIZE, 4 * TILE_SPRITE_SIZE, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
        m_framesDown.append(m_charTileset.copy(down));

        QRect left(i * TILE_SPRITE_SIZE, 5 * TILE_SPRITE_SIZE, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
        m_framesLeft.append(m_charTileset.copy(left));

        QRect right(i * TILE_SPRITE_SIZE, 6 * TILE_SPRITE_SIZE, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
        m_framesRight.append(m_charTileset.copy(right));

        QRect up(i * TILE_SPRITE_SIZE, 7 * TILE_SPRITE_SIZE, TILE_SPRITE_SIZE, TILE_SPRITE_SIZE);
        m_framesUp.append(m_charTileset.copy(up));
    }

}

void Character::setPosition(int Row,int Col) //将角色所在行列设为 Row行 Col列
{
    m_charRow = Row;
    m_charCol = Col;
    m_x = Col * TILE_SIZE;  // 同步更新
    m_y = Row * TILE_SIZE;
}

void Character::setDirection(Direction dir) //将角色朝向设为 dir
{
    if (m_isAnimating) {
        // 如果正在动画中，缓存方向，等待动画结束处理
        m_queuedDirection = dir;
        m_hasQueuedDirection = true;
    } else {
        // 否则立即设置
        m_direction = dir;
        m_hasQueuedDirection = false;
    }
    if (dir != NONE) {
        m_facingDirection = dir; // 更新面向
    }
}

void Character::move() //按朝向进行移动
{
    if (m_isAnimating) {
        return; // 正在移动中，忽略新指令
    }

    int newRow = m_charRow;
    int newCol = m_charCol;

    m_isAnimating = true;
    m_isMoving = true; // 用于播放脚步动画

    switch (m_direction) {
    case UP:    newRow--; break;
    case DOWN:  newRow++; break;
    case LEFT:  newCol--; break;
    case RIGHT: newCol++; break;
    case NONE:  return;
    }

    // 如果目标是同一个格子，不移动
    if (newRow == m_charRow && newCol == m_charCol) {
        m_isMoving = false;
        m_moveTimer->stop();
        return;
    }

    // 开始滑动动画
    m_moveTimer->start(240);

    // 记录目标格子
    m_targetRow = newRow;
    m_targetCol = newCol;
}

int Character::getRow() const //返回角色所在行
{
    return m_charRow;
}

int Character::getCol() const //返回角色所在列
{
    return m_charCol;
}

Direction Character::getDirection() const //返回角色朝向
{
    return m_direction;
}

void Character::draw(QPainter &painter) //角色绘制函数
{

    QPixmap currentFrameImage;

    int frameIndex = m_currentFrame;

    if (!m_isMoving) {
        frameIndex = 1; // 第二帧
    }

    Direction drawDir = (m_direction == NONE) ? m_facingDirection : m_direction;

    switch (drawDir) {
    case UP:
        currentFrameImage = m_framesUp[frameIndex];
        break;
    case DOWN:
        currentFrameImage = m_framesDown[frameIndex];
        break;
    case LEFT:
        currentFrameImage = m_framesLeft[frameIndex];
        break;
    case RIGHT:
        currentFrameImage = m_framesRight[frameIndex];
        break;
    default:
        currentFrameImage = m_framesDown[1]; // 默认使用向下站立帧（第二帧）
        break;
    }

    painter.drawPixmap(static_cast<int>(m_x), static_cast<int>(m_y), TILE_SIZE, TILE_SIZE, currentFrameImage);
}

void Character::updateAnimation()
{

    int currentTime = s_globalTime.msecsTo(QTime::currentTime());

    // 只有在 m_isMoving 且时间到了才切帧
    if (m_isMoving && (currentTime - m_lastFrameTime) >= m_frameInterval) {
        m_currentFrame = (m_currentFrame + 1) % m_frameCount;
        m_lastFrameTime = currentTime;
    }

    // 如果没有在移动，显示站立帧
    if (!m_isMoving) {
        m_currentFrame = 1;
    }

}

void Character::updatePosition()
{
    if (!m_isAnimating) {
        return;
    }
    double targetX = m_targetCol * TILE_SIZE;
    double targetY = m_targetRow * TILE_SIZE;

    // 计算差值
    double dx = targetX - m_x;
    double dy = targetY - m_y;


    if (qFabs(dx) < m_moveSpeedPixels && qFabs(dy) < m_moveSpeedPixels) {
        m_x = targetX;
        m_y = targetY;
        m_charRow = m_targetRow;
        m_charCol = m_targetCol;
        m_isAnimating = false;
        m_isMoving = false;

        // 动画结束后处理缓存方向
        if (m_hasQueuedDirection) {
            m_direction = m_queuedDirection;
            m_hasQueuedDirection = false;
        }

    }else {
        // 否则向目标移动一步
        if (dx > 0) m_x += m_moveSpeedPixels;
        else if (dx < 0) m_x -= m_moveSpeedPixels;

        if (dy > 0) m_y += m_moveSpeedPixels;
        else if (dy < 0) m_y -= m_moveSpeedPixels;
    }

}

void Character::resetState()
{
    m_isMoving = false;
    m_isAnimating = false;
    setDirection(DOWN);
    m_moveTimer->stop();
    m_currentFrame = 1; // 站立帧
}
