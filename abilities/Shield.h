#ifndef SHIELD_H
#define SHIELD_H


#include <Ability.h>
#include "Player.h"

class Shield: public Ability {
public:
    Shield(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        // 可以改变这个技能球的颜色，比如变成红色代表是攻击技能
        setBrush(Qt::darkCyan);
    }

    void pickUp() override {
        // 将父类保存的 QGraphicsItem 指针安全地转换为 Player 指针
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            p->equipShield(6000); // 赋予玩家 6 秒的盾！
        }
    }
};

#endif // SHIELD_H
