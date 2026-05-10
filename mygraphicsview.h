#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MyGraphicsView(QWidget *parent = nullptr);

protected:
    // 重写滚轮事件以实现缩放
    void wheelEvent(QWheelEvent *event) override;

private:
    void setupScene();
    QGraphicsScene *scene;
};

#endif // MYGRAPHICSVIEW_H
