#include "mainwindow.h"
#include <QMenuBar>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QFileDialog>
#include <QScreen>
#include <QStyle>
#include <QGuiApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_petriNetModel(nullptr)
{
    setupUi();
    createMenus();

    // Set initial window size to 1024x768
    resize(1024, 768);

    // Optional: center the window on screen
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            QGuiApplication::primaryScreen()->availableGeometry()
        )
    );
}

void MainWindow::setPetriNetModel(PetriNetModel* model)
{
    m_petriNetModel = model;

    // Connect model signals
    if (m_petriNetModel) {
        connect(m_petriNetModel, &PetriNetModel::nodeSelected,
                this, &MainWindow::updateSelectedNode);
        connect(m_petriNetModel, &PetriNetModel::placePositionsChanged,
                this, &MainWindow::updateLists);
        connect(m_petriNetModel, &PetriNetModel::transitionPositionsChanged,
                this, &MainWindow::updateLists);
        connect(m_petriNetModel, &PetriNetModel::arcsChanged,
                this, &MainWindow::updateLists);

        // Update initial lists
        updateLists();
    }
}

void MainWindow::setupUi()
{
    // Central widget with QML view
    m_petriNetView = new QQuickWidget(this);
    m_petriNetView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_petriNetView->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_petriNetView->setClearColor(Qt::transparent);
    setCentralWidget(m_petriNetView);

    // Right dock widget for node information
    QDockWidget *infoDock = new QDockWidget(tr("Net Information"), this);
    infoDock->setAllowedAreas(Qt::RightDockWidgetArea);
    infoDock->setMinimumWidth(300);  // Largeur minimale

    // Create content widget for dock
    QWidget *dockContent = new QWidget(infoDock);
    QVBoxLayout *mainLayout = new QVBoxLayout(dockContent);

    // Places group
    QGroupBox *placesGroup = new QGroupBox(tr("Places"), dockContent);
    QVBoxLayout *placesLayout = new QVBoxLayout(placesGroup);
    m_placesList = new QListWidget(placesGroup);
    placesLayout->addWidget(m_placesList);
    mainLayout->addWidget(placesGroup);

    // Transitions group
    QGroupBox *transitionsGroup = new QGroupBox(tr("Transitions"), dockContent);
    QVBoxLayout *transitionsLayout = new QVBoxLayout(transitionsGroup);
    m_transitionsList = new QListWidget(transitionsGroup);
    transitionsLayout->addWidget(m_transitionsList);
    mainLayout->addWidget(transitionsGroup);

    // Arcs group
    QGroupBox *arcsGroup = new QGroupBox(tr("Arcs"), dockContent);
    QVBoxLayout *arcsLayout = new QVBoxLayout(arcsGroup);
    m_arcsList = new QListWidget(arcsGroup);
    arcsLayout->addWidget(m_arcsList);
    mainLayout->addWidget(arcsGroup);

    // Set dock content widget
    infoDock->setWidget(dockContent);
    addDockWidget(Qt::RightDockWidgetArea, infoDock);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Fichier"));
    fileMenu->addAction(tr("&Ouvrir"), this, &MainWindow::open, QKeySequence::Open);
    fileMenu->addAction(tr("&Quitter"), this, &QWidget::close, QKeySequence::Quit);
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Petri Net"), "",
        tr("Petri Net files (*.pn *.xml);;All files (*)"));

    if (!fileName.isEmpty()) {
        // TODO: Implement file loading
        // Currently does nothing
    }
}

void MainWindow::updateSelectedNode(int index, bool isPlace)
{
    if (index == -1) {
        return;
    }

    // Select corresponding item in appropriate list
    if (isPlace) {
        m_placesList->setCurrentRow(index);
    } else {
        m_transitionsList->setCurrentRow(index);
    }
}

void MainWindow::updateLists()
{
    if (!m_petriNetModel) return;

    // Update places list if needed
    if (m_placesList->count() != m_petriNetModel->placePositions().size()) {
        m_placesList->clear();
        for (int i = 0; i < m_petriNetModel->placePositions().size(); ++i) {
            m_placesList->addItem(tr("Place P%1 (tokens: %2)")
                .arg(i)
                .arg(0)); // TODO: Ajouter le nombre de tokens dans le modèle
        }
    }

    // Update transitions list if needed
    if (m_transitionsList->count() != m_petriNetModel->transitionPositions().size()) {
        m_transitionsList->clear();
        for (int i = 0; i < m_petriNetModel->transitionPositions().size(); ++i) {
            m_transitionsList->addItem(tr("Transition T%1").arg(i));
        }
    }

    // Update arcs list if needed
    if (m_arcsList->count() != m_petriNetModel->arcs().size()) {
        m_arcsList->clear();
        for (const Arc& arc : m_petriNetModel->arcs()) {
            QString source = arc.sourceIsPlace ? tr("P%1").arg(arc.sourceIndex)
                                             : tr("T%1").arg(arc.sourceIndex);
            QString target = arc.targetIsPlace ? tr("P%1").arg(arc.targetIndex)
                                             : tr("T%1").arg(arc.targetIndex);
            m_arcsList->addItem(tr("%1 → %2").arg(source, target));
        }
    }
}