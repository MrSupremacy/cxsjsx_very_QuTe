// Created by 樊轩楷 & 吉佑安

#include "Player.h"
#include <QLineF>
#include <QRandomGenerator>
#include <QtMath>   // 用于 std::mt19937 和 std::random_device
#include <QLabel>
#include <QMovie>        // === 1. 引入播放动图的头文件 === [1]
#include <QGraphicsView>
#include <QTimer>
#include <QGraphicsOpacityEffect>

#include "SoundPool.h"
#include "DataCarrier.h"


Player::Player()
{
    // setRect(0, 0, 20, 20);
    // setBrush(QBrush(Qt::white)); // 基础颜色为白色
    // setPen(Qt::NoPen); // 移除边框

    QPixmap stevePic(globalSkin::applyChoice("Player"));
    // 玩家可能稍微大一点，比如 40x40
    stevePic = stevePic.scaled(24, 24, Qt::KeepAspectRatio, Qt::FastTransformation);
    this->setPixmap(stevePic);

    this->setTransformOriginPoint(12, 12); // 设置中心点

    immuneTimer = new QTimer(this);
    immuneTimer->setSingleShot(true); // 设置为单次触发（只响一次）
    connect(immuneTimer, &QTimer::timeout, this, &Player::endImmune);


    // 光剑技能 配置光剑
    swordItem = new QGraphicsPixmapItem(this); // 改为 PixmapItem
    QPixmap spearPic(globalSkin::applyChoice("Spear")); // 你的长矛图片路径

    QTransform transform;
    transform.rotate(135); // 顺时针旋转 135 度

    // 执行旋转（旋转时建议用 Smooth 算法，防止边缘产生难看的锯齿）
    spearPic = spearPic.transformed(transform, Qt::SmoothTransformation);

    spearPic = spearPic.scaled(90, 90, Qt::KeepAspectRatio, Qt::FastTransformation);
    swordItem->setPixmap(spearPic);

    swordItem->hide(); // 初始状态隐藏
    swordItem->setFlag(QGraphicsItem::ItemStacksBehindParent);
    swordItem->setTransformOriginPoint(0, 45);
    swordItem->setPos(10, -35);

    swordTimer = new QTimer(this);
    QObject::connect(swordTimer, &QTimer::timeout, [=](){ // 当定时器时间到，隐藏这把剑
        swordItem->hide();
    });
    swordTimer->setSingleShot(true); // 设为单次触发模式


    // 咖喱棒技能 配置贴图、蓄力条
    tntItem = new QGraphicsPixmapItem(this);
    QPixmap tntPic(":/ImageResources/tnt.png");

    tntPic = tntPic.scaled(24, 24, Qt::KeepAspectRatio, Qt::FastTransformation);
    tntItem->setPixmap(tntPic);
    tntItem->hide(); // 初始状态隐藏，与护盾一致
    tntItem->setPos(-12, 12); // 放置在左下侧 (玩家宽度为24)

    chargeBar = new PlayerChargeBar(this);
    chargeBar->setPos({-16, 6});
    chargeBarTimer = new QTimer(this);
    QObject::connect(chargeBarTimer, &QTimer::timeout, [this](){
        this->onCharging();
    });


    // 护盾技能 配置护盾及图腾
    shieldItem = new QGraphicsRectItem(-16, -16, 32, 32, this);
    shieldItem->setBrush(Qt::NoBrush); // 无填充
    shieldItem->setPen(QPen(Qt::green)); // 涂成淡青色
    shieldItem->hide();
    shieldItem->setPos(12, 12); // 放到中间位置

    totemItem = new QGraphicsPixmapItem(this); // 生成图腾实体
    QPixmap totemPic(":/ImageResources/totemofundying.png"); // 你的图腾图片路径
    totemPic = totemPic.scaled(26, 26, Qt::KeepAspectRatio, Qt::FastTransformation);
    totemItem->setPixmap(totemPic);
    totemItem->hide();
    totemItem->setPos(10, 12); // 放置到右下侧

    shieldTimer = new QTimer(this);
    QObject::connect(shieldTimer, &QTimer::timeout, [=](){ // 当定时器时间到，隐藏这个盾
        shieldItem->hide();
        totemItem->hide();
        breakShieldAndExplode(120, 1000, false); // 盾自己破也要炸
    });
    shieldTimer->setSingleShot(true); // 设为单次触发模式



    // 射击技能 配置射击方法
    fireTimer = new QTimer(this); // 确保 fireTimer 已实例化
    fireTimer->setSingleShot(true); // 设置为单次触发模式

    QObject::connect(fireTimer, &QTimer::timeout, [&]() mutable {
        if (fireTimes > 0) {
            fireTimes--;
            double ang = QLineF({0, 0}, lastDir).angle();
            ang = qDegreesToRadians(ang);

            // 基础移动方向向量
            QPointF dir = {-qCos(ang), qSin(ang)};
            QPointF perp = {-dir.y(), dir.x()};

            // 播放发射音效
            SoundPool::instance().play("Arrow_shoot");

            // 生成 currNum 个子弹 (假定 currNum 必为奇数)
            for (int var = 0; var < currNum; ++var) {

                // 1. 【核心计算】：计算当前子弹相对于中心子弹的偏差索引
                // 比如 5 颗子弹，var 分别为 0, 1, 2, 3, 4
                // 计算出的 offsetIndex 分别为: -2, -1, 0, 1, 2 (中心刚好是 0)
                double offsetIndex = var - (currNum - 1) / 2.0;

                // 2. 计算生成位置
                // 💡 提示：如果你希望所有子弹像“散弹枪”一样完全从玩家同一个点（枪口）呈扇形喷出，
                // 可以在主程序中把传入的 distPx 设为 0。
                // 如果希望它们像原本一样“平行排开后再扇形飞出”，保持你的 distPx 即可。
                double offset = offsetIndex * distPx;
                QPointF currentPos = this->pos() + perp * offset;

                // 3. 【核心计算】：计算当前子弹的独立偏转角
                // 比如每往旁边一颗偏离 5.0 度（你可以通过调整 5.0 这个数值改变散射的宽窄）
                double angleOffsetDeg = offsetIndex * 4.5;
                double bulletAng = -ang - qDegreesToRadians(angleOffsetDeg);

                // 4. 生成子弹，传入微调后的独立角度
                // 我们之前写的 Bullet 构造函数会自动根据这个弧度角旋转图片并计算移动向量！
                Bullet* temp = new Bullet(bulletAng, currentPos);
                if (!temp) continue;

                // 5. 将子弹加入场景
                try {
                    if (temp->scene() != this->scene()) {
                        this->scene()->addItem(temp);
                    }
                } catch (...) {
                }
            }

            if (fireTimes > 0) {
                fireTimer->start(currInterval);
            }


        }
    });
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // 1. 先画出 Steve 图片
    QGraphicsPixmapItem::paint(painter, option, widget);

    // 2. 绘制 Steve 的边框（比如用红石科技感十足的红色，或者经典的黑色）
    QPen pen(Qt::lightGray, 2);
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);

    // 3. 绘制边框
    painter->drawRect(this->boundingRect().adjusted(1, 1, -1, -1));
}

