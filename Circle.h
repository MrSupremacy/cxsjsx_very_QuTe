// Created by 樊轩楷

#ifndef CIRCLE_H
#define CIRCLE_H

#include "Formation.h"
#include "Enemy.h"
#include <QtMath>         // 用于计算角度 qAtan2, qSin, qCos
#include <QTimer>         // 用于 2秒 状态切换
#include <QGraphicsScene>

class CircleFormation: public Formation {
    Q_OBJECT
public:
    // 定义阵型的两个状态
    enum class State {
        Waiting,      // 阶段一：原地缓慢旋转，给玩家压迫感和反应时间
        Contracting   // 阶段二：向中心极速收缩并快速旋转
    };

    CircleFormation(QGraphicsItem* playerTarget, int time, QObject* parent = nullptr) :
        Formation(playerTarget, time, parent), currentState(State::Waiting)
    {
        // 1. 设置生成参数
        int numEnemies = 16;       // 16个敌人组成一个大圈
        double radius = 300.0;     // 初始圆的半径，足够把玩家套在里面

        // 2. 极坐标排兵布阵
        for (int i = 0; i < numEnemies; ++i) {
            Enemy* e = new Enemy(playerTarget);

            // 计算每个敌人的角度 (弧度)
            double angle = (2 * M_PI * i) / numEnemies;

            // 将极坐标转换为相对于中心点 (0,0) 的 XY 坐标
            e->setPos(radius * qCos(angle), radius * qSin(angle));
            e->setInFormation(true);

            this->addToGroup(e);
        }

        // 3. 把阵型的变换(旋转)中心精确设置在圆心
        this->setTransformOriginPoint(0, 0);

        // 修改收缩时的速度
        Formation::speed = 0.8f;

        // 停用基类的按时自动销毁（由收缩到底部时触发销毁）
        if (lifeTimer) lifeTimer->stop();

        // 4. 设定 3 秒后开始收缩
        QTimer::singleShot(3000, this, [this]() {
            if (this->currentState == State::Waiting) {
                this->currentState = State::Contracting;
            }
        });
    }

    void rotate() override {
        // 第一阶段：像法阵一样缓慢转动
        if (currentState == State::Waiting) {
            this->setRotation(this->rotation() + 0.16);
        }
        // 第二阶段：变成绞肉机快速转动
        else if (currentState == State::Contracting) {
            this->setRotation(this->rotation() + 0.5);
        }
    }

    void move() override {
        if (!scene()) return;

        // 无论哪个阶段都调用旋转
        rotate();

        if (currentState == State::Waiting) {
            // 第一阶段不移动，只旋转锁定玩家位置
            return;
        }
        else if (currentState == State::Contracting) {
            bool reachedCenter = false;

            // 获取组内的所有敌人
            QList<QGraphicsItem*> children = this->childItems();

            for (QGraphicsItem* item : children) {
                Enemy* enemy = dynamic_cast<Enemy*>(item);
                if (enemy) {
                    // 获取敌人在阵型内的【局部坐标】
                    QPointF localPos = enemy->pos();

                    // 计算该敌人距离圆心(0,0)的距离
                    qreal dist = sqrt(localPos.x() * localPos.x() + localPos.y() * localPos.y());

                    // 如果距离已经极其接近圆心了（说明圈缩到底了）
                    if (dist <= 20) {
                        reachedCenter = true;
                        break;
                    } else {
                        // 向量归一化，向着 (0,0) 收缩移动
                        qreal dx = (-localPos.x() / dist) * Formation::speed;
                        qreal dy = (-localPos.y() / dist) * Formation::speed;
                        enemy->moveBy(dx, dy);
                    }
                }
            }

            // 第三阶段：如果任意一个敌人到达了中心（或者组里敌人被打光了），阵型解散爆开
            if (reachedCenter || children.isEmpty()) {
                deformation();
            }
        }
    }

private:
    State currentState; // 当前状态
};

#endif // CIRCLE_H