// Created by 樊轩楷

#ifndef FORMATION_H
#define FORMATION_H

#include <QQueue>
#include <QObject>
#include <QGraphicsItemGroup>
#include <QGraphicsItem>
#include <QTimer>
#include "Enemy.h"

// 注意：要使用 QTimer，必须继承 QObject，且放在第一个，并加上 Q_OBJECT 宏
class Formation : public QObject, public QGraphicsItemGroup {
    Q_OBJECT
public:
    Formation(QGraphicsItem *playerTarget, int time, QObject *parent = nullptr);
    virtual ~Formation();

    // 开始依次生成的函数
    virtual void beginSequentialSpawn(int timeMs);

    // 留给游戏主循环(Game Tick)调用的移动函数
    virtual void move() = 0;
    virtual void rotate() = 0;

public slots:
    // 解散阵型
    void deformation();

protected slots:
    // 新增：每次定时器触发时弹出下一个敌人
    void popNextEnemy();

protected:
    QGraphicsItem *playerTarget;
    double speed = 0.6f;
    QTimer *lifeTimer; // 生命周期计时器

    QQueue<QGraphicsItem*> unspawnedQueue; // 等待生成的敌人队列
    QTimer* spawnTimer = nullptr;          // 控制生成的计时器
};

#endif // FORMATION_H