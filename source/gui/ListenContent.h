/*******************************************************************************
**     FileName: ListenContent.h
**    ClassName: ListenContent
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/06 15:18
**  Description:
*******************************************************************************/

#ifndef LISTENCONTENT_H
#define LISTENCONTENT_H

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <cstdint>

class ListenContent : public QWidget
{
    Q_OBJECT
public:
    explicit ListenContent(QWidget *parent = nullptr);
    ~ListenContent() override;

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateWaveform();

private:
    void initUI();
    void initWaveform();

    QLabel *m_listenLabel;        // 显示"聆听中"文字
    QHBoxLayout *m_waveLayout;    // 波形布局
    QList<QFrame *> m_waveBars;   // 波形条集合
    QTimer *m_animationTimer;     // 动画定时器
    const int WAVE_BAR_COUNT = 7; // 波形条数量
    uint32_t m_waveCount = 0;     // 当前容器内波形条数量
}; // class ListenContent

#endif // LISTENCONTENT_H
