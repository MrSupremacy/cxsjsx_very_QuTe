#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>

class Player: public QGraphicsRectItem {
public:
    Player(); // 构造函数

    void move(bool w, bool a, bool s, bool d, bool up, bool left, bool down, bool right); // 玩家移动函数

private:
    double speed = 5.0f; // 玩家移动速度
};

#endif // PLAYER_H
