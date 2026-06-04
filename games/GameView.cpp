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

    QPixmap bgPixmap(globalSkin::applyChoice("Background"));

    bgPixmap = bgPixmap.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    scene->setBackgroundBrush(QBrush(bgPixmap));

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
    PortalPool::instance().init(scene, ":/ImageResources/nether_portal.png", 50, 2000);
}

void GameView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // scene->setSceneRect(edge, edge, width() -edge, height() -edge);
}

void GameView::initializeShader() {
    initializeOpenGLFunctions();

    // 顶点着色器：接收两套纹理坐标
    const char *vsrc = R"(
#version 440 core
layout (location = 0) in vec2 aPos;        // NDC 坐标
layout (location = 1) in vec2 aFboTex;     // 采样屏幕 FBO 的全屏坐标
layout (location = 2) in vec2 aSceneTex;   // 专门针对 Scene 的 0~1 坐标

out vec2 FboTexCoords;
out vec2 SceneTexCoords;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    FboTexCoords = aFboTex;
    SceneTexCoords = aSceneTex;
}
    )";

    const char *fsrc = R"(
#version 440 core
out vec4 FragColor;

in vec2 FboTexCoords;
in vec2 SceneTexCoords;

uniform sampler2D screenTexture;
uniform vec2 Pp;
uniform vec2 iResolution; // Scene的实际物理像素大小

void main() {
    // 1. 必须使用 FboTexCoords 才能在 FBO 的正确位置抠出像素
    vec4 col = texture(screenTexture, FboTexCoords);

    // ============================================
    // SceneTexCoords 严格从 (0,0) 对应 Scene 左下角到 (1,1) 对应右上角
    // 如果你需要类似 ShaderToy 的像素坐标：vec2 fragCoord = SceneTexCoords * iResolution;
    // ============================================

    FragColor = col;
}
    )";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
    m_program.link();

    // 初始化空的 VAO 和 VBO (因为每一帧我们要动态写入坐标)
    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vbo.bind();
    // 预先分配足以存放4个顶点（每个顶点 2+2+2=6 个 float）的内存
    m_vbo.allocate(4 * 6 * sizeof(float));

    m_program.bind();
    // Layout 0: aPos
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 2, 6 * sizeof(float));
    // Layout 1: aFboTex
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 2, 6 * sizeof(float));
    // Layout 2: aSceneTex
    m_program.enableAttributeArray(2);
    m_program.setAttributeBuffer(2, GL_FLOAT, 4 * sizeof(float), 2, 6 * sizeof(float));

    m_program.release();
    m_vao.release();

    m_glInitialized = true;
}

