#include "MyGraphicsView.h"
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPen>
// github 上传测试注释

MyGraphicsView::MyGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    // 1. 创建场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-200, -200, 400, 400); // 设置场景坐标范围
    this->setScene(scene);

    // 2. 优化视图设置
    setRenderHint(QPainter::Antialiasing);       // 抗锯齿
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag); // 允许鼠标左键拖拽画布
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // 以前鼠下为缩放中心

    setupScene();
}

void MyGraphicsView::setupScene()
{



}

void MyGraphicsView::wheelEvent(QWheelEvent *event)
{
    // // 滚轮缩放逻辑
    // const double scaleFactor = 1.15;
    // if (event->angleDelta().y() > 0) {
    //     // 向上滚动，放大
    //     scale(scaleFactor, scaleFactor);
    // } else {
    //     // 向下滚动，缩小
    //     scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    // }
}
