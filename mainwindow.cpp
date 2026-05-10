#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // ui 初始化
    ui->setupUi(this);

    ui->battle_field->hide();


    // 信号槽
    connect(ui->start_game, &QPushButton::clicked, this, [&]() {
        ui->main_menu->hide();
        ui->battle_field->show();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
