// Created by 樊轩楷

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <QObject>
#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QPen>
#include <QTimer>
#include <QGraphicsScene>

// 同时继承 QObject 和 QGraphicsEllipseItem，这样才能安全使用 QTimer
class Explosion : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT // 继承 QObject 最好加上这个宏

private:
    int lifeSpan; // 存活周期 (毫秒)

public:
    // r 代表半径 (Radius)。
    // 注意这里传给父类的矩形是 (-r, -r, 2*r, 2*r)，这样 Explosion 的坐标中心点 (0,0) 就是圆心！
    Explosion(qreal r, int time) : QGraphicsEllipseItem(-r, -r, 2 * r, 2 * r), lifeSpan(time) {
        this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        // 1. 设置外观 (比如半透明的红色，代表爆炸伤害区)
        this->setBrush(QColor(255, 50, 50, 150)); // R, G, B, Alpha(透明度)
        this->setPen(Qt::NoPen);                  // 不需要边框

        // 2. 定时自毁逻辑 (核心)
        // 使用 QTimer::singleShot，经过 lifeSpan 毫秒后，执行 Lambda 表达式中的代码
        // 传入 this 作为上下文，如果 Explosion 被提前销毁，定时器会自动取消，很安全
        QTimer::singleShot(lifeSpan, this, [this]() {
            if (this->scene()) {
                this->scene()->removeItem(this); // 从场景中移除
            }
            this->deleteLater(); // 安全释放内存
        });
    }
};

#endif // EXPLOSION_H