void GameView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // FPS 相关计算
    m_frameCount++;
    if (m_fpsTimer.elapsed() >= 1000) { // 如果经过了 1000 毫秒 (1秒)
        m_currentFps = m_frameCount;    // 记录上一秒的帧数
        m_frameCount = 0;               // 计数器清零
        m_fpsTimer.restart();           // 重新计时
    }


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
    // 获取场景的矩形，并将其转换为视口的逻辑坐标
    QRectF logicalSceneRect = mapFromScene(sceneRect()).boundingRect();
    // 转换为 OpenGL 的物理像素坐标
    QRectF physSceneRect(logicalSceneRect.x() * dpr,
                         logicalSceneRect.y() * dpr,
                         logicalSceneRect.width() * dpr,
                         logicalSceneRect.height() * dpr);

    // OpenGL 的 Y 轴是从下往上，而 Qt 是从上往下，需要翻转 Y 轴
    float glY = physicalSize.height() - (physSceneRect.y() + physSceneRect.height());

    // 提取宽高给后面使用
    float X = physSceneRect.x();
    float Y = glY;
    float W_sc = physSceneRect.width();
    float H_sc = physSceneRect.height();
    float W_fb = physicalSize.width();
    float H_fb = physicalSize.height();

    // ==========================================
    // 第二阶段：还原背景，并只在 Scene 区域跑 Shader
    // ==========================================
    // 【关键1】恢复默认帧缓冲后，先把 FBO "原样" 拷贝到屏幕上。
    // 这保证了 Scene 外面的背景不会黑屏，而是保留 drawBackground 画出的原本样子。
    QOpenGLFramebufferObject::blitFramebuffer(nullptr, m_fbo, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // 开启 Shader 接管
    m_program.bind();
    m_program.setUniformValue("screenTexture", 0);

    // 设置 iResolution (Scene 的物理像素尺寸)
    m_program.setUniformValue("iResolution", QVector2D(W_sc, H_sc));

    // (设置你原有的 Pp 逻辑)
    QPointF localCenter = player->boundingRect().center();
    QPointF sceneCenter = player->mapToScene(localCenter);
    QPointF viewportCenter = mapFromScene(sceneCenter);
    QPointF physicalCenter = viewportCenter * dpr;
    physicalCenter.setY(physicalSize.height() - physicalCenter.y());
    m_program.setUniformValue("Pp", physicalCenter);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());

    // 【关键2】动态计算仅仅覆盖 Scene 区域的 Quad
    // 1. 计算 NDC 坐标 (-1 到 1)
    float nx1 = (X / W_fb) * 2.0f - 1.0f;
    float ny1 = (Y / H_fb) * 2.0f - 1.0f;
    float nx2 = ((X + W_sc) / W_fb) * 2.0f - 1.0f;
    float ny2 = ((Y + H_sc) / H_fb) * 2.0f - 1.0f;

    // 2. 计算在 FBO 上的采样坐标 (0 到 1)
    float fu1 = X / W_fb;
    float fv1 = Y / H_fb;
    float fu2 = (X + W_sc) / W_fb;
    float fv2 = (Y + H_sc) / H_fb;

    // 3. 构建顶点数组：Pos(2), FboUV(2), SceneUV(2)
    float vertices[] = {
        // Pos(x, y)   FboTex(u, v)  SceneTex(u, v)
        nx1, ny2,      fu1, fv2,     0.0f, 1.0f,   // 左上角
        nx1, ny1,      fu1, fv1,     0.0f, 0.0f,   // 左下角
        nx2, ny2,      fu2, fv2,     1.0f, 1.0f,   // 右上角
        nx2, ny1,      fu2, fv1,     1.0f, 0.0f    // 右下角
    };

    // 更新 VBO 数据并绘制
    m_vao.bind();
    m_vbo.bind();
    m_vbo.write(0, vertices, sizeof(vertices)); // 动态更新顶点数据

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao.release();
    m_program.release();

    painter.endNativePainting();


    // 绘制 FPS
    painter.setPen(Qt::green); // 设为绿色字体，比较醒目
    painter.setFont(QFont("Arial", 16, QFont::Bold)); // 设置字体大小
    // 画在视口左上角，坐标是 Qt 的逻辑坐标
    painter.drawText(200, 30, QString("FPS: %1").arg(m_currentFps));
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
        painter->setOpacity(0.6);
        painter->fillRect(intersectRect, scene->backgroundBrush());
        painter->setOpacity(1.0);
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
    gameTimer->stop();
    enemySpawnTimer->stop();
    abilitySpawnTimer->stop();
    formationSpawnTimer->stop();
    secondTimer->stop();

    SoundPool::instance().stopAll();
    m_bgmPlayer->stop();

    // 2. 弹出一个提示框告诉玩家游戏结束（体验更好，不会死得太突兀）
    QMessageBox::information(this, "Game Over", "你被敌人抓住了！\n点击确定返回主菜单。");

    // 3. 发出“游戏结束”的信号！
    emit gameEnded({scores, seconds});

    // 4. 关闭当前游戏窗口（因为我们之前设置了 WA_DeleteOnClose，这里 close() 会自动释放内存）
    this->close();
}
