#ifndef DATACARRIER_H
#define DATACARRIER_H

#include <QMap>
#include <QString>
#include <QDebug>


struct DataCarrier
{
    DataCarrier(int a, int b, double c, bool d, int e)
        : moveMode(a)
        , difficulty(b)
        , volume(c)
        , timeLimited(d)
        , maxSeconds(e)
    {}

    int moveMode;
    int difficulty;
    double volume;

    bool timeLimited;
    int maxSeconds;
};

struct EndData
{
    EndData(int a, int b)
        : score(a)
        , second(b)
    {}

    int score;
    int second;
};

class globalSkin {
private:
    globalSkin() {}

public:
    static globalSkin& instance() {
        static globalSkin instance;
        return instance;
    }

    static QString applyChoice(const QString& type) {
        return skinChoices[type].at(instance().currChoice[type]);
    }

public:
    QMap<QString, int> currChoice = {
        {"Background", 0},
        {"Enemy", 0},
        {"Player", 0},
        {"Spear", 0}, {"SpearInve", 0},
        {"Arrow", 0}
    };

    inline static const QMap<QString, QVector<QString>> skinChoices = {
        {"Background", {":/ImageResources/Underwater.png",
                        ":/ImageResources/smooth_stone.jpg"}},

        {"Enemy", {":/ImageResources/drown.png",
                   ":/ImageResources/zombie.png"}},

        {"Player", {":/ImageResources/steve.jpg",
                    ":/ImageResources/alex.png"}},

        {"Spear", {":/ImageResources/diamondspear.png",
                   ":/ImageResources/wooden_spear.png"}},

        {"SpearInve", {":/ImageResources/speardisplay.png",
                   ":/ImageResources/wooden_spear_inventory.png"}},

        {"Arrow", {":/ImageResources/spectralarrow.png",
                   ":/ImageResources/Arrow_of_Strength.png"}}
    };
};

#endif // DATACARRIER_H
