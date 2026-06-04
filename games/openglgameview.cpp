// Created by 吉佑安

#include "openglgameview.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QtMath>
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


const char* fragmentShaderSource = R"(
#version 440 core

// 1. 从第一个 shader 拷贝过来的 Uniforms
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 playerPos;      // 玩家在屏幕上的逻辑坐标 (2D)
uniform float uCameraDist;   // 摄像机距离玩家的距离
uniform float uCameraZoom;   // 缩放因子 (1.0 / tan(fov/2))
uniform float TORUS_R;       // 大半径
uniform float TORUS_r;       // 小半径

out vec4 fragColor;

#define PI 3.14159265359

// 2. 拷贝过来的辅助函数，用于摄像机定位
vec3 getTorusPos(vec2 logicPos) {
    float phi = (logicPos.x / iResolution.x) * 2.0 * PI;
    float theta = (logicPos.y / iResolution.y) * 2.0 * PI;

    float x = (TORUS_R + TORUS_r * cos(theta)) * cos(phi);
    float y = TORUS_r * sin(theta);
    float z = (TORUS_R + TORUS_r * cos(theta)) * sin(phi);
    return vec3(x, y, z);
}

// 3. 修改 mapV 以使用 Uniforms
// 环面的隐式方程 (注意：结果是 r^2 - dist^2，所以内部为正，外部为负)
float mapV(vec3 p) {
    float xzLen = length(p.xz);
    float a = TORUS_R - xzLen;
    // 使用 uniform TORUS_r
    return TORUS_r * TORUS_r - (a * a + p.y * p.y);
}

void main() {
    // 归一化屏幕坐标
    vec2 uv = 2.0*(gl_FragCoord.xy - 0.5 * iResolution.xy) / iResolution.y;

    // --- 4. 植入第一个 shader 的完整摄像机矩阵构建逻辑 ---
    // 玩家 3D 位置 (Target)
    vec3 target = getTorusPos(playerPos);

    // 计算法线 (用于确定摄像机位置)
    float phi = (playerPos.x / iResolution.x) * 2.0 * PI;
    vec3 R_vec = vec3(TORUS_R * cos(phi), 0.0, TORUS_R * sin(phi));
    vec3 playerNormal = normalize(target - R_vec);

    // 摄像机位置
    vec3 ro = target + playerNormal * uCameraDist;

    // 计算 Up 向量
    vec3 upPoint = getTorusPos(playerPos + vec2(0.0, iResolution.y * 0.25));
    vec3 upVector = normalize(upPoint - R_vec);

    // 构建 LookAt 基向量
    vec3 forward = normalize(target - ro);
    vec3 right = normalize(cross(forward, upVector));
    vec3 up = cross(right, forward);

    // 生成射线方向
    vec3 rd = normalize(forward * uCameraZoom + right * uv.x + up * uv.y);
    // --- 摄像机逻辑结束 ---


    // --- 5. 使用第二个 shader 的渲染循环，并进行适配 ---
    vec3 col = vec3(0.0);
    float alpha = 1.0;

    // 我们不再使用边界球求交，而是直接步进
    float t = 0.0;
    float v_curr = mapV(ro + rd * t);

    for(int i = 0; i < 80; i++) {
        // 使用最大距离和透明度作为退出条件
        if(alpha < 0.01 || t > 50.0) break; // 增加最大距离限制

        vec3 p = ro + rd * t;
        float eps = 0.01;
        float v_eps = mapV(p + rd * eps);
        // 注意 mapV 的符号，梯度方向是 v 减小的方向
        float dirDeriv = (v_eps - v_curr) / eps;

        float dt = clamp(abs(v_curr) / (abs(dirDeriv) + 0.001), 0.02, 0.5);

        float t_next = t + dt;
        vec3 p_next = ro + rd * t_next;
        float v_next = mapV(p_next);

        // 检测到穿过表面 (从正到负，或从负到正)
        if(v_curr * v_next < 0.0) {
            float t0 = t, t1_sub = t_next;
            float v0 = v_curr;
            // 二分法精确查找交点
            for(int b = 0; b < 6; b++) {
                float tm = (t0 + t1_sub) * 0.5;
                float vm = mapV(ro + rd * tm);
                if(v0 * vm <= 0.0) t1_sub = tm;
                else { t0 = tm; v0 = vm; }
            }

            float t_hit = (t0 + t1_sub) * 0.5;
            vec3 p_hit = ro + rd * t_hit;

            // 计算法线
            vec2 e = vec2(0.002, 0.0);
            // mapV 的梯度方向指向环面中心，所以需要取反得到表面外法线
            vec3 n_obj = -normalize(vec3(
                mapV(p_hit + e.xyy) - mapV(p_hit - e.xyy),
                mapV(p_hit + e.yxy) - mapV(p_hit - e.yxy),
                mapV(p_hit + e.yyx) - mapV(p_hit - e.yyx)
            ));

            // 如果射线从内部射出，翻转法线
            if(dot(n_obj, rd) > 0.0) n_obj = -n_obj;

            // 计算环面 UV
            float angle_major = atan(p_hit.z, p_hit.x);
            // 使用 uniform TORUS_R
            float angle_minor = atan(p_hit.y, length(p_hit.xz) - TORUS_R);

            float u = angle_major / (2.0 * PI);
            float v = angle_minor / (2.0 * PI);

            // 网格和光照 (与原 shader 逻辑一致)
            float g1 = abs(fract(u * 20.0) - 0.5);
            float g2 = abs(fract(v * 10.0) - 0.5);
            float grid = smoothstep(0.05, 0.0, min(g1, g2));

            float ndotv = clamp(dot(n_obj, -rd), 0.0, 1.0);
            float fresnel = pow(1.0 - ndotv, 4.0);

            vec3 baseCol = mix(vec3(0.1, 0.4, 0.3), vec3(0.5, 0.2, 0.6), sin(u*PI*2.0)*0.5+0.5);
            vec3 gridCol = vec3(0.6, 1.0, 0.9);
            vec3 surfaceCol = mix(baseCol, gridCol, grid) + fresnel * 0.5;

            float surfaceAlpha = 0.3 + grid * 0.5 + fresnel * 0.2;

            // 混合颜色
            col += alpha * surfaceAlpha * surfaceCol;
            alpha *= (1.0 - surfaceAlpha);

            // 推进射线以继续在环面内部步进
            t = t_hit + 0.05;
            v_curr = mapV(ro + rd * t);
        } else {
            t = t_next;
            v_curr = v_next;
        }
    }

    // 混合背景色
    vec3 bg = mix(vec3(0.1, 0.15, 0.2), vec3(0.02), length(uv));
    col += alpha * bg;

    // Gamma 校正
    col = pow(col, vec3(0.8));
    fragColor = vec4(col, 1.0);
}
)";


