#ifndef OPENGLGAMEVIEW_H
#define OPENGLGAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QElapsedTimer>
#include <QOpenGLFunctions>
#include <QtOpenGL/QOpenGLShaderProgram>
#include <QtOpenGL/QOpenGLFunctions_4_4_Core>
#include <QtOpenGL/QOpenGLVertexArrayObject>
#include <QtOpenGL/QOpenGLBuffer>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QMatrix4x4>
#include <QVector3D>

#include "Player.h"
#include "enemy3d.h"

class OpenGLGameView: public QGraphicsView {
    Q_OBJECT
public:
    OpenGLGameView(const int moveMode);
    ~OpenGLGameView();

signals:
    void gameEnded();

private:
    QGraphicsScene *scene;
    QTimer *gameTimer;
    QTimer *enemySpawnTimer;

    bool keyW = false, keyA = false, keyS = false, keyD = false;
    bool keyUp = false, keyLeft = false, keyDown = false, keyRight = false;

    Player* player;
    const int moveMode = 0;
    QPointF mousePos = {0.0, 0.0};
    int spawn_num = 5;

    // --- Shader 渲染相关变量 ---
    bool m_glInitialized = false;
    QOpenGLShaderProgram m_shaderProgram;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QElapsedTimer m_elapsedTimer;

    // --- 环面映射相关 ---
    static constexpr qreal TORUS_R = 2.0; // 大圆半径
    static constexpr qreal TORUS_r = 1.0; // 小圆截面半径
    QVector3D mapToTorus(const QPointF& pos);

    // --- 摄像机与渲染参数 ---
    static constexpr float CAMERA_DIST = 6.0f; // 摄像机距离 (原 4.0 * 3.5)
    static constexpr float CAMERA_FOV  = 45.0f; // 视场角 Field of View

    void initShader();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // --- 核心绘制函数 ---
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    void updateGame();
    void spawnEnemy();
    void gameOver();
};

#endif // OPENGLGAMEVIEW_H
