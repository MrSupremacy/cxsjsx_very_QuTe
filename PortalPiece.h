#ifndef PORTALPIECE_H
#define PORTALPIECE_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QTimer>
#include <QPainter>

class PortalPiece : public QGraphicsObject
{
    Q_OBJECT

public:
    /**
     * @brief PortalPiece 构造函数
     * @param imagePath 精灵图资源路径
     * @param frameIntervalMs 动画帧切换间隔时间（毫秒）
     * @param lifeTimeMs 生存时间（毫秒），达到该时间后对象会自动隐藏。若设为 <= 0 则不自动隐藏。
     * @param parent 父级图形项
     */
    PortalPiece(const QString& imagePath,
                int frameIntervalMs,
                int lifeTimeMs,
                QGraphicsItem* parent = nullptr)
        : QGraphicsObject(parent)
        , m_currentFrame(0)
        , m_frameIntervalMs(frameIntervalMs)
        , m_lifeTimeMs(lifeTimeMs)
    {
        // 加载精灵图大图
        m_spriteSheet.load(imagePath);

        // 1. 初始化控制动画帧率的定时器（初始不启动）
        m_frameTimer = new QTimer(this);
        connect(m_frameTimer, &QTimer::timeout, this, &PortalPiece::nextFrame);

        // 2. 初始化定时隐藏计时器
        m_lifeTimer = new QTimer(this);
        m_lifeTimer->setSingleShot(true);
        connect(m_lifeTimer, &QTimer::timeout, this, &PortalPiece::hidePiece);

        // 3. 初始状态设为隐藏
        setVisible(false);
    }

    // 定义该 Item 的碰撞/绘制边界
    QRectF boundingRect() const override
    {
        return QRectF(0, 0, targetWd, targetHt);
    }

    // 2. 调整绘制逻辑，使用 targetRect 和 sourceRect
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        // 目标区域：在场景中实际绘制的较小正方形
        QRectF targetRect(0, 0, targetWd, targetHt);

        // 源区域：在原精灵图（Sprite Sheet）上裁剪的单帧区域
        QRectF sourceRect(0, m_currentFrame * Ht, Wd, Ht);

        // 启用平滑缩放以获得更好的画面质量（可选，可能会有微小的性能开销）
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

        // 绘制并自动缩放
        painter->drawPixmap(targetRect, m_spriteSheet, sourceRect);
    }

    /**
     * @brief 激活并显示对象
     * @param pos 激活时的场景坐标
     */
    void showPiece(const QPointF& pos)
    {
        setPos(pos);
        setVisible(true);
        m_currentFrame = 0;

        // 开启帧动画
        m_frameTimer->start(m_frameIntervalMs);

        // 开启定时隐藏
        if (m_lifeTimeMs > 0) {
            m_lifeTimer->start(m_lifeTimeMs);
        }
        update();
    }

    /**
     * @brief 隐藏对象并停止相关定时器（节省CPU开销）
     */
    void hidePiece()
    {
        setVisible(false);
        m_frameTimer->stop();
        m_lifeTimer->stop();
    }

private slots:
    // 切换到下一帧并触发重绘
    void nextFrame()
    {
        m_currentFrame = (m_currentFrame + 1) % frameN;
        update();
    }

private:
    QPixmap m_spriteSheet;  // 完整的精灵图
    const int Wd = 128;     // 单帧宽度
    const int Ht = 128;     // 单帧高度
    const int targetWd = 24;
    const int targetHt = 24;
    const int frameN = 32;  // 动画总帧数
    int m_currentFrame;     // 当前帧索引

    QTimer* m_frameTimer;   // 帧控制计时器
    QTimer* m_lifeTimer;    // 定时隐藏计时器
    int m_frameIntervalMs;  // 帧间隔
    int m_lifeTimeMs;       // 生存时间
};

#endif // PORTALPIECE_H
