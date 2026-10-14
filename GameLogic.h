#pragma once

#include <QVector>
#include <QPoint>
#include "GameBoard.h"

class GameLogic
{
public:
    static bool findPath(const int (&GameMap)[BOARD_HEIGHT][BOARD_WIDTH], const QPoint& start, const QPoint& end, QVector<QPoint>& path);
    static bool isMapSolvable(const int (&GameMap)[BOARD_HEIGHT][BOARD_WIDTH]);
};
