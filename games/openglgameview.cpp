// Created by 吉佑安

#include "openglgameview.h"
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QtMath>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QPainter>


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

const char* vertexShaderSource = R"(
#version 440 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";


OpenGLGameView::OpenGLGameView(const int moveMode)
    : moveMode(moveMode)
{
    // 基本设置
    setMouseTracking(true);

    // 1. 设置 QOpenGLWidget 为视口 (替代原本的 CPU 绘制)
    QOpenGLWidget *glWidget = new QOpenGLWidget(this);
    setViewport(glWidget);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate); // 每帧完全重绘
    glWidget->setMouseTracking(true);

    // 去掉滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 2. 创建场景并设置透明背景
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::transparent); // 关键：让底下的 OpenGL 露出来
    setScene(scene);

    // 创建玩家
    player = new Player();
    player->setPos(400, 300);
    scene->addItem(player);

    // 启动时间
    m_elapsedTimer.start();

    // 游戏主循环 (不仅更新逻辑，还要触发重绘)
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &OpenGLGameView::updateGame);
    gameTimer->start(16);

    enemySpawnTimer = new QTimer(this);
    connect(enemySpawnTimer, &QTimer::timeout, this, &OpenGLGameView::spawnEnemy);
    enemySpawnTimer->start(2000);
}

OpenGLGameView::~OpenGLGameView() {
    if (m_glInitialized) {
        m_vao.destroy();
        m_vbo.destroy();
    }
}

// 保证 Scene 和 View 永远一样大
void OpenGLGameView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    scene->setSceneRect(0, 0, width(), height());
}

// ---------------------------------------------------------
// OpenGL 渲染逻辑
// ---------------------------------------------------------
void OpenGLGameView::initShader() {
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();

    // 编译 Shader
    m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_shaderProgram.link();

    // 准备全屏覆盖的两个三角形 (Triangle Strip)
    float vertices[] = {
        -1.0f,  1.0f, // 左上
        -1.0f, -1.0f, // 左下
        1.0f,  1.0f, // 右上
        1.0f, -1.0f  // 右下
    };

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices, sizeof(vertices));

    m_shaderProgram.bind();
    m_shaderProgram.enableAttributeArray(0);
    m_shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);

    m_vao.release();
    m_vbo.release();
    m_shaderProgram.release();
}

QVector3D OpenGLGameView::mapToTorus(const QPointF& pos) {
    // 1. 将窗口坐标归一化到 [0, 2*PI] 的角度
    qreal phi = (pos.x() / width()) * 2.0 * M_PI;   // 对应Shader中的t2 (大圆角度)
    qreal theta = (pos.y() / height()) * 2.0 * M_PI; // 对应Shader中的t1 (小圆角度)

    // 2. 使用环面的参数方程计算3D坐标
    // (注意：为了匹配Shader中 y-z平面旋转t1, x-z平面旋转t2 的方式，参数方程需要对应)
    // 初始点在 x-y 平面，绕 y 轴转 (小圆)
    qreal x_local = TORUS_R + TORUS_r * qCos(theta);
    qreal y_local = TORUS_r * qSin(theta);

    // 再绕 y 轴转 (大圆)
    qreal x_world = x_local * qCos(phi);
    qreal z_world = x_local * qSin(phi);
    qreal y_world = y_local;

    return QVector3D(x_world, y_world, z_world);
}

