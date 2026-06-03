// Created by 樊轩楷 & 吉佑安

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QSoundEffect>
#include <QUrl>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "Player.h"
#include "DataCarrier.h"


class GameView: public QGraphicsView, protected QOpenGLFunctions {
    Q_OBJECT
public:
    GameView(const DataCarrier& dc); // 构造函数 加载地图 & 启动游戏

    // 缩放保持 edge 不变
    // const int edge = 100;
    void resizeEvent(QResizeEvent *event) override;

    // 别忘了在 updateGame() 里还有 viewport()->update();
    void initializeShader(); // 初始化着色器
    void paintEvent(QPaintEvent *event) override; // 在这里执行 FBO 渲染和 Shader 后处理
    void drawBackground(QPainter *painter, const QRectF &rect) override; // 画背景，逻辑不变

signals: // 定义信号的关键字
    void gameEnded(EndData ed); // 声明一个游戏结束的信号

private:
    // BGM
    QMediaPlayer *m_bgmPlayer;
    QAudioOutput *audioOutput;


    // OpenGL 相关的成员变量
    bool m_glInitialized = false;
    QOpenGLFramebufferObject* m_fbo = nullptr;
    QOpenGLShaderProgram m_program;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;

    // 用于计算 FPS 的变量
    QElapsedTimer m_fpsTimer;
    int m_frameCount = 0;
    int m_currentFps = 0;


    QGraphicsScene *scene; // 存储地图实体 & 处理碰撞检测

    // 游戏设置 (以及 moveMode)
    const int difficulty;   // 难度
    const double volume;    // 音量 [0, 1]
    const bool timeLimited; // 是计时模式
    const int maxSeconds;   // 计时时长
    int seconds = 0; // 已过时长
    QTimer *secondTimer;
    int scores = 0;  // 已得分数

    // 难度
    void initFromDifficulty();

    int enemyIntv = 5000;
    int abilityIntv = 8000;
    int formationIntv = 10000;
    int enemySpawnNum = 6; // 敌人生成个数
    int formationSpawnNum = 1; // 阵型生成个数

    // 计时板、计分板
    QLabel *timeRecordBoard;
    QLabel *scoreRecordBoard;


    QTimer *gameTimer;  // 每帧刷新（移动、碰撞）
    QTimer *enemySpawnTimer; // 定时生成敌人
    QTimer *abilitySpawnTimer; // 定时生成技能
    QTimer *formationSpawnTimer; // 定时生成阵型

    bool keyW = false, keyA = false, keyS = false, keyD = false;
    bool keyUp = false, keyLeft = false, keyDown = false, keyRight = false;

    Player* player; // 玩家
    const int moveMode;
    QPointF mousePos = {0.0, 0.0};



    const int mapWidth = 1280, mapHeight = 800;


protected:
    // 按键
    void keyPressEvent(QKeyEvent *event) override; // 按压 -> true
    void keyReleaseEvent(QKeyEvent *event) override; // 松开 -> false
    void mouseMoveEvent(QMouseEvent *event) override; // 储存鼠标在scene中相对左上角坐标

    void updateGame(); // 更新战场

    void spawnEnemy(); // 生成敌人

    void generateAbility(); // 生成技能

    void spawnFormation(); // 生成阵型

    void gameOver(); // 游戏结束

};


#endif // GAMEVIEW_H
