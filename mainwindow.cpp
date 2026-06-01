// Created by 樊轩楷 & 吉佑安

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "skinselect.h"
#include "GameView.h"

#include "DataCarrier.h"
#include <QPushButton>
#include <QFile>
#include <QDataStream>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // ui 初始化
    ui->setupUi(this);

    // 初始化单例 globalSkin
    // 初始化读取信息：音量，模式，计时时长，皮肤Map，难度
    QFile readFile("data.bin");
    if (readFile.open(QIODevice::ReadOnly)) {
        QDataStream in(&readFile);
        in.setVersion(QDataStream::Qt_6_10); // 读取时版本号必须与写入时一致
        in
            >> globalSkin::instance().currChoice
            >> Volume
            >> timeLimited
            >> maxSeconds
            >> Difficulty;
        readFile.close();
    }


    connect(ui->start_game, &QPushButton::clicked, this, [=](){
        // 1. 隐藏当前的主菜单界面
        this->hide();

        // 2. 创建并显示游戏界面
        DataCarrier para = {
            moveMode
            , Difficulty
            , Volume
            , timeLimited
            , maxSeconds
        };
        GameView *game = new GameView(para);


        // 关键设置：当关闭游戏窗口时，自动释放 game 占用的内存，防止内存泄漏
        game->setAttribute(Qt::WA_DeleteOnClose);

        // 当 game 发出 gameEnded 信号时，执行主界面的 show() 函数重新显示出来
        connect(game, &GameView::gameEnded, this, [this](EndData ed) {
            this->show();
            QString txt = QString("E %1 / %2 S")
                .arg(ed.score, 4, 10, QChar('0'))
                .arg(ed.second, 4, 10, QChar('0'));
            ui->last_score->setText(txt);
        });

        connect(game, &GameView::destroyed, this, &MainWindow::show);

        // 设置游戏窗口的大小，并显示出来
        // game->resize(900, 600);
        // game->showFullScreen();
        game->showMaximized();
        game->show();
    });

    // 控制移动模式切换按钮
    connect(ui->move_mode, &QPushButton::clicked, this, [=]() {
        if (moveMode == 0) {
            moveMode = 1;
            ui->move_mode->setText("启用鼠标移动");
            ui->move_mode->setStyleSheet("background-color: #bda; color: #222;");
        } else if (moveMode == 1) {
            moveMode = 0;
            ui->move_mode->setText("禁用鼠标移动");
            ui->move_mode->setStyleSheet("background-color: #336; color: #ddd;");
        }
    });

    // 设置 按钮
    connect(ui->Set, &QPushButton::clicked, this, [=]() {
        if (settingsMode == 1) {
            ui->settings->hide();
            settingsMode = 0;
        } else {
            ui->modes->hide();
            ui->settings->show();
            settingsMode = 1;
        }
    });

    // 模式 按钮
    connect(ui->Mod, &QPushButton::clicked, this, [=]() {
        if (settingsMode == 2) {
            ui->modes->hide();
            settingsMode = 0;
        } else {
            ui->settings->hide();
            ui->modes->show();
            settingsMode = 2;
        }
    });

    // 皮肤 按钮
    connect(ui->Skin, &QPushButton::clicked, this, &MainWindow::on_btnSkin_clicked);


    // 游戏模式切换
    connect(ui->infty_time, &QPushButton::clicked, this, [=]() {
        ui->infty_time->setStyleSheet("background-color: rgb(189, 17, 77);");
        ui->limit_time->setStyleSheet("");
        timeLimited = false;
    });
    connect(ui->limit_time, &QPushButton::clicked, this, [=]() {
        ui->limit_time->setStyleSheet("background-color: rgb(189, 17, 77);");
        ui->infty_time->setStyleSheet("");
        timeLimited = true;
    });

    // 滑块
    connect(ui->difficulty, &QSlider::valueChanged, this, [=](int value){
        Difficulty = value;
        ui->diff_label->setText(QString(" %1 ").arg(value));
    });
    connect(ui->volume, &QSlider::valueChanged, this, [=](int value){
        Volume = value / 100.0;
        ui->volume_label->setText(QString("%1%").arg(value, 3, 10, QChar(' ')));
    });
    connect(ui->max_seconds, &QSlider::valueChanged, this, [=](int value){
        maxSeconds = value *5;
        ui->maxtime_label->setText(QString("%1s").arg(value *5, 3, 10, QChar(' ')));
    });


    // 弃用键盘移动
    ui->move_mode->hide();

    ui->settings->hide();
    ui->modes->hide();


    // 设置初始化
    ui->max_seconds->setValue(maxSeconds /5);
    ui->maxtime_label->setText(QString("%1s").arg(maxSeconds, 3, 10, QChar(' ')));

    ui->difficulty->setValue(Difficulty);
    ui->diff_label->setText(QString(" %1 ").arg(Difficulty));

    ui->volume->setValue((int)(Volume *100));
    ui->volume_label->setText(QString("%1%").arg((int)(Volume *100), 3, 10, QChar(' ')));

    if (timeLimited) {
        ui->limit_time->setStyleSheet("background-color: rgb(189, 17, 77);");
        ui->infty_time->setStyleSheet("");
    } else {
        ui->infty_time->setStyleSheet("background-color: rgb(189, 17, 77);");
        ui->limit_time->setStyleSheet("");
    }
}

void MainWindow::on_btnSkin_clicked()
{
    // 创建局部对象，用 exec() 阻塞运行
    SkinSelect dialog(this);
    dialog.setGeometry(this->geometry());
    dialog.exec();
}

MainWindow::~MainWindow()
{
    // 保存数据
    QFile writeFile("data.bin");
    if (writeFile.open(QIODevice::WriteOnly)) {
        QDataStream out(&writeFile);
        // 设置版本号，确保未来跨 Qt 版本读取时的兼容性
        out.setVersion(QDataStream::Qt_6_10);
        out
            << globalSkin::instance().currChoice
            << Volume
            << timeLimited
            << maxSeconds
            << Difficulty;
        writeFile.close();
    }

    delete ui;
}
