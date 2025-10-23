/*******************************************************************************
**     FileName: TodoListItem.h
**    ClassName: TodoListItem
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/01 13:37
**  Description: 待办项列表的每一项
*******************************************************************************/
// TodoListItem.h
#ifndef TODOLISTITEM_H
#define TODOLISTITEM_H

#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <kernel/entities/Todo.h>
#include <qline.h>
#include <qwidget.h>

class TodoListItem : public QWidget
{
    Q_OBJECT
public:
    TodoListItem(const Todo &todo, QWidget *parent = nullptr);
    ~TodoListItem() override;

    // 更新项显示（用于进度或内容变化时）
    void updateItem(const Todo &todo);
    void paintEvent(QPaintEvent *event) override;

    // 尺寸变化事件，用于更新渐变区域
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // 完成按钮点击槽函数
    void onCompleteButtonClicked();

private:
    void initUI();
    void setPriorityStyle(Todo::Priority priority);
    float calculateProgress() const; // 计算进度的新方法 0-1之间
    // 根据进度计算渐变颜色
    void updateGradient();

    Todo m_todo;
    QWidget *m_leftTextArea;   // for showing title and content
    QWidget *m_rightLeftArea;  // for showing status and priority
    QWidget *m_rightRightArea; // for showing fast operation buttons
    QLinearGradient m_gradient;
};

#endif // TODOLISTITEM_H
