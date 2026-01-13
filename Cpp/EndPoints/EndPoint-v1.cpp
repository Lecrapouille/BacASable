#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>

struct Mapping
{
    std::vector<uint8_t>* input;
    std::vector<uint8_t>* output;
};

class FakeEndPoint;

// Pattern Strategy pour la préparation de lecture/écriture
class PrepareStrategy
{
public:
    virtual ~PrepareStrategy() = default;
    virtual void prepareRead(std::vector<uint8_t>& input, const std::string& name) = 0;
    virtual void prepareWrite(std::vector<uint8_t>& output, const std::string& name) = 0;
};

class StubStrategy : public PrepareStrategy
{
public:
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

class PublisherStrategy : public PrepareStrategy
{
public:
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

class EndPoint
{
public:
    EndPoint(const std::string& name)
    : m_name(name)
    {}

    ~EndPoint()
    {}

    void configure(Mapping& mapping)
    {
        std::cout << "Configuring " << m_name << std::endl;
        m_input = mapping.input;
        m_output = mapping.output;
    }

    void read()
    {
        std::cout << "Reading from " << m_name << " : " << int((*m_input)[0]) << std::endl;
    }

    void write()
    {
        std::cout << "Writing to " << m_name << " : " << int((*m_output)[0]) << std::endl;
    }

    std::string getName() const
    {
        return m_name;
    }

private:
    std::string m_name;
    std::vector<uint8_t>* m_input;
    std::vector<uint8_t>* m_output;
};


class FakeEndPoint
{
public:
    FakeEndPoint(EndPoint& endpoint, std::shared_ptr<PrepareStrategy> strategy = nullptr)
    : m_endpoint(endpoint)
    , m_strategy(strategy ? strategy : std::make_shared<StubStrategy>())
    {
        m_name = "Fake " + endpoint.getName();

        m_input.resize(1);
        m_output.resize(1);

        // Configure the endpoint with the mapping
        Mapping mapping = getMapping();
        m_endpoint.configure(mapping);
    }

    ~FakeEndPoint()
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
    Mapping getMapping()
    {
        return Mapping{&m_input, &m_output};
    }

private:
    EndPoint& m_endpoint;
    std::string m_name;
    std::vector<uint8_t> m_input;
    std::vector<uint8_t> m_output;
    std::shared_ptr<PrepareStrategy> m_strategy;
};

class Bus
{
public:

    void addEndPoint(std::shared_ptr<EndPoint> endpoint, std::shared_ptr<PrepareStrategy> strategy = nullptr)
    {
        // Add the endpoint to the bus
        std::cout << "Adding endpoint " << endpoint->getName() << std::endl;
        m_endpoints.push_back(endpoint);

        // Create a fake endpoint for the endpoint with la stratégie spécifiée
        std::shared_ptr<FakeEndPoint> fakeEndpoint = std::make_shared<FakeEndPoint>(*endpoint, strategy);
        m_fakeEndpoints.push_back(fakeEndpoint);
    }

    void read()
    {
        for (const auto& fakeEndpoint : m_fakeEndpoints)
        {
            fakeEndpoint->read();
        }
    }

    void write()
    {
        for (const auto& fakeEndpoint : m_fakeEndpoints)
        {
            fakeEndpoint->write();
        }
    }

private:
    std::vector<std::shared_ptr<EndPoint>> m_endpoints;
    std::vector<std::shared_ptr<FakeEndPoint>> m_fakeEndpoints;
};

int main()
{
    Bus bus;
    std::shared_ptr<EndPoint> endpoint1 = std::make_shared<EndPoint>("Endpoint1");
    std::shared_ptr<EndPoint> endpoint2 = std::make_shared<EndPoint>("Endpoint2");
    
    // Utilisation de la stratégie Stub (par défaut)
    bus.addEndPoint(endpoint1);
    
    // Utilisation de la stratégie Publisher
    std::shared_ptr<PrepareStrategy> publisherStrategy = std::make_shared<PublisherStrategy>();
    bus.addEndPoint(endpoint2, publisherStrategy);
    
    bus.read();
    bus.write();
    return 0;
}