#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GameView.h"
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // ui 初始化
    ui->setupUi(this);

    connect(ui->start_game, &QPushButton::clicked, this, [=](){
        // 1. 隐藏当前的主菜单界面
        this->hide();

        // 2. 创建并显示游戏界面
        GameView *game = new GameView();


        // 关键设置：当关闭游戏窗口时，自动释放 game 占用的内存，防止内存泄漏
        game->setAttribute(Qt::WA_DeleteOnClose);

        // 当 game 发出 gameEnded 信号时，执行主界面的 show() 函数重新显示出来
        connect(game, &GameView::gameEnded, this, &MainWindow::show);

        // 设置游戏窗口的大小，并显示出来
        game->resize(800, 600);
        game->show();
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}
