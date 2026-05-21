#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QTimer>
#include <QBrush>
#include <QColor>
#include <QPen>
#include <QtMath>


class Bullet: public QObject, public QGraphicsEllipseItem
{
public:
    QPointF speedV;
    int lifeFrames;

    inline static const int LIFE = 180;
    inline static const double SPEED = 4.5;

public:
    Bullet(qreal ang, const QPointF& p) // 伤害，半径，弧度制方向角，全局坐标
        : QObject()
        , QGraphicsEllipseItem(0, 0, 8, 8)
        , speedV(SPEED * QPointF(qCos(ang), qSin(ang)))
        , lifeFrames(LIFE)
    {
        setPos(p);

        setBrush(QBrush(QColor(1, 251, 255)));

        QPen pen(QColor(0, 0, 0));
        pen.setWidth(2);
        setPen(pen);
    }

    void init(qreal ang, const QPointF& p) {
        speedV = SPEED * QPointF(qCos(ang), qSin(ang));
        setPos(p);
        lifeFrames = LIFE;
        setVisible(true);
    }

    // 返回 false 表示子弹应该被销毁/回收
    bool updatePosition()
    {
        // 1. 生命周期递减
        lifeFrames--;
        if (lifeFrames <= 0) {
            setVisible(false); // 回收时只隐藏，不 removeItem
            return false;
        }

        // 2. 正常移动
        this->moveBy(speedV.x(), speedV.y());

        if (!this->scene()) return false;

        QRectF mapRect = this->scene()->sceneRect();
        qreal ex = this->x();
        qreal ey = this->y();
        int selfWidth = this->rect().width();
        int selfHeight = this->rect().height();

        // 3. 碰到边界直接返回 false 请求回收
        if (ex < mapRect.left() || ex + selfWidth > mapRect.right()) {
            return false;
        }
        if (ey < mapRect.top() || ey + selfHeight > mapRect.bottom()) {
            return false;
        }

        return true; // 存活
    }
};


#endif // BULLET_H
