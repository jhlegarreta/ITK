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
#ifndef itkCorrelationCoefficientHistogramImageToImageMetric_h
#define itkCorrelationCoefficientHistogramImageToImageMetric_h

#include "itkHistogramImageToImageMetric.h"

namespace itk
{
/** \class CorrelationCoefficientHistogramImageToImageMetric
    \brief Computes correlation coefficient similarity measure between two
    images to be registered.

    This class is templated over the type of the fixed and moving
    images to be compared.

    This metric computes the similarity measure between pixels in the
    moving image and pixels in the fixed images using a histogram.

    \ingroup RegistrationMetrics
 * \ingroup ITKRegistrationCommon
 */
template <typename TFixedImage, typename TMovingImage>
class ITK_TEMPLATE_EXPORT CorrelationCoefficientHistogramImageToImageMetric
  : public HistogramImageToImageMetric<TFixedImage, TMovingImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(CorrelationCoefficientHistogramImageToImageMetric);

  /** Standard class type aliases. */
  using Self = CorrelationCoefficientHistogramImageToImageMetric;
  using Superclass = HistogramImageToImageMetric<TFixedImage, TMovingImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(CorrelationCoefficientHistogramImageToImageMetric);

  /** Types transferred from the base class */
  using typename Superclass::RealType;
  using typename Superclass::TransformType;
  using typename Superclass::TransformPointer;
  using typename Superclass::TransformParametersType;
  using typename Superclass::TransformJacobianType;
  using typename Superclass::GradientPixelType;

  using typename Superclass::MeasureType;
  using typename Superclass::DerivativeType;
  using typename Superclass::FixedImageType;
  using typename Superclass::MovingImageType;
  using typename Superclass::FixedImageConstPointer;
  using typename Superclass::MovingImageConstPointer;

  using typename Superclass::HistogramType;

  using HistogramAbsoluteFrequencyType = typename HistogramType::AbsoluteFrequencyType;
  using HistogramFrequencyType = HistogramAbsoluteFrequencyType;

  using HistogramIteratorType = typename HistogramType::Iterator;
  using HistogramMeasurementVectorType = typename HistogramType::MeasurementVectorType;

protected:
  /** Constructor is protected to ensure that \c New() function is used to
      create instances. */
  /** @ITKStartGrouping */
  CorrelationCoefficientHistogramImageToImageMetric() = default;
  ~CorrelationCoefficientHistogramImageToImageMetric() override = default;
  /** @ITKEndGrouping */
  /** Evaluates the sum of squared differences from the histogram. */
  MeasureType
  EvaluateMeasure(HistogramType & histogram) const override;

private:
  /** Returns the mean in the x-direction. */
  MeasureType
  MeanX(HistogramType & histogram) const;

  /** Returns the mean in the y-direction. */
  MeasureType
  MeanY(HistogramType & histogram) const;

  /** Returns the variance in the x-direction. */
  MeasureType
  VarianceX(HistogramType & histogram) const;

  /** Returns the variance in the y-direction. */
  MeasureType
  VarianceY(HistogramType & histogram) const;

  /** Returns the variance. */
  MeasureType
  Covariance(HistogramType & histogram) const;
};
} // End namespace itk.

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkCorrelationCoefficientHistogramImageToImageMetric.hxx"
#endif

#endif // itkCorrelationCoefficientHistogramImageToImageMetric_h
