// Created by 樊轩楷 & 吉佑安

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVector>
#include <QString>
#include <QMediaPlayer>
#include <QAudioOutput>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void on_btnSkin_clicked();

private:
    Ui::MainWindow *ui;

    // 0: wasd/↑↓←→, 1: mouse tank
    int moveMode = 1;

    // 0: settings hide | modes hide
    // 1: settings show | modes hide
    // 2: settings hide | modes show
    int settingsMode = 0;

    // 游戏设置
    int Difficulty = 1;  // 1, 2, 3, 4
    double Volume = 0.5; // [0.00, 1.00]

    // BGM
    QMediaPlayer *m_bgmPlayer;
    QAudioOutput *audioOutput;

    // 游戏记录
    int maxScore = 0;

    // 随机文本当前值
    QTimer* randTextTimer;
    int textSize;
    int randTextCurr = -1;
    void updateLabelText();

    // 计时模式
    bool timeLimited = false;
    int maxSeconds = 30; // [30, 300]

};
#endif // MAINWINDOW_H
