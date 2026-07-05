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
    for (QGraphicsItem* item : std::as_const(children)) {
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

    // 获取阵型中心点
    QPointF center = this->sceneBoundingRect().center();

    // 1. 获取组内剩下的所有存活敌人
    QList<QGraphicsItem*> children = this->childItems();
    for (QGraphicsItem* item : children) {
        Enemy* enemy = dynamic_cast<Enemy*>(item);
        if (enemy) {
            // 防止隐形幽灵
            enemy->show();

            // 记录该敌人在地图上的绝对坐标
            QPointF absolutePos = enemy->scenePos();

            // --- 新增：计算向外的散开动量 ---
            qreal diffX = absolutePos.x() - center.x();
            qreal diffY = absolutePos.y() - center.y();
            qreal dist = sqrt(diffX * diffX + diffY * diffY);

            qreal svx = 0;
            qreal svy = 0;
            if (dist > 0.1) {
                // 如果敌人不在正中心，给它一个向外的初始爆破速度（比如 5.0，可以自己调）
                svx = (diffX / dist) * 5.0f;
                svy = (diffY / dist) * 5.0f;
            } else {
                // 兜底：如果敌人刚好在阵型绝对中心(dist==0)，给个默认的散开方向(比如向右)
                svx = 5.0;
                svy = 0;
            }

            // 将其从编组中移除
            this->removeFromGroup(enemy);
            // 重新设置其在地图上的绝对坐标（否则它会跑回地图左上角）
            enemy->setPos(absolutePos);
            // 告诉敌人它自由了，恢复自主移动状态
            enemy->setInFormation(false);
            enemy->resetTransform();

            // 赋予向外动量60帧
            enemy->applyScatter(svx, svy, 60);
        }
    }

    // 2. 阵型解散完毕，从场景中销毁这个隐形的组框架自身
    scene()->removeItem(this);
    this->deleteLater();
}
