#include "Enemy.h"
#include <math.h>
#include <QGraphicsScene>
#include <QDebug>

Enemy::Enemy(QGraphicsItem *target) {
    playerTarget = target;
    setRect(0, 0, 8, 8); // 设置敌人大小
    setBrush(QBrush(Qt::red)); // 基础颜色为红色
    setPen(Qt::NoPen); // 移除边框
}

// 常规版本索敌
// void Enemy::moveTowardsTarget() {
//     if (!playerTarget) return;

//     // 获取玩家和敌人的坐标
//     qreal px = playerTarget->x();
//     qreal py = playerTarget->y();
//     qreal ex = this->x();
//     qreal ey = this->y();

//     // 计算差值
//     qreal dx = px - ex;
//     qreal dy = py - ey;

//     // 计算距离
//     qreal distance = sqrt(dx * dx + dy * dy);

//     // 按比例移动 (归一化向量 * 速度)
//     if (distance > 0) {
//         qreal moveX = (dx / distance) * speed;
//         qreal moveY = (dy / distance) * speed;

//         this->moveBy(moveX, moveY);
//     }
// }

// 带穿越版本索敌
void Enemy::moveTowardsTarget() {
    if (!playerTarget || !this->scene()) return;

    qreal mapWidth = this->scene()->sceneRect().width();
    qreal mapHeight = this->scene()->sceneRect().height();

    qreal ex = this->x();
    qreal ey = this->y();
    qreal px = playerTarget->x();
    qreal py = playerTarget->y();

    // --- 核心算法：最短穿越位移 ---

    // 计算原始位移
    qreal dx = px - ex;
    qreal dy = py - ey;

    // 如果 dx 的绝对值大于地图的一半，说明穿越走更近
    if (qAbs(dx) > mapWidth / 2) {
        // 如果 dx 是正的，说明玩家在右，敌人在左，穿越路径是让 dx 变负
        dx = (dx > 0) ? (dx - mapWidth) : (dx + mapWidth);
    }

    // Y 轴同理
    if (qAbs(dy) > mapHeight / 2) {
        dy = (dy > 0) ? (dy - mapHeight) : (dy + mapHeight);
    }

    // 计算实际距离
    qreal distance = sqrt(dx * dx + dy * dy);

    // 按比例移动
    if (distance > 0.1) { // 留一点点阈值，防止抖动
        qreal moveX = (dx / distance) * speed;
        qreal moveY = (dy / distance) * speed;
        this->moveBy(moveX, moveY);
    }
}

void Enemy::teleportThroughWall() {
    if(!playerTarget || !this->scene()) return; // 确保有目标且在场景中

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


