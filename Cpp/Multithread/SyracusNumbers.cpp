#include <iostream>
#include <queue>
#include <cmath>
#include <limits>
#include <vector>
#include <thread>

// ========================================================
// Return the number of binary digits
static uint64_t length(uint64_t x)
{
    return uint64_t(log2(x) + 1.0);
}

static inline bool typeB(uint64_t x)
{
    return (x % 3u) == 0u;
}

static inline bool typeA(uint64_t x)
{
    return (x % 3u) == 2u;
}

static inline bool typeC(uint64_t x)
{
    return (x % 3u) == 1u;
}

static inline bool Ag(uint64_t x)
{
    return (((x - 17u) % 24u) == 0u);
}

static inline uint64_t V(uint64_t x)
{
    return 4u * x + 1u;
}

// ========================================================
// Just a wrapper on FIFO: First In First Out (not thread safe)
template <typename T>
class Fifo
{
public:

    bool pop(T& item)
    {
        if (m_queue.empty())
            return false;
        item = m_queue.front();
        m_queue.pop();
        return true;
    }

    void push(const T& item)
    {
        m_queue.push(item);
    }

    size_t size()
    {
        return m_queue.size();
    }

    size_t empty()
    {
        return m_queue.size() == 0u;
    }

private:

    std::queue<T> m_queue;
};

// ========================================================
// Number of workers = 2 * CPU cores (shall be >= 1)
#define JOBS 16

// ========================================================
// g++ --std=c++11 -W -Wall -Wextra -Wshadow-local SyracusNumbers.cpp -o idriss-threaded-queue-v0 -lpthread
int main(int argc, char *argv[])
{
    // Command line
    if (argc != 3)
    {
        std::cerr << "Error! Bad number of arguments:" << std::endl;
        std::cerr << "  " << argv[0] << " <s> <max_bin_len>" << std::endl;
        return EXIT_FAILURE;
    }

    // Inputs
    uint64_t s = std::stoul(argv[1]);
    uint64_t max = std::stoul(argv[2]); // max bin length
    uint64_t ls = length(s);

    std::cout << "Inputs: s=" << s << ", max=" << max << ":" << std::endl;

    // Transitioning elements for the algorithm
    Fifo<uint64_t> fifos[JOBS]; // 1 FIFO for each thread
    Fifo<uint64_t> fifo; // 1 FIFO for the main thread (note: could be fifos[0])

    // Push the initial number
    fifo.push(s);

    // Output (counter)
    uint64_t XX[JOBS]; // X for each threads
    uint64_t X = 0u; //  X for the main thread (note: could bne XX[0])

    for (int job = 0; job < JOBS; ++job)
    {
        XX[job] = 0u;
    }

    // Main thread to fill the first results into the FIFO
    {
        uint64_t n;

        while (fifo.pop(n))
        {
            // Halt the main thread when enough data is reached for filling
            // other threads' FIFO (each thread fifo should have >= 1 element)
            if (fifo.size() > JOBS)
            {
                std::cout << "Initial FIFO size: " << fifo.size() << std::endl;
                fifo.push(n);
                break;
            }

            uint64_t ln = length(n);
            uint64_t vn = V(n);

            if (typeA(n))
            {
                fifo.push((2u * n - 1u) / 3u);

                if (Ag(n))
                {
                    X += 1u;
                }
                if (ln < ls + max - 1u)
                {
                    fifo.push(vn);
                }
            }
            else if (typeB(n))
            {
                if (ln < ls + max - 1u)
                {
                    fifo.push(vn);
                }
            }
            else if (typeC(n))
            {
                if ((ln < ls + max) && (n > 1u))
                {
                    fifo.push((4u * n - 1u) / 3u);
                }
                if (ln < ls + max - 1u)
                {
                    fifo.push(vn);
                }
            }
        }
    }

    // Transfer elements: FIFO => FIFOs
    {
        uint64_t n;
        int job = 0;
        while (fifo.pop(n))
        {
            fifos[job].push(n);
            ++job;
            if (job == JOBS)
                job = 0;
        }
    }

    // Threaded algorithm: 1 FIFO by thread
    {
        std::thread threads[JOBS];

        for (int job = 0; job < JOBS; ++job)
        {
            //pool.push_task([&Fifos, &XX, &ls, &max, job] {
            threads[job] = std::thread([&fifos, &XX, &ls, &max, job] {
                uint64_t n;
                while (fifos[job].pop(n))
                {
                    uint64_t ln = length(n);
                    uint64_t vn = V(n);

                    if (typeA(n))
                    {
                        fifos[job].push((2u * n - 1u) / 3u);

                        if (Ag(n))
                        {
                            XX[job] += 1u;
                        }
                        if (ln < ls + max - 1u)
                        {
                            fifos[job].push(vn);
                        }
                    }
                    else if (typeB(n))
                    {
                        if (ln < ls + max - 1u)
                        {
                            fifos[job].push(vn);
                        }
                    }
                    else if (typeC(n))
                    {
                        if ((ln < ls + max) && (n > 1u))
                        {
                            fifos[job].push((4u * n - 1u) / 3u);
                        }
                        if (ln < ls + max - 1u)
                        {
                            fifos[job].push(vn);
                        }
                    }
                }
            }); // end thread func
        } // end for

        // Wait for tasks ending
        for (int job = 0; job < JOBS; ++job)
        {
            if (threads[job].joinable())
            {
                threads[job].join();
            }
        }
    } // end scope

    // Summation on X
    {
        for (int job = 0; job < JOBS; ++job)
        {
            X += XX[job];
        }
    }

    std::cout << " Res: " << X << std::endl;

    return EXIT_SUCCESS;
}
