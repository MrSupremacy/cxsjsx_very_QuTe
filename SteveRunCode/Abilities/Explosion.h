// Created by 樊轩楷

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <QObject>
#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QPen>
#include <QTimer>
#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QPainterPath>
#include <QtMath>
#include <QList>

// 1. 定义像素粒子结构体 [2]
struct MCParticle {
    QPointF pos;  // 粒子相对圆心的坐标
    QPointF vel;  // 速度向量 (vx, vy)
    QColor color; // 粒子颜色
    qreal size;   // 像素大小 (正方形边长)
    int maxLife;  // 最大寿命 (帧数)
    int life;     // 剩余寿命
};

class Explosion : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT

private:
    int lifeSpan;          // 物理伤害判定存活周期 (毫秒)
    qreal physicalRadius;  // 真实的物理杀伤半径 [2]
    QList<MCParticle> particles; // 粒子列表
    QTimer* updateTimer;   // 粒子物理更新计时器 (60 FPS)


    bool m_isDamageActive = true;

public:

    Explosion(qreal r, int time)
        : QGraphicsEllipseItem(-2 * r, -2 * r, 4 * r, 4 * r)
        , lifeSpan(time)
        , physicalRadius(r)
    {
        // 粒子更新频繁，关闭设备缓存以提升性能
        this->setCacheMode(QGraphicsItem::NoCache);

        // 2. 物理隐形：画笔画刷设为空，整个物体在地图上完全隐形，但碰撞依然生效 [2]
        this->setBrush(Qt::NoBrush);
        this->setPen(Qt::NoPen);

        // 3. 初始化生成你设定的 60 个像素烟花粒子
        int particleCount = 40 * r * r / 1296;

        // 【筛选后的经典 MC 亮色调调色板
        QList<QColor> mcColors = {
            QColor(255, 255, 85),  // 黄色
            QColor(110, 255, 110), // 浅绿色
            QColor(40, 175, 40),   // 深绿色
        };

        int longestLifeFrames = 0; // 记录这批粒子中最长的寿命，以便动态决定实体的绝对生命

        for (int i = 0; i < particleCount; ++i) {
            MCParticle p;
            p.pos = QPointF(0, 0); // 初始都在圆心爆开

            // 【你的新参数】：随机散射角度和初速度
            double angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;
            double speed = (1.2 + QRandomGenerator::global()->generateDouble() * 3.2) * r / 36; // 速度 1.2 ~ 4.8
            p.vel = QPointF(qCos(angle) * speed, qSin(angle) * speed);

            // 从调色板随机选一个颜色
            p.color = mcColors[QRandomGenerator::global()->bounded(mcColors.size())];

            // 像素大小（3x3 到 5x5 的正方形）
            p.size = 3.0 + QRandomGenerator::global()->bounded(3);


            p.maxLife = (40 + QRandomGenerator::global()->bounded(40)) * sqrt(r) / 6;
            p.life = p.maxLife;

            // 记录下最长的一个粒子活了多少帧
            if (p.maxLife > longestLifeFrames) {
                longestLifeFrames = p.maxLife;
            }

            particles.append(p);
        }

        // 4. 驱动粒子进行物理位移的计时器 (16ms = 约60帧)
        updateTimer = new QTimer(this);
        connect(updateTimer, &QTimer::timeout, this, &Explosion::updatePhysics);
        updateTimer->start(16);


        QTimer::singleShot(lifeSpan, this, [this]() {
            // 物理伤害时间到，关闭物理伤害判定开关！ [4]
            // 此时圆形判定区在物理上消失，不再阻挡或杀伤怪物，但画面依然会继续播放 [4]
            this->m_isDamageActive = false;
        });


        // 计算最长寿命粒子对应的毫秒数（帧数 * 16ms 加上 100 毫秒的安全余量）
        int visualLifeSpanMs = longestLifeFrames * 16 + 100;

        // 等到你的长寿命粒子确实自然死光了（最长可达 4 秒），我们才物理销毁这个实体本身！
        QTimer::singleShot(visualLifeSpanMs, this, [this]() {
            if (this->scene()) {
                this->scene()->removeItem(this);
            }
            this->deleteLater();
        });
    }


    QPainterPath shape() const override {
        QPainterPath path;
        // 如果伤害开关开着，返回圆形杀伤范围；如果关了，返回空路径，代表物理上已经解除了武装 [2, 4]
        if (m_isDamageActive) {
            path.addEllipse(QPointF(0, 0), physicalRadius, physicalRadius);
        }
        return path;
    }

protected:

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);

        // 遍历所有粒子并绘制
        for (const MCParticle& p : particles) {
            if (p.life <= 0) continue;

            // 随着生命消逝，粒子会逐渐变透明
            qreal alphaFraction = (qreal)p.life / p.maxLife;
            QColor drawColor = p.color;
            drawColor.setAlphaF(alphaFraction);

            painter->setBrush(QBrush(drawColor));
            painter->setPen(Qt::NoPen);

            // 绘制经典的 MC 硬朗正方形像素颗粒
            painter->drawRect(QRectF(p.pos.x() - p.size / 2, p.pos.y() - p.size / 2, p.size, p.size));
        }
    }

private slots:
    // 物理法则模拟
    void updatePhysics() {
        for (int i = 0; i < particles.size(); ++i) {
            MCParticle& p = particles[i];
            p.life--;

            if (p.life > 0) {
                // 1. 根据速度进行位移
                p.pos += p.vel;


                p.vel.setX(p.vel.x() * 0.87);
                p.vel.setY(p.vel.y() * 0.87);
            }
        }
        // 强制重绘，更新画面
        this->update();
    }
};

#endif // EXPLOSION_H