// Created by 吉佑安

#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QBrush>
#include <QColor>
#include <QPen>
#include <QtMath>


class Bullet: public QObject, public QGraphicsPixmapItem
{
public:
    QPointF speedV;
    int lifeFrames;

    inline static const int LIFE = 180;
    inline static const double SPEED = 8.0;

public:
    Bullet(qreal ang, const QPointF& p)
        : QObject()
        , QGraphicsPixmapItem()
        , speedV(SPEED * QPointF(qCos(ang), qSin(ang)))
        , lifeFrames(LIFE)
    {
        QPixmap arrowPic(":/ImageResources/spectralarrow.png");

        // MC 的箭矢通常是向右上方 45° 倾斜的。
        // 我们顺时针旋转 45 度，使箭尖水平向右，作为 0 度的基准方向
        QTransform transform;
        transform.rotate(45);
        arrowPic = arrowPic.transformed(transform, Qt::SmoothTransformation);

        // 箭矢是长条形，我们把它等比例拉伸为：长 24 像素，宽 8 像素（比例你可以自己微调）
        arrowPic = arrowPic.scaled(44, 44, Qt::KeepAspectRatio, Qt::FastTransformation);
        this->setPixmap(arrowPic);

        // === 🛠️ 核心步骤 3：设置箭矢自身的旋转中心 ===
        // 假设箭矢长 24、宽 8，我们将它的旋转轴心设在正中心：X = 12, Y = 4
        this->setTransformOriginPoint(22, 22);

        // === 🛠️ 核心步骤 4：根据玩家开火的方向角进行旋转 ===
        // 弧度转角度：qRadiansToDegrees
        qreal degrees = qRadiansToDegrees(ang);
        this->setRotation(degrees);

        // 为了让箭矢的中心点 (12, 4) 刚好重合在你的发射坐标 p 上
        // 我们需要把它的坐标向左上偏移半个身位：
        this->setPos(p.x() - 22, p.y() - 22);

        // 开启设备坐标缓存，大幅提升同屏多子弹时的性能
        this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }

    QPainterPath shape() const override
    {
        QPainterPath path;
        // 44x44的正方形中，箭矢水平居中 (Y 的中心在 22)
        // 我们在 Y=19 到 Y=25 (高度为6) 之间画一个细长的碰撞矩形
        // 长度设为 34 (X从 5 到 39，避开了尾部不具有伤害判定的羽翼)
        path.addRect(10, 10, 24, 24); // [2]
        return path;
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
        int selfWidth = this->boundingRect().width();
        int selfHeight = this->boundingRect().height();

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