void OpenGLGameView::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->beginNativePainting();

    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();

    // -------------------------------------------------------------
    // 【核心修复 1】暴力关闭 Qt QPainter 残留的各种测试和遮罩！
    // -------------------------------------------------------------
    gl->glDisable(GL_SCISSOR_TEST);  // 关闭矩形裁剪
    gl->glDisable(GL_STENCIL_TEST);  // 关闭模板测试（解决“圆”的罪魁祸首）
    gl->glDisable(GL_DEPTH_TEST);    // 关闭深度测试
    gl->glDisable(GL_CULL_FACE);     // 关闭面剔除
    gl->glDisable(GL_BLEND);         // 关闭混合

    // -------------------------------------------------------------
    // 【核心修复 2】重新映射视口，适配高分屏 (DPI 缩放)
    // -------------------------------------------------------------
    qreal dpr = window()->devicePixelRatio();
    gl->glViewport(0, 0, width() * dpr, height() * dpr);

    if (!m_glInitialized) {
        initShader();
        m_glInitialized = true;
    }

    gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderProgram.bind();

    // 传入 Uniform 变量
    m_shaderProgram.setUniformValue("playerPos", dpr*QVector2D(player->x(), player->y()));
    m_shaderProgram.setUniformValue("iResolution", dpr*QVector2D(width(), height()));

    m_shaderProgram.setUniformValue("iTime", (float)(m_elapsedTimer.elapsed()) / 1000.0f);

    m_shaderProgram.setUniformValue("uCameraDist", CAMERA_DIST);
    // 根据在 .h 文件中定义的 FOV (视场角) 来计算缩放因子
    // 公式是: zoom = 1.0 / tan(fov_in_radians / 2.0)
    float fov_rad = qDegreesToRadians(CAMERA_FOV);
    float zoom_factor = 1.0f / tan(fov_rad / 2.0f);
    m_shaderProgram.setUniformValue("uCameraZoom", zoom_factor);

    m_shaderProgram.setUniformValue("TORUS_R", (float)TORUS_R);
    m_shaderProgram.setUniformValue("TORUS_r", (float)TORUS_r);

    m_vao.bind();
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao.release();
    m_shaderProgram.release();

    // 恢复 Qt 的绘制状态
    painter->endNativePainting();
}

// 我们不再使用 QGraphicsScene 的自动绘制，所以需要重写 drawForeground
// drawForeground 在 drawBackground 之后，在滚动条等前景元素之前绘制
void OpenGLGameView::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    if (!player) return;

    // --- 1. 设置3D摄像机 ---
    // 摄像机位置由玩家的2D逻辑位置决定
    QPointF playerPos = player->pos();
    QVector3D player3DPos = mapToTorus(playerPos);

    // 计算玩家位置的法线 (向外)
    qreal phi = (playerPos.x() / width()) * 2.0 * M_PI;
    QVector3D R_vec(TORUS_R * qCos(phi), 0, TORUS_R * qSin(phi));
    QVector3D playerNormal = (player3DPos - R_vec).normalized();

    // 直接使用头文件中的距离
    float cameraDist = CAMERA_DIST;
    QVector3D cameraPos = player3DPos + playerNormal * cameraDist;
    QVector3D cameraTarget = player3DPos;

    QVector3D upVector = mapToTorus(playerPos + QPointF(0, height()*0.25)); // 全局向上向量
    upVector = (upVector - R_vec).normalized();

    // 创建视图矩阵
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(cameraPos, cameraTarget, upVector);

    // 创建投影矩阵
    QMatrix4x4 projectionMatrix;
    projectionMatrix.perspective(CAMERA_FOV, (float)width() / height(), 0.1f, 100.0f);

    // --- 2. 绘制玩家 (始终在屏幕中心) ---
    painter->setBrush(Qt::green); // 玩家颜色
    painter->setPen(Qt::NoPen);
    qreal playerSize = player->rect().width();
    painter->drawRect(width()/2.0 - playerSize/2.0, height()/2.0 - playerSize/2.0, playerSize, playerSize);


    // --- 3. 绘制所有敌人 ---
    painter->setBrush(Qt::red); // 敌人颜色
    for (QGraphicsItem *item : scene->items()) {
        Enemy3D *enemy3d = dynamic_cast<Enemy3D*>(item);
        if (!enemy3d) continue;

        // 获取敌人的3D世界坐标
        QVector3D enemy3DPos = mapToTorus(enemy3d->pos());

        // QVector3D toEnemy = (enemy3DPos - player3DPos).normalized();
        // // playerNormal是摄像机朝向的反方向
        // if (QVector3D::dotProduct(toEnemy, -playerNormal) < 0.1) { // 0.1作为阈值，避免在边缘闪烁
        //     continue; // 敌人在背面，不绘制
        // }

        // 将敌人的3D世界坐标变换到2D屏幕坐标
        QMatrix4x4 modelMatrix; // 单位矩阵，因为我们的坐标已经是世界坐标
        QVector3D screenPos = (projectionMatrix * viewMatrix * modelMatrix) * enemy3DPos;

        // 归一化设备坐标 (NDC) [-1, 1] -> 视口坐标 [0, w] & [0, h]
        if (screenPos.z() < 1.0f) { // 简单的裁剪
            qreal x = (screenPos.x() + 1.0) / 2.0 * width();
            qreal y = (1.0 - screenPos.y()) / 2.0 * height(); // Y轴反转

            // 根据距离调整大小 (简单的透视效果)
            qreal dist = (enemy3DPos - cameraPos).length();
            qreal scale = 10.0 / dist; // 调整这个20.0来控制大小
            qreal enemySize = enemy3d->rect().width() * scale;

            painter->drawRect(x - enemySize/2.0, y - enemySize/2.0, enemySize, enemySize);
        }
    }
}

