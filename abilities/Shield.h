// Created by 樊轩楷

#ifndef SHIELD_H
#define SHIELD_H


#include <Ability.h>
#include "Player.h"
#include "SoundPool.h"

class Shield: public Ability {
public:
    Shield(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        // 可以改变这个技能球的颜色，比如变成红色代表是攻击技能
        setBrush(QColor(130, 255, 238));

        gifMovie = new QMovie(":/ImageResources/totemdisplay.gif");
        if (gifMovie->isValid()) {
            // 设置该技能独有的 GIF 缩放大小
            textureRect = QRectF(-14, -14, 28, 28);

            // 当 GIF 帧改变时，调用 update() 刷新绘制
            QObject::connect(gifMovie, &QMovie::frameChanged, [this]() {
                this->update();
            });
            gifMovie->start();
        }
    }

    ~Shield() {
        if (gifMovie) {
            gifMovie->stop();
            delete gifMovie;
        }
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        // 1. 先绘制背景圆形
        Ability::paint(painter, option, widget);

        // 2. 再叠加绘制 GIF 当前帧
        if (gifMovie && gifMovie->state() == QMovie::Running) {
            painter->setRenderHint(QPainter::SmoothPixmapTransform);
            QPixmap currentFrame = gifMovie->currentPixmap();
            painter->drawPixmap(textureRect, currentFrame, currentFrame.rect());
        }
    }

    void pickUp() override {
        // 将父类保存的 QGraphicsItem 指针安全地转换为 Player 指针
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            // 播放音效
            SoundPool::instance().play("Shield_get");

            p->equipShield(6000); // 赋予玩家 6 秒的盾！
        }
    }

private:
    QMovie *gifMovie = nullptr;
    QRectF textureRect; // 同样用于控制 GIF 的缩放和位置

};

#endif // SHIELD_H
