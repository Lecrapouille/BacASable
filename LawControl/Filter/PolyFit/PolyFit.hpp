#ifndef POLYFIT_HPP
#define POLYFIT_HPP

#include <vector>
#include <cmath>

//******************************************************************************
//! \brief Recursive least squares to perform a polynomial regression (i.e. it
//! fits a polynomial to a set of data points without requiring a large
//! buffer). For details on the math behind this recursive least squares
//! implementation, see Gentlemen & Kung's famous 1981 paper entitled "Matrix
//! triangularization by systolic arrays".
//! \note This class is a C++ portage of https://github.com/splicer/polyfit
//******************************************************************************
class PolyFit
{
    const double FORGETTING_FACTOR = (1.0 - 1.0E-11);
    const double SMALL_VALUE = 1.0E-32;

public:

    //-------------------------------------------------------------------------
    //! \brief Default constructor passing the desired polynomial degree.
    //-------------------------------------------------------------------------
    PolyFit(size_t degree)
        : m_rows(degree + 1u),
          m_cols(degree + 2u),
          m_cells(m_rows * (m_cols + 1u), SMALL_VALUE)
    {
        m_weights = &m_cells[m_rows * m_cols];
    }

    //-------------------------------------------------------------------------
    //! \brief Add x and f(x)
    //-------------------------------------------------------------------------
    void add(double x, double y)
    {
        double in[m_cols];

        in[0] = 1.0;
        for (size_t j = 1u; j < m_cols - 1u; j++)
        {
            in[j] = in[j - 1u] * x;
        }
        in[m_cols - 1] = y;

        for (size_t i = 0u; i < m_rows; i++)
        {
            double c, s;
            boundary_cell(m_cells[i * m_cols + i], c, s, in[i]);
            for (size_t j = i + 1; j < m_cols; j++)
            {
                double out = internal_cell(m_cells[i * m_cols + j], c, s, in[j]);
                if (i < m_rows - 1u)
                {
                    in[j] = out;
                }
            }
        }

        m_weights_need_update = true;
    }

    //-------------------------------------------------------------------------
    //! \brief Return the weights of the polynom.
    //-------------------------------------------------------------------------
    void weights(std::vector<double>& w)
    {
        w.clear();
        w.resize(m_rows);

        compute_weights();
        for (size_t i = 0u; i < m_rows; i++)
        {
            w[i] = m_weights[i];
        }
    }

    //-------------------------------------------------------------------------
    //! \brief Compute the estimated y.
    //-------------------------------------------------------------------------
    double operator()(double x)
    {
        size_t i = m_rows - 1u;
        double y;

        compute_weights();

        y = m_weights[i];
        while (i-- > 0u)
        {
            y = y * x + m_weights[i];
        }

        return y;
    }

private:

    void compute_weights()
    {
        if (!m_weights_need_update)
            return;

        for (int i = int(m_rows - 1u); i >= 0; i--)
        {
            m_weights[i] = m_cells[i * m_cols + m_cols - 1u];
            for (size_t j = i + 1u; j < m_cols - 1u; j++)
            {
                m_weights[i] -= m_cells[i * m_cols + j] * m_weights[j];
            }
            m_weights[i] /= m_cells[i * m_cols + i];
        }

        m_weights_need_update = false;
    }

    void boundary_cell(double& cell, double& c, double& s, double in)
    {
        if (fabs(in) < SMALL_VALUE)
        {
            // close enough to zero
            c = 1.0;
            s = 0.0;
        }
        else
        {
            double norm = std::sqrt(cell * cell + in * in);
            c = cell / norm;
            s = in / norm;
            cell = FORGETTING_FACTOR * norm;
        }
    }

    double internal_cell(double& cell, double c, double s, double in)
    {
        double out = c * in - FORGETTING_FACTOR * s * cell;
        cell = s * in + FORGETTING_FACTOR * c * cell;
        return out;
    }

private:

    size_t m_rows;
    size_t m_cols;
    bool m_weights_need_update = true;
    double *m_weights;
    std::vector<double> m_cells;
};

#endif
