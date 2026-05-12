#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QtMath>
#include <QTimer>
#include <QPointF>


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
    double speed = 3.0f; // 玩家移动速度

// 光剑技能相关
public:
    // 新增：装备剑的函数
    void equipSword(int durationMs);
    // 新增：获取剑的指针（用于在 GameView 里做碰撞检测）
    QGraphicsRectItem* getSword();

private:
    QPointF lastDir = QPointF(1, 0); // 记录最后一次的面朝方向（默认朝右）

    QGraphicsRectItem *swordItem;    // 剑的图形
    QTimer *swordTimer;              // 控制剑持续时间的定时器

};

#endif // PLAYER_H
