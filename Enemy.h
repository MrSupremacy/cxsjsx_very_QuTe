#ifndef ENEMY_H
#define ENEMY_H

// Enemy.h
#include <QGraphicsRectItem>

class Enemy : public QGraphicsRectItem {
public:
    Enemy(QGraphicsItem *target); // 传入玩家作为目标
    void moveTowardsTarget(); // 朝玩家移动
    void teleportThroughWall(); // 通过边界墙瞬移

private:
    QGraphicsItem *playerTarget; // 玩家
    double speed = 0.6f; // 敌人速度
};

#endif // ENEMY_H
