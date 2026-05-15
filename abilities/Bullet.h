#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QObject>
#include <QTimer>
#include <QBrush>
#include <QColor>
#include <QPen>
#include <QtMath>


class Bullet: public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    const int damage;
    const int radius;
    const QPointF speedV;
    inline static const double speed = 3.0;

public:
    Bullet(int dmg, int r, qreal ang, QPointF& p) // 伤害，半径，弧度制方向角，全局坐标
        : QObject()
        , QGraphicsEllipseItem(0, 0, 10, 10)
        , damage(dmg)
        , radius(r)
        , speedV(speed * QPointF(qCos(ang), qSin(ang)))
    {
        setPos(p);

        setBrush(QBrush(QColor(1, 251, 255)));

        QPen pen(QColor(0, 0, 0));
        pen.setWidth(2);
        setPen(pen);

        QTimer::singleShot(3000, this, &Bullet::deleteLater);
    }

    void updatePosition()
    {
        // 先移动
        this->moveBy(speedV.x(), speedV.y());

        QRectF mapRect = this->scene()->sceneRect(); // 获取地图范围
        qreal ex = this->x(); // 获取自身在地图上的绝对坐标
        qreal ey = this->y();
        int selfWidth = this->rect().width(); // 获取自身大小
        int selfHeight = this->rect().height();

        // ---------------- 处理 X 轴 ----------------
        if(ex < mapRect.left()) {
            this->setPos(mapRect.right() - selfWidth, ey);
        }
        else if(ex + selfWidth > mapRect.right()) {
            this->setPos(mapRect.left(), ey);
        }

        // 重新获取一下现在的坐标（防止因为上面 X 轴传送了，下面的 ex 还是老数据）
        ex = this->x();
        ey = this->y();

        // ---------------- 处理 Y 轴 ----------------
        if(ey < mapRect.top()) {
            this->setPos(ex, mapRect.bottom() - selfHeight);
        }
        else if(ey + selfHeight > mapRect.bottom()) {
            this->setPos(ex, mapRect.top());
        }
    }
};


#endif // BULLET_H
