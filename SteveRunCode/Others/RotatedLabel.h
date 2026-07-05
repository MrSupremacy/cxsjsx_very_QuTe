#ifndef ROTATEDLABEL_H
#define ROTATEDLABEL_H

#pragma once

#include <QLabel>
#include <QPainter>
#include <QPaintEvent>

class RotatedLabel : public QLabel {
    Q_OBJECT
public:
    explicit RotatedLabel(QWidget *parent = nullptr) : QLabel(parent), m_angle(30.0) {

        setAttribute(Qt::WA_TranslucentBackground);
    }

    void setRotationAngle(double angle, bool b) {
        m_angle = angle;
        needTransBG = b;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override {


        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿
        painter.setRenderHint(QPainter::TextAntialiasing);

        // 1. 将原点移至控件中心
        painter.translate(rect().center());
        // 2. 旋转坐标系
        painter.rotate(m_angle);

        // 3. 计算文本所需的尺寸，以便动态适配背景矩形大小
        QFontMetrics fm(font());
        QRect textRect = fm.boundingRect(text());
        int tw = textRect.width();
        int th = textRect.height();

        // 4. 定义背景矩形（稍微加一点内边距 padding）
        int paddingX = 10;
        int paddingY = 6;
        QRectF bgRect(-tw / 2.0 - paddingX, -th / 2.0 - paddingY,
                      tw + 2 * paddingX, th + 2 * paddingY);

        // 5. 绘制半透明背景矩形
        // QColor(红, 绿, 蓝, 透明度)，其中透明度 0-255，120 大约是半透明
        if (needTransBG) {
            QColor bgColor(0, 0, 0, 120); // 黑色半透明
            painter.setPen(Qt::NoPen);    // 不需要边框
            painter.setBrush(bgColor);    // 设置填充画刷
            painter.drawRoundedRect(bgRect, 4, 4); // 绘制带 4 像素圆角的矩形
        }

        // 6. 绘制文本
        painter.setPen(palette().color(QPalette::WindowText)); // 使用当前调色板的文本颜色
        // 在背景矩形内居中绘制文本
        painter.drawText(bgRect, Qt::AlignCenter, text());
    }

private:
    double m_angle;
    bool needTransBG = true;
};

#endif // ROTATEDLABEL_H
