#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include "Player.h"
#include "Enemy.h"

class GameView: public QGraphicsView {
    Q_OBJECT
public:
    GameView(); // 构造函数 加载地图 & 启动游戏

signals: // 定义信号的关键字
    void gameEnded(); // 声明一个游戏结束的信号

private:
    QGraphicsScene *scene; // 存储地图实体 & 处理碰撞检测

    QTimer *gameTimer;  // 每帧刷新（移动、碰撞）
    QTimer *enemySpawnTimer; // 定时生成敌人

    bool keyW, keyA, keyS, keyD, keyUp, keyLeft, keyDown, keyRight; // 记录按键状态

    Player* player; // 玩家

    int spawn_num = 5; // 敌人生成个数



protected:
    // 按键
    void keyPressEvent(QKeyEvent *event) override; // 按压 -> true
    void keyReleaseEvent(QKeyEvent *event) override; // 松开 -> false

    void updateGame(); // 更新战场

    void spawnEnemy(); // 生成敌人

    void gameOver(); // 游戏结束

};


#endif // GAMEVIEW_H
