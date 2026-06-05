// Created by 樊轩楷 & 吉佑安

#include "GameView.h"
#include "Player.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QtMath> // 提供 qSin, qCos 和 M_PI
#include <QMessageBox>
#include <QColor>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QPropertyAnimation>

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
#include "DeathVFX.h"
#include "SoundPool.h"
#include "DataCarrier.h"
#include "SpawnIndicator.h"
#include "PortalPool.h"


GameView::GameView(const DataCarrier& dc)
    : difficulty(dc.difficulty)
    , volume(dc.volume)
    , timeLimited(dc.timeLimited)
    , maxSeconds(dc.maxSeconds)
    , moveMode(dc.moveMode)
{
    // 基本设置
    QOpenGLWidget *glWidget = new QOpenGLWidget(this);
    setViewport(glWidget);
    glWidget->setMouseTracking(true);

    setMouseTracking(true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    // 另外，关闭抗锯齿可以大幅提升性能
    // view->setRenderHint(QPainter::Antialiasing, false);

    // 缓存优化
    this->setCacheMode(QGraphicsView::CacheBackground);

    // 启动 FPS 计时器
    m_fpsTimer.start();

    setWindowTitle("DTRD Game by Qute");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    // 音频
    const QHash<QString, QString> mySounds = {
        {"Arrow_hit",        "qrc:/SoundResources/Arrow_hit.wav"},
        {"Arrow_shoot",      "qrc:/SoundResources/Arrow_shoot.wav"},
        {"Enemy_die",        "qrc:/SoundResources/Enemy_die.wav"},
        {"Missile_explode",  "qrc:/SoundResources/Missile_explode.wav"},
        {"Missile_launch",   "qrc:/SoundResources/Missile_launch.wav"},
        {"Shield_break",     "qrc:/SoundResources/Shield_break.wav"},
        {"Lochunhin_fuse",   "qrc:/SoundResources/Lochunhin_fuse.wav"},
        {"Lochunhin_launch", "qrc:/SoundResources/Lochunhin_launch.wav"},
        {"Shield_get",       "qrc:/SoundResources/Shield_get.wav"},
        {"Spear_get",        "qrc:/SoundResources/Spear_get.wav"},
        {"Circle_begin",     "qrc:/SoundResources/Circle_begin.wav"},
        {"Square_begin",     "qrc:/SoundResources/Square_begin.wav"},
        {"Triangle_begin",   "qrc:/SoundResources/Triangle_begin.wav"}
    };

    SoundPool::instance().init(mySounds, volume * 0.9);

    SoundPool::instance().setSoundWeight("Arrow_hit",        0.4);
    SoundPool::instance().setSoundWeight("Arrow_shoot",      0.15);
    SoundPool::instance().setSoundWeight("Enemy_die",        0.7);
    SoundPool::instance().setSoundWeight("Missile_explode",  1.0);
    SoundPool::instance().setSoundWeight("Missile_launch",   1.0);
    SoundPool::instance().setSoundWeight("Shield_break",     1.0);
    SoundPool::instance().setSoundWeight("Lochunhin_fuse",   1.0);
    SoundPool::instance().setSoundWeight("Lochunhin_launch", 1.0);
    SoundPool::instance().setSoundWeight("Shield_get",       1.0);
    SoundPool::instance().setSoundWeight("Spear_get",        1.0);
    SoundPool::instance().setSoundWeight("Circle_begin",     0.63);
    SoundPool::instance().setSoundWeight("Square_begin",     0.7);
    SoundPool::instance().setSoundWeight("Triangle_begin",   0.7);


    m_bgmPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);

    m_bgmPlayer->setAudioOutput(audioOutput);
    m_bgmPlayer->setSource(QUrl("qrc:/bgm.ogg"));
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);
    audioOutput->setVolume(volume * 2.0);
    m_bgmPlayer->play();


    // 难度设置
    initFromDifficulty();


    // 创建 计时板、计分板 & 设置样式
    scoreRecordBoard = new QLabel("Score:    0", this);
    scoreRecordBoard->setStyleSheet(R"(
        QLabel {
           color: #1A1A1A;                  /* 深灰色/黑色文字 */
           font-family: 'Consolas';
           font-size: 22px;
           font-weight: bold;
           background-color: #FFFFFF;       /* 白色背景板 */
           border: 4px double #1A1A1A;      /* 4px 双重黑框 */
           border-radius: 2px;              /* 双线边框下圆角不宜过大，微圆角或直角视觉效果更好 */
           padding: 4px 4px;               /* 适当微调内边距 */
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
           color: #1A1A1A;
           font-family: 'Consolas';
           font-size: 22px;
           font-weight: bold;
           background-color: #FFFFFF;
           border: 4px double #1A1A1A;
           border-radius: 2px;
           padding: 4px 4px;
        }
    )");

    scoreRecordBoard->move(10, 10);
    scoreRecordBoard->adjustSize(); // 自动计算并设置合适的大小

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
            // QMessageBox::information(this, "Game Over", "计时结束！\n点击确定返回主菜单。");
            timeOutGameOver();

            emit gameEnded({scores, seconds});
            this->close();
        }
    });
    // 启动
    secondTimer->start(1000);
    int nextX = scoreRecordBoard->geometry().right() + 12;
    timeRecordBoard->move(nextX, 10);
    timeRecordBoard->adjustSize();


    // 创建场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, mapWidth, mapHeight);

    QPixmap bgPixmap(globalSkin::applyChoice("Background"));

    bgPixmap = bgPixmap.scaled(1250, 720, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    scene->setBackgroundBrush(QBrush(bgPixmap));

    // 场景边框
    borderPixmap = QPixmap(globalSkin::applyChoice("GroundBorder"));
    borderParas = globalSkin::instance().applyBorderParas();

    setScene(scene);

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
    enemySpawnTimer->start(enemyIntv);

    // 技能生成 计时器
    abilitySpawnTimer = new QTimer(this);
    connect(abilitySpawnTimer, &QTimer::timeout, this, &GameView::generateAbility);
    abilitySpawnTimer->start(abilityIntv);


    // 阵型生成计时器
    formationSpawnTimer = new QTimer(this);
    connect(formationSpawnTimer, &QTimer::timeout, this, &GameView::spawnFormation);
    formationSpawnTimer->start(formationIntv);

    // 传送门 Pool
    PortalPool::instance().init(scene, ":/ImageResources/np.png", 33, 33, 25);
}

