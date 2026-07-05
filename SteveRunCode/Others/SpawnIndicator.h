// Created by 樊轩楷

#ifndef SPAWNINDICATOR_H
#define SPAWNINDICATOR_H

#include <QGraphicsObject>
#include <QPainter>
#include <QTimer>
#include <QGraphicsScene>

class SpawnIndicator : public QGraphicsObject {
public:
    SpawnIndicator(QPointF pos, QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent)
        , m_show(true)
    {
        this->setPos(pos);
        this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        static QPixmap redstonePic;
        if (redstonePic.isNull()) {
            // 假设你的图片放在 ImageResources 里，记得核对文件名大小写！
            redstonePic.load(":/ImageResources/enemybirth.png");

            redstonePic = redstonePic.scaled(40, 40, Qt::KeepAspectRatio, Qt::FastTransformation);
        }

        // 将缓存好的静态贴图赋给当前实体
        m_pixmap = redstonePic;

        // 1. 闪烁控制：每 150 毫秒切换一次可见性 (红叉狂闪)
        QTimer* flashTimer = new QTimer(this);
        connect(flashTimer, &QTimer::timeout, this, [this]() {
            m_show = !m_show;
            update(); // 触发重绘
        });
        flashTimer->start(950);

        // 2. 寿命控制：1000 毫秒 (1秒) 后，预警结束，自动从地图上销毁
        QTimer::singleShot(1100, this, [this]() {
            if (this->scene()) {
                this->scene()->removeItem(this);
            }
            this->deleteLater();
        });
    }

    QRectF boundingRect() const override {
        return QRectF(-20, -20, 40, 40); // 32x32 的居中包围盒
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);

        // 如果当前是“隐藏”帧，直接返回不画
        if (!m_show) return;

        painter->drawPixmap(-20, -20, m_pixmap);
    }

private:
    bool m_show; // 控制当前帧是否显示
    QPixmap m_pixmap; // 保存红石贴图
};

#endif // SPAWNINDICATOR_H