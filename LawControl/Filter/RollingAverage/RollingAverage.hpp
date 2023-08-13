#ifndef ROLLING_AVERAGE_HPP
#define ROLLING_AVERAGE_HPP

#include <cstddef>
#include <iostream>

//*****************************************************************************
//! \brief Rolling average filter (ie the mean of the N last samples).
//!                  N-1
//! Formula: y[n] = Sum(x[n-i]) / N
//!                  i=0
//! \tparam N the size of the rolling windows.
//*****************************************************************************
template<typename T, size_t N>
class RollingAverage
{
public:

    //-------------------------------------------------------------------------
    //! \brief Default constructor: initialize all values to 0
    //-------------------------------------------------------------------------
    RollingAverage()
    {
       size_t i = N;
       while (i--)
       {
           m_samples[i] = T{};
       }
    }

    //-------------------------------------------------------------------------
    //! \brief Compute y[n] = y[n-1] + x[n] - x[n - N]
    //!    Where: N is the size of the rolling windows.
    //! \param[in] x: the new sample
    //! \return y[n] / N
    //-------------------------------------------------------------------------
    T operator()(const T x)
    {
        // Remove last inserted sample (m_samples[m_index]) add the new sample (x)
        m_sum += (x - m_samples[m_index]);
        m_samples[m_index] = x;
        ++m_index;

        // Still the buffer is not filled compute the mean with the number of present elements
        // else compute the mean with the window size.
        if (m_buffering)
        {
             m_buffering = (m_index < N);
        }

        // Mean
        T y = m_sum / ((m_buffering) ? T(m_index) : T(N));

        // Circular index
        if (m_index == N)
        {
            m_index = 0u;
        }

        return y;
    }

private:

    T      m_samples[N];
    size_t m_index = 0u;
    T      m_sum = T{};
    bool   m_buffering = true;
};

#endif
