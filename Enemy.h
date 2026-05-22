// Created by 樊轩楷

#ifndef ENEMY_H
#define ENEMY_H

// Enemy.h
#include <QGraphicsEllipseItem>

class Enemy : public QGraphicsEllipseItem {
public:
    Enemy(QGraphicsItem *target); // 传入玩家作为目标
    void moveTowardsTarget(); // 朝玩家移动
    void teleportThroughWall(); // 通过边界墙瞬移
    void setInFormation(bool state) { inFormation = state; } // 修改阵型状态
    bool isInFormation() const { return inFormation; }
    void applyScatter(qreal vx, qreal vy, int frames); // 接收动量函数
private:
    QGraphicsItem *playerTarget; // 玩家
    double speed = 0.6f; // 敌人速度
    bool inFormation = false; // 是否在阵型

    qreal scatterVx = 0.0; // 动量x分量
    qreal scatterVy = 0.0; // 动量y分量
    int scatterFrames = 0; // 持续帧数
};

#endif // ENEMY_H