void GameView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // scene->setSceneRect(edge, edge, width() -edge, height() -edge);
}

void GameView::initializeShader() {
    initializeOpenGLFunctions();

    // 顶点着色器：现在只需要全屏顶点和对应纹理坐标
    const char *vsrc = R"(
#version 440 core
layout (location = 0) in vec2 aPos;        // NDC 坐标
layout (location = 1) in vec2 aFboTex;     // 采样屏幕 FBO 的全屏坐标

out vec2 FboTexCoords;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    FboTexCoords = aFboTex;
}
    )";

    const char *fsrc = R"(
#version 440 core
out vec4 FragColor;

in vec2 FboTexCoords;

uniform sampler2D screenTexture;
uniform vec2 Pp;
uniform vec2 iResolution; // 屏幕全局物理像素分辨率
uniform vec4 sceneRect;   // Scene的矩形信息 (x, y, width, height)，基于左上角
uniform float colScale;

void main() {
    // 采样当前屏幕全貌
    vec4 col = texture(screenTexture, FboTexCoords);

    // 【全局屏幕坐标】 (相对屏幕左上角)
    vec2 screenCoord = vec2(FboTexCoords.x, 1.0 - FboTexCoords.y) * iResolution;

    // 【相对Scene坐标】 (相对 Scene 左上角)
    vec2 sceneCoord = screenCoord - sceneRect.xy;

    bool inScene = (0 < sceneCoord.x) && (sceneCoord.x < sceneRect.z)
                && (0 < sceneCoord.y) && (sceneCoord.y < sceneRect.w);

    col *= vec4(vec3(colScale), 1.0);

    FragColor = col;
}
    )";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
    m_program.link();

    // 构建一个静态的“全屏矩形”，不需要每帧计算了
    float vertices[] = {
        // aPos(x, y)   aFboTex(u, v)
        -1.0f,  1.0f,   0.0f, 1.0f,   // 左上角
        -1.0f, -1.0f,   0.0f, 0.0f,   // 左下角
        1.0f,  1.0f,   1.0f, 1.0f,   // 右上角
        1.0f, -1.0f,   1.0f, 0.0f    // 右下角
    };

    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices, sizeof(vertices)); // 静态分配并写入一次即可

    m_program.bind();
    // Layout 0: aPos
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 2, 4 * sizeof(float));
    // Layout 1: aFboTex
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 2, 4 * sizeof(float));

    m_program.release();
    m_vao.release();

    m_glInitialized = true;
}

