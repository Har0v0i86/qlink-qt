#pragma once

#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QTime>
#include <QObject>

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

class Character : public QObject
{
    Q_OBJECT

public:
    Character(const QString &tilesetPath = ":/images/Actor1.png");

    void loadResources(const QString &tilesetPath);
    void loadResources();
    void resetState();

    void setPosition(int Row, int Col);
    void setDirection(Direction dir);
    void move();

    int getRow() const;
    int getCol() const;
    Direction getDirection() const;

    void draw(QPainter &painter);
    void updateAnimation();
    void updatePosition(); // 每帧更新浮点位置
    bool isAnimating() const { return m_isAnimating; }
    bool isMoving() const { return m_isMoving; }

private:
    static QTime s_globalTime;

    int m_charRow,  m_charCol;
    int m_targetRow, m_targetCol;
    double m_x, m_y;
    Direction m_direction;
    Direction m_queuedDirection = NONE;
    Direction m_facingDirection; //面向方向
    bool m_hasQueuedDirection = false;

    QPixmap m_charTileset;
    QVector<QPixmap> m_framesUp;
    QVector<QPixmap> m_framesDown;
    QVector<QPixmap> m_framesLeft;
    QVector<QPixmap> m_framesRight;

    QString m_tilesetPath;

    int m_currentFrame;           // 当前动画帧索引
    int m_frameCount;             // 动画帧数量
    bool m_isMoving;              // 是否正在移动
    bool m_isAnimating;         // 是否正在播放移动动画
    double m_moveSpeedPixels;      // 每次 update 移动的像素
    int m_moveAnimationDuration;  // 动画持续时间
    int m_lastFrameTime;      // 上次切帧的时间（ms）
    int m_frameInterval;      // 帧间隔
    QTimer* m_moveTimer;

};
