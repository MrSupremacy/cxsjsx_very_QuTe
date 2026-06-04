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
                        ":/ImageResources/smooth_stone.jpg",
                        ":/ImageResources/sandstone_top.png"}},

        {"Enemy", {":/ImageResources/drown.png",
                    ":/ImageResources/zombie.png",
                    ":/ImageResources/HuskFace.png"}},

        {"Player", {":/ImageResources/steve.jpg",
                    ":/ImageResources/alex.png",
                    ":/ImageResources/EntitySprite_efe.png",
                    ":/ImageResources/EntitySprite_kai.png"}},

        {"Spear", {":/ImageResources/diamondspear.png",
                    ":/ImageResources/wooden_spear.png",
                    ":/ImageResources/Held_Golden_Spear.png",
                    ":/ImageResources/Held_Netherite_Spear.png",
                    ":/ImageResources/Held_Stone_Spear.png",
                    ":/ImageResources/Held_Iron_Spear.png",
                    ":/ImageResources/Held_Copper_Spear.png"}},

        {"SpearInve", {":/ImageResources/speardisplay.png",
                    ":/ImageResources/wooden_spear_inventory.png",
                    ":/ImageResources/Invicon_Golden_Spear.png",
                    ":/ImageResources/Invicon_Netherite_Spear.png",
                    ":/ImageResources/Invicon_Stone_Spear.png",
                    ":/ImageResources/Invicon_Iron_Spear.png",
                    ":/ImageResources/Invicon_Copper_Spear.png"}},

        {"Arrow", {":/ImageResources/spectralarrow.png",
                   ":/ImageResources/Arrow_of_Strength.png",
                   ":/ImageResources/Invicon_Arrow.png",
                   ":/ImageResources/Invicon_Arrow_of_Healing.png",
                   ":/ImageResources/Invicon_Arrow_of_Swiftness.png",
                   ":/ImageResources/Invicon_Arrow_of_the_Turtle_Master.png",
                   ":/ImageResources/Invicon_Arrow_of_Wind_Charging.png"}}
    };
};


const QString buttonOn = R"(
    QPushButton {
        border-image: url(:/ImageResources/button_on.png);
        padding: 4px;
    }
    QPushButton:hover {
        border-image: url(:/ImageResources/button_on_hover.png);
        padding: 4px;
    }
    QPushButton:pressed {
        border-image: url(:/ImageResources/button_on_press.png);
        padding: 4px;
    }
)";

const QString buttonOff = R"(
    QPushButton {
        border-image: url(:/ImageResources/button_off.png);
        padding: 4px;
    }
    QPushButton:hover {
        border-image: url(:/ImageResources/button_off_hover.png);
        padding: 4px;
    }
    QPushButton:pressed {
        border-image: url(:/ImageResources/button_off_press.png);
        padding: 4px;
    }
)";


const QVector<QString> randomTexts = {
      "what can I say?\nIt's just a random text."
    , "Swing your spear."
    , "Be careful."
    , "I hate the square hitbox."
    , "Pro sound effects. Who made them?"
    , "UI is nice\n--written by me, the designer."
    , "TNT doesn't work as you thought."
    , "Difficulty 4 is too hard."
    , "I can play difficulty one\nfor a year."
};



#endif // DATACARRIER_H