void GameView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // FPS 相关计算
    // m_frameCount++;
    // if (m_fpsTimer.elapsed() >= 1000) {
    //     m_currentFps = m_frameCount;
    //     m_frameCount = 0;
    //     m_fpsTimer.restart();
    // }

    const qreal dpr = viewport()->devicePixelRatioF();
    QSize physicalSize = viewport()->size() * dpr;

    QPainter painter(viewport());
    painter.beginNativePainting();

    if (!m_glInitialized) {
        initializeShader();
    }

    if (!m_fbo || m_fbo->size() != physicalSize) {
        if (m_fbo) delete m_fbo;
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_fbo = new QOpenGLFramebufferObject(physicalSize, format);
    }

    // ==========================================
    // 第一阶段：将 View（包含背景和 Scene）全部画到 FBO
    // ==========================================
    m_fbo->bind();
    glViewport(0, 0, physicalSize.width(), physicalSize.height());
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QOpenGLPaintDevice device(m_fbo->size());
    QPainter fboPainter(&device);
    fboPainter.setRenderHint(QPainter::Antialiasing, false);
    this->render(&fboPainter);
    fboPainter.end();
    m_fbo->release();

    // ==========================================
    // 中间阶段：计算 Scene 的物理坐标和大小
    // ==========================================
    QRectF logicalSceneRect = mapFromScene(sceneRect()).boundingRect();

    // Qt 的 mapFromScene 返回的本来就是相对于 View 左上角的逻辑坐标，直接乘 dpr 即可
    QRectF physSceneRect(logicalSceneRect.x() * dpr,
                         logicalSceneRect.y() * dpr,
                         logicalSceneRect.width() * dpr,
                         logicalSceneRect.height() * dpr);

    // ==========================================
    // 第二阶段：直接全屏绘制 Shader（移除了动态顶点和冗余Blit）
    // ==========================================
    m_program.bind();
    m_program.setUniformValue("screenTexture", 0);

    // 设置 iResolution 为全屏尺寸
    m_program.setUniformValue("iResolution", QVector2D(physicalSize.width(), physicalSize.height()));

    // 传入 Scene 所在屏幕位置 (x, y, w, h)，供 Shader 计算 sceneCoord
    m_program.setUniformValue("sceneRect", QVector4D(physSceneRect.x(), physSceneRect.y(), physSceneRect.width(), physSceneRect.height()));

    m_program.setUniformValue("colScale", uniformColorRate);

    // (你原有的 Pp 逻辑不变)
    QPointF localCenter = player->boundingRect().center();
    QPointF sceneCenter = player->mapToScene(localCenter);
    QPointF viewportCenter = mapFromScene(sceneCenter);
    QPointF physicalCenter = viewportCenter * dpr;
    physicalCenter.setY(physicalSize.height() - physicalCenter.y()); // 如果在Shader里依然采用OpenGL底侧原点需要保持反转，具体看你的Pp需求
    m_program.setUniformValue("Pp", physicalCenter);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());

    // 绑定 VAO 直接画全屏矩形，0开销
    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao.release();
    m_program.release();

    painter.endNativePainting();

    // 绘制 FPS
    // painter.setPen(Qt::green);
    // painter.setFont(QFont("Arial", 16, QFont::Bold));
    // painter.drawText(200, 30, QString("FPS: %1").arg(m_currentFps));
}