OpenGLGameView::OpenGLGameView(const int moveMode)
    : moveMode(moveMode)
{
    // 1. 设置 OpenGL 视口，激活硬件加速
    QOpenGLWidget *glWidget = new QOpenGLWidget(this);
    setViewport(glWidget);
    glWidget->setMouseTracking(true);

    setMouseTracking(true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // 创建场景
    scene = new QGraphicsScene(this);
    scene->setSceneRect(edge, edge, width() - edge, height() - edge);
    setScene(scene);

    // 创建玩家
    player = new Player();
    player->setPos(400, 250);
    scene->addItem(player);

    // 计时器设置
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &OpenGLGameView::updateGame);
    gameTimer->start(16);

    enemySpawnTimer = new QTimer(this);
    connect(enemySpawnTimer, &QTimer::timeout, this, &OpenGLGameView::spawnEnemy);
    enemySpawnTimer->start(3000);

    abilitySpawnTimer = new QTimer(this);
    connect(abilitySpawnTimer, &QTimer::timeout, this, &OpenGLGameView::generateAbility);
    abilitySpawnTimer->start(4000);
}

OpenGLGameView::~OpenGLGameView() {
    // 将 viewport 转换为 QOpenGLWidget 指针
    QOpenGLWidget *glWidget = qobject_cast<QOpenGLWidget*>(viewport());
    if (glWidget) {
        glWidget->makeCurrent(); // 激活当前视口的 OpenGL 上下文

        if (m_fbo) {
            delete m_fbo;
            m_fbo = nullptr;
        }
        m_vao.destroy();
        m_vbo.destroy();

        glWidget->doneCurrent(); // 释放上下文
    }
}

// ---------------------------------------------------------
// 核心改动 1：初始化 OpenGL 顶点与着色器
// ---------------------------------------------------------
void OpenGLGameView::initializeShader() {
    initializeOpenGLFunctions();

    // 顶点着色器：直接把全屏矩形画在屏幕上
    const char *vsrc = R"(
#version 440 core
layout (location = 0) in vec4 aPosTex;
out vec2 TexCoords;
void main() {
    gl_Position = vec4(aPosTex.x, aPosTex.y, 0.0, 1.0);
    TexCoords = vec2(aPosTex.z, aPosTex.w);
}
    )";

    const char *fsrc = R"(
#version 440 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 Pp;

void main() {
    vec4 col = texture(screenTexture, TexCoords);
    col.rgb = vec3(1.0) - col.rgb;
    if (length(gl_FragCoord.xy - Pp) < 6.0) {
        col = vec4(vec3(0.2), 1.0);
    }
    FragColor = col;
}
    )";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
    m_program.link();

    // 准备一个覆盖全屏的矩形 (x, y, u, v)
    float quadVertices[] = {
        // 位置 (x,y)   // 纹理坐标 (u,v)
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f
    };

    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(quadVertices, sizeof(quadVertices));

    m_program.bind();
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 4, 4 * sizeof(float));
    m_program.release();
    m_vao.release();

    m_glInitialized = true;
}

