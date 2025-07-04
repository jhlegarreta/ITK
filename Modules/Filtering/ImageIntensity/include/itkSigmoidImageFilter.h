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
#ifndef itkSigmoidImageFilter_h
#define itkSigmoidImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkMath.h"

namespace itk
{
/**
 * \class SigmoidImageFilter
 * \brief Computes the sigmoid function pixel-wise
 *
 * A linear transformation is applied first on the argument of
 * the sigmoid function. The resulting total transform is given by
 *
 * \f[
 * f(x) = (Max-Min) \cdot \frac{1}{\left(1+e^{- \frac{ x - \beta }{\alpha}}\right)} + Min
 * \f]
 *
 * Every output pixel is equal to f(x). Where x is the intensity of the
 * homologous input pixel, and alpha and beta are user-provided constants.
 *
 * \ingroup IntensityImageFilters  MultiThreaded
 *
 * \ingroup ITKImageIntensity
 *
 * \sphinx
 * \sphinxexample{Filtering/ImageIntensity/ComputeSigmoid,Compute Sigmoid}
 * \endsphinx
 */

namespace Functor
{
template <typename TInput, typename TOutput>
class Sigmoid
{
public:
  Sigmoid()
    : m_OutputMinimum(NumericTraits<TOutput>::min())
    , m_OutputMaximum(NumericTraits<TOutput>::max())
  {}

  ~Sigmoid() = default;


  bool
  operator==(const Sigmoid & other) const
  {
    return Math::ExactlyEquals(m_Alpha, other.m_Alpha) && Math::ExactlyEquals(m_Beta, other.m_Beta) &&
           Math::ExactlyEquals(m_OutputMaximum, other.m_OutputMaximum) &&
           Math::ExactlyEquals(m_OutputMinimum, other.m_OutputMinimum);
  }

  ITK_UNEQUAL_OPERATOR_MEMBER_FUNCTION(Sigmoid);

  inline TOutput
  operator()(const TInput & A) const
  {
    const double x = (static_cast<double>(A) - m_Beta) / m_Alpha;
    const double e = 1.0 / (1.0 + std::exp(-x));
    const double v = (m_OutputMaximum - m_OutputMinimum) * e + m_OutputMinimum;

    return static_cast<TOutput>(v);
  }

  void
  SetAlpha(double alpha)
  {
    m_Alpha = alpha;
  }

  void
  SetBeta(double beta)
  {
    m_Beta = beta;
  }

  [[nodiscard]] double
  GetAlpha() const
  {
    return m_Alpha;
  }

  [[nodiscard]] double
  GetBeta() const
  {
    return m_Beta;
  }

  void
  SetOutputMinimum(TOutput min)
  {
    m_OutputMinimum = min;
  }

  void
  SetOutputMaximum(TOutput max)
  {
    m_OutputMaximum = max;
  }

  [[nodiscard]] TOutput
  GetOutputMinimum() const
  {
    return m_OutputMinimum;
  }

  [[nodiscard]] TOutput
  GetOutputMaximum() const
  {
    return m_OutputMaximum;
  }

private:
  double  m_Alpha{ 1.0 };
  double  m_Beta{ 0.0 };
  TOutput m_OutputMinimum;
  TOutput m_OutputMaximum;
};
} // namespace Functor

template <typename TInputImage, typename TOutputImage>
class SigmoidImageFilter
  : public UnaryFunctorImageFilter<TInputImage,
                                   TOutputImage,
                                   Functor::Sigmoid<typename TInputImage::PixelType, typename TOutputImage::PixelType>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(SigmoidImageFilter);

  /** Standard class type aliases. */
  using Self = SigmoidImageFilter;
  using Superclass =
    UnaryFunctorImageFilter<TInputImage,
                            TOutputImage,
                            Functor::Sigmoid<typename TInputImage::PixelType, typename TOutputImage::PixelType>>;

  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using OutputPixelType = typename TOutputImage::PixelType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(SigmoidImageFilter);

  void
  SetAlpha(double alpha)
  {
    if (Math::ExactlyEquals(alpha, this->GetFunctor().GetAlpha()))
    {
      return;
    }
    this->GetFunctor().SetAlpha(alpha);
    this->Modified();
  }

  [[nodiscard]] double
  GetAlpha() const
  {
    return this->GetFunctor().GetAlpha();
  }

  void
  SetBeta(double beta)
  {
    if (Math::ExactlyEquals(beta, this->GetFunctor().GetBeta()))
    {
      return;
    }
    this->GetFunctor().SetBeta(beta);
    this->Modified();
  }

  [[nodiscard]] double
  GetBeta() const
  {
    return this->GetFunctor().GetBeta();
  }

  void
  SetOutputMinimum(OutputPixelType min)
  {
    if (Math::ExactlyEquals(min, this->GetFunctor().GetOutputMinimum()))
    {
      return;
    }
    this->GetFunctor().SetOutputMinimum(min);
    this->Modified();
  }

  OutputPixelType
  GetOutputMinimum() const
  {
    return this->GetFunctor().GetOutputMinimum();
  }

  void
  SetOutputMaximum(OutputPixelType max)
  {
    if (Math::ExactlyEquals(max, this->GetFunctor().GetOutputMaximum()))
    {
      return;
    }
    this->GetFunctor().SetOutputMaximum(max);
    this->Modified();
  }

  OutputPixelType
  GetOutputMaximum() const
  {
    return this->GetFunctor().GetOutputMaximum();
  }

  itkConceptMacro(InputConvertibleToDoubleCheck, (Concept::Convertible<typename TInputImage::PixelType, double>));
  itkConceptMacro(OutputAdditiveOperatorsCheck, (Concept::AdditiveOperators<OutputPixelType>));
  itkConceptMacro(DoubleConvertibleToOutputCheck, (Concept::Convertible<double, OutputPixelType>));
  itkConceptMacro(OutputTimesDoubleCheck, (Concept::MultiplyOperator<OutputPixelType, double>));
  itkConceptMacro(OutputDoubleAdditiveOperatorsCheck,
                  (Concept::AdditiveOperators<OutputPixelType, OutputPixelType, double>));

protected:
  SigmoidImageFilter() = default;
  ~SigmoidImageFilter() override = default;
};
} // end namespace itk

#endif
