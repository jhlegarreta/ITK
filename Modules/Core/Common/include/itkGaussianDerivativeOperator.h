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
#ifndef itkGaussianDerivativeOperator_h
#define itkGaussianDerivativeOperator_h

#include "itkGaussianOperator.h"
#include "itkDerivativeOperator.h"

#include <algorithm>

namespace itk
{

/**
 * \class GaussianDerivativeOperatorEnums
 * \brief GaussianDerivativeOperator class enum classes.
 * \ingroup ITKCommon
 */
class GaussianDerivativeOperatorEnums
{
public:
  /**
   * \ingroup ITKCommon
   * Interpolation modes
   */
  enum class InterpolationMode : uint8_t
  {
    NearestNeighbourInterpolation,
    LinearInterpolation
  };
};
// Define how to print enumeration
extern ITKCommon_EXPORT std::ostream &
                        operator<<(std::ostream & out, GaussianDerivativeOperatorEnums::InterpolationMode value);

/**
 * \class GaussianDerivativeOperator
 * \brief A NeighborhoodOperator whose coefficients are a one dimensional,
 * discrete derivative Gaussian kernel.
 *
 * GaussianDerivativeOperator can be used to calculate Gaussian derivatives
 * by taking its inner product with to a Neighborhood
 * (NeighborhoodIterator) that is swept across an image region.
 * It is a directional operator.  N successive applications
 * oriented along each dimensional direction will calculate separable,
 * efficient, N-D Gaussian derivatives of an image region.
 *
 * GaussianDerivativeOperator takes three parameters:
 *
 * (1) The floating-point variance of the desired Gaussian function.
 *
 * (2) The order of the derivative to be calculated (zero order means
 *     it performs only smoothing as a standard itk::GaussianOperator)
 *
 * (3) The "maximum error" allowed in the discrete Gaussian
 * function.  "Maximum error" is defined as the difference between the area
 * under the discrete Gaussian curve and the area under the continuous
 * Gaussian. Maximum error affects the Gaussian operator size. Care should
 * be taken not to make this value too small relative to the variance
 * lest the operator size become unreasonably large.
 *
 * References:
 * The Gaussian kernel contained in this operator was described
 * by Tony Lindeberg  (Discrete Scale-Space Theory and the Scale-Space
 * Primal Sketch. Dissertation. Royal Institute of Technology, Stockholm,
 * Sweden. May 1991.).
 *
 * \author Ivan Macia, Vicomtech, Spain, https://www.vicomtech.org/en
 *
 * This implementation is derived from the Insight Journal paper:
 * https://doi.org/10.54294/mrg5is
 *
 * \note GaussianDerivativeOperator does not have any user-declared "special member function",
 * following the C++ Rule of Zero: the compiler will generate them if necessary.
 *
 * \sa GaussianOperator
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 * \sa Neighborhood
 *
 * \ingroup Operators
 * \ingroup ITKCommon
 *
 * \sphinx
 * \sphinxexample{Core/Common/CreateGaussianDerivativeKernel,Create Gaussian Derivative Kernel}
 * \endsphinx
 */
template <typename TPixel, unsigned int VDimension = 2, typename TAllocator = NeighborhoodAllocator<TPixel>>
class ITK_TEMPLATE_EXPORT GaussianDerivativeOperator : public NeighborhoodOperator<TPixel, VDimension, TAllocator>
{
public:
  /** Standard class type aliases. */
  using Self = GaussianDerivativeOperator;
  using Superclass = NeighborhoodOperator<TPixel, VDimension, TAllocator>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(GaussianDerivativeOperator);

  using InterpolationModeEnum = GaussianDerivativeOperatorEnums::InterpolationMode;

  /** Neighborhood operator types. */
  using GaussianOperatorType = GaussianOperator<TPixel, VDimension, TAllocator>;
  using DerivativeOperatorType = DerivativeOperator<TPixel, VDimension, TAllocator>;

  /** Set/Get the flag for calculating scale-space normalized
   * derivatives.
   *
   * Normalized derivatives are obtained multiplying by the scale
   * parameter $t^1/order$. This use useful for scale-space selection
   * algorithms such as blob detection. The scaling results in the
   * value of the derivatives being independent of the size of an
   * object. */
  void
  SetNormalizeAcrossScale(bool flag)
  {
    m_NormalizeAcrossScale = flag;
  }
  itkGetConstNonVirtualMacro(NormalizeAcrossScale, bool);
  itkBooleanMacro(NormalizeAcrossScale);

  /** Set/Get the variance of the Gaussian kernel. */
  void
  SetVariance(const double variance)
  {
    m_Variance = variance;
  }
  itkGetConstNonVirtualMacro(Variance, double);

  /** Set/Get the spacing for the direction of this kernel. */
  void
  SetSpacing(const double spacing)
  {
    m_Spacing = spacing;
  }
  itkGetConstNonVirtualMacro(Spacing, double);

  /** Set/Get the desired maximum error of the Gaussian approximation.
   * The maximum error is the difference between the area under the discrete
   * Gaussian curve and the area under the continuous Gaussian. Maximum error
   * affects the Gaussian operator size. The value is clamped between 0.00001
   * and 0.99999. */
  void
  SetMaximumError(const double maxerror)
  {
    constexpr double Min = 0.00001;
    const double     Max = 1.0 - Min;

    m_MaximumError = std::clamp(maxerror, Min, Max);
  }
  itkGetNonVirtualMacro(MaximumError, double);

  /** Set/Get a limit for growth of the kernel. Small maximum error values with
   *  large variances will yield very large kernel sizes. This value can be
   *  used to truncate a kernel in such instances. A warning will be given when
   *  the specified maximum error causes the kernel to exceed this size. */
  void
  SetMaximumKernelWidth(unsigned int n)
  {
    m_MaximumKernelWidth = n;
  }
  itkGetConstMacro(MaximumKernelWidth, unsigned int);

  /** Set/Get the order of the derivative. */
  void
  SetOrder(const unsigned int order)
  {
    m_Order = order;
  }
  itkGetConstNonVirtualMacro(Order, unsigned int);

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

protected:
  /** Type alias support for coefficient vector type.*/
  using typename Superclass::CoefficientVector;

  /** Returns the value of the modified Bessel function I0(x) at a point x >= 0.
   */
  static double
  ModifiedBesselI0(double);

  /** Returns the value of the modified Bessel function I1(x) at a point x,
   * x real.  */
  static double
  ModifiedBesselI1(double);

  /** Returns the value of the modified Bessel function Ik(x) at a point x>=0,
   * where k>=2. */
  static double
  ModifiedBesselI(int, double);

  /** Calculates operator coefficients. */
  CoefficientVector
  GenerateCoefficients() override;

  /** Arranges coefficients spatially in the memory buffer. */
  void
  Fill(const CoefficientVector & coeff) override
  {
    this->FillCenteredDirectional(coeff);
  }

private:
  /* Methods for generations of the coefficients for a Gaussian
   * operator of 0-order respecting the remaining parameters. */
  CoefficientVector
  GenerateGaussianCoefficients() const;

  bool         m_NormalizeAcrossScale{ true };
  double       m_Variance{ 1.0 };
  double       m_MaximumError{ 0.005 };
  unsigned int m_MaximumKernelWidth{ 30 };
  unsigned int m_Order{ 1 };
  double       m_Spacing{ 1.0 };
};

} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkGaussianDerivativeOperator.hxx"
#endif

#endif
