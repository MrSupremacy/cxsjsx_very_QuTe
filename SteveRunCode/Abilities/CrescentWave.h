// Created by 吉佑安

#ifndef CRESCENTWAVE_H
#define CRESCENTWAVE_H

#include <QGraphicsObject>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QGraphicsScene>
#include <QtMath>

class CrescentWave : public QGraphicsObject {
public:
    /**
     * @brief 咖喱棒月牙剑气
     * @param angle     发射角度 (度，0度为默认向右)
     * @param startPos  起始坐标
     */
    CrescentWave(qreal angle, QPointF startPos, QGraphicsItem *parent = nullptr)
        : QGraphicsObject(parent), m_isDead(false)
    {
        this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        setPos(startPos);
        setRotation(angle);

        // 1. 构建月牙形的 QPainterPath (局部坐标系：默认面向右，X轴正方向)
        m_path.moveTo(-15, -60);             // 上方尖端
        m_path.quadTo(60, 0, -15, 60);       // 外侧弧线 (剑气前锋)
        m_path.quadTo(25, 0, -15, -60);      // 内侧弧线 (剑气尾部)

        // 自动计算内切包围盒
        m_boundingRect = m_path.boundingRect();

        // 初始属性
        setScale(0.5);
        setOpacity(1.0);
        m_speed = 5.0;
        m_scaleGrowth = 0.03;

        // 2. 提前构建好渐变光影并缓存 (极大提升性能)
        // 局部坐标：从尾部(x=-15) 到 前锋(x=60)
        // 因为坐标系会随 setRotation 自动旋转，所以光影在任何角度下都是正确的！
        m_gradient = QLinearGradient(-15, 0, 60, 0);
        m_gradient.setColorAt(0.0, QColor(255, 255, 255, 0));     // 尾部边缘：完全透明
        m_gradient.setColorAt(0.3, QColor(255, 255, 255, 150));   // 尾部过渡：半透明金黄
        m_gradient.setColorAt(0.8, QColor(255, 255, 255, 255));   // 剑气主体：纯正金黄
        m_gradient.setColorAt(1.0, QColor(255, 255, 255, 255)); // 剑气尖端：高亮白光

        // 3. 定时器驱动动画
        QTimer *timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, [this]() {
            this->updateFrame();
        });
        timer->start(16); // 约 60 FPS
    }

    // 返回渲染用的包围盒
    QRectF boundingRect() const override { return m_boundingRect; }

    // 返回精确的月牙形碰撞体积
    QPainterPath shape() const override { return m_path; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_gradient); // 使用缓存好的光影渐变
        painter->drawPath(m_path);
    }

private:
    void updateFrame() {
        if (m_isDead) return;

        // 1. 移动逻辑：沿当前的 Rotation 角度向前飞
        qreal rad = qDegreesToRadians(rotation());
        qreal dx = m_speed * qCos(rad);
        qreal dy = m_speed * qSin(rad);
        moveBy(dx, dy);

        // 2. 放大逻辑 (碰撞体积 shape 也会被 Qt 自动等比放大)
        setScale(scale() + m_scaleGrowth);

        // 3. 销毁判定标志
        bool shouldDestroy = false;

        // 判定 B：飞出了场景边界
        if (scene()) {
            // 如果剑气的中心点(pos)超出了 Scene 的边界矩形
            if (!scene()->sceneRect().contains(pos())) {
                shouldDestroy = true;
            }
        }

        // 执行销毁
        if (shouldDestroy) {
            m_isDead = true; // 锁定状态，防止多次 delete
            if (scene()) {
                scene()->removeItem(this);
            }
            deleteLater();
        }
    }

private:
    QPainterPath m_path;        // 缓存的月牙形状
    QRectF m_boundingRect;      // 缓存的包围盒
    QLinearGradient m_gradient; // 缓存的光影渐变

    qreal m_speed;
    qreal m_scaleGrowth;
    bool m_isDead;              // 避免重复销毁的标记
};

#endif // CRESCENTWAVE_H
