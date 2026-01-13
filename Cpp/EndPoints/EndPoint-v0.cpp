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

    std::shared_ptr<FakeEndPoint> getFakeEndPoint()
    {
        return std::make_shared<FakeEndPoint>(m_name);
    }

private:
    std::string m_name;
    std::vector<uint8_t>* m_input;
    std::vector<uint8_t>* m_output;
};


class FakeEndPoint
{
public:
    FakeEndPoint(const std::string& name) // Passer EndPoint
    : m_name(name)
    {
        m_input.resize(1);
        m_output.resize(1);
    }

    ~FakeEndPoint()
    {}

    Mapping getMapping()
    {
        return Mapping{&m_input, &m_output};
    }

    void prepareRead()
    {
        std::cout << "Preparing to read from " << m_name << std::endl;
        m_input[0] = 42;
    }

    void prepareWrite()
    {
        std::cout << "Preparing to write to " << m_name << std::endl;
        m_output[0] = 43;
    }

private:
    std::string m_name;
    std::vector<uint8_t> m_input;
    std::vector<uint8_t> m_output;
};

class Bus
{
public:

    void addEndPoint(const std::string& name)
    {
        std::cout << "Adding endpoint " << name << std::endl;
        std::shared_ptr<EndPoint> endpoint = std::make_shared<EndPoint>(name);
        m_endpoints.push_back(endpoint);
        std::cout << "Creating fake endpoint " << name << std::endl;
        std::shared_ptr<FakeEndPoint> fakeEndpoint = endpoint->getFakeEndPoint();
        std::cout << "Configuring endpoint " << name << std::endl;
        Mapping mapping = fakeEndpoint->getMapping();
        endpoint->configure(mapping);
        m_fakeEndpoints.push_back(fakeEndpoint);
    }

    void read()
    {
        for (const auto& fakeEndpoint : m_fakeEndpoints)
        {
            fakeEndpoint->prepareRead();
        }
        for (const auto& endpoint : m_endpoints)
        {
            endpoint->read();
        }
    }

    void write()
    {
        for (const auto& fakeEndpoint : m_fakeEndpoints)
        {
            fakeEndpoint->prepareWrite();
        }
        for (const auto& endpoint : m_endpoints)
        {
            endpoint->write();
        }
    }

private:
    std::vector<std::shared_ptr<EndPoint>> m_endpoints;
    std::vector<std::shared_ptr<FakeEndPoint>> m_fakeEndpoints;
};

int main()
{
    Bus bus;
    bus.addEndPoint("Endpoint1");
    bus.addEndPoint("Endpoint2");
    bus.read();
    bus.write();
    return 0;
}