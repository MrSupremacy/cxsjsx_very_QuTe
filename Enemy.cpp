// Created by 樊轩楷

#include "Enemy.h"
#include <math.h>
#include <QGraphicsScene>
#include <QDebug>

Enemy::Enemy(QGraphicsItem *target) {
    playerTarget = target;
    setRect(0, 0, 8, 8); // 设置敌人大小
    setBrush(QBrush(Qt::red)); // 基础颜色为红色
    setPen(Qt::NoPen); // 移除边框

    this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    this->setData(0, "enemy"); // id 标记，省的 include Enemy 来判断类型
}

// 常规版本索敌
void Enemy::moveTowardsTarget() {
    if (inFormation) return; // 阵型中不自主移动

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

    qreal finalMoveX = 0;
    qreal finalMoveY = 0;

    // 按比例移动 (归一化向量 * 速度)
    if (distance > 0.1) {
        qreal moveX = (dx / distance) * speed;
        qreal moveY = (dy / distance) * speed;

        finalMoveX += moveX;
        finalMoveY += moveY;

        if (scatterFrames > 0) {
            finalMoveX += scatterVx;
            finalMoveY += scatterVy;

            // 模拟物理摩擦力/空气阻力：让散开的速度越来越慢，看起来更自然
            scatterVx *= 0.9f;
            scatterVy *= 0.9f;

            scatterFrames--; // 帧数递减
        }

        this->moveBy(finalMoveX, finalMoveY);
    }
}

// 带穿越版本索敌
// void Enemy::moveTowardsTarget() {
//     if (inFormation) return; // 阵型中不自主移动

//     if (!playerTarget || !this->scene()) return;

//     // --- 1. 计算原本的追击玩家向量 (你原本的代码) ---
//     qreal ex = this->scenePos().x(); // 建议统一用 scenePos 防止坐标系错乱
//     qreal ey = this->scenePos().y();
//     qreal px = playerTarget->scenePos().x();
//     qreal py = playerTarget->scenePos().y();

//     qreal mapWidth = this->scene()->sceneRect().width();
//     qreal mapHeight = this->scene()->sceneRect().height();

//     qreal dx = px - ex;
//     qreal dy = py - ey;

//     if (qAbs(dx) > mapWidth / 2) dx = (dx > 0) ? (dx - mapWidth) : (dx + mapWidth);
//     if (qAbs(dy) > mapHeight / 2) dy = (dy > 0) ? (dy - mapHeight) : (dy + mapHeight);

//     qreal distance = sqrt(dx * dx + dy * dy);

//     // 最终要移动的步长
//     qreal finalMoveX = 0;
//     qreal finalMoveY = 0;

//     if (distance > 0.1) {
//         finalMoveX = (dx / distance) * speed;
//         finalMoveY = (dy / distance) * speed;
//     }

//     // --- 2. 核心修改：如果处于散开状态，叠加散开动量 ---
//     if (scatterFrames > 0) {
//         finalMoveX += scatterVx;
//         finalMoveY += scatterVy;

//         // 模拟物理摩擦力/空气阻力：让散开的速度越来越慢，看起来更自然
//         scatterVx *= 0.9f;
//         scatterVy *= 0.9f;

//         scatterFrames--; // 帧数递减
//     }

//     // --- 3. 最终统一移动 ---
//     this->moveBy(finalMoveX, finalMoveY);
// }

void Enemy::teleportThroughWall() {
    if(!playerTarget || !this->scene()) return; // 确保有目标且在场景中

    if (inFormation) return; // 在阵型中，就不自主移动

    // 获取地图范围
    QRectF mapRect = this->scene()->sceneRect();

    // 获取自身在地图上的绝对坐标
    qreal ex = this->x();
    qreal ey = this->y();

    // 获取自身大小（本地 rect 的宽高是正确的）
    int selfWidth = this->rect().width();
    int selfHeight = this->rect().height();

    // ---------------- 处理 X 轴 ----------------
    // 若接触左壁 (当前 X 小于地图左边缘)
    if(ex < mapRect.left()) {
        // 传送到右侧：地图右边缘坐标 - 自身宽度
        this->setPos(mapRect.right() - selfWidth, ey);
    }
    // 若接触右壁 (当前 X + 自身宽度 大于地图右边缘)
    else if(ex + selfWidth > mapRect.right()) {
        // 传送到左侧
        this->setPos(mapRect.left(), ey);
    }

    // 重新获取一下现在的坐标（防止因为上面 X 轴传送了，下面的 ex 还是老数据）
    ex = this->x();
    ey = this->y();

    // ---------------- 处理 Y 轴 ----------------
    // 若接触上壁
    if(ey < mapRect.top()) {
        this->setPos(ex, mapRect.bottom() - selfHeight);
    }
    // 若接触下壁
    else if(ey + selfHeight > mapRect.bottom()) {
        this->setPos(ex, mapRect.top());
    }
}

void Enemy::applyScatter(qreal vx, qreal vy, int frames) {
    this->scatterVx = vx;
    this->scatterVy = vy;
    this->scatterFrames = frames;
}

