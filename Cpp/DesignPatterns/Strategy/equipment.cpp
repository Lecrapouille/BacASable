#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

class Equipment;

// ============================================================================
// DiagnosticStrategy
// ============================================================================
class DiagnosticStrategy
{
public:
    virtual ~DiagnosticStrategy() = default;
    /**
     * @brief Perform diagnostic on any equipment
     * @param[in] equipment Equipment to diagnose
     * @return true if diagnostic passed, false otherwise
     */
    virtual bool performDiagnostic(const Equipment& equipment) const = 0;
};

// ============================================================================
// Equipment
// ============================================================================
class Equipment
{
public:
    Equipment(std::string serialNumber, std::string manufacturer, 
              std::unique_ptr<DiagnosticStrategy> diagnosticStrategy)
        : m_serialNumber(serialNumber), m_manufacturer(manufacturer),
          m_diagnosticStrategy(std::move(diagnosticStrategy))
    {}

    virtual ~Equipment() = default;
    virtual std::string getType() const = 0;
    
    /**
     * @brief Check if the equipment is operational
     * @return true if the equipment passes its diagnostic
     * @throw std::runtime_error if no diagnostic strategy is set
     */
    bool isOperational() const
    {
        if (!m_diagnosticStrategy) {
            throw std::runtime_error("Diagnostic strategy not set");
        }
        return m_diagnosticStrategy->performDiagnostic(*this);
    }

    // Getters communs
    std::string getSerialNumber() const { return m_serialNumber; }
    std::string getManufacturer() const { return m_manufacturer; }
    double getPowerConsumption() const { return m_powerConsumption; }

protected:
    double m_powerConsumption = 0.0;

private:
    std::string m_serialNumber;
    std::string m_manufacturer;
    std::unique_ptr<DiagnosticStrategy> m_diagnosticStrategy;
};

// ============================================================================
// Motor
// ============================================================================
class Motor : public Equipment
{
public:
    Motor(std::string serialNumber, std::string manufacturer,
          std::unique_ptr<DiagnosticStrategy> diagnosticStrategy)
        : Equipment(serialNumber, manufacturer, std::move(diagnosticStrategy))
    {
        m_powerConsumption = 200.0;
    }

    std::string getType() const override { return "Motor"; }
};

// ============================================================================
// Motor Diagnostic
// ============================================================================ 
class MotorDiagnostic : public DiagnosticStrategy
{
public:
    bool performDiagnostic(const Equipment& equipment) const override
    {
        // Verify if equipment is a Motor
        if (const auto* motor = dynamic_cast<const Motor*>(&equipment)) {
            // Perform motor specific diagnostic
            bool result = motor->getPowerConsumption() < 1000.0;

            std::cout << "Motor " << motor->getSerialNumber() << " diagnostic: "
                      << motor->getPowerConsumption() << " < 1000 ?"
                      << (result ? "Functionnal" : "Not functional")
                      << std::endl;
            return result;
        }
        return false;
    }
};

// ============================================================================
// WashingMachine
// ============================================================================
class WashingMachine : public Equipment
{
public:
    WashingMachine(std::string serialNumber, std::string manufacturer,
                   std::unique_ptr<DiagnosticStrategy> diagnosticStrategy,
                   std::unique_ptr<Motor> motor)
        : Equipment(serialNumber, manufacturer, std::move(diagnosticStrategy))
        , m_motor(std::move(motor))
    {
        m_powerConsumption = 2000.0;
    }

    std::string getType() const override { return "WashingMachine"; }
    const Motor& getMotor() const { return *m_motor; }

private:
    std::unique_ptr<Motor> m_motor;
};

// ============================================================================
// Motor Diagnostic
// ============================================================================ 
class WashingMachineDiagnostic : public DiagnosticStrategy
{
public:
    bool performDiagnostic(const Equipment& equipment) const override
    {
        if (const auto* washingMachine = dynamic_cast<const WashingMachine*>(&equipment)) {
            // Diagnostic du lave-linge lui-même
            bool mainDiagnostic = washingMachine->getPowerConsumption() < 2200.0;
            std::cout << "WashingMachine " << washingMachine->getSerialNumber() << " diagnostic: "
                      << washingMachine->getPowerConsumption() << " < 2200 ?"
                      << (mainDiagnostic ? "Functionnal" : "Not functional")
                      << std::endl;

            // Diagnostic du moteur
            bool motorDiagnostic = washingMachine->getMotor().isOperational();
            
            std::cout << "WashingMachine " << washingMachine->getSerialNumber() << " diagnostic: "
                      << (mainDiagnostic && motorDiagnostic ? "Functionnal" : "Not functional")
                      << std::endl;
            return mainDiagnostic && motorDiagnostic;
        }
        return false;
    }
};

// ============================================================================
// NetworkModule
// ============================================================================
class NetworkModule : public Equipment
{
public:
    NetworkModule(std::string serialNumber, std::string manufacturer,
                  std::unique_ptr<DiagnosticStrategy> diagnosticStrategy)
        : Equipment(serialNumber, manufacturer, std::move(diagnosticStrategy))
    {
        m_powerConsumption = 30.0;
    }

    std::string getType() const override { return "NetworkModule"; }

    void connect(const std::string& ip)
    {
        m_ipAddress = ip;
        m_isConnected = true;
    }
    
    void disconnect() {
        m_isConnected = false;
    }
    
