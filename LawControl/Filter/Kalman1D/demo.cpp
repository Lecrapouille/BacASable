#include "Kalman.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

class CsvReader
{
public:

    bool open(std::string const& path)
    {
        m_file.open(path, std::ios::in);
        if (!m_file.is_open())
            return false;

        std::string line;
        std::getline(m_file, line);
        return true;
    }

    bool readline(std::vector<double>& values)
    {
        values.clear();

        if (!m_file.is_open())
            return false;

        std::string line;
        if (!getline(m_file, line))
            return false;

        std::string word;
        std::stringstream str(line);
        while (getline(str, word, ';'))
        {
            values.push_back(std::stod(word));
        }
        return true;
    }

private:

    std::fstream m_file;
};

// g++ --std=c++11 -Wall -Wextra -Wshadow *.cpp -o kalman
int main()
{
    CsvReader reader;
    if (!reader.open("shock.csv"))
        return EXIT_FAILURE;

    Kalman1D::Settings s;
    s.initial_measurement_uncertainty = 15.0;
    s.initial_estimation_uncertainty = s.initial_measurement_uncertainty;
    s.process_variance = 0.2;
    Kalman1D kalman(s);

    std::cout << "Time;Coarse;Fine" << std::endl;
    std::vector<double> values;
    while (reader.readline(values))
    {
        double val = kalman.update(values[2]);
        values.push_back(val);
        std::cout << values[0] << ";" << values[2] << ";" << val << std::endl;
    }

    return EXIT_SUCCESS;
}
