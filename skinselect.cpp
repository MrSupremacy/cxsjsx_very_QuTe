#include "skinselect.h"
#include "ui_skinselect.h"
#include <QPushButton>

#include "DataCarrier.h"

SkinSelect::SkinSelect(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SkinSelect)
{
    ui->setupUi(this);

    // 主题风格选择
    connect(ui->ba_1, &QPushButton::clicked, this, [this]() {
        activateStyle(1, 0);
    });
    connect(ui->en_1, &QPushButton::clicked, this, [this]() {
        activateStyle(1, 0);
    });
    connect(ui->ba_2, &QPushButton::clicked, this, [this]() {
        activateStyle(0, 1);
    });
    connect(ui->en_2, &QPushButton::clicked, this, [this]() {
        activateStyle(0, 1);
    });

    // player
    connect(ui->player_1, &QPushButton::clicked, this, [this]() {
        activatePlayer(1, 0);
    });
    connect(ui->player_2, &QPushButton::clicked, this, [this]() {
        activatePlayer(0, 1);
    });

    // spear
    connect(ui->spear_1, &QPushButton::clicked, this, [this]() {
        activateSpear(1, 0);
    });
    connect(ui->spear_2, &QPushButton::clicked, this, [this]() {
        activateSpear(0, 1);
    });

    // arrow
    connect(ui->arrow_1, &QPushButton::clicked, this, [this]() {
        activateArrow(1, 0);
    });
    connect(ui->arrow_2, &QPushButton::clicked, this, [this]() {
        activateArrow(0, 1);
    });


    // 初始化
    initActive();
}

void SkinSelect::activateStyle(int a, int b) {
    if (a + b != 1) return;

    ui->style_1->setStyleSheet(ActDeact[a]);
    ui->style_2->setStyleSheet(ActDeact[b]);

    globalSkin::instance().currChoice["Background"] = a *0 + b *1;
    globalSkin::instance().currChoice["Enemy"] = a *0 + b *1;
}
void SkinSelect::activatePlayer(int a, int b) {
    if (a + b != 1) return;

    ui->pl1_f->setStyleSheet(ActDeact[a]);
    ui->pl2_f->setStyleSheet(ActDeact[b]);

    globalSkin::instance().currChoice["Player"] = a *0 + b *1;
}
void SkinSelect::activateSpear(int a, int b) {
    if (a + b != 1) return;

    ui->sp1_f->setStyleSheet(ActDeact[a]);
    ui->sp2_f->setStyleSheet(ActDeact[b]);

    globalSkin::instance().currChoice["Spear"] = a *0 + b *1;
    globalSkin::instance().currChoice["SpearInve"] = a *0 + b *1;
}
void SkinSelect::activateArrow(int a, int b) {
    if (a + b != 1) return;

    ui->ar1_f->setStyleSheet(ActDeact[a]);
    ui->ar2_f->setStyleSheet(ActDeact[b]);

    globalSkin::instance().currChoice["Arrow"] = a *0 + b *1;
}

void SkinSelect::initActive()
{
    auto& cc = globalSkin::instance().currChoice;
    activateStyle(cc["Background"] == 0, cc["Background"] == 1);
    activatePlayer(cc["Player"] == 0, cc["Player"] == 1);
    activateSpear(cc["Spear"] == 0, cc["Spear"] == 1);
    activateArrow(cc["Arrow"] == 0, cc["Arrow"] == 1);
}

SkinSelect::~SkinSelect()
{
    delete ui;
}
