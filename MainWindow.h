#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QStackedWidget;
class GameBoard;
class MainMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void startNewGame();
    void startTwoPlayerGame();
    void loadAndStartGame();
    void backToMainMenu();

private:
    QStackedWidget *m_stackedWidget;
    GameBoard *m_gameBoard;
    MainMenu *m_mainMenu;
};
#endif // MAINWINDOW_H
