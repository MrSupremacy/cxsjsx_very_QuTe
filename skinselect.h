#ifndef SKINSELECT_H
#define SKINSELECT_H

#include <QDialog>
#include "DataCarrier.h"


namespace Ui {
class SkinSelect;
}

class SkinSelect : public QDialog
{
    Q_OBJECT

public:
    explicit SkinSelect(QWidget *parent = nullptr);
    ~SkinSelect();

    const QString ActDeact[2] = {
        "border: 3px solid #000; border-radius: 0px;",
        "border: 3px solid #ff0; border-radius: 0px;"
    };

private:
    Ui::SkinSelect *ui;

    void activateStyle(int a, int b);
    void activatePlayer(int a, int b);
    void activateSpear(int a, int b);
    void activateArrow(int a, int b);

    void initActive();
};

#endif // SKINSELECT_H
