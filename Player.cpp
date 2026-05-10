#include "Player.h"
#include <math.h>

Player::Player() {
    setRect(0, 0, 10, 10);
}

void Player::move(bool w, bool a, bool s, bool d, bool up, bool left, bool down, bool right) {
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

    this->moveBy(dx, dy);
}