// ---------------------------------------------------------
// 拦截绘制事件，实现 FBO -> Shader 链路
// ---------------------------------------------------------
void OpenGLGameView::paintEvent(QPaintEvent *event)
{
    // 保持使用真实的 dpr 计算高清物理尺寸
    const qreal dpr = viewport()->devicePixelRatioF();
    QSize physicalSize = viewport()->size() * dpr;

    QPainter painter(viewport());
    painter.beginNativePainting(); // 暂停 QPainter，接管 OpenGL 底层

    // 1. 初始化 Shader 和缓冲
    if (!m_glInitialized) {
        initializeShader();
    }

    // 2. 动态维护 FBO 的尺寸（窗口改变大小时重新生成）
    if (!m_fbo || m_fbo->size() != physicalSize) {
        if (m_fbo) delete m_fbo;
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_fbo = new QOpenGLFramebufferObject(physicalSize, format);
    }

    // ==========================================
    // 第一阶段：让 CPU 发出指令，GPU 把场景画到 FBO
    // ==========================================
    m_fbo->bind();
    glViewport(0, 0, physicalSize.width(), physicalSize.height());
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QOpenGLPaintDevice device(m_fbo->size());
    QPainter fboPainter(&device);
    fboPainter.setRenderHint(QPainter::Antialiasing, false);

    // 此时 this->render 会自动将逻辑视口等比放大填满物理尺寸的 FBO
    this->render(&fboPainter);

    fboPainter.end();
    m_fbo->release();

    // ==========================================
    // 第二阶段：把 FBO 当作纹理，喂给你的自定义 Shader
    // ==========================================
    glViewport(0, 0, physicalSize.width(), physicalSize.height());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program.bind();
    // Uniform IS HERE
    m_program.setUniformValue("screenTexture", 0);

    // 1. 获取玩家在其自身坐标系下的中心点
    QPointF localCenter = player->boundingRect().center();
    // 2. 将玩家中心点映射到【场景坐标系】(Scene)
    QPointF sceneCenter = player->mapToScene(localCenter);
    // 3. 利用 QGraphicsView 自带的方法，将【场景坐标】精确转换为【视口逻辑坐标】(Viewport)
    // 这步会自动扣除 edge 以及任何视口的偏移
    QPointF viewportCenter = mapFromScene(sceneCenter);
    // 4. 将【视口逻辑坐标】乘以 dpr，转换为【OpenGL 物理像素坐标】
    QPointF physicalCenter = viewportCenter * dpr;
    // 5. 翻转 Y 轴以匹配 OpenGL 坐标系 (左下角为原点)
    physicalCenter.setY(physicalSize.height() - physicalCenter.y());
    m_program.setUniformValue("Pp", physicalCenter);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());

    m_vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 画出全屏矩形
    m_vao.release();

    m_program.release();

    painter.endNativePainting(); // 归还控制权
}



void OpenGLGameView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    scene->setSceneRect(edge, edge, width() - edge, height() - edge);
}

void OpenGLGameView::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->fillRect(rect, QColor(128, 128, 128));
    QRectF sRect = sceneRect();
    QRectF intersectRect = rect.intersected(sRect);
    if (!intersectRect.isEmpty()) {
        painter->fillRect(intersectRect, QColor(191, 191, 191));
    }
}

void OpenGLGameView::mouseMoveEvent(QMouseEvent *event) {
    mousePos = mapToScene(event->pos());
    QGraphicsView::mouseMoveEvent(event);
}

void OpenGLGameView::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
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
    QGraphicsView::keyPressEvent(event);
}

void OpenGLGameView::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
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

void OpenGLGameView::updateGame() {
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
        } else if (dynamic_cast<Enemy*>(item)) {
            gameOver();
            return;
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
            } else {
                // 如果是敌人等其他物品，按原计划物理移除并销毁
                scene->removeItem(item);
                delete item;
            }
        }
    }

    viewport()->update();
}

void OpenGLGameView::spawnEnemy() {
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
            if (spawnX >= edge && spawnX <= scene->width() +edge
                && spawnY >= edge && spawnY <= scene->height() +edge) {
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

void OpenGLGameView::generateAbility() {
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
        if (spawnX >= edge && spawnX <= scene->width() +edge
            && spawnY >= edge && spawnY <= scene->height() +edge) {
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

void OpenGLGameView::gameOver() {
    // 停止游戏循环和生成敌人的定时器
    gameTimer->stop();
    enemySpawnTimer->stop();
    abilitySpawnTimer->stop();

    QMessageBox::information(this, "Game Over", "你被敌人抓住了！\n点击确定返回主菜单。");

    emit gameEnded();
    this->close();
}
