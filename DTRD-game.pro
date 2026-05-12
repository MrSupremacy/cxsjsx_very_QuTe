QT       += core gui openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Ability.cpp \
    Enemy.cpp \
    GameView.cpp \
    Player.cpp \
    enemy3d.cpp \
    main.cpp \
    mainwindow.cpp \
    openglgameview.cpp

HEADERS += \
    Ability.h \
    Enemy.h \
    GameView.h \
    Player.h \
    enemy3d.h \
    mainwindow.h \
    openglgameview.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
