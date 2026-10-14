#include "MainWindow.h"
#include "GameBoard.h"
#include "MainMenu.h"

#include <QStackedWidget>
#include <QApplication>
#include <QMessageBox>

enum PageIndex {
    MainMenuPage = 0,
    GameBoardPage = 1
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建 QStackedWidget
    m_stackedWidget = new QStackedWidget(this);

    // 创建页面
    m_mainMenu = new MainMenu(this);
    m_gameBoard = new GameBoard(this); // GameBoard的父对象是MainWindow

    // 将页面添加到 QStackedWidget
    m_stackedWidget->addWidget(m_mainMenu);
    m_stackedWidget->addWidget(m_gameBoard);

    // 将 QStackedWidget 设置为中央控件
    this->setCentralWidget(m_stackedWidget);
    this->setFixedSize(m_gameBoard->size()); // 设置窗口大小与游戏板一致

    // 连接主菜单的信号到MainWindow的槽
    connect(m_mainMenu, &MainMenu::singlePlayerClicked, this, &MainWindow::startNewGame);
    connect(m_mainMenu, &MainMenu::twoPlayerClicked, this, &MainWindow::startTwoPlayerGame);
    connect(m_mainMenu, &MainMenu::loadGameClicked, this, &MainWindow::loadAndStartGame);
    connect(m_mainMenu, &MainMenu::exitClicked, this, &QApplication::quit); // 退出按钮直接退出程序

    // 连接游戏板返回主菜单的信号
    connect(m_gameBoard, &GameBoard::backToMainClicked, this, &MainWindow::backToMainMenu);
    connect(m_gameBoard, &GameBoard::gameFinished, this, &MainWindow::backToMainMenu);

    // 初始显示主菜单
    m_stackedWidget->setCurrentIndex(MainMenuPage);
}

void MainWindow::startNewGame()
{
    m_gameBoard->newGame(GameMode::SinglePlayer); // 开始一个新游戏
    m_stackedWidget->setCurrentIndex(GameBoardPage); // 切换到游戏界面
}

void MainWindow::startTwoPlayerGame()
{
    m_gameBoard->newGame(GameMode::TwoPlayer);
    m_stackedWidget->setCurrentIndex(GameBoardPage);
}

void MainWindow::loadAndStartGame()
{
    if (m_gameBoard->loadGame()) {
        m_stackedWidget->setCurrentIndex(GameBoardPage); // 切换到游戏界面
    } else {
        QMessageBox::warning(
            this,
            "加载失败",
            "未找到存档文件，或文件已损坏。"
            );
    }
}

void MainWindow::backToMainMenu()
{
    m_stackedWidget->setCurrentIndex(MainMenuPage); // 切换回主菜单
}