void Player::keyboardMove(bool w, bool a, bool s, bool d, bool up, bool left, bool down, bool right) {
    double dx = 0, dy = 0;
    int dirx = 0, diry = 0;

    if(w || up) diry --;
    if(a || left) dirx --;
    if(s || down) diry ++;
    if(d || right) dirx ++;

    if(dirx != 0 || diry != 0) {
        dx = speed * dirx / sqrt(dirx * dirx + diry * diry);
        dy = speed * diry / sqrt(dirx * dirx + diry * diry);
    }

    // 预判玩家下一步的位置
    double nextX = this->x() + dx;
    double nextY = this->y() + dy;

    // 只有当玩家已经被加入到场景中时，才进行边界判定
    if (this->scene()) {
        // 获取地图范围
        QRectF mapRect = this->scene()->sceneRect();

        // 获取玩家自身的范围
        QRectF playerRect = this->boundingRect();

        // 限制 X 坐标 (左右碰壁)
        if (nextX < mapRect.left()) {
            nextX = mapRect.left(); // 撞到左墙，贴紧左墙
        }
        else if (nextX + playerRect.width() > mapRect.right()) {
            nextX = mapRect.right() - playerRect.width(); // 撞到右墙，贴紧右墙
        }

        // 限制 Y 坐标 (上下碰壁)
        if (nextY < mapRect.top()) {
            nextY = mapRect.top(); // 撞到上墙，贴紧上墙
        }
        else if (nextY + playerRect.height() > mapRect.bottom()) {
            nextY = mapRect.bottom() - playerRect.height(); // 撞到下墙，贴紧下墙
        }
    }

    // 把玩家放到指定地点（移动）
    this->setPos(nextX, nextY);
}

