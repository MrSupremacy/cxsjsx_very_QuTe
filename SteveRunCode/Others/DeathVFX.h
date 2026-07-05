// Created by 樊轩楷

#ifndef DEATHVFX_H
#define DEATHVFX_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QGraphicsScene>

// 特效最大大小
#define VFX_SIZE 36.0

class DeathVFX : public QGraphicsObject {
public:

    DeathVFX(QPointF spawnPos, QPixmap enemyPixmap, QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent)
        , currentFrame(0)
    {
        this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        this->setPos(spawnPos);

        // 1. 缓存敌人的贴图（等比例缩放到死前一模一样的 32x32 大小）
        m_enemyPixmap = enemyPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::FastTransformation);

        // 2. 静态加载 8 帧原图并缓存
        static QList<QPixmap> originalFrames;
        if (originalFrames.isEmpty()) {
            for (int i = 0; i < 8; ++i) {
                QString path = QString(":/ImageResources/generic_%1.png").arg(i);
                QPixmap pix(path);

                pix = pix.scaled(VFX_SIZE, VFX_SIZE, Qt::KeepAspectRatio, Qt::FastTransformation);
                originalFrames.append(pix);
            }
        }

        m_smokeFrames = originalFrames;

        // 3. 设定定时器，每 48ms（刚好3个游戏帧）切换到下一帧
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this, timer]() {
            currentFrame++;

            // 【总帧数变化】：4 帧红色受伤 (4 * 48ms = 192ms ≈ 200ms) + 8 帧白烟消失 = 12 帧后自毁
            if (currentFrame >= 12) {
                timer->stop();
                if (scene()) {
                    scene()->removeItem(this);
                }
                this->deleteLater();
            } else {
                update();
            }
        });
        timer->start(112);
    }

    // 边界取最大的 VFX_SIZE 包围盒，防止画面残影 [2]
    QRectF boundingRect() const override {
        return QRectF(-VFX_SIZE / 2.0, -VFX_SIZE / 2.0, VFX_SIZE, VFX_SIZE);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);

        // === 【第一阶段】：前 4 帧（约 200ms）绘制变红的敌人 ===
        if (currentFrame < 1) {
            if (!m_enemyPixmap.isNull()) {
                QRectF enemyRect(-12, -12, 24, 24); // 保持 32x32 居中

                // 绘制敌人原贴图
                painter->drawPixmap(enemyRect.toRect(), m_enemyPixmap);

                // 核心：在贴图上面覆盖一层半透明的红色纯色块 (MC 经典受伤变红，160的透明度极佳！)
                painter->setOpacity(0.85);
                painter->fillRect(enemyRect, QColor(255, 0, 0, 160));
                painter->setOpacity(1.0);
            }
        }

        else {
            int smokeIndex = currentFrame - 1; // 计算当前白烟是第几帧

            if (smokeIndex < m_smokeFrames.size() && !m_smokeFrames[smokeIndex].isNull()) {

                // 透明度随帧数增加而增加（每帧多透明 5%）
                qreal opacity = 1.0 - (smokeIndex * 0.04);
                if (opacity < 0.0) opacity = 0.0;

                painter->setOpacity(opacity);

                QRectF drawRect(-VFX_SIZE / 2.0, -VFX_SIZE / 2.0, VFX_SIZE, VFX_SIZE);
                painter->drawPixmap(drawRect.toRect(), m_smokeFrames[smokeIndex]);

                painter->setOpacity(1.0); // 恢复画笔透明度
            }
        }
    }

private:
    int currentFrame;
    QPixmap m_enemyPixmap;   // 缓存敌人的死前贴图
    QList<QPixmap> m_smokeFrames;
};

#endif // DEATHVFX_H