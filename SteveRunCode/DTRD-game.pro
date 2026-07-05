QT       += core gui openglwidgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    "$$PWD/" \
    "$$PWD/Abilities" \
    "$$PWD/Entity" \
    "$$PWD/Games" \
    "$$PWD/Formation" \
    "$$PWD/Others" \
    "$$PWD/Window"

SOURCES += \
    Entity/Enemy.cpp \
    Formation/Formation.cpp \
    ENtity/Player.cpp \
    Abilities/Ability.cpp \
    Games/GameView.cpp \
    Games/openglgameview.cpp \
    main.cpp \
    Window/mainwindow.cpp \
    Window/skinselect.cpp

HEADERS += \
    Formation/Arrow.h \
    Others/DataCarrier.h \
    Formation/Circle.h \
    Others/DeathVFX.h \
    Entity/Enemy.h \
    Formation/Formation.h \
    Entity/Player.h \
    Others/PortalPiece.h \
    Others/PortalPool.h \
    Others/RotatedLabel.h \
    Others/SoundPool.h \
    Others/SpawnIndicator.h \
    Formation/Tetris.h \
    Abilities/Ability.h \
    Abilities/Bullet.h \
    Abilities/CrescentWave.h \
    Abilities/IntelligentWipeOut.h \
    Abilities/Explosion.h \
    Abilities/LightSaber.h \
    Abilities/Lochunhin.h \
    Abilities/Missile.h \
    Abilities/PlayerChargeBar.h \
    Abilities/Shield.h \
    Abilities/WipeOut.h \
    Games/GameView.h \
    Games/openglgameview.h \
    Window/mainwindow.h \
    Window/skinselect.h

FORMS += \
    Window/mainwindow.ui \
    Window/skinselect.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    mainicon.ico

RC_ICONS = mainicon.ico
