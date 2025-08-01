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
#ifndef itkRescaleIntensityImageFilter_h
#define itkRescaleIntensityImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkMath.h"

namespace itk
{
// This functor class applies a linear transformation A.x + B
// to input values.
namespace Functor
{
template <typename TInput, typename TOutput>
class ITK_TEMPLATE_EXPORT IntensityLinearTransform
{
public:
  using RealType = typename NumericTraits<TInput>::RealType;
  IntensityLinearTransform()
    : m_Factor(1.0)
    , m_Offset(0.0)
    , m_Maximum(NumericTraits<TOutput>::max())
    , m_Minimum(NumericTraits<TOutput>::NonpositiveMin())
  {}

  ~IntensityLinearTransform() = default;
  void
  SetFactor(RealType a)
  {
    m_Factor = a;
  }
  void
  SetOffset(RealType b)
  {
    m_Offset = b;
  }
  void
  SetMinimum(TOutput min)
  {
    m_Minimum = min;
  }
  void
  SetMaximum(TOutput max)
  {
    m_Maximum = max;
  }

  bool
  operator==(const IntensityLinearTransform & other) const
  {
    return Math::ExactlyEquals(m_Factor, other.m_Factor) && Math::ExactlyEquals(m_Offset, other.m_Offset) &&
           Math::ExactlyEquals(m_Maximum, other.m_Maximum) && Math::ExactlyEquals(m_Minimum, other.m_Minimum);
  }

  ITK_UNEQUAL_OPERATOR_MEMBER_FUNCTION(IntensityLinearTransform);

  inline TOutput
  operator()(const TInput & x) const
  {
    const RealType value = static_cast<RealType>(x) * m_Factor + m_Offset;
    auto           result = static_cast<TOutput>(value);

    result = (result > m_Maximum) ? m_Maximum : result;
    result = (result < m_Minimum) ? m_Minimum : result;
    return result;
  }

private:
  RealType m_Factor;
  RealType m_Offset;
  TOutput  m_Maximum;
  TOutput  m_Minimum;
};
} // end namespace Functor

/**
 * \class RescaleIntensityImageFilter
 * \brief Applies a linear transformation to the intensity levels of the
 * input Image.
 *
 * RescaleIntensityImageFilter applies pixel-wise a linear transformation
 * to the intensity values of input image pixels. The linear transformation
 * is defined by the user in terms of the minimum and maximum values that
 * the output image should have.
 *
 * The following equation gives the mapping of the intensity values
 *
 * \par
 * \f[
 *  outputPixel = ( inputPixel - inputMin) \cdot
 *  \frac{(outputMax - outputMin )}{(inputMax - inputMin)} + outputMin
 * \f]
 *
 * All computations are performed in the precision of the input pixel's
 * RealType. Before assigning the computed value to the output pixel.
 *
 * NOTE: In this filter the minimum and maximum values of the input image are
 * computed internally using the MinimumMaximumImageCalculator. Users are not
 * supposed to set those values in this filter. If you need a filter where you
 * can set the minimum and maximum values of the input, please use the
 * IntensityWindowingImageFilter. If you want a filter that can use a
 * user-defined linear transformation for the intensity, then please use the
 * ShiftScaleImageFilter.
 *
 * \sa IntensityWindowingImageFilter
 *
 * \ingroup IntensityImageFilters  MultiThreaded
 *
 * \ingroup ITKImageIntensity
 *
 * \sphinx
 * \sphinxexample{Filtering/ImageIntensity/RescaleAnImage,Rescale An Image}
 * \endsphinx
 */
template <typename TInputImage, typename TOutputImage = TInputImage>
class ITK_TEMPLATE_EXPORT RescaleIntensityImageFilter
  : public UnaryFunctorImageFilter<
      TInputImage,
      TOutputImage,
      Functor::IntensityLinearTransform<typename TInputImage::PixelType, typename TOutputImage::PixelType>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(RescaleIntensityImageFilter);

  /** Standard class type aliases. */
  using Self = RescaleIntensityImageFilter;
  using Superclass = UnaryFunctorImageFilter<
    TInputImage,
    TOutputImage,
    Functor::IntensityLinearTransform<typename TInputImage::PixelType, typename TOutputImage::PixelType>>;

  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using OutputPixelType = typename TOutputImage::PixelType;
  using InputPixelType = typename TInputImage::PixelType;
  using RealType = typename NumericTraits<InputPixelType>::RealType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(RescaleIntensityImageFilter);

  itkSetMacro(OutputMinimum, OutputPixelType);
  itkSetMacro(OutputMaximum, OutputPixelType);
  itkGetConstReferenceMacro(OutputMinimum, OutputPixelType);
  itkGetConstReferenceMacro(OutputMaximum, OutputPixelType);

  /** Get the Scale and Shift used for the linear transformation
      of gray level values.
   \warning These Values are only valid after the filter has been updated */
  /** @ITKStartGrouping */
  itkGetConstReferenceMacro(Scale, RealType);
  itkGetConstReferenceMacro(Shift, RealType);
  /** @ITKEndGrouping */
  /** Get the Minimum and Maximum values of the input image.
   \warning These Values are only valid after the filter has been updated */
  /** @ITKStartGrouping */
  itkGetConstReferenceMacro(InputMinimum, InputPixelType);
  itkGetConstReferenceMacro(InputMaximum, InputPixelType);
  /** @ITKEndGrouping */
  /** Process to execute before entering the multithreaded section */
  void
  BeforeThreadedGenerateData() override;

  /** Print internal ivars */
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  itkConceptMacro(InputHasNumericTraitsCheck, (Concept::HasNumericTraits<InputPixelType>));
  itkConceptMacro(OutputHasNumericTraitsCheck, (Concept::HasNumericTraits<OutputPixelType>));
  itkConceptMacro(RealTypeMultiplyOperatorCheck, (Concept::MultiplyOperator<RealType>));
  itkConceptMacro(RealTypeAdditiveOperatorsCheck, (Concept::AdditiveOperators<RealType>));

protected:
  RescaleIntensityImageFilter();
  ~RescaleIntensityImageFilter() override = default;

private:
  RealType m_Scale{};
  RealType m_Shift{};

  InputPixelType m_InputMinimum{};
  InputPixelType m_InputMaximum{};

  OutputPixelType m_OutputMinimum{};
  OutputPixelType m_OutputMaximum{};
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkRescaleIntensityImageFilter.hxx"
#endif

#endif
