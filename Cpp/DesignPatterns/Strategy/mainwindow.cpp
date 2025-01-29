#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    updateEquipmentTable();
}

void MainWindow::setupUi()
{
    // Create central widget and layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Create table
    m_equipmentTable = new QTableWidget(this);
    m_equipmentTable->setColumnCount(4);
    m_equipmentTable->setHorizontalHeaderLabels(
        {"Serial Number", "Manufacturer", "Power Consumption", "Status"});
    m_equipmentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Create buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_addWashingMachineBtn = new QPushButton("Add Washing Machine", this);
    m_addRefrigeratorBtn = new QPushButton("Add Refrigerator", this);
    m_removeEquipmentBtn = new QPushButton("Remove Equipment", this);
    m_removeFailedBtn = new QPushButton("Remove Failed", this);

    buttonLayout->addWidget(m_addWashingMachineBtn);
    buttonLayout->addWidget(m_addRefrigeratorBtn);
    buttonLayout->addWidget(m_removeEquipmentBtn);
    buttonLayout->addWidget(m_removeFailedBtn);

    // Connect signals
    connect(m_addWashingMachineBtn, &QPushButton::clicked, this, &MainWindow::addWashingMachine);
    connect(m_addRefrigeratorBtn, &QPushButton::clicked, this, &MainWindow::addRefrigerator);
    connect(m_removeEquipmentBtn, &QPushButton::clicked, this, &MainWindow::removeEquipment);
    connect(m_removeFailedBtn, &QPushButton::clicked, this, &MainWindow::removeFailedEquipment);

    // Add widgets to main layout
    mainLayout->addWidget(m_equipmentTable);
    mainLayout->addLayout(buttonLayout);

    setCentralWidget(centralWidget);
    resize(800, 600);
}

void MainWindow::addWashingMachine()
{
    bool ok;
    QString serial = QInputDialog::getText(this, "Add Washing Machine",
                                         "Enter serial number:", QLineEdit::Normal,
                                         "W", &ok);
    if (ok && !serial.isEmpty()) {
        try {
            auto washingMachine = std::make_unique<WashingMachine>(
                serial.toStdString(), "WashCo",
                std::make_unique<WashingMachineDiagnostic>(),
                std::make_unique<Motor>(
                    "M" + serial.toStdString(), "MotorCo",
                    std::make_unique<MotorDiagnostic>()));

            m_manager.addEquipment(std::move(washingMachine));
            updateEquipmentTable();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    }
}

void MainWindow::addRefrigerator()
{
    bool ok;
    QString serial = QInputDialog::getText(this, "Add Refrigerator",
                                         "Enter serial number:", QLineEdit::Normal,
                                         "R", &ok);
    if (ok && !serial.isEmpty()) {
        try {
            auto refrigerator = std::make_unique<Refrigerator>(
                serial.toStdString(), "RefrigeratorCo",
                std::make_unique<RefrigeratorDiagnostic>(),
                std::make_unique<NetworkModule>(
                    "N" + serial.toStdString(), "NetworkCo",
                    std::make_unique<NetworkModuleDiagnostic>()));

            m_manager.addEquipment(std::move(refrigerator));
            updateEquipmentTable();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", e.what());
        }
    }
}

void MainWindow::removeEquipment()
{
    bool ok;
    QString serial = QInputDialog::getText(this, "Remove Equipment",
                                         "Enter serial number to remove:", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !serial.isEmpty()) {
        m_manager.removeEquipment(serial.toStdString());
        updateEquipmentTable();
    }
}

void MainWindow::removeFailedEquipment()
{
    m_manager.removeFailedEquipment();
    updateEquipmentTable();
}

void MainWindow::updateEquipmentTable()
{
    m_equipmentTable->setRowCount(0);
    
    const auto& equipments = m_manager.getEquipments();
    m_equipmentTable->setRowCount(equipments.size());
    
    int row = 0;
    for (const auto& equipment : equipments) {
        m_equipmentTable->setItem(row, 0, 
            new QTableWidgetItem(QString::fromStdString(equipment->getSerialNumber())));
        m_equipmentTable->setItem(row, 1, 
            new QTableWidgetItem(QString::fromStdString(equipment->getManufacturer())));
        m_equipmentTable->setItem(row, 2, 
            new QTableWidgetItem(QString::number(equipment->getPowerConsumption())));
        
        bool isOperational = false;
        try {
            isOperational = equipment->isOperational();
        } catch (const std::exception&) {
            isOperational = false;
        }
        
        m_equipmentTable->setItem(row, 3, 
            new QTableWidgetItem(isOperational ? "Functional" : "Failed"));
        
        row++;
    }
} 