#ifndef INTELLIGENTWIPEOUT_H
#define INTELLIGENTWIPEOUT_H


#include <Ability.h>
#include <QColor>
#include "Player.h"

// 导弹技能
class IntelligentWipeOut: public Ability {
public:
    IntelligentWipeOut(QPointF spawnPos, QGraphicsItem *target)
        : Ability(spawnPos, target)
    {
        setBrush(QColor(255, 127, 0));
    }

    void pickUp() override
    {
        Player *p = dynamic_cast<Player*>(playerTarget);
        if (p) {
            p->launchMissile(5); // 发射导弹
        }
    }
};

#endif // INTELLIGENTWIPEOUT_H
