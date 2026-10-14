#pragma once

#include <QWidget>
class QPushButton;

class MainMenu : public QWidget
{
    Q_OBJECT
public:
    explicit MainMenu(QWidget *parent = nullptr);

signals:
    void singlePlayerClicked();
    void twoPlayerClicked();
    void loadGameClicked();
    void exitClicked();
private:
    QPushButton* m_singlePlayerBtn;
    QPushButton* m_twoPlayerBtn;
    QPushButton* m_loadGameBtn;
    QPushButton* m_exitBtn;
};