void GameView::drawBackground(QPainter *painter, const QRectF &rect) {
    // 1. 首先用深灰色填充地图外的灰色虚无区 [2]
    painter->fillRect(rect, QColor(128, 128, 128));

    if (scene) {
        // 2. 【核心】：获取地图（场景）的正中心点坐标！ (由 mapWidth/2, mapHeight/2 决定)
        QRectF sRect = sceneRect();
        QPointF centerPoint = sRect.center();

        // 3. 只在地图区域内填充平滑石背景 [2]
        QRectF intersectRect = rect.intersected(sRect);
        if (!intersectRect.isEmpty()) {
            painter->fillRect(intersectRect, scene->backgroundBrush());
            painter->fillRect(intersectRect, QColor(255, 255, 255, 50));
        }

        if (borderPixmap.isNull()) {
            qDebug() << "边框图片加载失败！";
        } else {
            // 获取画框原图的真实比例
            qreal origW = borderPixmap.width();
            qreal origH = borderPixmap.height();

            // 【大小调节开关】：设定你想要的画框总宽度
            // 你可以随意修改这个数字（比如 1600、1800、2000）
            // 无论调多大或多小，画框都会自动等比缩放，且【永远保持在屏幕正中心】！
            qreal drawW = borderParas[0];

            // 自动计算高度（绝对不拉伸变形）
            qreal drawH = drawW * (origH / origW);

            // 【核心数学公式】：自动计算居中的 X 和 Y 坐标！
            // 原理：用地图的中心点坐标，减去画框大小的一半
            qreal drawX = centerPoint.x() - drawW / 2.0;
            qreal drawY = centerPoint.y() - drawH / 2.0;

            // 绘制不失真、完美居中的画框！ [4]
            painter->drawPixmap(drawX, drawY + borderParas[1], drawW, drawH, borderPixmap);
        }
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

void GameView::initFromDifficulty()
{
    switch (difficulty)
    {
    case 1:
        enemyIntv = 6000;
        abilityIntv = 6000;
        formationIntv = 12000;
        enemySpawnNum = 5;
        break;
    case 2:
        enemyIntv = 5000;
        abilityIntv = 6800;
        formationIntv = 11000;
        enemySpawnNum = 5;
        break;
    case 3:
        enemyIntv = 4400;
        abilityIntv = 7400;
        formationIntv = 10000;
        enemySpawnNum = 6;
        break;
    case 4:
        enemyIntv = 4400;
        abilityIntv = 8000;
        formationIntv = 9000;
        enemySpawnNum = 7;
        break;
    }
}

void GameView::updateGame() {

    if (gameEnds) {
        viewport()->update();
        return;
    }

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
            QVector<qreal> teleInfo = enemy->teleportThroughWall();
            if (teleInfo[4] > 0.5) {
                PortalPiece* tp = PortalPool::instance().getHiddenPiece();
                if (tp) {
                    tp->showPiece({teleInfo[0], teleInfo[1]});
                }

                tp = PortalPool::instance().getHiddenPiece();
                if (tp) {
                    tp->showPiece({teleInfo[2], teleInfo[3]});
                }
            }
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

                    SoundPool::instance().play("Arrow_hit");

                    break; // 停止检测这个子弹
                }
            }
        }
        // 咖喱棒剑气碰撞检测
        else if (CrescentWave* wave = dynamic_cast<CrescentWave*>(item)) {
            // 已经自己处理移动和碰壁删除了。见 CrescentWave.h

            QList<QGraphicsItem*> waveCollisions = wave->collidingItems();
            for (QGraphicsItem* colItem : std::as_const(waveCollisions)) {
                if (Enemy* e = dynamic_cast<Enemy*>(colItem)) {
                    // 标记敌人要删除
                    itemsToRemove.insert(e);

                    // // 播放音效
                    // SoundPool::instance().play("Lochunhin_launch");
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
                scene->removeItem(b);
                delete b;
            }
            else if (item->data(0).toString() == "enemy") {
                // 1. 获取敌人死前的绝对中心点
                QPointF deadCenter = item->sceneBoundingRect().center();

                // 2. 【核心修改】：提取该敌人此时真实的贴图
                // 不管他是僵尸还是溺尸，这一步都会动态抓取到它当前的贴图
                QGraphicsPixmapItem* pixItem = dynamic_cast<QGraphicsPixmapItem*>(item);
                QPixmap enemyPixmap;
                if (pixItem) {
                    enemyPixmap = pixItem->pixmap();
                }

                // 3. 【核心修改】：生成死亡红闪 + 冒烟特效 (传入坐标和敌人贴图)
                DeathVFX* vfx = new DeathVFX(deadCenter, enemyPixmap);
                scene->addItem(vfx);

                // 播放死亡音效
                SoundPool::instance().play("Enemy_die");

                // 4. 增加得分 (保持原样)
                scores++;
                scoreRecordBoard->setText(
                    QString("Score: %1").arg(scores, 4, 10, QChar(' '))
                    );

                // 5. 销毁敌人 (保持原样)
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

    // 更新
    viewport()->update();
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

            // 这样敌人就会刷在距离玩家 300 到 600 的环形区域内
            qreal distance = 250.0 + QRandomGenerator::global()->generateDouble() * 650.0;

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

        // 间隔 200ms。如果你觉得太慢/太快，只需调整这个 200 即可
        int delayMs = i * 100;

        // 2. 延迟 delayMs 毫秒后：在地图上亮起预警红叉
        QTimer::singleShot(delayMs, this, [this, spawnX, spawnY]() {
            if (!this->scene) return;
            SpawnIndicator* indicator = new SpawnIndicator(QPointF(spawnX, spawnY));
            this->scene->addItem(indicator);
        });

        QTimer::singleShot(delayMs / 2 + 1300, this, [this, spawnX, spawnY]() {
            // 安全检查：确保游戏还在进行，玩家还没死
            if (!this->scene || !this->player) return;

            // 预警结束，真正的僵尸降临！
            Enemy *enemy = new Enemy(this->player);
            enemy->setPos(spawnX, spawnY);
            this->scene->addItem(enemy);
        });

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
        qreal distance = 250.0 + QRandomGenerator::global()->generateDouble() * 650.0;

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
                qreal distance = 350 + QRandomGenerator::global()->generateDouble() * 450.0;

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

        // 播放音效
        switch(randomValue) {
        case 0:
            SoundPool::instance().play("Circle_begin", 2500);
            break;
        case 1:
            SoundPool::instance().play("Circle_begin", 2500);
            break;
        case 2:
            SoundPool::instance().play("Circle_begin");
            break;
        }

        formation->setPos(spawnX, spawnY);
        scene->addItem(formation);
    }
}


void GameView::gameOver() {
    // 停止定时器
    gameEnds = true;
    // gameTimer->stop();
    enemySpawnTimer->stop();
    abilitySpawnTimer->stop();
    formationSpawnTimer->stop();
    secondTimer->stop();

    SoundPool::instance().stopAll();
    m_bgmPlayer->stop();

    uniformColorRate = 0.6;


    // 1. 临时拼凑 UI 面板
    QWidget* panel = new QWidget();
    panel->setFixedSize(300, 150);
    panel->setStyleSheet("background: #222; border: 2px solid red; border-radius: 8px;");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    QPushButton* exitBtn = new QPushButton("EXIT", panel);
    exitBtn->setStyleSheet(R"(
    QPushButton {
        background-color: #D32F2F;  /* 现代感的红色 */
        color: white;
        padding: 8px 18px;          /* 调整内边距，使比例更协调 */
        font-weight: bold;
        font-size: 13px;
        border-radius: 6px;         /* 增加微小的圆角 */
        border: none;               /* 去掉默认的立体边框 */
    }
    QPushButton:hover {
        background-color: #E53935;  /* 鼠标悬浮时颜色稍微变亮 */
    }
    QPushButton:pressed {
        background-color: #B71C1C;  /* 鼠标按下时颜色变暗 */
    }
)");

    QLabel* label = new QLabel(
        QString("YOU DIED. Score %1").arg(scores)
    , panel);
    label->setStyleSheet("color: red; font-size: 24px; border: none;");
    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);
    layout->addWidget(exitBtn);

    // 2. 把 UI 塞进场景
    QGraphicsProxyWidget* proxy = scene->addWidget(panel);
    proxy->setZValue(9999); // 确保在最顶层
    proxy->setFocus();      // 抢夺焦点，拦截WASD等键盘事件

    // 3. 精准计算中心位置 (利用当前视图的视口中心映射到场景，比 sceneRect 更稳妥)
    QPointF viewCenterInScene = mapToScene(viewport()->rect().center());
    QPointF endPos = viewCenterInScene - QPointF(panel->width() / 2.0, panel->height() / 2.0);
    QPointF startPos = endPos - QPointF(0, 400); // 从中心点往上偏 400 像素飞下来

    // 4. 设置初始位置
    proxy->setPos(startPos);

    // 5. 播放快速动画
    QPropertyAnimation* anim = new QPropertyAnimation(proxy, "pos", this); // 绑定 this 防止内存泄露
    anim->setDuration(500); // 0.5秒
    anim->setStartValue(startPos); // !!! 必须显式设置 StartValue
    anim->setEndValue(endPos);
    anim->setEasingCurve(QEasingCurve::OutBack); // 带回弹效果
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 6. 绑定退出按钮 (通过值捕获 [this, proxy] 避免野指针崩溃)
    // 传入 this 作为上下文参数，保证安全关联
    QObject::connect(exitBtn, &QPushButton::clicked, this, [this, proxy]() {
        // 安全地从场景移除
        if (scene && proxy) {
            scene->removeItem(proxy);
        }

        // 假设 scores 和 seconds 是 GameView 的成员变量，通过 this 访问
        emit gameEnded({this->scores, this->seconds});

        // 关闭当前游戏窗口
        this->close();
    });
}

