#ifndef INTELLIGENTWIPEOUT_H
#define INTELLIGENTWIPEOUT_H


#include <Ability.h>
#include <QColor>
#include "Player.h"

// 导弹技能
class IntelligentWipeOut: public Ability {
public:
    IntelligentWipeOut(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        setBrush(QColor(255, 204, 128));

        texture.load(":/ImageResources/Firework_Loaded_Crossbow.png");
        textureRect = QRectF(-12, -12, 24, 24);
    }

    void pickUp() override
    {
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            p->launchMissile(8); // 发射导弹
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

#endif // INTELLIGENTWIPEOUT_H
