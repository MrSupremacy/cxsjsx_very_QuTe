// Created by 吉佑安

#ifndef WIPEOUT_H
#define WIPEOUT_H

#include <Ability.h>
#include "Player.h"


// 射击技能类
class WipeOut: public Ability {
public:
    WipeOut(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        setBrush(QColor(164, 222, 249));

        texture.load(":/ImageResources/arrowedbow.png");
        textureRect = QRectF(-14, -14, 28, 28);

    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        // 1. 首先调用基类的 paint，画出红色的圆形背景和白色边框
        Ability::paint(painter, option, widget);

        // 2. 然后在圆形背景上方，绘制您的贴图
        if (!texture.isNull()) {
            painter->setRenderHint(QPainter::SmoothPixmapTransform); // 开启平滑缩放防止锯齿
            painter->drawPixmap(textureRect, texture, texture.rect());
        }
    }

    void pickUp() override
    {
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            p->autoFire(16, 240, 3); // 赋予射击
        }
    }

private:
    QPixmap texture;     // 存储静态贴图
    QRectF textureRect;  // 用于控制每个技能独有的贴图位置与缩放比例
};


#endif // WIPEOUT_H
