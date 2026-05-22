// Created by 樊轩楷

#include "Ability.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QDebug>

Ability::Ability(QPointF spawnPos, QGraphicsItem *target)
    : playerTarget(target), anchorPos(spawnPos)
{
    // 1. 设置外观
    // 设定一个直径为 20 的圆，圆心位于 (0,0)
    setRect(-7.5, -7.5, 15, 15);
    setBrush(QBrush(Qt::yellow)); // 基础颜色为黄色
    setPen(QPen(Qt::white, 2));   // 白色边框

    // 2. 设置初始位置
    setPos(anchorPos);

    // 3. 初始化悬浮参数
    // 让每个技能的初始角度随机，这样多个技能同时存在时，不会“神同步”晃动
    angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;
    floatRange = 10.0; // 在锚点上下 10 像素范围内晃动
    floatSpeed = 0.05; // 每帧角度增加量
}

void Ability::updateFloating() {
    // 使用正弦波计算垂直偏移量
    angle += floatSpeed;
    if (angle > 2 * M_PI) angle -= 2 * M_PI;

    // 计算 Y 轴偏移 ( -floatRange 到 +floatRange )
    // offsetX 也可以加，但通常上下漂浮更有“悬浮感”
    double offsetY = qSin(angle) * floatRange;
    double offsetX = qCos(angle * 0.7) * (floatRange * 0.5); // 加上微弱的左右晃动

    // 更新位置：出生点 + 偏移量
    setPos(anchorPos.x() + offsetX, anchorPos.y() + offsetY);
}

void Ability::pickUp() {
    // 这是一个基类的实现
    // 子类（如 MachineGunAbility）会在这里写具体逻辑：playerTarget->enableMachineGun();
    qDebug() << "Ability picked up!";

    // 拾取后，通常我们需要把这个物体从场景中移除，并在外部 delete 它
    // 注意：不要直接在这里 delete this，建议在主循环检测到碰撞后统一处理
}
