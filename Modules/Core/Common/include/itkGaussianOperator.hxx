/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkGaussianOperator_hxx
#define itkGaussianOperator_hxx
#include "itkOutputWindow.h"
#include "itkMacro.h"
namespace itk
{
template <typename TPixel, unsigned int VDimension, typename TAllocator>
auto
GaussianOperator<TPixel, VDimension, TAllocator>::GenerateCoefficients() -> CoefficientVector
{
  const double et = std::exp(-m_Variance);

  // Create the kernel coefficients as a std::vector
  double            sum = 0.0;
  CoefficientVector coeff;
  coeff.push_back(et * ModifiedBesselI0(m_Variance));
  sum += coeff[0];
  coeff.push_back(et * ModifiedBesselI1(m_Variance));
  sum += coeff[1] * 2.0;

  const double cap = 1.0 - m_MaximumError;
  for (int i = 2; sum < cap; ++i)
  {
    coeff.push_back(et * ModifiedBesselI(i, m_Variance));
    sum += coeff[i] * 2.0;
    if (coeff[i] <= 0.0)
    {
      break; // failsafe
    }
    if (coeff.size() > m_MaximumKernelWidth)
    {
      itkDebugMacro("Kernel size has exceeded the specified maximum width of "
                    << m_MaximumKernelWidth << " and has been truncated to " << coeff.size()
                    << " elements.  You can raise the maximum width using the SetMaximumKernelWidth method.");
      break;
    }
  }
  // Normalize the coefficients so that their sum is one.
  for (auto it = coeff.begin(); it < coeff.end(); ++it)
  {
    *it /= sum;
  }

  // Make symmetric
  const int j = static_cast<int>(coeff.size()) - 1;
  coeff.insert(coeff.begin(), j, 0);
  {
    int i = 0;
    for (auto it = coeff.end() - 1; i < j; --it, ++i)
    {
      coeff[i] = *it;
    }
  }
  return coeff;
}

template <typename TPixel, unsigned int VDimension, typename TAllocator>
double
GaussianOperator<TPixel, VDimension, TAllocator>::ModifiedBesselI0(double y)
{
  const double d = itk::Math::abs(y);
  double       accumulator = NAN;

  if (d < 3.75)
  {
    double m = y / 3.75;
    m *= m;
    accumulator =
      1.0 + m * (3.5156229 + m * (3.0899424 + m * (1.2067492 + m * (0.2659732 + m * (0.360768e-1 + m * 0.45813e-2)))));
  }
  else
  {
    const double m = 3.75 / d;
    accumulator =
      (std::exp(d) / std::sqrt(d)) *
      (0.39894228 +
       m * (0.1328592e-1 +
            m * (0.225319e-2 +
                 m * (-0.157565e-2 +
                      m * (0.916281e-2 +
                           m * (-0.2057706e-1 + m * (0.2635537e-1 + m * (-0.1647633e-1 + m * 0.392377e-2))))))));
  }
  return accumulator;
}

template <typename TPixel, unsigned int VDimension, typename TAllocator>
double
GaussianOperator<TPixel, VDimension, TAllocator>::ModifiedBesselI1(double y)
{
  const double d = itk::Math::abs(y);
  double       accumulator = NAN;

  if (d < 3.75)
  {
    double m = y / 3.75;
    m *= m;
    accumulator =
      d * (0.5 + m * (0.87890594 +
                      m * (0.51498869 + m * (0.15084934 + m * (0.2658733e-1 + m * (0.301532e-2 + m * 0.32411e-3))))));
  }
  else
  {
    const double m = 3.75 / d;
    accumulator = 0.2282967e-1 + m * (-0.2895312e-1 + m * (0.1787654e-1 - m * 0.420059e-2));
    accumulator =
      0.39894228 + m * (-0.3988024e-1 + m * (-0.362018e-2 + m * (0.163801e-2 + m * (-0.1031555e-1 + m * accumulator))));

    accumulator *= (std::exp(d) / std::sqrt(d));
  }

  if (y < 0.0)
  {
    return -accumulator;
  }

  return accumulator;
}

template <typename TPixel, unsigned int VDimension, typename TAllocator>
double
GaussianOperator<TPixel, VDimension, TAllocator>::ModifiedBesselI(int n, double y)
{
  constexpr double ACCURACY = 40.0;

  if (n < 2)
  {
    throw ExceptionObject(__FILE__, __LINE__, "Order of modified bessel is > 2.", ITK_LOCATION); //
                                                                                                 //
                                                                                                 //
                                                                                                 // placeholder
  }
  if (y == 0.0)
  {
    return 0.0;
  }

  const double toy = 2.0 / itk::Math::abs(y);
  double       qip = 0.0;
  double       accumulator = 0.0;
  double       qi = 1.0;

  for (int j = 2 * (n + static_cast<int>(std::sqrt(ACCURACY * n))); j > 0; j--)
  {
    const double qim = qip + j * toy * qi;
    qip = qi;
    qi = qim;
    if (itk::Math::abs(qi) > 1.0e10)
    {
      accumulator *= 1.0e-10;
      qi *= 1.0e-10;
      qip *= 1.0e-10;
    }
    if (j == n)
    {
      accumulator = qip;
    }
  }

  accumulator *= ModifiedBesselI0(y) / qi;

  if (y < 0.0 && (n & 1))
  {
    return -accumulator;
  }
  else
  {
    return accumulator;
  }
}
} // end namespace itk

#endif
