#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QtMath>
#include <QTimer>
#include <QPointF>
#include "Bullet.h"


class Player: public QGraphicsEllipseItem {
public:
    Player(); // 构造函数

    void keyboardMove(
        bool w, bool a, bool s, bool d, bool up, bool left, bool down, bool right); // 键盘移动

    void mouseMove( // 鼠标移动
        const QPointF posInScene, const double sensibility); // posInScene是相对于左上角坐标

    void mouse3Dmove(
        const QPointF mouseDiff, const double sensibility);


private:
    double speed = 3.0f;             // 玩家移动速度
    QPointF lastDir = QPointF(1, 0); // 记录最后一次的面朝方向（默认朝右）


public:
    // 光剑技能
    QGraphicsRectItem *swordItem;    // 剑的图形
    QTimer *swordTimer;              // 控制剑持续时间的定时器
    void equipSword(int durationMs); // 装备剑的函数
    QGraphicsRectItem* getSword();   // 获取剑的指针（用于在 GameView 里做碰撞检测）

    // 射击技能
    const int distPx = 10;
    int fireTimes;     // 记录已射击次数
    int currNum, currInterval;
    QTimer *fireTimer; // 发送射击信号
    void autoFire(int rounds, int interval, int num); // num: 一排子弹个数

};

#endif // PLAYER_H
