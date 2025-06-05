#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *p_parent = nullptr);
    ~MainWindow();

private slots:
    // Slots for the buttons of the vertical panel
    void onAction1();
    void onAction2();
    void onAction3();
    void onSettings();

    // Slots for the menu actions
    void onOpen();
    void onSave();
    void toggleFullScreen();

private:
    void createDisplayDock();
    void connectButtons();

private:
    Ui::MainWindow *ui;
    QDockWidget *m_displayDock;
    QTextEdit *m_displayArea;
};