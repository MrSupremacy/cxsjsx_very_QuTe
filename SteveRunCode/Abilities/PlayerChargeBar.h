// Created by 吉佑安

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
        return QRectF(0, 0, wd, ht);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
        Q_UNUSED(option); Q_UNUSED(widget);

        // 1. 画外框 (黑色边框，透明或灰色背景)
        painter->setPen(QPen(Qt::white, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(0, 0, wd, ht);

        // 2. 画内部进度条 (绿色)
        if (m_progress > 0) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(Qt::red));
            qreal fillWidth = (wd - 2) * m_progress; // 减去边框宽度
            painter->drawRect(1, 1, fillWidth, 6);
        }
    }

private:
    qreal m_progress = 0.0;
    const int wd = 24;
    const int ht = 4;
};

#endif // PLAYERCHARGEBAR_H
