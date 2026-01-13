// ****************************************************************************
// @file EndPoint-v3.cpp
// @brief Demonstration of the Strategy pattern combined with a type-based
//        registry for automatic FakeEndPoint creation.
// ****************************************************************************

#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <functional>

// Forward declaration
class FakeEndPoint;

// ****************************************************************************
// EndPoint class
// @brief Base class representing a real endpoint that can read/write data.
//        The endpoint is configured with a Mapping that provides pointers
//        to input/output buffers managed by a FakeEndPoint.
// ****************************************************************************
class EndPoint
{
public:

    // ------------------------------------------------------------------------
    // @brief Mapping structure holding pointers to input/output buffers
    // ------------------------------------------------------------------------
    struct Mapping
    {
        std::vector<uint8_t>* input;
        std::vector<uint8_t>* output;
    };

    // ------------------------------------------------------------------------
    // @brief Constructor
    // @param name The name of the endpoint
    // ------------------------------------------------------------------------
    EndPoint(const std::string& name)
      : m_name(name)
    {}

    // ------------------------------------------------------------------------
    // @brief Virtual destructor for proper polymorphic destruction
    // ------------------------------------------------------------------------
    virtual ~EndPoint() = default;

    // ------------------------------------------------------------------------
    // @brief Configure the endpoint with input/output buffers
    // @param mapping The mapping containing buffer pointers
    // ------------------------------------------------------------------------
    void configure(Mapping& mapping)
    {
        std::cout << "Configuring " << m_name << std::endl;
        m_mapping = mapping;
    }

