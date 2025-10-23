/*******************************************************************************
**     FileName: Card.h
**    ClassName: Card
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/10 20:24
**  Description:
*******************************************************************************/

#ifndef CARD_H
#define CARD_H

#include <QEvent>
#include <QFrame>
#include <QPainter>
#include <QStyleOption>

class Card : public QFrame
{
    Q_OBJECT
public:
    explicit Card(QWidget *parent = nullptr);
    ~Card() = default;

    void paintEvent(QPaintEvent *) override;

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
}; // class Card

#endif // CARD_H
