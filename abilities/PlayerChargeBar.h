#ifndef PLAYERCHARGEBAR_H
#define PLAYERCHARGEBAR_H

#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QPainter>


class PlayerChargeBar : public QGraphicsItem {
public:
    PlayerChargeBar(QGraphicsItem* parent = nullptr) : QGraphicsItem(parent) {
        setFlag(ItemHasNoContents, false);
        setVisible(false); // 默认隐藏
    }

    // 设置进度 (0.0 到 1.0)
    void setProgress(qreal progress) {
        m_progress = qBound(0.0, progress, 1.0);
        update(); // 触发重绘
    }

    // 规定形状大小
    QRectF boundingRect() const override {
        return QRectF(0, 0, 50, 8); // 宽50，高8
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);

        // 1. 画外框 (黑色边框，透明或灰色背景)
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(0, 0, 50, 8);

        // 2. 画内部进度条 (绿色)
        if (m_progress > 0) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(Qt::green));
            qreal fillWidth = (50 - 2) * m_progress; // 减去边框宽度
            painter->drawRect(1, 1, fillWidth, 6);
        }
    }

private:
    qreal m_progress = 0.0;
};

#endif // PLAYERCHARGEBAR_H
