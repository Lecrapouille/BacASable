#include <algorithm>
#include <any>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ============================================================================
// Interface base Observer based on the Java implementation.
// ============================================================================
class IObserver
{
public:

    virtual ~IObserver() = default;

    // Push model: receive the data directly
    virtual void update(const std::any& data) = 0;

    // Pull model: can query the observable
    virtual void update() = 0;
};

// ============================================================================
// Interface base Observable based on the Java implementation.
// ============================================================================
class IObservable
{
public:

    virtual ~IObservable() = default;

    virtual void attach(std::shared_ptr<IObserver> observer) = 0;
    virtual void detach(std::shared_ptr<IObserver> observer) = 0;
    virtual void notify() = 0;
    virtual void notify(const std::any& data) = 0;
};

// ============================================================================
// Implementation of Observable based on the Java implementation.
// ============================================================================
template <typename DataType = std::any>
class Observable: public IObservable
{
public:

    Observable() = default;
    Observable(const DataType& initial_state) : current_state_(initial_state) {}

    // ------------------------------------------------------------------------
    // Attach an observer
    // ------------------------------------------------------------------------
    void attach(std::shared_ptr<IObserver> observer) override
    {
        if (observer)
        {
            observers_.emplace_back(observer);
        }
    }

    // ------------------------------------------------------------------------
    // Detach an observer
    // ------------------------------------------------------------------------
    void detach(std::shared_ptr<IObserver> observer) override
    {
        observers_.erase(
            std::remove_if(observers_.begin(),
                           observers_.end(),
                           [&observer](const std::weak_ptr<IObserver>& wp)
                           {
                               auto sp = wp.lock();
                               return !sp || sp == observer;
                           }),
            observers_.end());
    }

    // ------------------------------------------------------------------------
    // Notification Pull - the observers query the state
    // ------------------------------------------------------------------------
    void notify() override
    {
        // Traverse the vector backwards to remove expired observers
        size_t valid_count = 0;
        for (size_t i = observers_.size(); i > 0; --i)
        {
            size_t current_pos = i - 1;
            if (auto obs = observers_[current_pos].lock())
            {
                // Valid observer, notify it
                obs->update();
                // Move it to the end of valid elements if not already there
                if (current_pos != valid_count)
                {
                    std::swap(observers_[current_pos], observers_[valid_count]);
                }
                ++valid_count;
            }
        }
        // Resize the vector to remove expired observers
        observers_.resize(valid_count);
    }

    // ------------------------------------------------------------------------
    // Notification Push - send the data to the observers
    // ------------------------------------------------------------------------
    void notify(const std::any& data) override
    {
        // Traverse the vector backwards to remove expired observers
        size_t valid_count = 0;
        for (size_t i = observers_.size(); i > 0; --i)
        {
            size_t current_pos = i - 1;
            if (auto obs = observers_[current_pos].lock())
            {
                // Valid observer, notify it
                obs->update(data);
                // Move it to the end of valid elements if not already there
                if (current_pos != valid_count)
                {
                    std::swap(observers_[current_pos], observers_[valid_count]);
                }
                ++valid_count;
            }
        }
        // Resize the vector to remove expired observers
        observers_.resize(valid_count);
    }

    // ------------------------------------------------------------------------
    // Notification Push typed
    // ------------------------------------------------------------------------
    void notify(const DataType& data)
    {
        current_state_ = data;
        notify(std::any(data));
    }

    // ------------------------------------------------------------------------
    // Getter for the pull model
    // ------------------------------------------------------------------------
    const DataType& get_state() const
    {
        return current_state_;
    }

    // ------------------------------------------------------------------------
    // Setter that automatically triggers a notification
    // ------------------------------------------------------------------------
    void set_state(const DataType& new_state)
    {
        current_state_ = new_state;
        notify(std::any(new_state));
    }

    // ------------------------------------------------------------------------
    // Get the number of observers
    // ------------------------------------------------------------------------
    size_t observer_count() const
    {
        return std::count_if(observers_.begin(),
                             observers_.end(),
                             [](const std::weak_ptr<IObserver>& wp)
                             { return !wp.expired(); });
    }


private:

    std::vector<std::weak_ptr<IObserver>> observers_;
    DataType current_state_;
};

// ============================================================================
// Concrete Observer with support Push/Pull
// ============================================================================
template <typename DataType>
class Observer: public IObserver
{
private:

    std::string name_;
    std::weak_ptr<Observable<DataType>> observable_;

    // Callbacks optional
    std::function<void(const DataType&)> on_push_update_;
    std::function<void(const DataType&)> on_pull_update_;

public:

    Observer(const std::string& name) : name_(name) {}

