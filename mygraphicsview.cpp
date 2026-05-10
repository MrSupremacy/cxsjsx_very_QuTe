#include "MyGraphicsView.h"
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPen>

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
    // 添加一个矩形
    QGraphicsRectItem *rect = scene->addRect(0, 0, 100, 60);
    rect->setBrush(Qt::blue);
    rect->setPen(QPen(Qt::black, 2));
    rect->setFlag(QGraphicsItem::ItemIsMovable);    // 使物体可移动
    rect->setFlag(QGraphicsItem::ItemIsSelectable); // 使物体可选中

    // 添加一个圆形
    QGraphicsEllipseItem *ellipse = scene->addEllipse(-100, -50, 80, 80);
    ellipse->setBrush(Qt::red);
    ellipse->setFlag(QGraphicsItem::ItemIsMovable);

    // 添加文本
    QGraphicsTextItem *text = scene->addText("Hello QGraphicsView!");
    text->setPos(-50, 100);
    text->setDefaultTextColor(Qt::darkGreen);
    text->setFlag(QGraphicsItem::ItemIsMovable);
}

void MyGraphicsView::wheelEvent(QWheelEvent *event)
{
    // 滚轮缩放逻辑
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        // 向上滚动，放大
        scale(scaleFactor, scaleFactor);
    } else {
        // 向下滚动，缩小
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}
