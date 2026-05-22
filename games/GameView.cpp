// Created by 樊轩楷 & 吉佑安

#include "GameView.h"
#include "Player.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QtMath> // 提供 qSin, qCos 和 M_PI
#include <QMessageBox>
#include <QColor>

#include "BulletPool.h"
#include "Enemy.h"
#include "Ability.h"
#include "LightSaber.h"
#include "Lochunhin.h"
#include "WipeOut.h"
#include "Explosion.h"
#include "Shield.h"
#include "IntelligentWipeOut.h"
#include "Formation.h"
#include "Arrow.h"
#include "Tetris.h"
#include "Circle.h"

GameView::GameView(const DataCarrier& dc)
    : difficulty(dc.difficulty)
    , volume(dc.volume)
    , timeLimited(dc.timeLimited)
    , maxSeconds(dc.maxSeconds)
    , moveMode(dc.moveMode)
{
    // 基本设置
    setMouseTracking(true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    // 另外，关闭抗锯齿可以大幅提升性能（如果不需要特别圆滑的边缘）
    // view->setRenderHint(QPainter::Antialiasing, false);

    // 创建 计时板、计分板 & 设置样式
    scoreRecordBoard = new QLabel("Score:    0", this);
    scoreRecordBoard->setStyleSheet(R"(
        QLabel {
           color: white;
           font-family: 'Consolas';
           font-size: 16px;
           font-weight: bold;
           background-color: rgba(0, 0, 0, 95);
           border-radius: 5px;
           padding: 5px;
        }
    )");

    QString initT = timeLimited
        ? QString("%1:%2")
            .arg(maxSeconds / 60, 2, 10, QChar('0'))
            .arg(maxSeconds % 60, 2, 10, QChar('0'))
        : "00:00";
    timeRecordBoard = new QLabel(initT, this);
    timeRecordBoard->setStyleSheet(R"(
        QLabel {
           color: white;
           font-family: 'Consolas';
           font-size: 16px;
           font-weight: bold;
           background-color: rgba(0, 0, 0, 95);
           border-radius: 5px;
           padding: 5px;
        }
    )");

    scoreRecordBoard->setGeometry(10, 10, 120, 30);
    // 计时 Timer
    secondTimer = new QTimer(this);
    // 绑定
    connect(secondTimer, &QTimer::timeout, this, [&]() {
        seconds++;
        QString T = timeLimited
            ? QString("%1:%2")
                .arg((maxSeconds -seconds) / 60, 2, 10, QChar('0'))
                .arg((maxSeconds -seconds) % 60, 2, 10, QChar('0'))
            : QString("%1:%2")
                .arg(seconds / 60, 2, 10, QChar('0'))
                .arg(seconds % 60, 2, 10, QChar('0'));
        timeRecordBoard->setText(T);

        if (timeLimited && seconds >= maxSeconds)
        {
            gameTimer->stop();
            enemySpawnTimer->stop();
            abilitySpawnTimer->stop();
            formationSpawnTimer->stop();
            secondTimer->stop();

            // 弹出一个提示框告诉玩家游戏结束
            QMessageBox::information(this, "Game Over", "计时结束！\n点击确定返回主菜单。");

            emit gameEnded({scores, seconds});
            this->close();
        }
    });
    // 启动
    secondTimer->start(1000);
    timeRecordBoard->setGeometry(130, 10, 60, 30);


    // 创建场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, mapWidth, mapHeight);
    setScene(scene);

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
    enemySpawnTimer->start(5000);

    // 技能生成 计时器
    abilitySpawnTimer = new QTimer(this);
    connect(abilitySpawnTimer, &QTimer::timeout, this, &GameView::generateAbility);
    abilitySpawnTimer->start(8000);


    // 阵型生成计时器
    formationSpawnTimer = new QTimer(this);
    connect(formationSpawnTimer, &QTimer::timeout, this, &GameView::spawnFormation);
    formationSpawnTimer->start(10000);
}

void GameView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // scene->setSceneRect(edge, edge, width() -edge, height() -edge);
}

