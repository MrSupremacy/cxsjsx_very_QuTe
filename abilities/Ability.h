#ifndef ABILITY_H
#define ABILITY_H

#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QPen>
#include <QPointF>



// 基础技能/增益类
class Ability: public QGraphicsEllipseItem {
public:
    // 构造函数：传入生成位置和玩家指针
    Ability(QPointF spawnPos, QGraphicsItem *target);

    // 虚函数：玩家捡起时触发的行为，子类可以重写它实现具体功能
    virtual void pickUp();

    // 悬浮动画函数：在游戏主循环中调用
    void updateFloating();

protected:
    QGraphicsItem *playerTarget; // 玩家指针
    QPointF anchorPos;           // 初始生成点（锚点）

    // 悬浮动画相关的参数
    double angle;       // 当前正弦波的角度
    double floatRange;  // 悬浮上下晃动的像素范围
    double floatSpeed;  // 悬浮的速度
};


#endif // ABILITY_H
