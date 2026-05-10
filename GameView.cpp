#include "GameView.h"
#include "Player.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QtMath> // 提供 qSin, qCos 和 M_PI
#include <QMessageBox>

GameView::GameView() {
    // 创建场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, 800, 600); // 800 * 600 像素
    setScene(scene);

    // 创建玩家
    player = new Player();
    player->setPos(400, 300); // 放在地图中间
    scene->addItem(player);

    // 主循环 计时器 60 FPS
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameView::updateGame);
    gameTimer->start(16);

    // 敌人生成 计时器
    enemySpawnTimer = new QTimer(this);
    connect(enemySpawnTimer, &QTimer::timeout, this, &GameView::spawnEnemy);
    enemySpawnTimer->start(2000);


}

void GameView::keyPressEvent(QKeyEvent *event) {
    // 关键点1：忽略长按系统自动生成的重复按键事件
    if (event->isAutoRepeat()) {
        return;
    }

    // 根据按下的按键，将对应的标志位设为 true
    switch (event->key()) {
    case Qt::Key_W: keyW = true; break;
    case Qt::Key_A: keyA = true; break;
    case Qt::Key_S: keyS = true; break;
    case Qt::Key_D: keyD = true; break;
    case Qt::Key_Up: keyUp = true; break;
    case Qt::Key_Left: keyLeft = true; break;
    case Qt::Key_Right: keyRight = true; break;
    case Qt::Key_Down: keyDown = true; break;
    }

    // 把事件传递给父类（如果不写这句，可能会导致某些默认快捷键失效）
    QGraphicsView::keyPressEvent(event);
}

void GameView::keyReleaseEvent(QKeyEvent *event) {
    // 同样忽略重复的释放事件
    if (event->isAutoRepeat()) {
        return;
    }

    // 按键松开，将标志位设为 false
    switch (event->key()) {
    case Qt::Key_W: keyW = false; break;
    case Qt::Key_A: keyA = false; break;
    case Qt::Key_S: keyS = false; break;
    case Qt::Key_D: keyD = false; break;
    case Qt::Key_Up: keyUp = false; break;
    case Qt::Key_Left: keyLeft = false; break;
    case Qt::Key_Right: keyRight = false; break;
    case Qt::Key_Down: keyDown = false; break;
    }

    QGraphicsView::keyReleaseEvent(event);
}

void GameView::updateGame() {
    // 1. 移动玩家
    player->move(keyW, keyA, keyS, keyD, keyUp, keyLeft, keyDown, keyRight);

    // 2. 遍历场景中的所有敌人，让它们移动并检测碰撞
    QList<QGraphicsItem *> items = scene->items(); // 获取场景所有物体
    for (QGraphicsItem *item : items) {
        Enemy *enemy = dynamic_cast<Enemy*>(item);
        if (enemy) {
            enemy->moveTowardsTarget(); // 敌人朝玩家移动
        }
    }

    // 3. 碰撞检测 (死亡判定)
    // player->collidingItems() 会返回当前与玩家重叠的所有物体
    QList<QGraphicsItem *> collisions = player->collidingItems();
    for (QGraphicsItem *item : collisions) {
        if (dynamic_cast<Enemy*>(item)) {
            // 碰到了敌人！游戏结束
            gameOver();
            return;
        }
    }
}

void GameView::spawnEnemy() {
    if(!player) return;

    qreal px = player->x();
    qreal py = player->y();

    qreal spawnX = 0;
    qreal spawnY = 0;
    bool validPos = false;

    // 2. 循环生成坐标，直到生成的坐标在地图范围内
    // 假设你的地图 (Scene) 大小是 800 x 600
    int mapWidth = 800;
    int mapHeight = 600;

    for(int i = 0; i < spawn_num; i ++) {
        while (!validPos) {
            // 生成一个 0 到 2π 之间的随机弧度 (相当于 0 到 360 度)
            qreal angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;

            // 生成一个距离：基础距离 100 + 随机浮动距离 (例如 0~300)
            // 这样敌人就会刷在距离玩家 100 到 400 的环形区域内
            qreal distance = 100.0 + QRandomGenerator::global()->generateDouble() * 300.0;

            // 根据极坐标公式算出生成的 X 和 Y 坐标
            spawnX = px + distance * qCos(angle);
            spawnY = py + distance * qSin(angle);

            // 检查生成的坐标是否在地图内部
            // 如果超出了地图边界，validPos 依然是 false，while 循环会重新生成一次
            if (spawnX >= 0 && spawnX <= mapWidth && spawnY >= 0 && spawnY <= mapHeight) {
                validPos = true;
            }
        }

        // 3. 在合法坐标处生成并添加敌人
        Enemy *enemy = new Enemy(player);
        enemy->setPos(spawnX, spawnY); // 使用刚刚算出的安全坐标
        scene->addItem(enemy);
        validPos = false;
    }
}

void GameView::gameOver() {
    // 1. 停止游戏循环和生成敌人的定时器
    gameTimer->stop();
    enemySpawnTimer->stop();

    // 2. 弹出一个提示框告诉玩家游戏结束（体验更好，不会死得太突兀）
    QMessageBox::information(this, "Game Over", "你被敌人抓住了！\n点击确定返回主菜单。");

    // 3. 发出“游戏结束”的信号！
    emit gameEnded();

    // 4. 关闭当前游戏窗口（因为我们之前设置了 WA_DeleteOnClose，这里 close() 会自动释放内存）
    this->close();
}


