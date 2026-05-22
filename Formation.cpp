// Created by 樊轩楷

#include "Formation.h"
#include "Enemy.h" // 假设你的Enemy类头文件
#include <QGraphicsScene>
#include <math.h>

Formation::Formation(QGraphicsItem *player, int time, QObject *parent)
    : QObject(parent), playerTarget(player) {

    // 初始化 5秒 计时器，时间到自动解散
    lifeTimer = new QTimer(this);
    connect(lifeTimer, &QTimer::timeout, this, &Formation::deformation);
    lifeTimer->start(time); // 阵型维持时间
}

Formation::~Formation() {
    // Timer 会随着 this 的销毁自动回收
}

void Formation::beginSequentialSpawn(int timeMs) {
    // 1. 获取当前组内的所有子敌人
    QList<QGraphicsItem*> children = this->childItems();

    // 2. 将它们按顺序全部隐藏，并加入等待队列
    for (QGraphicsItem* item : children) {
        item->hide();
        unspawnedQueue.enqueue(item);
    }

    // 3. 启动计时器，每 32ms 触发一次 popNextEnemy
    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &Formation::popNextEnemy);
    spawnTimer->start(timeMs);
}

void Formation::popNextEnemy() {
    // 如果队列空了，说明所有敌人都生成完毕了
    if (unspawnedQueue.isEmpty()) {
        if (spawnTimer) {
            spawnTimer->stop();
            spawnTimer->deleteLater(); // 清理计时器
            spawnTimer = nullptr;
        }
        return;
    }

    // 拿出最前面的一个敌人，让它显示在场景中
    QGraphicsItem* item = unspawnedQueue.dequeue();
    item->show();

}

void Formation::deformation() {
    if (!scene()) return;

    // 1. 获取组内剩下的所有存活敌人
    QList<QGraphicsItem*> children = this->childItems();
    for (QGraphicsItem* item : children) {
        Enemy* enemy = dynamic_cast<Enemy*>(item);
        if (enemy) {
            // 防止隐形幽灵
            enemy->show();

            // 记录该敌人在地图上的绝对坐标
            QPointF absolutePos = enemy->scenePos();

            // 将其从编组中移除
            this->removeFromGroup(enemy);

            // 重新设置其在地图上的绝对坐标（否则它会跑回地图左上角）
            enemy->setPos(absolutePos);

            // 告诉敌人它自由了，恢复自主移动状态
            enemy->setInFormation(false);
        }
    }

    // 2. 阵型解散完毕，从场景中销毁这个隐形的组框架自身
    scene()->removeItem(this);
    this->deleteLater();
}