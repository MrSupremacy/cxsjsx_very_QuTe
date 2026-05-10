#ifndef ENEMY_H
#define ENEMY_H

// Enemy.h
#include <QGraphicsRectItem>

class Enemy : public QGraphicsRectItem {
public:
    Enemy(QGraphicsItem *target); // 传入玩家作为目标
    void moveTowardsTarget(); // 朝玩家移动

private:
    QGraphicsItem *playerTarget; // 玩家
    double speed = 3.0f; // 敌人速度
};

#endif // ENEMY_H
