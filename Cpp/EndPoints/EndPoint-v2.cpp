#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>
#include <stdexcept>

class FakeEndPoint;

// ****************************************************************************
// EndPoint class
// ****************************************************************************
class EndPoint
{
public:

    struct Mapping
    {
        std::vector<uint8_t>* input;
        std::vector<uint8_t>* output;
    };

    EndPoint(const std::string& name)
      : m_name(name)
    {}

    ~EndPoint()
    {}

    void configure(Mapping& mapping)
    {
        std::cout << "Configuring " << m_name << std::endl;
        m_mapping = mapping;
    }

    void read()
    {
        std::cout << "Reading from " << m_name << " : " << int((*m_mapping.input)[0]) << std::endl;
    }

    void write()
    {
        std::cout << "Writing to " << m_name << " : " << int((*m_mapping.output)[0]) << std::endl;
    }

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
// ****************************************************************************
class FakeEndPoint
{
public:

    // ************************************************************************
    // Strategy pattern for preparing read/write operations feeding the real
    // endpoint with the data to be sent/received.
    // ************************************************************************
    class PrepareStrategy
    {
    public:

        // ---------------------------------------------------------------------
        // Type enum
        // ---------------------------------------------------------------------
        enum class Type
        {
            Stub,
            Publisher
        };

        PrepareStrategy(Type type): m_type(type){}
        virtual ~PrepareStrategy() = default;
        Type type() const { return m_type; }

        // ---------------------------------------------------------------------
        // Pure virtual method for preparing read operations for the real endpoint
        // ---------------------------------------------------------------------
        virtual void prepareRead(std::vector<uint8_t>& input, const std::string& name) = 0;

        // ---------------------------------------------------------------------
        // Pure virtual method for preparing write operations for the real endpoint
        // ---------------------------------------------------------------------
        virtual void prepareWrite(std::vector<uint8_t>& output, const std::string& name) = 0;

    private:
        PrepareStrategy::Type m_type;
    };

    // ------------------------------------------------------------------------
    // @brief FakeEndPoint class
    // @param endpoint The real endpoint
    // @param type The strategy type
    // ------------------------------------------------------------------------
    FakeEndPoint(EndPoint& endpoint, std::unique_ptr<PrepareStrategy> strategy)
    : m_endpoint(endpoint)
    , m_strategy(std::move(strategy))
    {
        m_name = "Fake " + endpoint.getName();

        m_input.resize(1);
        m_output.resize(1);

        // Configure the endpoint with the mapping
        EndPoint::Mapping mapping = getMapping();
        m_endpoint.configure(mapping);
    }

    virtual ~FakeEndPoint()
    {}

    void read()
    {
        m_strategy->prepareRead(m_input, m_name);
        m_endpoint.read();
    }

    void write()
    {
        m_strategy->prepareWrite(m_output, m_name);
        m_endpoint.write();
    }

    std::string getName() const
    {
        return m_name;
    }

private:
    EndPoint::Mapping getMapping()
    {
        return EndPoint::Mapping{&m_input, &m_output};
    }

protected:
    EndPoint& m_endpoint;
    std::string m_name;
    std::vector<uint8_t> m_input;
    std::vector<uint8_t> m_output;
    std::unique_ptr<PrepareStrategy> m_strategy;
};

// ****************************************************************************
// FakeEndPoint1 class
// ****************************************************************************
class FakeEndPoint1 : public FakeEndPoint
{
    public:
    FakeEndPoint1(EndPoint& endpoint, FakeEndPoint::PrepareStrategy::Type Type)
    : FakeEndPoint(endpoint, createStrategy(Type))
    {}

private:
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

    static std::unique_ptr<FakeEndPoint::PrepareStrategy> createStrategy(FakeEndPoint::PrepareStrategy::Type Type)
    {
        switch (Type)
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
// FakeEndPoint2 class
// ****************************************************************************
class FakeEndPoint2 : public FakeEndPoint
{
public:

    FakeEndPoint2(EndPoint& endpoint, FakeEndPoint::PrepareStrategy::Type Type)
    : FakeEndPoint(endpoint, createStrategy(Type))
    {}

private:
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

    static std::unique_ptr<FakeEndPoint::PrepareStrategy> createStrategy(FakeEndPoint::PrepareStrategy::Type Type)
    {
        switch (Type)
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
// Bus class
// ****************************************************************************
class Bus
{
public:
    Bus(FakeEndPoint::PrepareStrategy::Type strategy_type)
    : m_strategy_type(strategy_type)
    {}

    void addEndPoint(std::shared_ptr<EndPoint> endpoint)
    {
        // Add the endpoint to the bus
        std::cout << "Adding endpoint " << endpoint->getName() << std::endl;
        m_endpoints.push_back(endpoint);

        // Create a fake endpoint with the default strategy
        std::shared_ptr<FakeEndPoint> fakeEndpoint = std::make_shared<FakeEndPoint1>(*endpoint, m_strategy_type);
        m_fakes.push_back(fakeEndpoint);
    }

    void read()
    {
        for (const auto& fake : m_fakes)
        {
            fake->read();
        }
    }

    void write()
    {
        for (const auto& fake : m_fakes)
        {
            fake->write();
        }
    }

private:
    std::vector<std::shared_ptr<EndPoint>> m_endpoints;
    std::vector<std::shared_ptr<FakeEndPoint>> m_fakes;
    FakeEndPoint::PrepareStrategy::Type m_strategy_type;
};

int main()
{
    Bus bus(FakeEndPoint::PrepareStrategy::Type::Stub);
    bus.addEndPoint(std::make_shared<EndPoint>("Endpoint1"));
    bus.addEndPoint(std::make_shared<EndPoint>("Endpoint2"));
    bus.read();
    bus.write();
    return 0;
}