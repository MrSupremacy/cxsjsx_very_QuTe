#include "skinselect.h"
#include "ui_skinselect.h"
#include <QPushButton>

#include "DataCarrier.h"

SkinSelect::SkinSelect(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SkinSelect)
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/skinicon.ico"));

    // 主题风格选择
    connect(ui->ba_1, &QPushButton::clicked, this, [this]() {
        activateStyle(0);
    });
    connect(ui->en_1, &QPushButton::clicked, this, [this]() {
        activateStyle(0);
    });
    connect(ui->ba_2, &QPushButton::clicked, this, [this]() {
        activateStyle(1);
    });
    connect(ui->en_2, &QPushButton::clicked, this, [this]() {
        activateStyle(1);
    });
    connect(ui->ba_3, &QPushButton::clicked, this, [this]() {
        activateStyle(2);
    });
    connect(ui->en_3, &QPushButton::clicked, this, [this]() {
        activateStyle(2);
    });

    // player
    connect(ui->player_1, &QPushButton::clicked, this, [this]() {
        activatePlayer(0);
    });
    connect(ui->player_2, &QPushButton::clicked, this, [this]() {
        activatePlayer(1);
    });
    connect(ui->player_3, &QPushButton::clicked, this, [this]() {
        activatePlayer(2);
    });
    connect(ui->player_4, &QPushButton::clicked, this, [this]() {
        activatePlayer(3);
    });

    // spear
    connect(ui->spear_1, &QPushButton::clicked, this, [this]() {
        activateSpear(0);
    });
    connect(ui->spear_2, &QPushButton::clicked, this, [this]() {
        activateSpear(1);
    });
    connect(ui->spear_3, &QPushButton::clicked, this, [this]() {
        activateSpear(2);
    });
    connect(ui->spear_4, &QPushButton::clicked, this, [this]() {
        activateSpear(3);
    });
    connect(ui->spear_5, &QPushButton::clicked, this, [this]() {
        activateSpear(4);
    });
    connect(ui->spear_6, &QPushButton::clicked, this, [this]() {
        activateSpear(5);
    });
    connect(ui->spear_7, &QPushButton::clicked, this, [this]() {
        activateSpear(6);
    });

    // arrow
    connect(ui->arrow_1, &QPushButton::clicked, this, [this]() {
        activateArrow(0);
    });
    connect(ui->arrow_2, &QPushButton::clicked, this, [this]() {
        activateArrow(1);
    });
    connect(ui->arrow_3, &QPushButton::clicked, this, [this]() {
        activateArrow(2);
    });
    connect(ui->arrow_4, &QPushButton::clicked, this, [this]() {
        activateArrow(3);
    });
    connect(ui->arrow_5, &QPushButton::clicked, this, [this]() {
        activateArrow(4);
    });
    connect(ui->arrow_6, &QPushButton::clicked, this, [this]() {
        activateArrow(5);
    });
    connect(ui->arrow_7, &QPushButton::clicked, this, [this]() {
        activateArrow(6);
    });


    // 初始化
    initActive();
}

void SkinSelect::activateStyle(int i) {
    ui->style_1->setStyleSheet(ActDeact[i == 0]);
    ui->style_2->setStyleSheet(ActDeact[i == 1]);
    ui->style_3->setStyleSheet(ActDeact[i == 2]);

    globalSkin::instance().currChoice["Background"] = i;
    globalSkin::instance().currChoice["Enemy"] = i;
}
void SkinSelect::activatePlayer(int i) {
    ui->plf_1->setStyleSheet(ActDeact[i == 0]);
    ui->plf_2->setStyleSheet(ActDeact[i == 1]);
    ui->plf_3->setStyleSheet(ActDeact[i == 2]);
    ui->plf_4->setStyleSheet(ActDeact[i == 3]);

    globalSkin::instance().currChoice["Player"] = i;
}
void SkinSelect::activateSpear(int i) {
    ui->spf_1->setStyleSheet(ActDeact[i == 0]);
    ui->spf_2->setStyleSheet(ActDeact[i == 1]);
    ui->spf_3->setStyleSheet(ActDeact[i == 2]);
    ui->spf_4->setStyleSheet(ActDeact[i == 3]);
    ui->spf_5->setStyleSheet(ActDeact[i == 4]);
    ui->spf_6->setStyleSheet(ActDeact[i == 5]);
    ui->spf_7->setStyleSheet(ActDeact[i == 6]);

    globalSkin::instance().currChoice["Spear"] = i;
    globalSkin::instance().currChoice["SpearInve"] = i;
}
void SkinSelect::activateArrow(int i) {
    ui->arf_1->setStyleSheet(ActDeact[i == 0]);
    ui->arf_2->setStyleSheet(ActDeact[i == 1]);
    ui->arf_3->setStyleSheet(ActDeact[i == 2]);
    ui->arf_4->setStyleSheet(ActDeact[i == 3]);
    ui->arf_5->setStyleSheet(ActDeact[i == 4]);
    ui->arf_6->setStyleSheet(ActDeact[i == 5]);
    ui->arf_7->setStyleSheet(ActDeact[i == 6]);

    globalSkin::instance().currChoice["Arrow"] = i;
}

void SkinSelect::initActive()
{
    auto& cc = globalSkin::instance().currChoice;
    activateStyle(cc["Background"]);
    activatePlayer(cc["Player"]);
    activateSpear(cc["Spear"]);
    activateArrow(cc["Arrow"]);
}

SkinSelect::~SkinSelect()
{
    delete ui;
}