void GameView::timeOutGameOver()
{
    // 停止定时器
    gameEnds = true;
    // gameTimer->stop();
    enemySpawnTimer->stop();
    abilitySpawnTimer->stop();
    formationSpawnTimer->stop();
    secondTimer->stop();

    SoundPool::instance().stopAll();
    m_bgmPlayer->stop();

    // 1. 临时拼凑 UI 面板
    QWidget* panel = new QWidget();
    panel->setFixedSize(300, 150);
    // 给面板设置 ObjectName，防止子控件（如 Label）错误继承面板的背景和边框样式
    panel->setObjectName("timeoutPanel");
    panel->setStyleSheet(R"(
        QWidget#timeoutPanel {
            background-color: #FCFCFC;            /* 柔和的偏白背景 */
            border: 4px double #1A1A1A;          /* 双线黑边框 */
            border-radius: 8px;                  /* 轻微圆角 */
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    // 调整边距与间距，使视觉分布更匀称
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(15);

    // 2. 计时结束文本标签
    QLabel* label = new QLabel(
        QString("TIME OUT. Score %1").arg(scores),
        panel
        );
    label->setStyleSheet(R"(
        QLabel {
            color: #1A1A1A;                      /* 深色字，避免使用红色 */
            font-size: 20px;
            font-weight: bold;
            font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
            border: none;
            background: transparent;
        }
    )");
    label->setAlignment(Qt::AlignCenter);

    // 3. 退出按钮（采用高对比度的极简黑色设计）
    QPushButton* exitBtn = new QPushButton("EXIT", panel);
    exitBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1A1A1A;          /* 纯深色背景 */
            color: #FFFFFF;                     /* 白色文字 */
            padding: 8px 18px;
            font-weight: bold;
            font-size: 12px;
            border-radius: 4px;                 /* 略微锐利的圆角，匹配整体几何感 */
            border: none;
        }
        QPushButton:hover {
            background-color: #404040;          /* 悬浮时变为深灰 */
        }
        QPushButton:pressed {
            background-color: #000000;          /* 按下时变为纯黑反馈 */
        }
    )");

    layout->addWidget(label);
    layout->addWidget(exitBtn);

    // 2. 把 UI 塞进场景
    QGraphicsProxyWidget* proxy = scene->addWidget(panel);
    proxy->setZValue(9999); // 确保在最顶层
    proxy->setFocus();      // 抢夺焦点，拦截WASD等键盘事件

    // 3. 精准计算中心位置 (利用当前视图的视口中心映射到场景，比 sceneRect 更稳妥)
    QPointF viewCenterInScene = mapToScene(viewport()->rect().center());
    QPointF endPos = viewCenterInScene - QPointF(panel->width() / 2.0, panel->height() / 2.0);
    QPointF startPos = endPos - QPointF(0, 400); // 从中心点往上偏 400 像素飞下来

    // 4. 设置初始位置
    proxy->setPos(startPos);

    // 5. 播放快速动画
    QPropertyAnimation* anim = new QPropertyAnimation(proxy, "pos", this); // 绑定 this 防止内存泄露
    anim->setDuration(500); // 0.5秒
    anim->setStartValue(startPos); // !!! 必须显式设置 StartValue
    anim->setEndValue(endPos);
    anim->setEasingCurve(QEasingCurve::OutBack); // 带回弹效果
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 6. 绑定退出按钮 (通过值捕获 [this, proxy] 避免野指针崩溃)
    // 传入 this 作为上下文参数，保证安全关联
    QObject::connect(exitBtn, &QPushButton::clicked, this, [this, proxy]() {
        // 安全地从场景移除
        if (scene && proxy) {
            scene->removeItem(proxy);
        }

        // 假设 scores 和 seconds 是 GameView 的成员变量，通过 this 访问
        emit gameEnded({this->scores, this->seconds});

        // 关闭当前游戏窗口
        this->close();
    });
}
