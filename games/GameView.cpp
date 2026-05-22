// Created by 樊轩楷 & 吉佑安

#include "GameView.h"
#include "Player.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QtMath> // 提供 qSin, qCos 和 M_PI
#include <QMessageBox>

#include "BulletPool.h"
#include "Enemy.h"
#include "Ability.h"
#include "LightSaber.h"
#include "Lochunhin.h"
#include "WipeOut.h"
#include "Explosion.h"
#include "Shield.h"
#include "Formation.h"
#include "Arrow.h"


GameView::GameView(const int moveMode)
    : moveMode(moveMode)
{
    // 基本设置
    setMouseTracking(true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    // 另外，关闭抗锯齿可以大幅提升性能（如果不需要特别圆滑的边缘）
    // view->setRenderHint(QPainter::Antialiasing, false);

    // 创建场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, mapWidth, mapHeight); // 800 * 500 像素
    setScene(scene);

    // 创建背景
    QGraphicsRectItem *backgroundItem = new QGraphicsRectItem(scene->sceneRect());
    backgroundItem->setBrush(QBrush(Qt::lightGray)); // 设置为浅灰色
    backgroundItem->setPen(Qt::NoPen); // 移除边框
    backgroundItem->setZValue(-1); // 设置 Z 值为最低，确保它在所有其他图元的下方
    scene->addItem(backgroundItem);

    // 创建玩家
    player = new Player();
    player->setPos(400, 250); // 放在地图中间
    scene->addItem(player);

    // 初始化 BulletPool
    BulletPool::getInstance().addToScene(scene);

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
    abilitySpawnTimer->start(4000);

    formationSpawnTimer = new QTimer(this);
    connect(formationSpawnTimer, &QTimer::timeout, this, &GameView::spawnFormation);
    formationSpawnTimer->start(6000);
}

GameView::~GameView()
{
    // 游戏窗口销毁时，清空子弹对象池释放内存
    // BulletPool::getInstance().clear();
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
    // 移动玩家
    if (moveMode == 0) {
        player->keyboardMove(keyW, keyA, keyS, keyD, keyUp, keyLeft, keyDown, keyRight);
    } else if (moveMode == 1) {
        player->mouseMove(mousePos);
    }

    // 1. 准备一个集合，记录这一帧需要被删除的物体
    QSet<QGraphicsItem*> itemsToRemove;

    // 获取场景所有物体
    QList<QGraphicsItem *> items = scene->items();

    // 第一遍遍历：逻辑更新与碰撞记录
    for (QGraphicsItem *item : std::as_const(items)) {
        // 【关键】如果这个物体在之前的逻辑里已经被标为删除了，直接跳过
        if (itemsToRemove.contains(item)) continue;

        // 敌人移动
        if (Enemy *enemy = dynamic_cast<Enemy*>(item)) {
            enemy->moveTowardsTarget();
            enemy->teleportThroughWall();
        }
        // 阵型移动
        else if (Formation *formation = dynamic_cast<Formation*>(item)) {
            formation->move();
        }
        // 技能浮动
        else if (Ability* ability = dynamic_cast<Ability*>(item)) {
            ability->updateFloating();
        }
        // 子弹逻辑
        else if (Bullet* bullet = dynamic_cast<Bullet*>(item)) {
            // 如果 updatePosition 返回 false（撞墙或超时），直接标记待处理并跳过碰撞检测
            if (!bullet->updatePosition()) {
                itemsToRemove.insert(bullet);
                continue;
            }

            // 子弹碰撞检测
            QList<QGraphicsItem*> bulletCollisions = bullet->collidingItems();
            for (QGraphicsItem* colItem : std::as_const(bulletCollisions)) {
                if (Enemy* e = dynamic_cast<Enemy*>(colItem)) {
                    // 标记敌人和子弹都要移除
                    itemsToRemove.insert(e);
                    itemsToRemove.insert(bullet);
                    break; // 停止检测这个子弹
                }
            }
        }
        // 咖喱棒剑气碰撞检测
        else if (CrescentWave* wave = dynamic_cast<CrescentWave*>(item)) {
            // 已经自己处理移动和碰壁删除了。见 CrescentWave.h
            // 子弹碰撞检测
            QList<QGraphicsItem*> waveCollisions = wave->collidingItems();
            for (QGraphicsItem* colItem : std::as_const(waveCollisions)) {
                if (Enemy* e = dynamic_cast<Enemy*>(colItem)) {
                    // 标记敌人要删除
                    itemsToRemove.insert(e);
                }
            }
        }
        // 爆炸区域碰撞检测
        else if (Explosion* explosion = dynamic_cast<Explosion*>(item)) {
            // 清除所有碰到的敌人
            QList<QGraphicsItem*> explosionCollisions = explosion->collidingItems();
            for (QGraphicsItem* colItem : std::as_const(explosionCollisions)) {
                if (Enemy* e = dynamic_cast<Enemy*>(colItem)) {
                    // 标记敌人要删除
                    itemsToRemove.insert(e);
                }
            }
        }
    }

    // 2. 剑的碰撞检测（同样使用标记法）
    if (player->getSword()->isVisible()) {
        QList<QGraphicsItem*> swordHits = player->getSword()->collidingItems();
        for (QGraphicsItem* item : std::as_const(swordHits)) {
            if (Enemy* enemy = dynamic_cast<Enemy*>(item)) {
                itemsToRemove.insert(enemy);
            }
        }
    }

    // 3. 护盾的碰撞检测
    if (player->getShield()->isVisible()) {
        QList<QGraphicsItem*> shieldHits = player->getShield()->collidingItems();
        for (QGraphicsItem* item : std::as_const(shieldHits)) {
            if (Enemy* enemy = dynamic_cast<Enemy*>(item)) {
                player->breakShieldAndExplode();
                break;
            }
        }
    }

    // 4. 玩家的碰撞检测
    QList<QGraphicsItem *> playerCollisions = player->collidingItems();
    for (QGraphicsItem *item : std::as_const(playerCollisions)) {
        if (itemsToRemove.contains(item)) continue; // 如果该物体已被子弹打死，就不算撞到玩家

        if (Ability *ability = dynamic_cast<Ability *>(item)) {
            ability->pickUp();
            itemsToRemove.insert(ability);
        }
        else if (dynamic_cast<Enemy*>(item)) {
            if(!player->getIsImmune()) {
                gameOver();
                return;
            }
        }
    }

    // 4. 最后统一物理销毁（这一步才真正 delete）
    for (QGraphicsItem* item : itemsToRemove) {
        // 防止重复删除
        if (item->scene() == scene) {
            // 类型判断
            if (Bullet* b = dynamic_cast<Bullet*>(item)) {
                // 如果是子弹，不要 delete，把它交还给对象池
                BulletPool::getInstance().recycle(b);
            } else {
                // 如果是敌人等其他物品，按原计划物理移除并销毁
                scene->removeItem(item);
                delete item;
            }
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

    for(int i = 0; i < enemySpawnNum; i ++) {
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

    // 生成新技能
    int randomValue = QRandomGenerator::global()->bounded(4);
    Ability* ability = nullptr;

    switch (randomValue) {
    case 0:
        ability = new LightSaber({spawnX, spawnY}, player);
        break;
    case 1:
        ability = new WipeOut({spawnX, spawnY}, player);
        break;
    case 2:
        ability = new Lochunhin({spawnX, spawnY}, player);
        break;
    case 3:
        ability = new Shield({spawnX, spawnY}, player);
    }
    if (ability) {
        ability->setPos(spawnX, spawnY);
        scene->addItem(ability);
    }
}

void GameView::spawnFormation() {
    if(!player || !scene) return;

    // 建议统一使用 scenePos() 获取绝对坐标，防止玩家被编入其他节点后 x() 失准
    qreal px = player->scenePos().x();
    qreal py = player->scenePos().y();

    for(int i = 0; i < formationSpawnNum; i++) {

        // 1. 先把阵型 new 出来 (这里以 ArrowFormation 为例，你也可以加个随机数来决定生成哪种阵型)
        Formation *formation = new ArrowFormation(player, 0, this); // 参数按你实际的构造函数来

        // 获取阵型的本地边界框！
        // 它会返回一个 QRectF，包含了这个阵型所有敌人组合起来的总宽度和总高度
        // 例如 formRect.left() 是最左侧敌人边缘的相对坐标，formRect.right() 是最右侧的
        QRectF formRect = formation->boundingRect();

        // 依次出现在屏幕上
        formation->beginSequentialSpawn(64);

        qreal spawnX = 0;
        qreal spawnY = 0;
        bool validPos = false;

        // --- 核心：安全防卡死机制 ---
        int maxAttempts = 100; // 最多尝试 100 次
        int attempts = 0;

        // 2. 循环生成坐标，直到完全在地图范围内，或者超过最大尝试次数
        while (!validPos && attempts < maxAttempts) {
            attempts++;

            qreal angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;
            qreal distance = 250 + QRandomGenerator::global()->generateDouble() * 300.0;

            spawnX = px + distance * qCos(angle);
            spawnY = py + distance * qSin(angle);

            // 检查整个阵型的四条边是否都在地图内
            // spawnX 和 spawnY 是阵型的中心(0,0)，加上 boundingBox 的边缘值就是真实边缘
            bool isLeftSafe = (spawnX + formRect.left()) >= 0;
            bool isRightSafe = (spawnX + formRect.right()) <= mapWidth;
            bool isTopSafe = (spawnY + formRect.top()) >= 0;
            bool isBottomSafe = (spawnY + formRect.bottom()) <= mapHeight;

            if (isLeftSafe && isRightSafe && isTopSafe && isBottomSafe) {
                validPos = true;
            }
        }

        // 3. 兜底处理：如果 100 次都没找到合法坐标（比如玩家被堵在墙角）
        // 我们就强行把阵型的坐标限制（Clamp）在地图的安全边缘，确保它绝对不会出界
        if (!validPos) {
            // qBound(min, value, max) 会把 value 强行限制在 min 和 max 之间
            spawnX = qBound(-formRect.left(), spawnX, mapWidth - formRect.right());
            spawnY = qBound(-formRect.top(), spawnY, mapHeight - formRect.bottom());
        }

        // 4. 将坐标赋给阵型，并加入地图
        formation->setPos(spawnX, spawnY);
        scene->addItem(formation);

    }
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
