#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include "petrinetmodel.h"

/**
 * @brief Main window of the Petri net viewer application
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// \brief Constructor
    /// \param[in] parent Parent widget
    explicit MainWindow(QWidget *parent = nullptr);

    /// \brief Get the QML view widget
    /// \return Pointer to the QQuickWidget
    QQuickWidget* quickWidget() const { return m_petriNetView; }

    /// \brief Set the Petri net model
    /// \param[in] model Pointer to the model
    void setPetriNetModel(PetriNetModel* model);

private slots:
    /// \brief Open a Petri net file
    void open();
    void updateSelectedNode(int index, bool isPlace);
    void updateLists();

private:
    /// \brief Setup the UI components
    void setupUi();

    /// \brief Create menus
    void createMenus();

private:
    QQuickWidget *m_petriNetView;      ///< QML view widget
    PetriNetModel *m_petriNetModel;    ///< Petri net model
    QListWidget *m_placesList;         ///< List of places
    QListWidget *m_transitionsList;    ///< List of transitions
    QListWidget *m_arcsList;           ///< List of arcs
};

#endif // MAINWINDOW_H