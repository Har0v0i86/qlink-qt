#include "MainMenu.h"
#include <QPushButton>
#include <QVBoxLayout>

MainMenu::MainMenu(QWidget *parent) : QWidget(parent)
{
    // 创建按钮
    m_singlePlayerBtn = new QPushButton("单人游戏", this);
    m_twoPlayerBtn = new QPushButton("双人游戏", this);
    m_loadGameBtn = new QPushButton("读取存档", this);
    m_exitBtn = new QPushButton("退出游戏", this);

    // 设置样式
    m_singlePlayerBtn->setMinimumHeight(40);
    m_twoPlayerBtn->setMinimumHeight(40);
    m_loadGameBtn->setMinimumHeight(40);
    m_exitBtn->setMinimumHeight(40);

    // 创建一个垂直布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(m_singlePlayerBtn);
    layout->addWidget(m_twoPlayerBtn);
    layout->addWidget(m_loadGameBtn);
    layout->addWidget(m_exitBtn);
    layout->addStretch();
    layout->setContentsMargins(150, 50, 150, 50);

    setLayout(layout);
    connect(m_singlePlayerBtn, &QPushButton::clicked, this, &MainMenu::singlePlayerClicked);
    connect(m_loadGameBtn, &QPushButton::clicked, this, &MainMenu::loadGameClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &MainMenu::exitClicked);
    connect(m_twoPlayerBtn, &QPushButton::clicked, this, &MainMenu::twoPlayerClicked);

}
