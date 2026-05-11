#include "Player.h"
#include <math.h>
#include <QGraphicsScene>

Player::Player() {
    setRect(0, 0, 10, 10);
}

void Player::keyboardMove(bool w, bool a, bool s, bool d, bool up, bool left, bool down, bool right) {
    double dx = 0, dy = 0;
    int dirx = 0, diry = 0;

    if(w || up) diry --;
    if(a || left) dirx --;
    if(s || down) diry ++;
    if(d || right) dirx ++;

    if(dirx != 0 || diry != 0) {
        dx = speed * dirx / sqrt(dirx * dirx + diry * diry);
        dy = speed * diry / sqrt(dirx * dirx + diry * diry);
    }

    // 预判玩家下一步的位置
    double nextX = this->x() + dx;
    double nextY = this->y() + dy;

    // 只有当玩家已经被加入到场景中时，才进行边界判定
    if (this->scene()) {
        // 获取地图范围
        QRectF mapRect = this->scene()->sceneRect();

        // 获取玩家自身的范围
        QRectF playerRect = this->rect();

        // 限制 X 坐标 (左右碰壁)
        if (nextX < mapRect.left()) {
            nextX = mapRect.left(); // 撞到左墙，贴紧左墙
        }
        else if (nextX + playerRect.width() > mapRect.right()) {
            nextX = mapRect.right() - playerRect.width(); // 撞到右墙，贴紧右墙
        }

        // 限制 Y 坐标 (上下碰壁)
        if (nextY < mapRect.top()) {
            nextY = mapRect.top(); // 撞到上墙，贴紧上墙
        }
        else if (nextY + playerRect.height() > mapRect.bottom()) {
            nextY = mapRect.bottom() - playerRect.height(); // 撞到下墙，贴紧下墙
        }
    }

    // 把玩家放到指定地点（移动）
    this->setPos(nextX, nextY);
}

void Player::mouseMove(const QPointF posInScene, const double sensibility)
{
    const QPointF pos = posInScene - this->pos();
    double L = sqrt(QPointF::dotProduct(pos, pos));
    if (L < 2.0) return;

    // 跟随鼠标角度
    double dx = pos.x() / L * speed;
    double dy = pos.y() / L * speed;

    this->moveBy(dx, dy);
}

void Player::mouse3Dmove(const QPointF mouseDiff, const double sensibility)
{
    double L = sqrt(QPointF::dotProduct(mouseDiff, mouseDiff));
    if (L < 30.0) return;

    // 跟随鼠标角度
    double dx = -mouseDiff.x() / L * speed;
    double dy = -mouseDiff.y() / L * speed;

    this->moveBy(dx, dy);
}
