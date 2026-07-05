// Created by 吉佑安

#ifndef OPENGLGAMEVIEW_H
#define OPENGLGAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "Player.h"

// 继承 QGraphicsView，并引入 OpenGL 基础功能
class OpenGLGameView : public QGraphicsView, protected QOpenGLFunctions {
    Q_OBJECT
public:
    OpenGLGameView(const int moveMode);
    ~OpenGLGameView() override;

    const int edge = 100;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void gameEnded();

protected:
    // 拦截绘制事件，在这里执行 FBO 渲染和 Shader 后处理
    void paintEvent(QPaintEvent *event) override;

    // 画背景（逻辑不变，但在 paintEvent 中被手动调用）
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    // 按键与鼠标
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void updateGame();
    void spawnEnemy();
    void generateAbility();
    void gameOver();

    // --- 新增：OpenGL 相关的成员变量 ---
    void initializeShader(); // 初始化着色器
    bool m_glInitialized = false;
    QOpenGLFramebufferObject* m_fbo = nullptr;
    QOpenGLShaderProgram m_program;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    // --------------------------------

    QGraphicsScene *scene;
    QTimer *gameTimer;
    QTimer *enemySpawnTimer;
    QTimer *abilitySpawnTimer;

    bool keyW = false, keyA = false, keyS = false, keyD = false;
    bool keyUp = false, keyLeft = false, keyDown = false, keyRight = false;

    Player* player;
    const int moveMode;
    QPointF mousePos = {0.0, 0.0};

    int spawnNum = 6;
};

#endif // OPENGLGAMEVIEW_H
