#ifndef PORTALPOOL_H
#define PORTALPOOL_H

#include <QList>
#include <QPointer>
#include <QGraphicsScene>
#include "PortalPiece.h"

class PortalPool
{
public:
    // 获取单例实例
    static PortalPool& instance()
    {
        static PortalPool inst;
        return inst;
    }

    /**
     * @brief 初始化对象池，生成20个对象并加入到指定的场景中
     */
    void init(QGraphicsScene* scene, const QString& imagePath, int frameIntervalMs, int lifeFrame, int Brightness)
    {
        if (!scene) return;

        // 清理旧的数据，避免重复初始化
        clear();

        for (int i = 0; i < NP; ++i) {
            PortalPiece* piece = new PortalPiece(imagePath, frameIntervalMs, lifeFrame, Brightness);
            scene->addItem(piece); // 将其加入场景
            m_pool.append(piece);
        }
    }

    /**
     * @brief 获取当前处于隐藏状态的对象指针
     * @return 如果有空闲对象则返回指针，如果全部都在显示则返回 nullptr
     */
    PortalPiece* getHiddenPiece()
    {
        for (const QPointer<PortalPiece>& piece : std::as_const(m_pool)) {
            if (piece && !piece->isVisible()) {
                return piece.data();
            }
        }
        return nullptr;
    }

    /**
     * @brief 手动清空池子，释放内存
     */
    void clear()
    {
        for (const QPointer<PortalPiece>& piece : std::as_const(m_pool)) {
            if (piece) {
                delete piece.data();
            }
        }
        m_pool.clear();
    }

private:
    PortalPool() = default;
    ~PortalPool() = default;

    // 禁用拷贝与赋值
    PortalPool(const PortalPool&) = delete;
    PortalPool& operator=(const PortalPool&) = delete;

    const int NP = 40;
    QList<QPointer<PortalPiece>> m_pool; // 使用 QPointer 避免野指针
};

#endif // PORTALPIECEPOOL_H
