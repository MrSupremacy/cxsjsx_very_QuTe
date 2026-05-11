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
    GameView(const int moveMode); // 构造函数 加载地图 & 启动游戏

signals: // 定义信号的关键字
    void gameEnded(); // 声明一个游戏结束的信号

private:
    QGraphicsScene *scene; // 存储地图实体 & 处理碰撞检测

    QTimer *gameTimer;  // 每帧刷新（移动、碰撞）
    QTimer *enemySpawnTimer; // 定时生成敌人

    bool keyW = false, keyA = false, keyS = false, keyD = false;
    bool keyUp = false, keyLeft = false, keyDown = false, keyRight = false;

    Player* player; // 玩家
    const int moveMode;
    QPointF mousePos = {0.0, 0.0};

    int spawnNum = 10; // 敌人生成个数

    int mapWidth = 800, mapHeight = 500; // 地图尺寸



protected:
    // 按键
    void keyPressEvent(QKeyEvent *event) override; // 按压 -> true
    void keyReleaseEvent(QKeyEvent *event) override; // 松开 -> false
    void mouseMoveEvent(QMouseEvent *event) override; // 储存鼠标在scene中相对左上角坐标

    void updateGame(); // 更新战场

    void spawnEnemy(); // 生成敌人

    void gameOver(); // 游戏结束

};


#endif // GAMEVIEW_H
