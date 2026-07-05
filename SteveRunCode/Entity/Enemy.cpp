// Created by 樊轩楷

#include "Enemy.h"
#include <math.h>
#include <QGraphicsScene>
#include <QDebug>
#include <QPen>
#include <QPainter>
#include <QVector>

#include "DataCarrier.h"


Enemy::Enemy(QGraphicsItem *target) {
    playerTarget = target;
    // setRect(0, 0, 16, 16); // 设置敌人大小
    // setBrush(QBrush(Qt::red)); // 基础颜色为红色
    // setPen(Qt::NoPen); // 移除边框


    QPixmap enemyPic(globalSkin::applyChoice("Enemy"));

    // 把它从 600x600 缩小成你游戏里想要的 32x32 物理大小

    enemyPic = enemyPic.scaled(selfHt, selfWd, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    this->setPixmap(enemyPic);
    this->setTransformOriginPoint(selfHt /2, selfWd /2); // 旋转中心设为一半


    this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    this->setData(0, "enemy"); // id 标记，省的 include Enemy 来判断类型
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

void Enemy::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // 1. 首先，调用父类默认的绘制函数，把僵尸/溺尸图片画出来
    QGraphicsPixmapItem::paint(painter, option, widget);

    // 2. 然后，我们用画笔在它的边缘叠加上一个描边方框

    QColor borderColor = inFormation ? QColor(180, 0, 0) : Qt::black;
    QPen pen(borderColor, 2);


    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);

    // 3. 绘制边框
    // 为了防止画笔由于太粗而被图片边缘裁剪，将边框往内微调 1 像素 (adjusted)
    painter->drawRect(this->boundingRect().adjusted(1, 1, -1, -1));
}

// 带穿越版本索敌
void Enemy::moveTowardsTarget() {
    if (inFormation) return; // 阵型中不自主移动

    if (!playerTarget || !this->scene()) return;


    qreal ex = this->scenePos().x();
    qreal ey = this->scenePos().y();
    qreal px = playerTarget->scenePos().x();
    qreal py = playerTarget->scenePos().y();

    qreal mapWidth = this->scene()->sceneRect().width();
    qreal mapHeight = this->scene()->sceneRect().height();

    qreal dx = px - ex;
    qreal dy = py - ey;

    if (qAbs(dx) > mapWidth / 2) dx = (dx > 0) ? (dx - mapWidth) : (dx + mapWidth);
    if (qAbs(dy) > mapHeight / 2) dy = (dy > 0) ? (dy - mapHeight) : (dy + mapHeight);

    qreal distance = sqrt(dx * dx + dy * dy);

    // 最终要移动的步长
    qreal finalMoveX = 0;
    qreal finalMoveY = 0;

    if (distance > 0.1) {
        finalMoveX = (dx / distance) * speed;
        finalMoveY = (dy / distance) * speed;
    }


    if (scatterFrames > 0) {
        finalMoveX += scatterVx;
        finalMoveY += scatterVy;

        // 模拟物理摩擦力/空气阻力：让散开的速度越来越慢，看起来更自然
        scatterVx *= 0.93f;
        scatterVy *= 0.93f;

        scatterFrames--; // 帧数递减
    }


    this->moveBy(finalMoveX, finalMoveY);
}

QVector<qreal> Enemy::teleportThroughWall() {
    // 默认返回 5 个 0，最后一位 0 表示未传送
    if(!playerTarget || !this->scene() || inFormation) {
        return {0, 0, 0, 0, 0};
    }

    QRectF mapRect = this->scene()->sceneRect();
    qreal ex = this->x();
    qreal ey = this->y();
    qreal tx = ex; // 目标 X 坐标
    qreal ty = ey; // 目标 Y 坐标

    bool teleported = false;



    if(ex < mapRect.left()) {
        tx = mapRect.right() - selfWd; // 右边界对齐
        ty = ey;                          // 确保 Y 轴（垂直方向）不发生改变
        ex = mapRect.left();
        teleported = true;
    }
    else if(ex + selfWd > mapRect.right()) {
        tx = mapRect.left();              // 左边界对齐
        ty = ey;                          // 确保 Y 轴（垂直方向）不发生改变
        ex = mapRect.right() - selfWd;
        teleported = true;
    }

    else if(ey < mapRect.top()) {
        ty = mapRect.bottom() - selfHt; // 下边界对齐
        tx = ex;                            // 确保 X 轴（水平方向）不发生改变
        ey = mapRect.top();
        teleported = true;
    }
    else if(ey + selfHt > mapRect.bottom()) {
        ty = mapRect.top();                 // 上边界对齐
        tx = ex;                            // 确保 X 轴（水平方向）不发生改变
        ey = mapRect.bottom() - selfHt;
        teleported = true;
    }

    if (teleported) {
        this->setPos(tx, ty);
        // 返回 [前X, 前Y, 后X, 后Y, 1]
        return {ex, ey, tx, ty, 1.0};
    }

    // 没有传送，最后一位返回 0.0
    return {ex, ey, ex, ey, 0.0};
}

void Enemy::applyScatter(qreal vx, qreal vy, int frames) {
    this->scatterVx = vx;
    this->scatterVy = vy;
    this->scatterFrames = frames;
}
