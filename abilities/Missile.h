#ifndef MISSILE_H
#define MISSILE_H

#include <QGraphicsObject> // 改用 QGraphicsObject，与剑气保持一致
#include <QGraphicsScene>
#include <QPainter>
#include <QTimer>
#include <QtMath>

#include "Explosion.h"
#include "SoundPool.h"

// 烟花火箭粒子类
class MissileTrailSpark : public QGraphicsObject {
public:
    MissileTrailSpark(QPointF spawnPos, QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent)
        , maxLife(9) // 固定的 8 帧寿命 (128ms)
        , life(9)
    {
        this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        this->setPos(spawnPos);

        // 1. 【性能优化】：静态加载并缓存 8 帧火花原画（避免频繁读盘卡顿） [2]
        static QList<QPixmap> originalSparks;
        if (originalSparks.isEmpty()) {
            for (int i = 0; i < 8; ++i) { // 对应你的 spark_0 到 spark_7
                QString path = QString(":/ImageResources/spark_%1.png").arg(i);
                QPixmap pix(path);

                // 火箭是 40x40，火花粒子缩放到 12x12 大小最适合
                pix = pix.scaled(10, 10, Qt::KeepAspectRatio, Qt::FastTransformation);
                originalSparks.append(pix);
            }
        }

        // 2. 从这 8 张图里随机抽取一张作为当前粒子的外观贴图 [2]
        if (!originalSparks.isEmpty()) {
            int randIdx = QRandomGenerator::global()->bounded(originalSparks.size());
            m_pixmap = originalSparks[randIdx];
        }

        // 3. 给火花附加微小的扩散方向，让火花运动显得灵动、不生硬
        vx = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.5;
        vy = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.5;

        // 4. 每 16ms (60 FPS) 更新一次物理移动与寿命
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this, timer]() {
            life--;
            if (life <= 0) {
                timer->stop();
                if (scene()) scene()->removeItem(this);
                this->deleteLater(); // 8帧到期，自动销毁释放内存
            } else {
                moveBy(vx, vy);
                update();   // 强制重绘，触发 paint 更新透明度
            }
        });
        timer->start(16);
    }

    QRectF boundingRect() const override {
        return QRectF(-5, -5, 10, 10); // 12x12 居中包围盒
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);
        if (!m_pixmap.isNull()) {
            // === 随着 8 帧生命时间递减，透明度从 100% 逐渐变为 0% ===
            qreal alpha = (qreal)life / maxLife;
            painter->setOpacity(alpha);

            // 居中绘制选中的火花
            painter->drawPixmap(QRectF(-6, -6, 12, 12).toRect(), m_pixmap);

            painter->setOpacity(1.0); // 恢复透明度
        }
    }

private:
    QPixmap m_pixmap;
    int maxLife;
    int life;
    double vx, vy;
};


// 烟花火箭类
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
        , m_trailFrame(0)
    {
        this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        setPos(startPos);
        setRotation(angleDeg);

        double rad = qDegreesToRadians(angleDeg);
        direction = QPointF(qCos(rad), qSin(rad));

        // 加载烟花火箭
        QPixmap rocketPic(":/ImageResources/firework.png");

        QTransform transform;
        transform.rotate(90);
        rocketPic = rocketPic.transformed(transform, Qt::SmoothTransformation);

        // === 🛠️ 核心修改 1：在此处自由调节你想显示的尺寸！ ===
        // 比如：你想让火箭看起来是 48 像素长，24 像素宽 [2]
        m_visualRect = QRectF(-20, -20, 40, 40);

        // 缩放贴图，使其大小与我们设计的视觉尺寸完全一致 [2]
        m_pixmap = rocketPic.scaled(40, 40, Qt::KeepAspectRatio, Qt::FastTransformation);


        // === 🛠️ 核心修改 2：在此处设定真实的碰撞箱尺寸（可以很小，方便走位） ===
        // 碰撞箱依然保持原来小巧的 26x10 大小，中心依然对齐 (0, 0)
        m_hitboxRect = QRectF(-13, -5, 26, 10);

        timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, [this]() {
            this->moveTowardsTarget();
        });
        timer->start(16);
    }


    // 实现 QGraphicsObject 必须提供的两个纯虚函数
    QRectF boundingRect() const override {
        return m_visualRect;
    }

    QPainterPath shape() const override {
        QPainterPath path;
        path.addRect(m_hitboxRect); // 物理判定只生效这个小框 [2]
        return path;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);

        // === 🛠️ 核心修改 5：使用大尺寸(m_visualRect)绘制，图片就能变大了！ === [2]
        painter->drawPixmap(m_visualRect.toRect(), m_pixmap);
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

                    const double TURN_RATE = 0.08;
                    direction = direction + (desiredDirection - direction) * TURN_RATE;

                    // 重新归一化
                    double len = std::hypot(direction.x(), direction.y());
                    if (len > 1e-3) {
                        direction /= len;
                    }
                }
            }
        }

        // 1. 执行移动 并生成粒子效果
        setPos(pos() + direction * SPEED);

        if (sc) {
            m_trailFrame++;
            // 每过 3 帧（16ms * 3 = 48ms）生成一个火花 [4]
            if (m_trailFrame % 8 == 0) {
                // 计算火箭屁股的位置（中心点 pos() 往飞行相反方向后退 20 像素，因为火箭长 40）
                QPointF tailPos = this->pos() - direction * 20;

                // 生成随机的火花粒子并加入场景
                MissileTrailSpark* spark = new MissileTrailSpark(tailPos);
                sc->addItem(spark);
            }
        }

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
            Explosion* exp = new Explosion(36.0, 500);
            exp->setPos(this->pos());
            sc->addItem(exp);

            // 播放音效
            SoundPool::instance().play("Missile_explode");
        }
    }

public:
    inline static const double SPEED = 3.4;

    QPolygonF m_polygon;

    QRectF m_visualRect; // 视觉显示大小 [2]
    QRectF m_hitboxRect; // 物理碰撞箱大小 [2]

    QPixmap m_pixmap;
    QPointF direction;
    QGraphicsItem *target;
    QTimer *timer;
    bool m_isDead; // 防止多次析构
    int m_trailFrame;
};

#endif // MISSILE_H
