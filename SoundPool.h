#ifndef SOUNDPOOL_H
#define SOUNDPOOL_H

#pragma once

#include <QSoundEffect>
#include <QHash>
#include <QList>
#include <QString>
#include <QUrl>
#include <QDebug>
#include <QFile>
#include <QTimer>    // 新增：用于延时停止
#include <QVariant>  // 新增：用于动态属性

class SoundPool {
public:
    static SoundPool& instance() {
        static SoundPool instance;
        return instance;
    }

    SoundPool(const SoundPool&) = delete;
    SoundPool& operator=(const SoundPool&) = delete;

    // 批量初始化
    inline void init(const QHash<QString, QString>& soundMap, qreal Vol) {
        for (auto it = soundMap.constBegin(); it != soundMap.constEnd(); ++it) {
            load(it.key(), it.value());
        }
        qDebug() << "SoundPool initialized. Audio types loaded:" << m_soundUrls.size();

        volume = Vol;
    }

    // 单独加载音效（只记录路径并预热1个实例）
    inline void load(const QString& name, const QString& path) {
        if (m_soundUrls.contains(name)) return;

        m_soundUrls.insert(name, QUrl(path));
        // 预热
        for (int i = 0; i < MAX_INSTANCES_PER_SOUND; ++i) createInstance(name);
    }

    // 更新音效相对大小
    inline void setSoundWeight(const QString& name, qreal weight) {
        qreal clampedWeight = qBound(0.0, weight, 1.0);
        m_soundWeights[name] = clampedWeight;

        // 如果该音效已经被创建了，实时刷新一下它的实际音量
        if (m_soundPlayers.contains(name)) {
            for (QSoundEffect* effect : m_soundPlayers[name]) {
                effect->setVolume(volume * clampedWeight);
            }
        }
    }

    // 播放音效（核心并发逻辑）
    inline void play(const QString& name, int timeMs = -1) {

        if (!m_soundUrls.contains(name)) {
            qWarning() << "SoundPool: 找不到音效 ->" << name;
            return;
        }

        QList<QSoundEffect*>& pool = m_soundPlayers[name];
        QSoundEffect* effectToPlay = nullptr;

        // 1. 遍历当前该音效的池子，寻找一个“空闲”（未在播放）的实例
        for (int i = 0; i < pool.size(); ++i) {
            if (!pool.at(i)->isPlaying()) {
                effectToPlay = pool.at(i);
                break;
            }
        }

        // 2. 如果池子里所有的实例都在播放，说明并发太高了
        if (!effectToPlay) {
            // 达到上限（Voice Stealing）。强行掐断并征用最老的一个实例（索引0）
            effectToPlay = pool.takeFirst(); // 拿出最老的
            pool.append(effectToPlay);       // 放到队尾，把它标记为最新的
        }

        if (effectToPlay->status() == QSoundEffect::Error
                    || effectToPlay->status() == QSoundEffect::Null) {
            qDebug() << "检测到失效的音频句柄，正在重建...";
            QUrl source = effectToPlay->source();
            effectToPlay->setSource(QUrl()); // 清空
            effectToPlay->setSource(source); // 重新加载，触发底层重新申请系统音频句柄
        }

        // 3. 播放
        if (effectToPlay) {
            // 播放前计算应有音量（自动读取当前是否有全局压制状态）
            qreal weight = m_soundWeights.value(name, 1.0);
            qreal currentDuck = (name == m_duckExemptName) ? 1.0 : m_duckRatio;
            effectToPlay->setVolume(volume * weight * currentDuck);

            effectToPlay->play();

            // 【修改点】：为本次播放分配一个独立的标识（自增ID）
            // 目的：防止实例被复用(Voice Stealing)后，被旧的定时器误杀
            int currentPlayId = effectToPlay->property("playId").toInt() + 1;
            effectToPlay->setProperty("playId", currentPlayId);

            // 如果设置了最大播放时间
            if (timeMs > 0) {
                // 启动单次定时器
                QTimer::singleShot(timeMs, effectToPlay, [effectToPlay, currentPlayId]() {
                    // 检查当前播放标识是否匹配，且确实还在播放
                    if (effectToPlay->property("playId").toInt() == currentPlayId && effectToPlay->isPlaying()) {
                        effectToPlay->stop();
                    }
                });
            }
        }
    }

    inline void duckOthers(const QString& exemptName, int durationMs, qreal duckRatio = 0.2) {
        if (durationMs <= 0) return;

        m_duckRatio = qBound(0.0, duckRatio, 1.0);
        m_duckExemptName = exemptName;
        m_duckingId++;           // 刷新ID，如果连续调用会打断上一次的恢复定时器
        int currentDuckId = m_duckingId;

        // 1. 立刻应用压制，其他所有声音会瞬间被压低或静音
        refreshAllVolumes();

        // 2. 时间到了之后恢复全局音量
        QTimer::singleShot(durationMs, [this, currentDuckId]() {
            if (this->m_duckingId == currentDuckId) {
                this->m_duckRatio = 1.0;     // 恢复正常比例
                this->m_duckExemptName.clear(); // 清除特权
                this->refreshAllVolumes();   // 恢复所有人音量
            }
        });
    }

    inline void stopAll() {
        for (const QList<QSoundEffect*>& pool : std::as_const(m_soundPlayers)) {
            for (QSoundEffect* effect : pool) {
                if (effect->isPlaying()) {
                    effect->stop();
                }
            }
        }
    }

private:
    SoundPool() {}

    ~SoundPool() {
        // 清理所有动态分配的 QSoundEffect 内存
        for (const QList<QSoundEffect*>& pool : std::as_const(m_soundPlayers)) {
            qDeleteAll(pool);
        }
        m_soundPlayers.clear();
        m_soundUrls.clear();
    }

    // 内部方法：创建一个新的 QSoundEffect 实例并加入池中
    inline QSoundEffect* createInstance(const QString& name) {
        QSoundEffect* effect = new QSoundEffect();
        effect->setSource(m_soundUrls.value(name));
        m_soundPlayers[name].append(effect);
        return effect;
    }

    // 内部核心功能：刷新所有实例的音量 (公式：全局主音量 * 音效相对权重 * 闪避倍率)
    inline void refreshAllVolumes() {
        for (auto it = m_soundPlayers.constBegin(); it != m_soundPlayers.constEnd(); ++it) {
            const QString& sName = it.key();
            qreal weight = m_soundWeights.value(sName, 1.0);
            qreal currentDuck = (sName == m_duckExemptName) ? 1.0 : m_duckRatio;
            qreal finalVol = volume * weight * currentDuck;

            for (QSoundEffect* effect : it.value()) {
                effect->setVolume(finalVol);
            }
        }
    }

private:
    // 同一个音效最大允许重叠播放的次数。
    const int MAX_INSTANCES_PER_SOUND = 1;

    // 记录音量全局配置
    qreal volume;

    // 记录每个声效的声音强度
    QHash<QString, qreal> m_soundWeights;

    // ----- 压闪(Ducking)状态 -----
    qreal m_duckRatio = 1.0;
    QString m_duckExemptName;
    int m_duckingId = 0;
    // ----------------------------

    // 记录音效别名和对应的文件路径
    QHash<QString, QUrl> m_soundUrls;

    // 真正的对象池：音效别名 -> 多个 QSoundEffect 实例的列表
    QHash<QString, QList<QSoundEffect*>> m_soundPlayers;
};

#endif // SOUNDPOOL_H
