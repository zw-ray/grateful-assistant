/*******************************************************************************
**     FileName: FloatMessage.h
**    ClassName: FloatMessage
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/06 10:28
**  Description:
*******************************************************************************/

#ifndef FLOATMESSAGE_H
#define FLOATMESSAGE_H

#include <QLabel>
#include <QTimer>
#include <QWidget>

class FloatMessage : public QWidget
{
    Q_OBJECT
public:
    FloatMessage(const QString &text, QWidget *parent = nullptr,
                 int duration = 2000);
    ~FloatMessage();

    void closeEvent(QCloseEvent *event) override;

    void setPosition(int x, int y);

signals:
    void closed();

private:
    QLabel *m_label;
}; // class FloatMessage

#endif // FLOATMESSAGE_H
