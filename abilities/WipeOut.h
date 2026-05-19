#ifndef WIPEOUT_H
#define WIPEOUT_H

#include <Ability.h>
#include "Player.h"


// 射击技能类
class WipeOut: public Ability {
public:
    WipeOut(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        setBrush(Qt::blue);
    }

    void pickUp() override
    {
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            p->autoFire(6, 500, 4); // 赋予射击
        }
    }

};


#endif // WIPEOUT_H
