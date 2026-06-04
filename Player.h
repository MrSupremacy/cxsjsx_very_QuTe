// Created by 樊轩楷 & 吉佑安

#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QtMath>
#include <QTimer>
#include <QPointF>
#include "Bullet.h"
#include "CrescentWave.h"
#include "PlayerChargeBar.h"
#include "Explosion.h"
#include "Missile.h"


class Player: public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    Player(); // 构造函数

    // 重写绘制函数，用于画边框
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void keyboardMove(
        bool w, bool a, bool s, bool d, bool up, bool left, bool down, bool right); // 键盘移动

    void mouseMove( // 鼠标移动
        const QPointF posInScene); // posInScene是相对于左上角坐标

    void mouse3Dmove(
        const QPointF mouseDiff);


public:
    double speed = 4.5f;             // 玩家移动速度
    QPointF lastDir = QPointF(1, 0); // 记录最后一次的面朝方向（默认朝右）
    bool isImmune = false; // 无敌状态

private slots:
    void endImmune();

public:
    // 无敌效果
    QTimer *immuneTimer;
    void giveImmune(int timeMs);
    bool getIsImmune() const { return isImmune;}

    // 光剑 光剑技能
    QGraphicsPixmapItem* swordItem;
    QTimer *swordTimer;              // 控制剑持续时间的定时器
    void equipSword(int durationMs); // 装备剑的函数
    QGraphicsPixmapItem* getSword();

    // 蓄力条 咖喱棒技能
    QTimer *chargeBarTimer;
    PlayerChargeBar *chargeBar;
    double currProgress;
    double deltaP;
    QGraphicsPixmapItem *tntItem; // TNT 贴图

    void startCharging(double time_in_s);
    void onCharging();
    void launchLochunhin();

    // 护盾 护盾技能
    QGraphicsRectItem *shieldItem; // 护盾图形
    QGraphicsPixmapItem *totemItem; // 不死图腾
    QTimer *shieldTimer;              // 控制护盾持续时间的定时器
    void equipShield(int durationMs); // 装备护盾的函数
    void breakShieldAndExplode(int radius = 120, int lifeTime = 1000, bool haveTotem = true); // 销毁护盾并产生爆炸
    QGraphicsRectItem* getShield();   // 获取护盾的指针（用于在 GameView 里做碰撞检测）

    // 射击技能
    const int distPx = 10;
    int fireTimes;     // 记录已射击次数
    int currNum, currInterval;
    QTimer *fireTimer; // 发送射击信号
    void autoFire(int rounds, int interval, int num); // num: 一排子弹个数

    // 导弹技能
    void launchMissile(int n);



};

#endif // PLAYER_H
