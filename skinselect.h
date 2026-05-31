#ifndef SKINSELECT_H
#define SKINSELECT_H

#include <QDialog>

namespace Ui {
class SkinSelect;
}

class SkinSelect : public QDialog
{
    Q_OBJECT

public:
    explicit SkinSelect(QWidget *parent = nullptr);
    ~SkinSelect();

private:
    Ui::SkinSelect *ui;
};

#endif // SKINSELECT_H