void GameView::drawBackground(QPainter *painter, const QRectF &rect) {
    // 1. 首先用“外部颜色”（例如灰色）填充整个需要绘制的区域
    painter->fillRect(rect, QColor(128, 128, 128));

    // 2. 获取场景的边界
    QRectF sRect = sceneRect();

    // 3. 计算当前绘制区域与场景区域的交集
    QRectF intersectRect = rect.intersected(sRect);

    // 4. 只在交集区域内填充“场景内部颜色”（例如黑色）
    if (!intersectRect.isEmpty()) {
        painter->fillRect(intersectRect, QColor(191, 191, 191));
    }
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
        // 导弹碰撞检测
        else if (Missile* missile = dynamic_cast<Missile*>(item)) {
            // 已经自己处理移动和碰壁删除了。见 Missile.h
            // 子弹碰撞检测
            QList<QGraphicsItem*> missileCollisions = missile->collidingItems();
            for (QGraphicsItem* colItem : std::as_const(missileCollisions)) {
                if (Enemy* e = dynamic_cast<Enemy*>(colItem)) {
                    // 标记双方要删除
                    itemsToRemove.insert(e);
                    itemsToRemove.insert(missile);
                    missile->explode();
                }
            }
            if (missile->m_isDead) itemsToRemove.insert(missile);
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
            if (item->data(0).toString() == "enemy") {
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
                // BulletPool::getInstance().recycle(b);
                scene->removeItem(b);
                delete b;
            }
            else if (item->data(0).toString() == "enemy") {
                scores++;
                scoreRecordBoard->setText(
                    QString("Score: %1").arg(scores, 4, 10, QChar(' '))
                );

                scene->removeItem(item);
                delete item;
            }
            else {
                // 如果是其他物品，按原计划物理移除并销毁
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
            if (spawnX >= 0 && spawnX <= mapWidth
                && spawnY >= 0 && spawnY <= mapHeight) {
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
        if (spawnX >= 0 && spawnX <= mapWidth
            && spawnY >= 0 && spawnY <= mapHeight) {
            validPos = true;
        }
    }

    // 生成新技能
    int randomValue = QRandomGenerator::global()->bounded(5);
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
        break;
    case 4:
        ability = new IntelligentWipeOut({spawnX, spawnY}, player);
        break;
    }
    if (ability) {
        ability->setPos(spawnX, spawnY);
        scene->addItem(ability);
    }
}


void GameView::spawnFormation() {
    if(!player || !scene) return;

    qreal px = player->scenePos().x();
    qreal py = player->scenePos().y();

    for(int i = 0; i < formationSpawnNum; i++) {

        Formation *formation = nullptr;
        bool isSpawnOnPlayer = false; // 标记是否在玩家脚下生成（包围圈专用）
        qreal spawnX = 0;
        qreal spawnY = 0;

        // 随机 0, 1, 2。假设 2 是包围圈 (Circle)
        int randomValue = QRandomGenerator::global()->bounded(3);

        // --- 特殊处理：如果是包围圈阵型 ---
        if (randomValue == 2) {
            formation = new CircleFormation(player, 0, this);
            QRectF formRect = formation->boundingRect();

            // 检测以玩家为中心生成的话，会不会出界
            bool isLeftSafe = (px + formRect.left()) >= 0;
            bool isRightSafe = (px + formRect.right()) <= mapWidth;
            bool isTopSafe = (py + formRect.top()) >= 0;
            bool isBottomSafe = (py + formRect.bottom()) <= mapHeight;

            if (isLeftSafe && isRightSafe && isTopSafe && isBottomSafe) {
                // 安全！确认生成包围圈
                isSpawnOnPlayer = true;
                spawnX = px;
                spawnY = py;
            } else {
                // 不安全！玩家离墙太近了，包围圈会出界
                // 销毁刚刚临时 new 出来的包围圈
                delete formation;
                formation = nullptr;
                // 将随机数强制降级，在 0(Arrow) 和 1(Tetris) 之间重新选一个！
                randomValue = QRandomGenerator::global()->bounded(2);
            }
        }

        // --- 常规处理：如果是箭头或方块（包含被降级退回来的情况） ---
        if (!isSpawnOnPlayer) {
            switch(randomValue) {
            case 0:
                formation = new ArrowFormation(player, 0, this);
                break;
            case 1:
                formation = new TetrisFormation(player, 11000, this); // 11s后解散
                break;
            }

            QRectF formRect = formation->boundingRect();
            bool validPos = false;
            int maxAttempts = 100;
            int attempts = 0;

            // 在玩家附近随机找一个安全点生成
            while (!validPos && attempts < maxAttempts) {
                attempts++;

                qreal angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;
                qreal distance = 250 + QRandomGenerator::global()->generateDouble() * 300.0;

                spawnX = px + distance * qCos(angle);
                spawnY = py + distance * qSin(angle);

                bool isLeftSafe = (spawnX + formRect.left()) >= 0;
                bool isRightSafe = (spawnX + formRect.right()) <= mapWidth;
                bool isTopSafe = (spawnY + formRect.top()) >= 0;
                bool isBottomSafe = (spawnY + formRect.bottom()) <= mapHeight;

                if (isLeftSafe && isRightSafe && isTopSafe && isBottomSafe) {
                    validPos = true;
                }
            }

            // 兜底处理
            if (!validPos) {
                spawnX = qBound(-formRect.left(), spawnX, mapWidth - formRect.right());
                spawnY = qBound(-formRect.top(), spawnY, mapHeight - formRect.bottom());
            }
        }

        // --- 通用步骤：无论哪种阵型，都在这里统一调用显示和加入场景 ---

        // 依次出现在屏幕上 (包围圈一个个画圈出现的效果也非常惊艳！)
        formation->beginSequentialSpawn(64);

        formation->setPos(spawnX, spawnY);
        scene->addItem(formation);
    }
}


void GameView::gameOver() {
    // 停止定时器
    gameTimer->stop();
    enemySpawnTimer->stop();
    abilitySpawnTimer->stop();
    formationSpawnTimer->stop();
    secondTimer->stop();

    // 2. 弹出一个提示框告诉玩家游戏结束（体验更好，不会死得太突兀）
    QMessageBox::information(this, "Game Over", "你被敌人抓住了！\n点击确定返回主菜单。");

    // 3. 发出“游戏结束”的信号！
    emit gameEnded({scores, seconds});

    // 4. 关闭当前游戏窗口（因为我们之前设置了 WA_DeleteOnClose，这里 close() 会自动释放内存）
    this->close();
}
