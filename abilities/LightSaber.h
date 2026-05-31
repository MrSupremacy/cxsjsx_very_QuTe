// Created by 樊轩楷

#ifndef LIGHTSABER_H
#define LIGHTSABER_H

#include <Ability.h>
#include <QPixmap>
#include "Player.h"

// 光剑技能类
class LightSaber: public Ability {
public:
    LightSaber(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        // 可以改变这个技能球的颜色，比如变成红色代表是攻击技能
        setBrush(QColor("#D7BDE2"));

        texture.load(":/ImageResources/speardisplay.png");
        textureRect = QRectF(-12, -12, 24, 24);
    }

    void pickUp() override {
        // 将父类保存的 QGraphicsItem 指针安全地转换为 Player 指针
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            // 播放音效
            SoundPool::instance().play("Spear_get");

            p->equipSword(6000); // 赋予玩家 6 秒的剑！
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

#endif // LIGHTSABER_H
