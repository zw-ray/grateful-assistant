#include "TodoInput.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

TodoInput::TodoInput(QWidget *parent) : QWidget(parent) { initUI(); }

TodoInput::~TodoInput() {}

void TodoInput::initUI()
{
    // 创建输入框
    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setPlaceholderText(tr("请输入新的待办事项..."));
    m_inputEdit->setMinimumHeight(36);

    // 创建添加按钮
    m_addButton = new QPushButton(tr("添加"), this);
    m_addButton->setMinimumHeight(36);
    m_addButton->setMinimumWidth(80);

    // 连接信号槽
    connect(m_addButton, &QPushButton::clicked, this, &TodoInput::onAddClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this,
            &TodoInput::onAddClicked);

    // 主布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_inputEdit);
    layout->addWidget(m_addButton);
}

void TodoInput::onAddClicked()
{
    QString text = m_inputEdit->text().trimmed();
    if (!text.isEmpty())
    {
        emit todoAdded(text);
        m_inputEdit->clear();
    }
}