void Player::mouseMove(const QPointF posInScene) {
    // 1. 手感优化：获取玩家的中心点坐标，而不是左上角
    QRectF pRect = this->boundingRect();
    QPointF centerPos = this->scenePos() + QPointF(pRect.width() / 2.0, pRect.height() / 2.0);

    // 计算中心点到鼠标的向量
    const QPointF dirVector = posInScene - centerPos;

    // 计算距离
    double L = sqrt(QPointF::dotProduct(dirVector, dirVector));

    // 如果鼠标离玩家中心非常近（死区），则不移动也不旋转，防止鬼畜抖动
    if (L < 2.0) return;

    // 2. 计算位移分量 (跟随鼠标角度)
    double speedL = speed * L/(L + 10);
    double dx = dirVector.x() / L * speedL;
    double dy = dirVector.y() / L * speedL;

    // ================= 新增：360度丝滑旋转剑 =================
    // 更新最后面朝的方向（供静止时使用）
    lastDir = dirVector;

    // 只要有位移向量，且剑在显示状态，就计算出精确的 360 度弧度并旋转
    if (swordItem->isVisible()) {
        double angle = qRadiansToDegrees(qAtan2(dirVector.y(), dirVector.x()));
        swordItem->setRotation(angle);
    }
    // ========================================================

    // 3. 预判下一步位置
    double nextX = this->x() + dx;
    double nextY = this->y() + dy;

    // ================= 新增：边界限制逻辑 =================
    if (this->scene()) {
        QRectF mapRect = this->scene()->sceneRect();
        QRectF playerRect = this->boundingRect();

        // 限制 X 坐标 (左右碰壁)
        if (nextX < mapRect.left()) {
            nextX = mapRect.left();
        }
        else if (nextX + playerRect.width() > mapRect.right()) {
            nextX = mapRect.right() - playerRect.width();
        }

        // 限制 Y 坐标 (上下碰壁)
        if (nextY < mapRect.top()) {
            nextY = mapRect.top();
        }
        else if (nextY + playerRect.height() > mapRect.bottom()) {
            nextY = mapRect.bottom() - playerRect.height();
        }
    }
    // ========================================================

    // 4. 执行移动
    this->setPos(nextX, nextY);
}

void Player::mouse3Dmove(const QPointF mouseDiff)
{
    double L = sqrt(QPointF::dotProduct(mouseDiff, mouseDiff));
    if (L < 30.0) return;

    // 跟随鼠标角度
    double dx = -mouseDiff.x() / L * speed;
    double dy = -mouseDiff.y() / L * speed;

    this->moveBy(dx, dy);
}

// 无敌相关实现
void Player::giveImmune(int timeMs) {
    isImmune = true;
    // 启动（或重启）计时器。
    // 如果计时器已经在运行（玩家已经是无敌状态），调用 start 会重新开始计时！
    // 这完美解决了“吃两个无敌道具时间不叠加”的 Bug。
    immuneTimer->start(timeMs);
}

void Player::endImmune() {
    isImmune = false;
}


// 光剑技能相关实现

QGraphicsPixmapItem* Player::getSword() {
    return swordItem;
}

void Player::equipSword(int durationMs) {
    swordItem->show(); // 把剑显示出来
    swordTimer->start(durationMs);
}

// 射击技能
void Player::autoFire(int rounds, int interval, int num)
{
    fireTimes = rounds;
    currNum = num;
    currInterval = interval;

    fireTimer->start(interval);
}

// 护盾技能
QGraphicsRectItem* Player::getShield() {
    return shieldItem;
}

void Player::equipShield(int durationMs) {
    shieldItem->show();
    totemItem->show();
    shieldTimer->start(durationMs);
}

