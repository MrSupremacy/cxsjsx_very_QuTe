#ifndef ENEMY3D_H
#define ENEMY3D_H

#include <QGraphicsRectItem>

class Enemy3D : public QGraphicsRectItem {
public:
    Enemy3D(QGraphicsItem *target, int mapW, int mapH); // 修改
    void moveTowardsTarget();

private:
    QGraphicsItem *playerTarget;
    double speed = 0.2f;
    int mapWidth;
    int mapHeight;
};

#endif // ENEMY3D_H