    // ------------------------------------------------------------------------
    // @brief Read data from the input buffer
    // ------------------------------------------------------------------------
    void read()
    {
        std::cout << "Reading from " << m_name << " : ";
        for (size_t i = 0; i < m_mapping.input->size(); ++i)
        {
            std::cout << int((*m_mapping.input)[i]);
            if (i < m_mapping.input->size() - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }

    // ------------------------------------------------------------------------
    // @brief Write data to the output buffer
    // ------------------------------------------------------------------------
    void write()
    {
        std::cout << "Writing to " << m_name << " : ";
        for (size_t i = 0; i < m_mapping.output->size(); ++i)
        {
            std::cout << int((*m_mapping.output)[i]);
            if (i < m_mapping.output->size() - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }

    // ------------------------------------------------------------------------
    // @brief Get the endpoint name
    // @return The name of the endpoint
    // ------------------------------------------------------------------------
    std::string getName() const
    {
        return m_name;
    }

private:
    std::string m_name;
    Mapping m_mapping;
};

// ****************************************************************************
// FakeEndPoint base class
// @brief Base class for fake endpoints that simulate data preparation for
//        real endpoints. Uses the Strategy pattern for read/write preparation.
// ****************************************************************************
class FakeEndPoint
{
public:

    // ************************************************************************
    // PrepareStrategy class
    // @brief Strategy pattern interface for preparing read/write operations.
    //        Feeds the real endpoint with the data to be sent/received.
    // ************************************************************************
    class PrepareStrategy
    {
    public:

        // ---------------------------------------------------------------------
        // @brief Enum defining the available strategy types
        // ---------------------------------------------------------------------
        enum class Type
        {
            Stub,       // Fixed values for testing
            Publisher   // Simulated dynamic values
        };

        // ---------------------------------------------------------------------
        // @brief Constructor
        // @param type The strategy type
        // ---------------------------------------------------------------------
        PrepareStrategy(Type type): m_type(type){}

        // ---------------------------------------------------------------------
        // @brief Virtual destructor
        // ---------------------------------------------------------------------
        virtual ~PrepareStrategy() = default;

        // ---------------------------------------------------------------------
        // @brief Get the strategy type
        // @return The type of this strategy
        // ---------------------------------------------------------------------
        Type type() const { return m_type; }

        // ---------------------------------------------------------------------
        // @brief Pure virtual method for preparing read operations
        // @param input The input buffer to fill
        // @param name The name of the fake endpoint (for logging)
        // ---------------------------------------------------------------------
        virtual void prepareRead(std::vector<uint8_t>& input, const std::string& name) = 0;

        // ---------------------------------------------------------------------
        // @brief Pure virtual method for preparing write operations
        // @param output The output buffer to fill
        // @param name The name of the fake endpoint (for logging)
        // ---------------------------------------------------------------------
        virtual void prepareWrite(std::vector<uint8_t>& output, const std::string& name) = 0;

    private:
        PrepareStrategy::Type m_type;
    };

    // ------------------------------------------------------------------------
    // @brief Constructor
    // @param endpoint Reference to the real endpoint to configure
    // @param strategy The preparation strategy to use
    // ------------------------------------------------------------------------
    FakeEndPoint(EndPoint& endpoint, std::unique_ptr<PrepareStrategy> strategy)
    : m_endpoint(endpoint)
    , m_strategy(std::move(strategy))
    {
        m_name = "Fake " + endpoint.getName();
    }

    // ------------------------------------------------------------------------
    // @brief Virtual destructor
    // ------------------------------------------------------------------------
    virtual ~FakeEndPoint()
    {}

    // ------------------------------------------------------------------------
    // @brief Prepare input data and trigger endpoint read
    // ------------------------------------------------------------------------
    void read()
    {
        m_strategy->prepareRead(m_input, m_name);
        m_endpoint.read();
    }

    // ------------------------------------------------------------------------
    // @brief Prepare output data and trigger endpoint write
    // ------------------------------------------------------------------------
    void write()
    {
        m_strategy->prepareWrite(m_output, m_name);
        m_endpoint.write();
    }

    // ------------------------------------------------------------------------
    // @brief Get the fake endpoint name
    // @return The name of the fake endpoint
    // ------------------------------------------------------------------------
    std::string getName() const
    {
        return m_name;
    }

protected:

    // ------------------------------------------------------------------------
    // @brief Initialize buffers and configure the real endpoint
    //        Must be called by concrete classes after buffer initialization
    // ------------------------------------------------------------------------
    void configureEndpoint()
    {
        EndPoint::Mapping mapping = getMapping();
        m_endpoint.configure(mapping);
    }

private:
    // ------------------------------------------------------------------------
    // @brief Create a mapping pointing to internal buffers
    // @return The mapping structure
    // ------------------------------------------------------------------------
    EndPoint::Mapping getMapping()
    {
        return EndPoint::Mapping{&m_input, &m_output};
    }

protected:
    EndPoint& m_endpoint;                       // Reference to the real endpoint
    std::string m_name;                         // Name of the fake endpoint
    std::vector<uint8_t> m_input;               // Internal input buffer
    std::vector<uint8_t> m_output;              // Internal output buffer
    std::unique_ptr<PrepareStrategy> m_strategy; // The preparation strategy
};

// ****************************************************************************
// EndPoint1 class
// @brief Concrete EndPoint type 1. Used for type-based FakeEndPoint lookup.
// ****************************************************************************
class EndPoint1 : public EndPoint
{
public:
    EndPoint1(const std::string& name) : EndPoint(name) {}
};

// ****************************************************************************
// FakeEndPoint1 class
// @brief Fake endpoint implementation for EndPoint1 type.
//        Contains its own StubStrategy and PublisherStrategy implementations.
// ****************************************************************************
class FakeEndPoint1 : public FakeEndPoint
{
public:

    // ------------------------------------------------------------------------
    // @brief Constructor
    // @param endpoint Reference to the real EndPoint1
    // @param type The strategy type to use (Stub or Publisher)
    // ------------------------------------------------------------------------
    FakeEndPoint1(EndPoint& endpoint, FakeEndPoint::PrepareStrategy::Type type)
    : FakeEndPoint(endpoint, createStrategy(type))
    {
        // Initialize buffers with size specific to FakeEndPoint1
        m_input.resize(1);
        m_output.resize(1);

        // Configure the real endpoint with our buffers
        configureEndpoint();
    }

private:

    // ************************************************************************
    // StubStrategy class
    // @brief Stub strategy implementation for FakeEndPoint1.
    //        Returns fixed values for testing purposes.
    // ************************************************************************
    class StubStrategy : public FakeEndPoint::PrepareStrategy
    {
    public:
        StubStrategy() : FakeEndPoint::PrepareStrategy(FakeEndPoint::PrepareStrategy::Type::Stub) {}

        void prepareRead(std::vector<uint8_t>& input, const std::string& name) override
        {
            std::cout << "Preparing to read from " << name << " (Stub)" << std::endl;
            input[0] = 42;
        }

        void prepareWrite(std::vector<uint8_t>& output, const std::string& name) override
        {
            std::cout << "Preparing to write to " << name << " (Stub)" << std::endl;
            output[0] = 43;
        }
    };

    // ************************************************************************
    // PublisherStrategy class
    // @brief Publisher strategy implementation for FakeEndPoint1.
    //        Simulates dynamic data generation with incrementing counters.
    // ************************************************************************
    class PublisherStrategy : public FakeEndPoint::PrepareStrategy
    {
    public:
        PublisherStrategy() : FakeEndPoint::PrepareStrategy(FakeEndPoint::PrepareStrategy::Type::Publisher) {}

        void prepareRead(std::vector<uint8_t>& input, const std::string& name) override
        {
            std::cout << "Preparing to read from " << name << " (Publisher)" << std::endl;
            static uint8_t counter = 0;
            input[0] = 100 + counter++;
        }

        void prepareWrite(std::vector<uint8_t>& output, const std::string& name) override
        {
            std::cout << "Preparing to write to " << name << " (Publisher)" << std::endl;
            static uint8_t counter = 0;
            output[0] = 200 + counter++;
        }
    };

    // ------------------------------------------------------------------------
    // @brief Factory method to create the appropriate strategy
    // @param type The strategy type to create
    // @return A unique_ptr to the created strategy
    // @throws std::invalid_argument if type is invalid
    // ------------------------------------------------------------------------
    static std::unique_ptr<FakeEndPoint::PrepareStrategy> createStrategy(FakeEndPoint::PrepareStrategy::Type type)
    {
        switch (type)
        {
            case FakeEndPoint::PrepareStrategy::Type::Stub:
                return std::make_unique<FakeEndPoint1::StubStrategy>();
            case FakeEndPoint::PrepareStrategy::Type::Publisher:
                return std::make_unique<FakeEndPoint1::PublisherStrategy>();
            default:
                throw std::invalid_argument("Invalid strategy type");
        }
    }
};

// ****************************************************************************
// EndPoint2 class
// @brief Concrete EndPoint type 2. Used for type-based FakeEndPoint lookup.
// ****************************************************************************
class EndPoint2 : public EndPoint
{
public:
    EndPoint2(const std::string& name) : EndPoint(name) {}
};

// ****************************************************************************
// FakeEndPoint2 class
// @brief Fake endpoint implementation for EndPoint2 type.
//        Contains its own StubStrategy and PublisherStrategy implementations.
// ****************************************************************************
class FakeEndPoint2 : public FakeEndPoint
{
public:

    // ------------------------------------------------------------------------
    // @brief Constructor
    // @param endpoint Reference to the real EndPoint2
    // @param type The strategy type to use (Stub or Publisher)
    // ------------------------------------------------------------------------
    FakeEndPoint2(EndPoint& endpoint, FakeEndPoint::PrepareStrategy::Type type)
    : FakeEndPoint(endpoint, createStrategy(type))
    {
        // Initialize buffers with size specific to FakeEndPoint2 (2 bytes)
        m_input.resize(2);
        m_output.resize(2);

        // Configure the real endpoint with our buffers
        configureEndpoint();
    }

private:

    // ************************************************************************
    // StubStrategy class
    // @brief Stub strategy implementation for FakeEndPoint2.
    //        Returns fixed values for testing purposes.
    // ************************************************************************
    class StubStrategy : public FakeEndPoint::PrepareStrategy
    {
    public:
        StubStrategy() : FakeEndPoint::PrepareStrategy(FakeEndPoint::PrepareStrategy::Type::Stub) {}

        void prepareRead(std::vector<uint8_t>& input, const std::string& name) override
        {
            std::cout << "Preparing to read from " << name << " (Stub)" << std::endl;
            input[0] = 42;
            input[1] = 43;
        }

        void prepareWrite(std::vector<uint8_t>& output, const std::string& name) override
        {
            std::cout << "Preparing to write to " << name << " (Stub)" << std::endl;
            output[0] = 44;
            output[1] = 45;
        }
    };

    // ************************************************************************
    // PublisherStrategy class
    // @brief Publisher strategy implementation for FakeEndPoint2.
    //        Simulates dynamic data generation with incrementing counters.
    // ************************************************************************
    class PublisherStrategy : public FakeEndPoint::PrepareStrategy
    {
    public:
        PublisherStrategy() : FakeEndPoint::PrepareStrategy(FakeEndPoint::PrepareStrategy::Type::Publisher) {}

        void prepareRead(std::vector<uint8_t>& input, const std::string& name) override
        {
            std::cout << "Preparing to read from " << name << " (Publisher)" << std::endl;
            static uint8_t counter = 0;
            input[0] = 100 + counter++;
            input[1] = 101 + counter++;
        }

        void prepareWrite(std::vector<uint8_t>& output, const std::string& name) override
        {
            std::cout << "Preparing to write to " << name << " (Publisher)" << std::endl;
            static uint8_t counter = 0;
            output[0] = 200 + counter++;
            output[1] = 201 + counter++;
        }
    };

    // ------------------------------------------------------------------------
    // @brief Factory method to create the appropriate strategy
    // @param type The strategy type to create
    // @return A unique_ptr to the created strategy
    // @throws std::invalid_argument if type is invalid
    // ------------------------------------------------------------------------
    static std::unique_ptr<FakeEndPoint::PrepareStrategy> createStrategy(FakeEndPoint::PrepareStrategy::Type type)
    {
        switch (type)
        {
            case FakeEndPoint::PrepareStrategy::Type::Stub:
                return std::make_unique<FakeEndPoint2::StubStrategy>();
            case FakeEndPoint::PrepareStrategy::Type::Publisher:
                return std::make_unique<FakeEndPoint2::PublisherStrategy>();
            default:
                throw std::invalid_argument("Invalid strategy type");
        }
    }
};

// ****************************************************************************
// FakeBus class
// @brief Manages a collection of EndPoints and their corresponding FakeEndPoints.
//        Uses a type-based registry to automatically create the correct
//        FakeEndPoint for each EndPoint type using std::type_index.
// ****************************************************************************
class FakeBus
{
public:

    // ------------------------------------------------------------------------
    // @brief Constructor
    // @param strategy_type The strategy type to use for all FakeEndPoints
    // ------------------------------------------------------------------------
    FakeBus(FakeEndPoint::PrepareStrategy::Type strategy_type)
    : m_strategy_type(strategy_type)
    {
        // Register the mapping between EndPoint types and their FakeEndPoint factories
        registerFake<EndPoint1, FakeEndPoint1>();
        registerFake<EndPoint2, FakeEndPoint2>();
    }

    // ------------------------------------------------------------------------
    // @brief Add an endpoint to the bus (template version)
    // @tparam TEndPoint The concrete EndPoint type
    // @param endpoint The endpoint to add
    // @throws std::runtime_error if no FakeEndPoint is registered for the type
    // ------------------------------------------------------------------------
    template<typename TEndPoint>
    void addEndPoint(std::shared_ptr<TEndPoint> endpoint)
    {
        std::cout << "Adding endpoint " << endpoint->getName() << std::endl;
        m_endpoints.push_back(endpoint);

        // Lookup factory in registry using typeid on the template parameter (not the expression)
        // This avoids the warning about potentially-evaluated-expression
        auto it = m_registry.find(std::type_index(typeid(TEndPoint)));
        if (it == m_registry.end())
        {
            throw std::runtime_error("No FakeEndPoint registered for this EndPoint type");
        }

        // Create the appropriate FakeEndPoint using the registered factory
        std::shared_ptr<FakeEndPoint> fakeEndpoint = it->second(*endpoint, m_strategy_type);
        m_fakes.push_back(fakeEndpoint);
    }

    // ------------------------------------------------------------------------
    // @brief Trigger read operations on all fake endpoints
    // ------------------------------------------------------------------------
    void read()
    {
        for (const auto& fake : m_fakes)
        {
            fake->read();
        }
    }

    // ------------------------------------------------------------------------
    // @brief Trigger write operations on all fake endpoints
    // ------------------------------------------------------------------------
    void write()
    {
        for (const auto& fake : m_fakes)
        {
            fake->write();
        }
    }

private:

    // ------------------------------------------------------------------------
    // @brief Register a FakeEndPoint factory for a given EndPoint type
    // @tparam TEndPoint The EndPoint type to register
    // @tparam TFakeEndPoint The corresponding FakeEndPoint type to create
    // ------------------------------------------------------------------------
    template<typename TEndPoint, typename TFakeEndPoint>
    void registerFake()
    {
        m_registry[std::type_index(typeid(TEndPoint))] =
            [](EndPoint& ep, FakeEndPoint::PrepareStrategy::Type type) {
                return std::make_shared<TFakeEndPoint>(ep, type);
            };
    }

private:
    // @brief Type alias for the factory function signature
    using FakeFactory = std::function<std::shared_ptr<FakeEndPoint>(EndPoint&, FakeEndPoint::PrepareStrategy::Type)>;
    // @brief EndPoint type -> FakeEndPoint factory
    std::unordered_map<std::type_index, FakeFactory> m_registry;
    // @brief Collection of real endpoints
    std::vector<std::shared_ptr<EndPoint>> m_endpoints;
    // @brief Collection of fake endpoints
    std::vector<std::shared_ptr<FakeEndPoint>> m_fakes;
    // @brief Strategy type for all fakes
    FakeEndPoint::PrepareStrategy::Type m_strategy_type;
};

// ****************************************************************************
// Main function
// ****************************************************************************
int main()
{
    // Create a FakeBus with Publisher strategy
    FakeBus bus(FakeEndPoint::PrepareStrategy::Type::Publisher);

    // Add endpoints - the correct FakeEndPoint is automatically created
    bus.addEndPoint(std::make_shared<EndPoint1>("Endpoint1"));
    bus.addEndPoint(std::make_shared<EndPoint2>("Endpoint2"));

    // Trigger read/write operations
    bus.read();
    bus.write();

    return 0;
}
