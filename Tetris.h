// // Created by 樊轩楷

// #ifndef TETRIS_H
// #define TETRIS_H

// #include "Formation.h"
// #include "Enemy.h"
// #include <QtMath>
// #include <QTimer>
// #include <QGraphicsScene>

// class TetrisFormation: public Formation {
//     Q_OBJECT

// public:
//     // 定义两个阵型状态
//     enum class State {
//         Waiting,  // 阶段一：原地组合 给予玩家反应时间
//         Bouncing  // 阶段二：开始弹墙
//     };

//     TetrisFormation(QGraphicsItem* playerTarget, int time, QObject* parent = nullptr) :
//         Formation(playerTarget, time, parent), currentState(State::Waiting)
//     {
//         // 构建阵型
//         Enemy* e1 = new Enemy(playerTarget);
//         Enemy* e2 = new Enemy(playerTarget);
//         Enemy* e3 = new Enemy(playerTarget);
//         Enemy* e4 = new Enemy(playerTarget);
//         Enemy* e5 = new Enemy(playerTarget);
//         Enemy* e6 = new Enemy(playerTarget);
//         Enemy* e7 = new Enemy(playerTarget);
//         Enemy* e8 = new Enemy(playerTarget);
//         Enemy* e9 = new Enemy(playerTarget);

//         double dx = 9.0, dy = 9.0;
//         e1->setPos(0, 0);
//         e2->setPos(dx, 0);
//         e3->setPos(2 * dx, 0);
//         e4->setPos(2 * dx, dy);
//         e5->setPos(dx, dy);
//         e6->setPos(0, dy);
//         e7->setPos(0, 2 * dy);
//         e8->setPos(dx, 2 * dy);
//         e9->setPos(2 * dx, 2 * dy);

//         e1->setInFormation(true); e2->setInFormation(true);
//         e3->setInFormation(true); e4->setInFormation(true);
//         e5->setInFormation(true); e6->setInFormation(true);
//         e7->setInFormation(true); e8->setInFormation(true);
//         e9->setInFormation(true);

//         this->addToGroup(e1); this->addToGroup(e2);
//         this->addToGroup(e3); this->addToGroup(e4);
//         this->addToGroup(e5); this->addToGroup(e6);
//         this->addToGroup(e7); this->addToGroup(e8);
//         this->addToGroup(e9);

//         // 修改速度 (弹射速度)
//         Formation::speed = 4.0f;

//         // 此阵型永久存在
//         if (lifeTimer) lifeTimer->stop();

//         QTimer::singleShot(1000, this, [this]() {
//             if (this->currentState == State::Aiming) {
//                 this->currentState = State::Bouncing;
//             }
//         });
//     }

//     void rotate() override {
//         // 不用旋转
//         return;
//     }

//     void move() override {
//         if (!scene()) return;

//         if (currentState == State::Waiting) {
//             // 第一阶段：不动
//             return;
//         }
//         else if (currentState == State::Bouncing) {

//         }
//     }

// private:
//     State currentState; // 当前状态
// };

// #endif // TETRIS_H
