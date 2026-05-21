#ifndef MISSILE_H
#define MISSILE_H


#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QTimer>
#include <QBrush>
#include <QColor>
#include <QPen>
#include <QtMath>


class Missile: public QObject, public QGraphicsEllipseItem
{
public:
    Missile(QGraphicsItem* tg, double ang) // 目标指针 | 当前角度
        : QObject()
        , QGraphicsEllipseItem(0, 0, 10, 10)
        , direction(QPointF{qCos(ang), qSin(ang)})
        , target(tg)
    {
        setRotation(ang);
    }

    void moveTowardsTarget() {
        if (!target) {
            setPos(pos() + direction * SPEED);
            return;
        }

        QPointF targetCenter = target->sceneBoundingRect().center();
        QPointF missileCenter = this->sceneBoundingRect().center();

        QPointF desiredDirection = targetCenter - missileCenter;
        double distance = std::hypot(desiredDirection.x(), desiredDirection.y());

        if (distance > 1.0) {
            desiredDirection /= distance; // 归一化

            // 转向限制：转向速度因子
            const double TURN_RATE = 0.1;
            direction = direction + (desiredDirection - direction) * TURN_RATE;

            // 重新归一化方向向量
            double len = std::hypot(direction.x(), direction.y());
            if (len > 1e-3) {
                direction /= len;
            }
        }

        setPos(pos() + direction * SPEED);

        // 更新导弹的旋转角度，使其朝向移动方向
        double angle = qRadiansToDegrees(qAtan2(direction.y(), direction.x()));
        setRotation(angle);
    }

private:
    inline static const double SPEED = 3.0;
    QPointF direction;
    QGraphicsItem *target;
};



#endif // MISSILE_H
