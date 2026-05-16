#ifndef LOCHUNHIN_H
#define LOCHUNHIN_H

#include "Ability.h"
#include "Player.h"
#include "CrescentWave.h"


class Lochunhin: public Ability {
public:
    Lochunhin(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        setBrush(Qt::gray);
    }

    void pickUp() override
    {
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            p->startCharging(1.0);
        }
    }
};


#endif // LOCHUNHIN_H
