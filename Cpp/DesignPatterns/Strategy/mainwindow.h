#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QPushButton>
#include "foo.cpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    /**
     * @brief Add a new washing machine to the equipment manager
     */
    void addWashingMachine();

    /**
     * @brief Add a new refrigerator to the equipment manager
     */
    void addRefrigerator();

    /**
     * @brief Remove equipment by serial number
     */
    void removeEquipment();

    /**
     * @brief Remove all failed equipment
     */
    void removeFailedEquipment();

    /**
     * @brief Update the equipment table display
     */
    void updateEquipmentTable();

private:
    /**
     * @brief Setup the UI elements
     */
    void setupUi();

    EquipmentManager m_manager;
    QTableWidget* m_equipmentTable;
    QPushButton* m_addWashingMachineBtn;
    QPushButton* m_addRefrigeratorBtn;
    QPushButton* m_removeEquipmentBtn;
    QPushButton* m_removeFailedBtn;
}; 