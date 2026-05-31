#include "skinselect.h"
#include "ui_skinselect.h"

SkinSelect::SkinSelect(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SkinSelect)
{
    ui->setupUi(this);
}

SkinSelect::~SkinSelect()
{
    delete ui;
}
