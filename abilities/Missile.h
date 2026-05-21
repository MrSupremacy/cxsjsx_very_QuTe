#ifndef MISSILE_H
#define MISSILE_H

#include <QGraphicsObject> // 改用 QGraphicsObject，与剑气保持一致
#include <QGraphicsScene>
#include <QPainter>
#include <QTimer>
#include <QtMath>

#include "Explosion.h"

class Missile : public QGraphicsObject
{
public:
    /**
     * @brief 追踪导弹
     * @param tg        目标指针
     * @param angleDeg  发射角度 (注意：这里统一改为 角度 Degree，避免混乱)
     * @param startPos  起始坐标 (必须传入，否则出生在0,0立刻越界爆炸)
     */
    Missile(QGraphicsItem* tg, double angleDeg, QPointF startPos, QGraphicsItem *parent = nullptr)
        : QGraphicsObject(parent)
        , target(tg)
        , m_isDead(false)
    {
        // 1. 初始化位置和旋转
        setPos(startPos);
        setRotation(angleDeg);

        // 将角度转为弧度，计算初始方向向量
        double rad = qDegreesToRadians(angleDeg);
        direction = QPointF(qCos(rad), qSin(rad));

        // 2. 构建三角形多边形
        m_polygon << QPointF(7, 0)    // 尖端 (右)
                  << QPointF(-7, -5)  // 底角 (左上)
                  << QPointF(-7, 5);  // 底角 (左下)

        m_boundingRect = m_polygon.boundingRect();

        // 3. 定时器驱动
        timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, [this]() {
            this->moveTowardsTarget();
        });
        timer->start(16); // 约 60 FPS
    }

    // 实现 QGraphicsObject 必须提供的两个纯虚函数
    QRectF boundingRect() const override { return m_boundingRect; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(QBrush(QColor(70, 70, 70)));
        painter->setPen(QPen(Qt::black, 1));
        painter->drawPolygon(m_polygon);
    }

public:
    void moveTowardsTarget() {
        if (m_isDead) {
            return;
        }

        QGraphicsScene* sc = scene();
        if (!sc) return;

        // 如果目标存在，进行追踪转向
        if (target) {
            // 安全检查：防止目标已经被移出场景
            if (target && scene()->items().contains(target)) {
                QPointF targetCenter = target->sceneBoundingRect().center();
                QPointF missileCenter = this->sceneBoundingRect().center();

                QPointF desiredDirection = targetCenter - missileCenter;
                double distance = std::hypot(desiredDirection.x(), desiredDirection.y());

                if (distance > 1.0) {
                    desiredDirection /= distance; // 归一化

                    const double TURN_RATE = 0.1;
                    direction = direction + (desiredDirection - direction) * TURN_RATE;

                    // 重新归一化
                    double len = std::hypot(direction.x(), direction.y());
                    if (len > 1e-3) {
                        direction /= len;
                    }
                }
            }
        }

        // 1. 执行移动
        setPos(pos() + direction * SPEED);

        // 2. 更新旋转角度使其朝向飞行方向
        double angle = qRadiansToDegrees(qAtan2(direction.y(), direction.x()));
        setRotation(angle);

        // 3. 边界碰撞检测
        QRectF boundary = sc->sceneRect();
        if (!boundary.contains(pos())) {
            explode();
            return;
        }
    }

    void explode() {
        if (m_isDead) return;
        m_isDead = true; // 防止重复爆炸

        QGraphicsScene* sc = scene();
        if (sc) {
            // 创建爆炸特效
            Explosion* exp = new Explosion(20.0, 300);
            exp->setPos(this->pos());
            sc->addItem(exp);
        }
    }

public:
    inline static const double SPEED = 3.0;

    QPolygonF m_polygon;
    QRectF m_boundingRect;

    QPointF direction;
    QGraphicsItem *target;
    QTimer *timer;
    bool m_isDead; // 防止多次析构
};

#endif // MISSILE_H
