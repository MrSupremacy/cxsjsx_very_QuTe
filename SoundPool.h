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
        // 预热（Pre-warm）：预先创建1个实例存入池中，防止第一次播放时产生加载延迟
        createInstance(name);
    }

    // 播放音效（核心并发逻辑）
    inline void play(const QString& name) {

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
            if (pool.size() < MAX_INSTANCES_PER_SOUND) {
                // 策略A：还没达到并发上限，动态创建一个新的 QSoundEffect
                effectToPlay = createInstance(name);

            } else {
                // 策略B：达到上限（Voice Stealing）。强行掐断并征用最老的一个实例（索引0）
                effectToPlay = pool.takeFirst(); // 拿出最老的
                pool.append(effectToPlay);       // 放到队尾，把它标记为最新的
            }
        }

        // 3. 播放
        if (effectToPlay) {
            effectToPlay->setVolume(volume);
            effectToPlay->play();
        }
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

private:
    // 同一个音效最大允许重叠播放的次数。
    // 设为 5~8 就足够了，100个敌人同一帧死，人耳听起来其实就是一声巨大的爆炸，不需要100个实例。
    const int MAX_INSTANCES_PER_SOUND = 1;

    // 记录音量全局配置
    qreal volume;

    // 记录音效别名和对应的文件路径
    QHash<QString, QUrl> m_soundUrls;

    // 真正的对象池：音效别名 -> 多个 QSoundEffect 实例的列表
    QHash<QString, QList<QSoundEffect*>> m_soundPlayers;
};

#endif // SOUNDPOOL_H