    // ------------------------------------------------------------------------
    // Set the observable to observe (for the Pull mode)
    // ------------------------------------------------------------------------
    void set_observable(std::shared_ptr<Observable<DataType>> obs)
    {
        observable_ = obs;
    }

    // ------------------------------------------------------------------------
    // Set the callbacks
    // ------------------------------------------------------------------------
    void set_push_callback(std::function<void(const DataType&)> callback)
    {
        on_push_update_ = callback;
    }

    // ------------------------------------------------------------------------
    // Set the callbacks
    // ------------------------------------------------------------------------
    void set_pull_callback(std::function<void(const DataType&)> callback)
    {
        on_pull_update_ = callback;
    }

    // ------------------------------------------------------------------------
    // Push model: receive the data
    // ------------------------------------------------------------------------
    void update(const std::any& data) override
    {
        try
        {
            const DataType& typed_data = std::any_cast<const DataType&>(data);
            std::cout << name_ << " received push update: " << typed_data
                      << std::endl;

            if (on_push_update_)
            {
                on_push_update_(typed_data);
            }
            
            if (on_pull_update_)
            {
                on_pull_update_(typed_data);
            }
        }
        catch (const std::bad_any_cast& e)
        {
            std::cout << name_ << " failed to cast received data" << std::endl;
        }
    }

    // ------------------------------------------------------------------------
    // Pull model: query the observable
    // ------------------------------------------------------------------------
    void update() override
    {
        if (auto obs = observable_.lock())
        {
            const DataType& state = obs->get_state();
            std::cout << name_ << " pulled update: " << state << std::endl;

            if (on_pull_update_)
            {
                on_pull_update_(state);
            }
        }
        else
        {
            std::cout << name_ << " cannot pull - observable unavailable"
                      << std::endl;
        }
    }

    const std::string& get_name() const
    {
        return name_;
    }
};

// ============================================================================
// Example of usage
// ============================================================================
class TemperatureSensor: public Observable<double>
{
private:

    std::string location_;

public:

    TemperatureSensor(const std::string& location, double initial_temp = 20.0)
        : Observable<double>(initial_temp), location_(location)
    {
    }

    void simulate_temperature_change(double new_temp)
    {
        std::cout << "\n=== " << location_ << " temperature changed to "
                  << new_temp << "°C ===" << std::endl;
        set_state(new_temp);
    }

    const std::string& get_location() const
    {
        return location_;
    }
};

// ============================================================================
// Factory to easily create observers
// ============================================================================
template <typename DataType>
class ObserverFactory
{
public:

    static std::shared_ptr<Observer<DataType>>
    create_observer(const std::string& name,
                    std::shared_ptr<Observable<DataType>> observable = nullptr)
    {
        auto observer = std::make_shared<Observer<DataType>>(name);
        if (observable)
        {
            observer->set_observable(observable);
            observable->attach(observer);
        }
        return observer;
    }
};

// ============================================================================
// Demonstration
// ============================================================================
int main()
{
    // Creation of the temperature sensor
    auto temp_sensor = std::make_shared<TemperatureSensor>("Salon", 22.5);

    // Creation of the observers with the factory
    auto display =
        ObserverFactory<double>::create_observer("Display", temp_sensor);
    auto thermostat =
        ObserverFactory<double>::create_observer("Thermostat", temp_sensor);
    auto logger =
        ObserverFactory<double>::create_observer("Logger", temp_sensor);

    // Configuration of the custom callbacks
    display->set_push_callback(
        [](const double& temp)
        { std::cout << "  → Display shows: " << temp << "°C" << std::endl; });

    thermostat->set_pull_callback(
        [](const double& temp)
        {
            if (temp > 25.0)
            {
                std::cout << "  → Thermostat: Activating cooling!" << std::endl;
            }
            else if (temp < 18.0)
            {
                std::cout << "  → Thermostat: Activating heating!" << std::endl;
            }
        });

    logger->set_push_callback(
        [](const double& temp)
        {
            std::cout << "  → Logger: Recorded " << temp << "°C at "
                      << "timestamp" << std::endl;
        });

    std::cout << "Active observers: " << temp_sensor->observer_count()
              << std::endl;

    // Test of the Push notifications
    temp_sensor->simulate_temperature_change(26.5);
    temp_sensor->simulate_temperature_change(15.0);
    temp_sensor->simulate_temperature_change(21.0);
    temp_sensor->simulate_temperature_change(10.0);

    // Test of the manual Pull notification
    std::cout << "\n=== Manual Pull Update ===" << std::endl;
    temp_sensor->notify();

    // Test of the detachment
    std::cout << "\n=== Detaching Display ===" << std::endl;
    temp_sensor->detach(display);
    std::cout << "Active observers: " << temp_sensor->observer_count()
              << std::endl;

    temp_sensor->simulate_temperature_change(30.0);

    return 0;
}