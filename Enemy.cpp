#include "Enemy.h"
#include <math.h>

Enemy::Enemy(QGraphicsItem *target) {
    playerTarget = target;
    setRect(0, 0, 6, 6); // 设置敌人大小
}

void Enemy::moveTowardsTarget() {
    if (!playerTarget) return;

    // 获取玩家和敌人的坐标
    qreal px = playerTarget->x();
    qreal py = playerTarget->y();
    qreal ex = this->x();
    qreal ey = this->y();

    // 计算差值
    qreal dx = px - ex;
    qreal dy = py - ey;

    // 计算距离
    qreal distance = sqrt(dx * dx + dy * dy);

    // 按比例移动 (归一化向量 * 速度)
    if (distance > 0) {
        qreal moveX = (dx / distance) * speed;
        qreal moveY = (dy / distance) * speed;

        this->moveBy(moveX, moveY);
    }
}