#include "GameLogic.h"
#include <queue>
#include <map>

using namespace std;

bool GameLogic::findPath(const int (&originalMap)[BOARD_HEIGHT][BOARD_WIDTH], const QPoint& start, const QPoint& end, QVector<QPoint>& path) //寻找规定转折数以下的最短消除路径
{
    // 创建一个临时地图副本，并将起点视为空地
    int gameMap[BOARD_HEIGHT][BOARD_WIDTH];
    copy(&originalMap[0][0], &originalMap[0][0] + BOARD_WIDTH * BOARD_HEIGHT, &gameMap[0][0]);

    gameMap[start.y()][start.x()] = 0;
    gameMap[end.y()][end.x()] = 0;

    using State = pair<QPoint, pair<int, int>>;
    queue<State> q;
    vector<vector<int>> visited(BOARD_HEIGHT, vector<int>(BOARD_WIDTH, 4));
    vector<vector<QPoint>> parent(BOARD_HEIGHT, vector<QPoint>(BOARD_WIDTH, QPoint(-1,-1)));

    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};

    for (int i = 0; i < 4; ++i) { q.push({start, {i, 0}}); }
    visited[start.y()][start.x()] = 0;

    while (!q.empty()) {
        State current = q.front(); q.pop();
        QPoint pos = current.first; int dir = current.second.first; int turns = current.second.second;


        if (pos == end) {
            path.clear();
            QPoint p = end;
            while(p != QPoint(-1,-1)) {
                path.prepend(p);
                if (p.x() >= 0 && p.y() >= 0 && parent[p.y()][p.x()] == p) break;
                p = parent[p.y()][p.x()];
            }
            return true;
        }

        for(int i = 0; i < 4; ++i) {
            QPoint nextPos(pos.x() + dx[i], pos.y() + dy[i]);
            int nextTurns = turns + (i == dir ? 0 : 1);
            if (nextTurns > 2) continue;

            if (nextPos.x() >= 0 && nextPos.x() < BOARD_WIDTH && nextPos.y() >= 0 && nextPos.y() < BOARD_HEIGHT) {

                if ((gameMap[nextPos.y()][nextPos.x()] == 0 || gameMap[nextPos.y()][nextPos.x()] < -1) && nextTurns < visited[nextPos.y()][nextPos.x()]) {
                    visited[nextPos.y()][nextPos.x()] = nextTurns;
                    parent[nextPos.y()][nextPos.x()] = pos;
                    q.push({nextPos, {i, nextTurns}});
                }
            }
        }
    }
    return false;
}
bool GameLogic::isMapSolvable(const int (&gameMap)[BOARD_HEIGHT][BOARD_WIDTH]) //检测地图是否有解
{
    // 1. 将所有现存的箱子按类型分组
    map<int, QVector<QPoint>> boxPositions;
    for (int r = 0; r < BOARD_HEIGHT; ++r) {
        for (int c = 0; c < BOARD_WIDTH; ++c) {
            if (gameMap[r][c] > 0) { // 如果是箱子
                boxPositions[gameMap[r][c]].append(QPoint(c, r));
            }
        }
    }

    // 2. 遍历每一种类型的箱子
    for (auto const& [type, positions] : boxPositions) {
        if (positions.size() < 2) continue;

        // 3. 对该类型的每一对箱子进行路径查找
        for (int i = 0; i < positions.size() - 1; ++i) {
            for (int j = i + 1; j < positions.size(); ++j) {
                QVector<QPoint> dummyPath;
                if (findPath(gameMap, positions[i], positions[j], dummyPath)) {
                    return true; // 只要找到一对可消除的，地图就是可解的
                }
            }
        }
    }

    return false; // 遍历完所有对，都无法消除，则无解
}
