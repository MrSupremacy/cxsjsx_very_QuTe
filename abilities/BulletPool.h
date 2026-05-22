// Created by 吉佑安

#ifndef BULLETPOOL_H
#define BULLETPOOL_H

#include <QQueue>
#include <QGraphicsScene>
#include "Bullet.h"

class BulletPool {
public:
    // 获取单例
    static BulletPool& getInstance() {
        static BulletPool instance;
        return instance;
    }

    // 从池中获取一颗子弹
    Bullet* getBullet(qreal ang, QPointF p) {
        Bullet* b;
        if (pool.isEmpty()) {
            b = new Bullet(ang, p); // 只有当池子空了才 new
        } else {
            b = pool.dequeue(); // 否则直接复用
        }
        b->init(ang, p); // 重新初始化状态
        return b;
    }

    // 回收子弹到池中
    void recycle(Bullet* b) {
        if (b->scene()) {
            b->setVisible(false);
        }
        pool.enqueue(b);
    }

    // 清空对象池（用于关闭游戏时释放内存）
    void clear() {
        while (!pool.isEmpty()) {
            pool.dequeue()->deleteLater();
        }
    }

    void addToScene(QGraphicsScene *sc) {
        for (int i = 0; i < 60; ++i) {
            Bullet* b = new Bullet(0, QPointF(30, 30));
            b->setVisible(false);
            pool.enqueue(b);
            sc->addItem(b);
        }
    }

private:
    BulletPool() {} // 私有构造
    QQueue<Bullet*> pool;
};

#endif // BULLETPOOL_H
