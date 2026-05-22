QT       += core gui openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    "$$PWD/" \
    "$$PWD/abilities" \
    "$$PWD/games"

SOURCES += \
    Enemy.cpp \
    Formation.cpp \
    Player.cpp \
    abilities/Ability.cpp \
    enemy3d.cpp \
    games/GameView.cpp \
    games/openglgameview.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Arrow.h \
    Enemy.h \
    Formation.h \
    Player.h \
    Tetris.h \
    abilities/Ability.h \
    abilities/Bullet.h \
    abilities/BulletPool.h \
    abilities/CrescentWave.h \
    abilities/Explosion.h \
    abilities/LightSaber.h \
    abilities/Lochunhin.h \
    abilities/PlayerChargeBar.h \
    abilities/Shield.h \
    abilities/WipeOut.h \
    enemy3d.h \
    games/GameView.h \
    games/openglgameview.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
