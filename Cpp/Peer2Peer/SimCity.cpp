#include "Client.hpp"
#include "Host.hpp"
#include <iostream>
#include <thread>

// g++ --std=c++17 -Wall -Wextra -Wshadow *.cpp -o prog `pkg-config --cflags --libs sfml-graphics sfml-network`
int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " [host|client] [port]"
                      << std::endl;
            return 1;
        }

        std::string mode = argv[1];
        unsigned short port =
            (argc > 2) ? static_cast<unsigned short>(std::stoi(argv[2]))
                       : 45000;

        if (mode == "host")
        {
            Host host(port);
            host.run();
        }
        else if (mode == "client")
        {
            Client client(port);
            client.run();
        }
        else
        {
            std::cerr << "Mode invalide. Utilisez 'host' ou 'client'"
                      << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
