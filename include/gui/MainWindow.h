/*******************************************************************************
**     FileName: MainWindow.h
**    ClassName: MainWindow
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 11:57
**  Description:
*******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHBoxLayout>
#include <QMainWindow>
#include <QVBoxLayout>
#include <gui/GuiExport.h>
#include <kernel/Events.h>
#include <memory>

class GUI_API MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();
    QVBoxLayout *initLeftComponent();
    QVBoxLayout *initRightComponent();

signals:
    void systemMessageIn(const QString &message);
    void triggerShowListenContent();
    void triggerHideListenContent();

private slots:
    void handleSystemMessageIn(const QString &message);
    void updateSystemMessage();
    void onTriggerShowListenContent();
    void onTriggerHideListenContent();

private:
    void onValidWakeWord(const AIEvents::ValidWakeWordEvent &event);
    void onContentRecordingDone(
        const AudioEvents::AudioContentRecordingDoneEvent &event);
    void onSystemMessage(const SystemEvents::SystemMessageEvent &event);

private:
    struct Data;
    std::unique_ptr<Data> m_data;
};

#endif // MAINWINDOW_H
