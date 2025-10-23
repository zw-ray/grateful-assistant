/*******************************************************************************
**     FileName: TodoInput.h
**    ClassName: TodoInput
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:36
**  Description: 新建待办项的输入框
*******************************************************************************/

#ifndef TODOINPUT_H
#define TODOINPUT_H

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class TodoInput : public QWidget
{
    Q_OBJECT
public:
    TodoInput(QWidget *parent = nullptr);
    ~TodoInput();

protected:
    void initUI();

protected slots:
    void onAddClicked();

signals:
    void todoAdded(const QString &text);

private:
    QLineEdit *m_inputEdit;   // 输入框
    QPushButton *m_addButton; // 添加按钮
};

#endif // TODOINPUT_H
