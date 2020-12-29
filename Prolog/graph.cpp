#include "SWI-cpp.h"
#include <iostream>

// g++ -W -Wall graph.cpp `pkg-config swipl --cflags --libs` -o prog
// For MacOS X: export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/usr/local/lib/swipl/lib/x86_64-darwin/

int main(int argc, char **argv)
{
    PlEngine e(argv[0]);

    try
    {
        PlCall("consult('graph.pl').");

        // Get all nodes: node(X).
        {
            PlTermv av(1);
            PlQuery q("node", av);

            std::cout << "Nodes:" << std::endl;
            while (q.next_solution())
            {
                std::cout << "\t" << (char *)av[0] << std::endl;
            }
        }

        // Get all arcs: arc(X,Y).
        {
            PlTermv av(2);
            PlQuery q("arc", av);

            std::cout << "Arcs:" << std::endl;
            while (q.next_solution())
            {
                std::cout << "\t(" << (char *)av[0]
                          << ", " << (char *)av[1]
                          << ")" << std::endl;
            }
        }

        // Cycle de longeur N: chemin(X,X,2).
        {
            PlTermv av(3);
            PlQuery q("chemin", av);
            av[1] = av[0];
            av[2] = 2;

            std::cout << "Chemin1:" << std::endl;
            while (q.next_solution())
            {
                std::cout << "\t" << (char *)av[0]
                          << std::endl;
            }
        }

        // Cycle de longeur N: chemin(X,Y,2).
        {
            PlTermv av(3);
            PlQuery q("chemin", av);
            av[2] = 2;

            std::cout << "Chemin2:" << std::endl;
            while (q.next_solution())
            {
                std::cout << "\t(" << (char *)av[0]
                          << ", " << (char *)av[1]
                          << ")" << std::endl;
            }
        }

    }
    catch (PlException &ex)
    {
        std::cerr << (char *) ex << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
