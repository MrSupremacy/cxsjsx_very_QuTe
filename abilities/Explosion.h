// Created by 樊轩楷

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <QGraphicsObject> // 统一使用 QGraphicsObject
#include <QPainter>
#include <QTimer>
#include <QGraphicsScene>

class Explosion : public QGraphicsObject
{
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
                this->scene()->removeItem(this);
            }
            this->deleteLater();
        });
    }

    // 必须重写的两个纯虚函数
    QRectF boundingRect() const override {
        return QRectF(-radius, -radius, 2 * radius, 2 * radius);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(QColor(255, 50, 50, 150)); // 半透明红色
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(boundingRect());
    }

private:
    qreal radius;
};

#endif // EXPLOSION_H
