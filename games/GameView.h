// Created by 樊轩楷 & 吉佑安

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include "Player.h"


class GameView: public QGraphicsView {
    Q_OBJECT
public:
    GameView(const int moveMode); // 构造函数 加载地图 & 启动游戏

    // 缩放保持 edge 不变
    // const int edge = 100;
    void resizeEvent(QResizeEvent *event) override;

    // 画背景
    void drawBackground(QPainter *painter, const QRectF &rect) override;

signals: // 定义信号的关键字
    void gameEnded(); // 声明一个游戏结束的信号

private:
    QGraphicsScene *scene; // 存储地图实体 & 处理碰撞检测

    QTimer *gameTimer;  // 每帧刷新（移动、碰撞）
    QTimer *enemySpawnTimer; // 定时生成敌人
    QTimer *abilitySpawnTimer; // 定时生成技能
    QTimer *formationSpawnTimer; // 定时生成阵型

    bool keyW = false, keyA = false, keyS = false, keyD = false;
    bool keyUp = false, keyLeft = false, keyDown = false, keyRight = false;

    Player* player; // 玩家
    const int moveMode;
    QPointF mousePos = {0.0, 0.0};

    int enemySpawnNum = 6; // 敌人生成个数
    int formationSpawnNum = 1; // 阵型生成个数

    const int mapWidth = 800, mapHeight = 500;


protected:
    // 按键
    void keyPressEvent(QKeyEvent *event) override; // 按压 -> true
    void keyReleaseEvent(QKeyEvent *event) override; // 松开 -> false
    void mouseMoveEvent(QMouseEvent *event) override; // 储存鼠标在scene中相对左上角坐标

    void updateGame(); // 更新战场

    void spawnEnemy(); // 生成敌人

    void generateAbility(); // 生成技能

    void spawnFormation(); // 生成阵型

    void gameOver(); // 游戏结束

};


#endif // GAMEVIEW_H
