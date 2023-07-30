#ifndef KALMAN_HPP
#  define KALMAN_HPP

#include <cmath>

//******************************************************************************
//! \brief 1-dimension Kalman filter.
//! \note Based on Professor Michel van Biezen lectures:
//! https://www.youtube.com/playlist?list=PLX2gX-ftPVXU3oUFNATxGXY90AULiqnWT
//! and https://github.com/denyssene/SimpleKalmanFilter
//******************************************************************************
class Kalman1D
{
public:

    struct Settings
    {
        double initial_measurement_uncertainty;
        double initial_estimation_uncertainty;
        double process_variance;
    };

    //-------------------------------------------------------------------------
    //! \brief
    //-------------------------------------------------------------------------
    Kalman1D(Settings const& settings)
        : m_err_estimate(settings.initial_estimation_uncertainty),
          m_err_measure(settings.initial_measurement_uncertainty),
          m_variance(settings.process_variance)
    {
        if ((m_variance < 0.0) || (m_variance > 1.0))
        {
            throw "Variance shall be between 0 and 1";
        }
    }

    //-------------------------------------------------------------------------
    //! \brief
    //! \return the current estimation.
    //-------------------------------------------------------------------------
    double update(double const measure)
    {
        m_kalman_gain = m_err_estimate / (m_err_estimate + m_err_measure);
        m_current_estimate = m_last_estimate + m_kalman_gain * (measure - m_last_estimate);
        m_err_estimate = (1.0 - m_kalman_gain) * m_err_estimate + std::abs(m_last_estimate - m_current_estimate) * m_variance;
        m_last_estimate = m_current_estimate;

        return m_current_estimate;
    }

    inline double gain() const
    {
        return m_kalman_gain;
    }

    inline double estimate_error() const
    {
        return m_err_estimate;
    }

private:

    double m_err_estimate;
    double m_err_measure;
    double m_variance;
    double m_current_estimate = 0.0;
    double m_last_estimate = 0.0;
    double m_kalman_gain = 0.0;
};

#endif