void Player::breakShieldAndExplode(int radius, int lifeTime, bool haveTotem) {
    if (shieldItem) shieldItem->hide();
    if (totemItem) totemItem->hide();
    if (shieldTimer && shieldTimer->isActive()) shieldTimer->stop();

    this->giveImmune(32);

    if (haveTotem && this->scene() && !this->scene()->views().isEmpty()) {
        QGraphicsView* view = this->scene()->views().first();

        // 1. 【核心修改】：获取玩家当前的场景绝对坐标
        QPointF playerScenePos = this->scenePos();

        // 2. 【核心修改】：使用投影函数，将场景坐标转为当前屏幕上的像素坐标！ [4]
        // 哪怕你的地图相机在滚动，它也能精准算出玩家此时在屏幕上的哪个像素点 [4]
        QPoint screenPos = view->mapFromScene(playerScenePos);

        QLabel* totemLabel = new QLabel(view);
        totemLabel->setAttribute(Qt::WA_TranslucentBackground);
        totemLabel->setWindowFlags(Qt::SubWindow);
        totemLabel->setAlignment(Qt::AlignCenter);
        totemLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        totemLabel->setFocusPolicy(Qt::NoFocus);
        totemLabel->setAttribute(Qt::WA_ShowWithoutActivating);

        QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(totemLabel);

        // 半透明效果
        opacityEffect->setOpacity(0.85);
        totemLabel->setGraphicsEffect(opacityEffect);

        // QMovie* movie = new QMovie(":/ImageResources/totem-of-undying-faked-death.gif", QByteArray(), totemLabel);
        QMovie* movie = new QMovie(":/ImageResources/Totem_of_Undying_Animation.gif", QByteArray(), totemLabel);
        // movie->setSpeed(125);
        movie->setSpeed(80);

        int gifW = 180;
        int gifH = 180;
        movie->setScaledSize(QSize(gifW, gifH));
        totemLabel->setMovie(movie);

        // 3. 【核心修改】：将大动画标签的【中点】对准玩家所在的屏幕投影坐标
        int startX = screenPos.x() - gifW / 2;
        int startY = screenPos.y() - gifH / 2;
        totemLabel->setGeometry(startX, startY, gifW, gifH);

        totemLabel->show();
        movie->start();

        connect(movie, &QMovie::finished, totemLabel, &QLabel::deleteLater);
        QTimer::singleShot(2000, totemLabel, &QLabel::deleteLater);

        // 播放音效
        SoundPool::instance().play("Shield_break");
        SoundPool::instance().duckOthers("Shield_break", 1000, 0.2);
    }

    // 3. 生成爆炸类对象
    Explosion* boom = new Explosion(radius, lifeTime);

    // 4. 设置爆炸的中心点坐标为玩家的当前坐标
    boom->setPos(this->pos());

    // 5. 将爆炸对象添加到当前的游戏场景中
    if (this->scene()) {
        this->scene()->addItem(boom);
    }
}

// 蓄力条 咖喱棒
void Player::startCharging(double time_in_s)
{
    // 播放音效
    SoundPool::instance().play("Lochunhin_fuse");

    tntItem->setVisible(true);

    currProgress = 1.0;
    deltaP = 0.050 / time_in_s;  // 每50ms更新一次
    chargeBar->setProgress(0.0);
    chargeBar->setVisible(true);
    chargeBarTimer->start(50);   // 每50ms更新一次
}

void Player::onCharging() {
    currProgress -= deltaP;
    chargeBar->setProgress(std::min(1.0, 1.0 - currProgress));

    if (currProgress <= 0.0) {
        chargeBarTimer->stop();
        chargeBar->setVisible(false);
        tntItem->setVisible(false);

        launchLochunhin();
    }
}

void Player::launchLochunhin()
{
    double ang = - QLineF({0, 0}, lastDir).angle();
    QPointF currentPos = this->pos();
    CrescentWave* temp = new CrescentWave(ang, currentPos);
    this->scene()->addItem(temp);

    // 播放音效
    SoundPool::instance().play("Lochunhin_launch");
}

// 导弹
void Player::launchMissile(int N)
{
    QGraphicsScene* sc = scene();
    if (!sc) return;

    QList<QGraphicsItem*> enemyList;

    const QList<QGraphicsItem*> allItems = sc->items();
    for (QGraphicsItem* item : allItems) {
        // 通过 data() 系统判断类型，无需 include 敌人头文件
        if (item->data(0).toString() == "enemy") {
            enemyList.append(item);
        }
    }

    QRandomGenerator* random = QRandomGenerator::global();

    // 播放音效
    SoundPool::instance().play("Missile_launch");

    for (int i = 0; i < N; ++i) {
        // 无论敌人列表是否为空，都随机生成弧度值 (0 到 2*pi 之间)
        double ang = random->bounded(360.0);

        QGraphicsItem* target = nullptr;

        // 如果有敌人，随机选择一个作为目标
        // 当 n 大于敌人数量时，由于每次循环都是独立随机抽取，自然会出现多个导弹锁定同一个目标的情况
        if (!enemyList.isEmpty()) {
            int randomIndex = random->bounded(enemyList.size());
            target = enemyList.at(randomIndex);
        }

        // 实例化导弹
        Missile* missile = new Missile(target, ang, this->pos());

        // 将导弹添加到场景中
        sc->addItem(missile);
    }
}
