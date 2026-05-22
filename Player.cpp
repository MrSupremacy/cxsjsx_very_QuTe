// Created by 樊轩楷 & 吉佑安

#include "Player.h"
#include <QLineF>


Player::Player()
{
    setRect(0, 0, 12, 12);
    setBrush(QBrush(Qt::white)); // 基础颜色为白色
    setPen(Qt::NoPen); // 移除边框

    immuneTimer = new QTimer(this);
    immuneTimer->setSingleShot(true); // 设置为单次触发（只响一次）
    connect(immuneTimer, &QTimer::timeout, this, &Player::endImmune);

    // 光剑技能 配置光剑
    swordItem = new QGraphicsRectItem(0, -1.5, 50, 3, this);
    swordItem->setBrush(Qt::yellow); // 给剑涂成黄色
    swordItem->hide(); // 初始状态隐藏（没吃到技能时没有剑）
    swordItem->setPos(6, 6); // 放到中间位置

    swordTimer = new QTimer();
    QObject::connect(swordTimer, &QTimer::timeout, [=](){ // 当定时器时间到，隐藏这把剑
        swordItem->hide();
    });
    swordTimer->setSingleShot(true); // 设为单次触发模式

    // 咖喱棒技能 配置蓄力条
    chargeBar = new PlayerChargeBar(this);
    chargeBar->setPos({6, 6 - 12});
    chargeBarTimer = new QTimer();
    QObject::connect(chargeBarTimer, &QTimer::timeout, [this](){
        this->onCharging();
    });

    // 护盾技能 配置护盾
    shieldItem = new QGraphicsEllipseItem(-10, -10, 20, 20, this);
    shieldItem->setBrush(Qt::NoBrush); // 无填充
    shieldItem->setPen(QPen(Qt::darkCyan)); // 涂成淡青色
    shieldItem->hide();
    shieldItem->setPos(6, 6); // 放到中间位置

    shieldTimer = new QTimer();
    QObject::connect(shieldTimer, &QTimer::timeout, [=](){ // 当定时器时间到，隐藏这个盾
        shieldItem->hide();
        breakShieldAndExplode(); // 盾自己破也要炸
    });
    shieldTimer->setSingleShot(true); // 设为单次触发模式

    // 射击技能 配置射击方法
    fireTimer = new QTimer(); // 确保 fireTimer 已实例化
    fireTimer->setSingleShot(true); // 设置为单次触发模式

    QObject::connect(fireTimer, &QTimer::timeout, [&]() mutable {
        if (fireTimes > 0) {
            fireTimes--;
            double ang = QLineF({0, 0}, lastDir).angle();
            ang = qDegreesToRadians(ang);

            // 生成 currNum 个子弹
            QPointF dir = {-qCos(ang), qSin(ang)};
            QPointF perp = {-dir.y(), dir.x()};

            // 生成 currNum 个子弹 from 对象池
            for (int var = 0; var < currNum; ++var) {
                double offset = (var - (currNum - 1) / 2.0) * distPx;
                QPointF currentPos = this->pos() + perp *offset;

                // 从对象池获取子弹
                Bullet* temp = BulletPool::getInstance().getBullet(-ang, currentPos);
                if (!temp) break;

                // 如果子弹当前不在场景中，才把它加进去：额外检查，理论不会
                if (temp->scene() != this->scene()) {
                    this->scene()->addItem(temp);
                }
            }

            if (fireTimes > 0) {
                fireTimer->start(currInterval);
            }
        }
    });
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
        QRectF playerRect = this->rect();

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
    QRectF pRect = this->rect();
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
        QRectF playerRect = this->rect();

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

QGraphicsRectItem* Player::getSword() {
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
QGraphicsEllipseItem* Player::getShield() {
    return shieldItem;
}

void Player::equipShield(int durationMs) {
    shieldItem->show();
    shieldTimer->start(durationMs);
}

void Player::breakShieldAndExplode(int radius, int lifeTime) {
    // 1. 隐藏盾牌的图形
    if(shieldItem) {
        shieldItem->hide();
    }

    // 2. 停止盾牌的倒计时器
    if (shieldTimer && shieldTimer->isActive()) {
        shieldTimer->stop();
    }

    // 2.5 给玩家32ms无敌帧
    this->giveImmune(32);

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
void Player::startCharging(double time_in_s) {
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

        launchLochunhin();
    }
}

void Player::launchLochunhin()
{
    double ang = - QLineF({0, 0}, lastDir).angle();
    QPointF currentPos = this->pos();
    CrescentWave* temp = new CrescentWave(ang, currentPos);
    this->scene()->addItem(temp);
}
