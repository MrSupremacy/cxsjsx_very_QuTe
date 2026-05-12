#include "GameView.h"
#include "Player.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QtMath> // 提供 qSin, qCos 和 M_PI
#include <QMessageBox>

GameView::GameView(const int moveMode)
    : moveMode(moveMode)
{
    // 基本设置
    setMouseTracking(true);

    // 创建场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, mapWidth, mapHeight); // 800 * 500 像素
    setScene(scene);

    // 创建背景
    QGraphicsRectItem *backgroundItem = new QGraphicsRectItem(scene->sceneRect());
    backgroundItem->setBrush(QBrush(Qt::darkGreen)); // 设置为浅灰色
    backgroundItem->setPen(Qt::NoPen); // 移除边框
    backgroundItem->setZValue(-1); // 设置 Z 值为最低，确保它在所有其他图元的下方
    scene->addItem(backgroundItem);

    // 创建玩家
    player = new Player();
    player->setPos(400, 250); // 放在地图中间
    scene->addItem(player);

    // 主循环 计时器 60 FPS
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameView::updateGame);
    gameTimer->start(16);

    // 敌人生成 计时器
    enemySpawnTimer = new QTimer(this);
    connect(enemySpawnTimer, &QTimer::timeout, this, &GameView::spawnEnemy);
    enemySpawnTimer->start(3000);

    // 技能生成 计时器
    abilitySpawnTimer = new QTimer(this);
    connect(abilitySpawnTimer, &QTimer::timeout, this, &GameView::generateAbility);
    abilitySpawnTimer->start(8000);
}

void GameView::mouseMoveEvent(QMouseEvent *event) {
    mousePos = mapToScene(event->pos());
    QGraphicsView::mouseMoveEvent(event);
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
    if (moveMode == 0) {
        player->keyboardMove(keyW, keyA, keyS, keyD, keyUp, keyLeft, keyDown, keyRight);
    } else if (moveMode == 1) {
        // QPointF sceneWH = {scene->width(), scene->height()};
        player->mouseMove(mousePos, 0.1);
    }

    // 2. 遍历场景中的所有物体，让它们移动
    QList<QGraphicsItem *> items = scene->items(); // 获取场景所有物体
    for (QGraphicsItem *item : std::as_const(items)) {
        // 遍历敌人
        Enemy *enemy = dynamic_cast<Enemy*>(item);
        if (enemy) {
            enemy->moveTowardsTarget(); // 敌人朝玩家移动
            enemy->teleportThroughWall(); //敌人有可能穿越
        }

        // 遍历技能
        Ability* ability = dynamic_cast<Ability*>(item);
        if(ability) {
            ability->updateFloating(); // 更新浮动效果
        }
    }

    // 3. 剑和敌人的碰撞检测
    if (player->getSword()->isVisible()) {
        QList<QGraphicsItem*> swordHits = player->getSword()->collidingItems();
        for (QGraphicsItem* item : std::as_const(swordHits)) {
            Enemy* enemy = dynamic_cast<Enemy*>(item);
            if (enemy) {
                // 怪物碰到了剑！杀掉它！
                scene->removeItem(enemy);
                delete enemy;
            }
        }
    }

    // 4. 玩家和敌人/技能的碰撞检测
    // player->collidingItems() 会返回当前与玩家重叠的所有物体
    QList<QGraphicsItem *> collisions = player->collidingItems();
    for (QGraphicsItem *item : std::as_const(collisions)) {
        // 检测技能
        Ability *ability = dynamic_cast<Ability *>(item);

        if (ability) {
            ability->pickUp();        // 触发技能效果
            scene->removeItem(ability);   // 从地图上移除
            delete ability;           // 销毁内存
        }


        // 检测敌人 碰到了敌人！游戏结束
        if (dynamic_cast<Enemy*>(item)) {
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

    // 循环生成坐标，直到生成的坐标在地图范围内
    // 假设你的地图 (Scene) 大小是 800 x 500

    for(int i = 0; i < spawnNum; i ++) {
        while (!validPos) {
            // 生成一个 0 到 2π 之间的随机弧度 (相当于 0 到 360 度)
            qreal angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;

            // 这样敌人就会刷在距离玩家 150 到 600 的环形区域内
            qreal distance = 150.0 + QRandomGenerator::global()->generateDouble() * 400.0;

            // 根据极坐标公式算出生成的 X 和 Y 坐标
            spawnX = px + distance * qCos(angle);
            spawnY = py + distance * qSin(angle);

            // 检查生成的坐标是否在地图内部
            // 如果超出了地图边界，validPos 依然是 false，while 循环会重新生成一次
            if (spawnX >= 0 && spawnX <= mapWidth && spawnY >= 0 && spawnY <= mapHeight) {
                validPos = true;
            }
        }

        // 在合法坐标处生成并添加敌人
        Enemy *enemy = new Enemy(player);
        enemy->setPos(spawnX, spawnY); // 使用刚刚算出的安全坐标
        scene->addItem(enemy);
        validPos = false;
    }
}

void GameView::generateAbility() {
    if(!player) return;

    qreal px = player->x();
    qreal py = player->y();

    qreal spawnX = 0;
    qreal spawnY = 0;
    bool validPos = false;

    // 找到合适的位置生成
    while (!validPos) {
        // 生成一个 0 到 2π 之间的随机弧度 (相当于 0 到 360 度)
        qreal angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;

        // 这样技能就会刷在距离玩家 200 到 700 的环形区域内
        qreal distance = 200.0 + QRandomGenerator::global()->generateDouble() * 500.0;

        // 根据极坐标公式算出生成的 X 和 Y 坐标
        spawnX = px + distance * qCos(angle);
        spawnY = py + distance * qSin(angle);

        // 检查生成的坐标是否在地图内部
        // 如果超出了地图边界，validPos 依然是 false，while 循环会重新生成一次
        if (spawnX >= 0 && spawnX <= mapWidth && spawnY >= 0 && spawnY <= mapHeight) {
            validPos = true;
        }
    }

    // 生成新技能（目前只是基类，后续通过随机数实现随机生成某个）
    Ability* ability = new LightSaber({spawnX, spawnY}, player);
    ability->setPos(spawnX, spawnY);
    scene->addItem(ability);


}

void GameView::gameOver() {
    // 1. 停止游戏循环和生成敌人的定时器
    gameTimer->stop();
    enemySpawnTimer->stop();
    abilitySpawnTimer->stop();

    // 2. 弹出一个提示框告诉玩家游戏结束（体验更好，不会死得太突兀）
    QMessageBox::information(this, "Game Over", "你被敌人抓住了！\n点击确定返回主菜单。");

    // 3. 发出“游戏结束”的信号！
    emit gameEnded();

    // 4. 关闭当前游戏窗口（因为我们之前设置了 WA_DeleteOnClose，这里 close() 会自动释放内存）
    this->close();
}
