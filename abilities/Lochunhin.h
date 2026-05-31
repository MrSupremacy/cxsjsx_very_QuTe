// Created by 吉佑安

#ifndef LOCHUNHIN_H
#define LOCHUNHIN_H

#include "Ability.h"
#include "Player.h"


class Lochunhin: public Ability {
public:
    Lochunhin(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        setBrush(QColor("#aaa"));

        texture.load(":/ImageResources/tnt.png");
        textureRect = QRectF(-12, -12, 24, 24);
    }

    void pickUp() override
    {
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            p->startCharging(1.0);
        }
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

private:
    QPixmap texture;     // 存储静态贴图
    QRectF textureRect;  // 用于控制每个技能独有的贴图位置与缩放比例
};

#endif // LOCHUNHIN_H
