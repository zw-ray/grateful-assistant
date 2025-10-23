/*******************************************************************************
**     FileName: DateTimeCard.h
**    ClassName: DateTimeCard
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:36
**  Description: 显示时间的组件
*******************************************************************************/

#ifndef DATETIMECARD_H
#define DATETIMECARD_H

#include "Card.h"
#include <QWidget>

class DateTimeCard : public Card
{
    Q_OBJECT
public:
    DateTimeCard(QWidget *parent = nullptr);
    ~DateTimeCard();

protected:
    void initUI();
    void paintEvent(QPaintEvent *event) override;

protected slots:
    void updateTime();
    void updateDate();
}; // class DateTimeCard

#endif // DATETIMECARD_H
