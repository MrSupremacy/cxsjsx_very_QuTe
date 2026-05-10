#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QtMath>

class Player: public QGraphicsRectItem {
public:
    Player(); // 构造函数

    void keyboardMove(
        bool w, bool a, bool s, bool d, bool up, bool left, bool down, bool right); // 键盘移动

    void mouseMove( // 鼠标移动
        const QPointF posInScene, const double sensibility); // posInScene是相对于左上角坐标

private:
    double speed = 3.0f; // 玩家移动速度
};

#endif // PLAYER_H