// ---------------------------------------------------------
// 游戏逻辑 (保持 2D 的简单性，但增加边缘环绕)
// ---------------------------------------------------------
void OpenGLGameView::updateGame() {
    // 1. 移动玩家
    if (moveMode == 0) {
        player->keyboardMove(keyW, keyA, keyS, keyD, keyUp, keyLeft, keyDown, keyRight);
    } else if (moveMode == 1) {
        player->mouse3Dmove(mousePos - 0.5*QPointF(width(), height()));
    }

    // [新增特性] 地图是甜甜圈形状，所以玩家和敌人走到边界应该从另一边出来 (Wrap Around)
    int mapW = width();
    int mapH = height();
    if(player->x() < 0) player->setX(player->x() + mapW);
    if(player->x() > mapW) player->setX(player->x() - mapW);
    if(player->y() < 0) player->setY(player->y() + mapH);
    if(player->y() > mapH) player->setY(player->y() - mapH);

    // 2. 敌人逻辑
    QList<QGraphicsItem *> items = scene->items();
    for (QGraphicsItem *item : std::as_const(items)) {
        Enemy3D *enemy3d = dynamic_cast<Enemy3D*>(item);
        if (enemy3d) {
            enemy3d->moveTowardsTarget();
            // 敌人也遵循甜甜圈边界法则
            if(enemy3d->x() < 0) enemy3d->setX(enemy3d->x() + mapW);
            if(enemy3d->x() > mapW) enemy3d->setX(enemy3d->x() - mapW);
            if(enemy3d->y() < 0) enemy3d->setY(enemy3d->y() + mapH);
            if(enemy3d->y() > mapH) enemy3d->setY(enemy3d->y() - mapH);
        }
    }

    // 3. 碰撞检测
    QList<QGraphicsItem *> collisions = player->collidingItems();
    for (QGraphicsItem *item : std::as_const(collisions)) {
        if (dynamic_cast<Enemy3D*>(item)) {
            gameOver();
            return;
        }
    }

    // 必须调用！强制刷新视口，否则如果没有 2D 物品移动，Shader动画会卡住
    viewport()->update();
}

// 鼠标、按键等事件逻辑与原来完全一致...
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

void OpenGLGameView::spawnEnemy() {
    // 逻辑与你写的完全一致 (省略...)
    if(!player) return;
    qreal px = player->x(), py = player->y();
    qreal spawnX = 0, spawnY = 0;
    bool validPos = false;

    for(int i = 0; i < spawn_num; i ++) {
        while (!validPos) {
            qreal angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;
            qreal distance = 100.0 + QRandomGenerator::global()->generateDouble() * 300.0;
            spawnX = px + distance * qCos(angle);
            spawnY = py + distance * qSin(angle);

            // 放宽出生点判断，因为现在是甜甜圈循环地图
            validPos = true;
        }

        Enemy3D *enemy3d = new Enemy3D(player, width(), height()); // 传入地图大小
        enemy3d->setPos(spawnX, spawnY);
        scene->addItem(enemy3d);
        validPos = false;
    }
}

void OpenGLGameView::gameOver() {
    gameTimer->stop();
    enemySpawnTimer->stop();
    QMessageBox::information(this, "Game Over", "你被敌人抓住了！\n点击确定返回主菜单。");
    emit gameEnded();
    this->close();
}