    bool hasConnection() const { return m_isConnected; }

private:
    std::string m_ipAddress;
    bool m_isConnected = false;
};

// ============================================================================
// NetworkModule Diagnostic
// ============================================================================ 
class NetworkModuleDiagnostic: public DiagnosticStrategy
{
public:
    bool performDiagnostic(const Equipment& equipment) const override
    {
        if (const auto* networkModule = dynamic_cast<const NetworkModule*>(&equipment)) {
            std::cout << "NetworkModule " << networkModule->getSerialNumber() << " diagnostic: "
                      << (networkModule->hasConnection() ? "Yes" : "No")
                      << std::endl;
            return networkModule->hasConnection();
        }
        return false;
    }
};

// ============================================================================
// Refrigerator
// ============================================================================
class Refrigerator : public Equipment
{
public:
    Refrigerator(std::string serialNumber, std::string manufacturer,
                 std::unique_ptr<DiagnosticStrategy> diagnosticStrategy,
                 std::unique_ptr<NetworkModule> networkModule)
        : Equipment(serialNumber, manufacturer, std::move(diagnosticStrategy))
        , m_networkModule(std::move(networkModule))
    {
        m_powerConsumption = 1000.0;
    }

    std::string getType() const override { return "Refrigerator"; }
    const NetworkModule& getNetworkModule() const { return *m_networkModule; }

private:
    std::unique_ptr<NetworkModule> m_networkModule;
    double m_temperature = 0.0;
};

// ============================================================================
// Refrigerator Diagnostic
// ============================================================================ 
class RefrigeratorDiagnostic : public DiagnosticStrategy
{
public:
    bool performDiagnostic(const Equipment& equipment) const override
    {
        if (const auto* refrigerator = dynamic_cast<const Refrigerator*>(&equipment)) {
            // Diagnostic du réfrigérateur
            bool mainDiagnostic = true; // Logique spécifique ici
            std::cout << "Refrigerator " << refrigerator->getSerialNumber() << " diagnostic: "
                      << refrigerator->getPowerConsumption() << " < 1000 ?"
                      << (refrigerator->getPowerConsumption() < 1000.0 ? "Yes" : "No")
                      << std::endl;
            
            // Diagnostic du module réseau
            bool networkDiagnostic = refrigerator->getNetworkModule().isOperational();

            return mainDiagnostic && networkDiagnostic;
        }
        return false;
    }
};

class EquipmentManager
{
public:

    size_t getEquipmentCount() const { return m_equipments.size(); }

    /**
     * @brief Add a new equipment to the manager
     * @param[in] eq Equipment to add to the manager
     * @throw std::invalid_argument if equipment is null or already exists
     */
    void addEquipment(std::unique_ptr<Equipment> eq)
    {
        // Vérifier si l'équipement existe déjà (basé sur le numéro de série)
        auto it = std::find_if(m_equipments.begin(), m_equipments.end(),
            [&eq](const auto& existing) {
                return existing->getSerialNumber() == eq->getSerialNumber();
            });

        if (it != m_equipments.end()) {
            throw std::invalid_argument("Equipment with this serial number already exists");
        }

        std::cout << "Adding equipment " << eq->getType() << " " << eq->getSerialNumber() << std::endl;
        m_equipments.push_back(std::move(eq));
    }
    
    void detectProblem()
    {
        std::cout << "Detecting problems..." << std::endl;
        bool problemDetected = false;

        for (const auto& eq : m_equipments) {
            std::cout << "Checking equipment " << eq->getType() << "..." << std::endl;
            if (!eq->isOperational()) {
                std::cout << "Problem detected on " << eq->getType() << std::endl;
                problemDetected = true;
            }
        }
        if (!problemDetected) {
            std::cout << "No problem detected" << std::endl;
        }
    }

    void removeEquipment(const std::string& serial)
    {
        std::cout << "Removing equipment " << serial << std::endl;
        auto it = std::find_if(m_equipments.begin(), m_equipments.end(),
            [&serial](const auto& eq) {
                return eq->getSerialNumber() == serial;
            });
        if (it != m_equipments.end()) {
            std::cout << "Equipment " << serial << " removed" << std::endl;
            m_equipments.erase(it);
        } else {
            std::cout << "Equipment " << serial << " not found" << std::endl;
        }
    }

    /**
     * @brief Remove all failed equipment
     */
    void removeFailedEquipment()
    {
        std::cout << "Removing failed equipment..." << std::endl;
        auto it = std::remove_if(m_equipments.begin(), m_equipments.end(),
            [](const auto& eq) { return !eq->isOperational(); });
        if (it != m_equipments.end()) {
            std::cout << "Failed equipment " << (*it)->getSerialNumber() << " removed" << std::endl;
            m_equipments.erase(it, m_equipments.end());
        } else {
            std::cout << "No failed equipment found" << std::endl;
        }
    }

    void showAllEquipments() const
    {
        std::cout << "Showing all equipments..." << std::endl;
        for (const auto& eq : m_equipments) {
            std::cout << "Equipment " << eq->getType() << " " << eq->getSerialNumber() << std::endl;
        }
    }

    /**
     * @brief Get const reference to the equipment vector
     * @return const reference to equipment vector
     */
    const std::vector<std::unique_ptr<Equipment>>& getEquipments() const
    {
        return m_equipments;
    }

private:
    std::vector<std::unique_ptr<Equipment>> m_equipments;
};
