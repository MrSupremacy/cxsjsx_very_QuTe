// Created by 樊轩楷

#ifndef ARROW_H
#define ARROW_H

#include "Formation.h"
#include "Enemy.h"
#include <QtMath>         // 用于计算角度 qAtan2, qSin, qCos
#include <QTimer>         // 用于 2秒 状态切换
#include <QGraphicsScene>

class ArrowFormation: public Formation {
    Q_OBJECT
public:
    // 定义阵型的两个状态
    enum class State {
        Aiming,  // 阶段一：原地旋转瞄准
        Dashing  // 阶段二：保持方向猛冲
    };

    ArrowFormation(QGraphicsItem* playerTarget, int time, QObject* parent = nullptr) :
        Formation(playerTarget, time, parent), currentState(State::Aiming)
    {
        // --- 你的布阵代码 ---
        Enemy* e1 = new Enemy(playerTarget);
        Enemy* e2 = new Enemy(playerTarget);
        Enemy* e3 = new Enemy(playerTarget);
        Enemy* e4 = new Enemy(playerTarget);
        Enemy* e5 = new Enemy(playerTarget);
        Enemy* e6 = new Enemy(playerTarget);
        Enemy* e7 = new Enemy(playerTarget);

        double dx = 24.0, dy = 12.0;
        e1->setPos(0, 0);
        e2->setPos(dx, dy);
        e3->setPos(2 * dx, 2 * dy);
        e4->setPos(3 * dx, 3 * dy);
        e5->setPos(2 * dx, 4 * dy); // 箭头尖端
        e6->setPos(1 * dx, 5 * dy);
        e7->setPos(0, 6 * dy);

        e1->setInFormation(true); e2->setInFormation(true);
        e3->setInFormation(true); e4->setInFormation(true);
        e5->setInFormation(true); e6->setInFormation(true);
        e7->setInFormation(true);

        this->addToGroup(e1); this->addToGroup(e2);
        this->addToGroup(e3); this->addToGroup(e4);
        this->addToGroup(e5); this->addToGroup(e6);
        this->addToGroup(e7);

        // 修改速度 (冲刺速度)
        Formation::speed = 14.0f; // 可以调快一点体现“猛冲”

        // --- 核心新增配置 ---

        // 1. 设置旋转中心：如果你不设置，阵型会绕着(0,0)也就是e1旋转，看起来很怪

        // 所以我们把旋转中心设在图形正中间：
        this->setTransformOriginPoint(30.0, 33.0);

        // 2. 停用基类的自动销毁计时器（因为我们现在要在碰到墙壁时才解散）
        // 如果基类的生命周期计时器还在，可以在这里把它停掉，比如：
        if (lifeTimer) lifeTimer->stop();

        // 3. 设置2秒后进入“猛冲”状态 (第一阶段 -> 第二阶段)
        QTimer::singleShot(2500, this, [this]() {
            if (this->currentState == State::Aiming) {
                this->currentState = State::Dashing;
            }
        });
    }

    void rotate() override {
        // 只有在瞄准阶段才旋转，冲刺阶段保持不变
        if (!playerTarget || currentState != State::Aiming) return;

        // 获取旋转中心点在地图上的真实绝对坐标
        QPointF centerPos = mapToScene(transformOriginPoint());
        QPointF pPos = playerTarget->scenePos();

        // 计算 X 和 Y 的差值
        qreal deltaX = pPos.x() - centerPos.x();
        qreal deltaY = pPos.y() - centerPos.y();

        // 使用 qAtan2 计算弧度，并转为角度 (Qt 的旋转是顺时针的角度)
        qreal angle = qRadiansToDegrees(qAtan2(deltaY, deltaX));

        // 设置阵型的旋转角度 (因为你的箭头天生就是朝右(+x)的，所以 0 度正好对应正确朝向，无需额外偏移)
        this->setRotation(angle);
    }

    void move() override {
        if (!scene()) return;

        if (currentState == State::Aiming) {
            // 第一阶段：只调用旋转，原地不动
            rotate();
        }
        else if (currentState == State::Dashing) {
            // 第二阶段：猛冲

            // 1. 根据当前锁定的角度计算冲刺向量
            qreal angleRad = qDegreesToRadians(this->rotation());
            qreal moveX = qCos(angleRad) * speed;
            qreal moveY = qSin(angleRad) * speed;
            this->moveBy(moveX, moveY);

            // 第三阶段：检测是否撞墙
            QRectF mapRect = scene()->sceneRect();
            QList<QGraphicsItem*> children = this->childItems();

            for (QGraphicsItem* item : children) {
                Enemy* enemy = dynamic_cast<Enemy*>(item);
                if (enemy) {
                    // 获取敌人在场景中的绝对包围盒 (sceneBoundingRect 考虑了阵型的旋转和位移)
                    QRectF enemyRect = enemy->sceneBoundingRect();

                    // 如果该敌人的任意边界超出了地图范围
                    if (enemyRect.left() <= mapRect.left() ||
                        enemyRect.right() >= mapRect.right() ||
                        enemyRect.top() <= mapRect.top() ||
                        enemyRect.bottom() >= mapRect.bottom())
                    {
                        // 触发解散！
                        deformation();
                        return; // 立即跳出，防止解散后继续执行下方代码报错
                    }
                }
            }
        }
    }

private:
    State currentState; // 当前状态
};

#endif // ARROW_H