#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QDockWidget>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(QWidget *p_parent)
    : QMainWindow(p_parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Qt6 Application - Display with Buttons");
    resize(1000, 700);

    createCentralDrawingArea();
    createDisplayDock();
    createButtonPanelDock();
    connectButtons();

    // Configure dock constraints - docks can only be placed on left, top, bottom
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // Organize docks with specific sizes
    organizeDocks();

    // Configure the status bar
    statusBar()->showMessage("Application ready", 2000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createButtonPanelDock()
{
    // Create a dock for the button panel
    m_buttonPanelDock = new QDockWidget("Controls", this);
    m_buttonPanelDock->setAllowedAreas(Qt::RightDockWidgetArea); // Only right side
    
    // Extract the button panel from the UI and put it in the dock
    QWidget *buttonPanel = ui->buttonPanel;
    buttonPanel->setParent(nullptr); // Remove from current parent
    
    // Configure the dock
    m_buttonPanelDock->setWidget(buttonPanel);
    m_buttonPanelDock->setFeatures(QDockWidget::NoDockWidgetFeatures); // Fixed dock
    
    // Add the dock to the right area
    addDockWidget(Qt::RightDockWidgetArea, m_buttonPanelDock);
    
    // Split so that button panel is to the RIGHT of display dock
    splitDockWidget(m_displayDock, m_buttonPanelDock, Qt::Horizontal);
}

void MainWindow::createDisplayDock()
{
    // Create the QDockWidget
    m_displayDock = new QDockWidget("Main display zone", this);
    // Allow ALL areas including right (so it can be placed left of button panel)
    m_displayDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

    // Container widget for the display
    QWidget *displayWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(displayWidget);

    // Create a QTextEdit as an example of display
    m_displayArea = new QTextEdit();
    m_displayArea->setPlainText("Welcome to the Qt6 application!\n\n"
                               "This area can contain :\n"
                               "• A custom display widget\n"
                               "• Graphs\n"
                               "• Text\n"
                               "• Images\n"
                               "• Any other content\n\n"
                               "Use the buttons on the right to interact with the application.");

    // Information label
    QLabel *infoLabel = new QLabel("Main display zone - QDockWidget");
    infoLabel->setStyleSheet("QLabel { font-weight: bold; color: #333; padding: 5px; }");

    layout->addWidget(infoLabel);
    layout->addWidget(m_displayArea);

    // Configure the dock - make it fully detachable
    m_displayDock->setWidget(displayWidget);
    m_displayDock->setFeatures(QDockWidget::DockWidgetMovable |
                              QDockWidget::DockWidgetFloatable |
                              QDockWidget::DockWidgetClosable);

    // Add the dock to the right area FIRST
    addDockWidget(Qt::RightDockWidgetArea, m_displayDock);
}

void MainWindow::connectButtons()
{
    // Connect the buttons of the vertical panel
    connect(ui->btnAction1, &QPushButton::clicked, this, &MainWindow::onAction1);
    connect(ui->btnAction2, &QPushButton::clicked, this, &MainWindow::onAction2);
    connect(ui->btnAction3, &QPushButton::clicked, this, &MainWindow::onAction3);
    connect(ui->btnSettings, &QPushButton::clicked, this, &MainWindow::onSettings);

    // Connect the menu actions
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpen);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSave);
    connect(ui->actionFullscreen, &QAction::triggered, this, &MainWindow::toggleFullScreen);
}

void MainWindow::onAction1()
{
    m_displayArea->append("\n[Action 1] Button 1 clicked !");
    statusBar()->showMessage("Action 1 executed", 2000);
}

void MainWindow::onAction2()
{
    m_displayArea->append("\n[Action 2] Button 2 clicked !");
    statusBar()->showMessage("Action 2 executed", 2000);
}

void MainWindow::onAction3()
{
    m_displayArea->append("\n[Action 3] Button 3 clicked !");
    statusBar()->showMessage("Action 3 executed", 2000);
}

void MainWindow::onSettings()
{
    QMessageBox::information(this, "Settings",
                           "Here you could open a settings window.\n\n"
                           "This dialog is just an example.");
    m_displayArea->append("\n[Settings] Settings window opened");
}

void MainWindow::onOpen()
{
    m_displayArea->append("\n[Menu] Open - Functionality to implement");
    statusBar()->showMessage("Open selected", 2000);
}

void MainWindow::onSave()
{
    m_displayArea->append("\n[Menu] Save - Functionality to implement");
    statusBar()->showMessage("Save selected", 2000);
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen())
    {
        showNormal();
        statusBar()->showMessage("Windowed mode", 2000);
    }
    else
    {
        showFullScreen();
        statusBar()->showMessage("Fullscreen mode", 2000);
    }
}

void MainWindow::createCentralDrawingArea()
{
    // Create the drawing widget with grid and set it as central widget
    m_drawingArea = new DrawingWidget();
    setCentralWidget(m_drawingArea);
    
    // Hide the placeholder from UI file
    ui->centralwidget->hide();
}

void MainWindow::organizeDocks()
{
    // Set specific sizes for docks
    QList<QDockWidget*> docks = {m_displayDock};
    QList<int> sizes = {300}; // Width of display dock
    
    // Resize the display dock to have a reasonable width
    resizeDocks(docks, sizes, Qt::Horizontal);
    
    // Make sure the display dock is positioned nicely
    if (m_displayDock) {
        // Start with a reasonable size
        m_displayDock->resize(300, m_displayDock->height());
    }
}