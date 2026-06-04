// Created by 樊轩楷

#ifndef TETRIS_H
#define TETRIS_H

#include "Formation.h"
#include "Enemy.h"
#include <QtMath>
#include <QTimer>
#include <QGraphicsScene>

class TetrisFormation: public Formation {
    Q_OBJECT

public:
    // 定义两个阵型状态
    enum class State {
        Waiting,  // 阶段一：原地组合 给予玩家反应时间
        Bouncing  // 阶段二：开始弹墙
    };

    TetrisFormation(QGraphicsItem* playerTarget, int time, QObject* parent = nullptr) :
        Formation(playerTarget, time, parent), currentState(State::Waiting)
    {
        // 构建阵型
        Enemy* e1 = new Enemy(playerTarget);
        Enemy* e2 = new Enemy(playerTarget);
        Enemy* e3 = new Enemy(playerTarget);
        Enemy* e4 = new Enemy(playerTarget);
        Enemy* e5 = new Enemy(playerTarget);
        Enemy* e6 = new Enemy(playerTarget);
        Enemy* e7 = new Enemy(playerTarget);
        Enemy* e8 = new Enemy(playerTarget);
        Enemy* e9 = new Enemy(playerTarget);

        double dx = 30.0, dy = 30.0;
        e1->setPos(0, 0);
        e2->setPos(dx, 0);
        e3->setPos(2 * dx, 0);
        e4->setPos(2 * dx, dy);
        e5->setPos(dx, dy);
        e6->setPos(0, dy);
        e7->setPos(0, 2 * dy);
        e8->setPos(dx, 2 * dy);
        e9->setPos(2 * dx, 2 * dy);

        e1->setInFormation(true); e2->setInFormation(true);
        e3->setInFormation(true); e4->setInFormation(true);
        e5->setInFormation(true); e6->setInFormation(true);
        e7->setInFormation(true); e8->setInFormation(true);
        e9->setInFormation(true);

        this->addToGroup(e1); this->addToGroup(e2);
        this->addToGroup(e3); this->addToGroup(e4);
        this->addToGroup(e5); this->addToGroup(e6);
        this->addToGroup(e7); this->addToGroup(e8);
        this->addToGroup(e9);

        // 修改速度 (弹射速度)
        Formation::speed = 7.0f;

        QTimer::singleShot(2000, this, [this]() {
            if (this->currentState == State::Waiting) {
                this->currentState = State::Bouncing;
            }

            // --- 【新增】：在起步的一瞬间，计算出朝向玩家的初始发射速度 ---
            if (this->playerTarget && this->scene()) {
                QPointF center = this->sceneBoundingRect().center(); // 阵型中心点
                QPointF pPos = this->playerTarget->scenePos();       // 玩家位置

                qreal diffX = pPos.x() - center.x();
                qreal diffY = pPos.y() - center.y();
                qreal dist = sqrt(diffX * diffX + diffY * diffY);

                if (dist > 0) {
                    this->vx = (diffX / dist) * this->speed;
                    this->vy = (diffY / dist) * this->speed;
                } else {
                    // 兜底方案
                    this->vx = this->speed / sqrt(2);
                    this->vy = this->speed / sqrt(2);
                }
            }

        });
    }

    void rotate() override {
        // 不用旋转
        return;
    }

    void move() override {
        if (!scene()) return;

        if (currentState == State::Waiting) {
            // 第一阶段：不动
            return;
        }
        else if (currentState == State::Bouncing) {
            // 1. 先按照当前的 vx, vy 进行位移
            this->moveBy(vx, vy);

            // 2. 获取地图的绝对边界和方块阵型的绝对包围盒
            QRectF mapRect = scene()->sceneRect();
            QRectF formRect = this->sceneBoundingRect(); // 这个函数极其好用！

            // 3. 碰撞检测与反弹逻辑
            // X轴碰撞：左边出界或右边出界
            if (formRect.left() <= mapRect.left()) {
                // 撞到左墙，必须往右走 (x 轴速度强制变正数)
                vx = qAbs(vx);
            }
            else if (formRect.right() >= mapRect.right()) {
                // 撞到右墙，必须往左走 (x 轴速度强制变负数)
                vx = -qAbs(vx);
            }

            // Y轴碰撞：上边出界或下边出界
            if (formRect.top() <= mapRect.top()) {
                // 撞到上墙，必须往下走 (y 轴速度强制变正数)
                vy = qAbs(vy);
            }
            else if (formRect.bottom() >= mapRect.bottom()) {
                // 撞到下墙，必须往上走 (y 轴速度强制变负数)
                vy = -qAbs(vy);
            }
        }
    }

private:
    State currentState; // 当前状态
    qreal vx = 0.0;     // X轴分量速度
    qreal vy = 0.0;     // Y轴分量速度
};

#endif // TETRIS_H
