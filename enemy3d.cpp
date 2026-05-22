// Created by 吉佑安

#include "enemy3d.h"
#include <cmath> // 使用 <cmath> 而不是 <math.h>

Enemy3D::Enemy3D(QGraphicsItem *target, int mapW, int mapH)
    : playerTarget(target), mapWidth(mapW), mapHeight(mapH)
{
    setRect(0, 0, 6, 6);
}

void Enemy3D::moveTowardsTarget() {
    if (!playerTarget) return;

    qreal px = playerTarget->x();
    qreal py = playerTarget->y();
    qreal ex = this->x();
    qreal ey = this->y();

    qreal dx = px - ex;
    // 如果穿过边界走更近
    if (std::abs(dx) > mapWidth / 2.0) {
        dx = (dx > 0) ? dx - mapWidth : dx + mapWidth;
    }

    qreal dy = py - ey;
    // 如果穿过边界走更近
    if (std::abs(dy) > mapHeight / 2.0) {
        dy = (dy > 0) ? dy - mapHeight : dy + mapHeight;
    }

    qreal distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0) {
        qreal moveX = (dx / distance) * speed;
        qreal moveY = (dy / distance) * speed;
        moveBy(moveX, moveY);
    }
}
