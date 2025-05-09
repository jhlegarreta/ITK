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
#ifndef itkSyNImageRegistrationMethod_h
#define itkSyNImageRegistrationMethod_h

#include "itkImageRegistrationMethodv4.h"

#include "itkImageMaskSpatialObject.h"
#include "itkDisplacementFieldTransform.h"

namespace itk
{

/** \class SyNImageRegistrationMethod
 * \brief Interface method for the performing greedy SyN image registration.
 *
 * For greedy SyN we use \c m_Transform to map the time-parameterized middle
 * image to the fixed image (and vice versa using
 * \c m_Transform->GetInverseDisplacementField() ).  We employ another ivar,
 * \c m_InverseTransform, to map the time-parameterized middle image to the
 * moving image.
 *
 * Output: The output is the updated transform which has been added to the
 * composite transform.
 *
 * This implementation is based on the source code in Advanced
 * Normalization Tools (ANTs) \cite avants2011.
 *
 * The original paper discussing the method is \cite avants2008.
 *
 * The method evolved since that time with crucial contributions from Gang Song and
 * Nick Tustison. Though similar in spirit, this implementation is not identical.
 *
 * \todo Need to allow the fixed image to have a composite transform.
 *
 * \author Nick Tustison
 * \author Brian Avants
 *
 * \ingroup ITKRegistrationMethodsv4
 */
template <typename TFixedImage,
          typename TMovingImage,
          typename TOutputTransform = DisplacementFieldTransform<double, TFixedImage::ImageDimension>,
          typename TVirtualImage = TFixedImage,
          typename TPointSet = PointSet<unsigned int, TFixedImage::ImageDimension>>
class ITK_TEMPLATE_EXPORT SyNImageRegistrationMethod
  : public ImageRegistrationMethodv4<TFixedImage, TMovingImage, TOutputTransform, TVirtualImage, TPointSet>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(SyNImageRegistrationMethod);

  /** Standard class type aliases. */
  using Self = SyNImageRegistrationMethod;
  using Superclass = ImageRegistrationMethodv4<TFixedImage, TMovingImage, TOutputTransform, TVirtualImage, TPointSet>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** ImageDimension constants */
  static constexpr unsigned int ImageDimension = TFixedImage::ImageDimension;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(SyNImageRegistrationMethod);

  /** Input type alias for the images. */
  using FixedImageType = TFixedImage;
  using FixedImagePointer = typename FixedImageType::Pointer;
  using typename Superclass::FixedImagesContainerType;
  using MovingImageType = TMovingImage;
  using MovingImagePointer = typename MovingImageType::Pointer;
  using typename Superclass::MovingImagesContainerType;

  using typename Superclass::PointSetType;
  using PointSetPointer = typename PointSetType::Pointer;
  using typename Superclass::PointSetsContainerType;

  /** Metric and transform type alias */
  using typename Superclass::ImageMetricType;
  using ImageMetricPointer = typename ImageMetricType::Pointer;
  using MeasureType = typename ImageMetricType::MeasureType;

  using ImageMaskSpatialObjectType = ImageMaskSpatialObject<ImageDimension>;
  using typename Superclass::FixedImageMaskType;
  using FixedMaskImageType = typename ImageMaskSpatialObjectType::ImageType;
  using typename Superclass::FixedImageMasksContainerType;
  using typename Superclass::MovingImageMaskType;
  using MovingMaskImageType = typename ImageMaskSpatialObjectType::ImageType;
  using typename Superclass::MovingImageMasksContainerType;

  using VirtualImageType = typename Superclass::VirtualImageType;
  using typename Superclass::VirtualImageBaseType;
  using typename Superclass::VirtualImageBaseConstPointer;

  using typename Superclass::MultiMetricType;
  using typename Superclass::MetricType;
  using MetricPointer = typename MetricType::Pointer;
  using typename Superclass::PointSetMetricType;

  using typename Superclass::InitialTransformType;
  using OutputTransformType = TOutputTransform;
  using OutputTransformPointer = typename OutputTransformType::Pointer;
  using RealType = typename OutputTransformType::ScalarType;
  using DerivativeType = typename OutputTransformType::DerivativeType;
  using DerivativeValueType = typename DerivativeType::ValueType;
  using DisplacementFieldType = typename OutputTransformType::DisplacementFieldType;
  using DisplacementFieldPointer = typename DisplacementFieldType::Pointer;
  using DisplacementVectorType = typename DisplacementFieldType::PixelType;

  using typename Superclass::CompositeTransformType;
  using TransformBaseType = typename CompositeTransformType::TransformType;

  using typename Superclass::DecoratedOutputTransformType;
  using DecoratedOutputTransformPointer = typename DecoratedOutputTransformType::Pointer;

  using DisplacementFieldTransformType = DisplacementFieldTransform<RealType, ImageDimension>;
  using DisplacementFieldTransformPointer = typename DisplacementFieldTransformType::Pointer;

  using NumberOfIterationsArrayType = Array<SizeValueType>;

  /** Set/Get the learning rate. */
  /** @ITKStartGrouping */
  itkSetMacro(LearningRate, RealType);
  itkGetConstMacro(LearningRate, RealType);
  /** @ITKEndGrouping */
  /** Set/Get the number of iterations per level. */
  /** @ITKStartGrouping */
  itkSetMacro(NumberOfIterationsPerLevel, NumberOfIterationsArrayType);
  itkGetConstMacro(NumberOfIterationsPerLevel, NumberOfIterationsArrayType);
  /** @ITKEndGrouping */
  /** Set/Get the convergence threshold */
  /** @ITKStartGrouping */
  itkSetMacro(ConvergenceThreshold, RealType);
  itkGetConstMacro(ConvergenceThreshold, RealType);
  /** @ITKEndGrouping */
  /** Set/Get the convergence window size */
  /** @ITKStartGrouping */
  itkSetMacro(ConvergenceWindowSize, unsigned int);
  itkGetConstMacro(ConvergenceWindowSize, unsigned int);
  /** @ITKEndGrouping */
  /** Let the user control whether we compute metric derivatives in the downsampled or full-res space.
   *  The default is 'true' --- classic SyN --- but there may be advantages to the other approach.
   *  Classic SyN did not have this possibility. This implementation will let us explore the question.
   */
  /** @ITKStartGrouping */
  itkSetMacro(DownsampleImagesForMetricDerivatives, bool);
  itkGetConstMacro(DownsampleImagesForMetricDerivatives, bool);
  /** @ITKEndGrouping */
  /** Allow the user to average the gradients in the mid-point domain. Default false.
   *  One might choose to do this to further reduce bias.
   */
  /** @ITKStartGrouping */
  itkSetMacro(AverageMidPointGradients, bool);
  itkGetConstMacro(AverageMidPointGradients, bool);
  /** @ITKEndGrouping */
  /**
   * Get/Set the Gaussian smoothing variance for the update field.
   */
  /** @ITKStartGrouping */
  itkSetMacro(GaussianSmoothingVarianceForTheUpdateField, RealType);
  itkGetConstReferenceMacro(GaussianSmoothingVarianceForTheUpdateField, RealType);
  /** @ITKEndGrouping */
  /**
   * Get/Set the Gaussian smoothing variance for the total field.
   */
  /** @ITKStartGrouping */
  itkSetMacro(GaussianSmoothingVarianceForTheTotalField, RealType);
  itkGetConstReferenceMacro(GaussianSmoothingVarianceForTheTotalField, RealType);
  /** @ITKEndGrouping */
  /** Get modifiable FixedToMiddle and MovingToMiddle transforms to save the current state of the registration. */
  /** @ITKStartGrouping */
  itkGetModifiableObjectMacro(FixedToMiddleTransform, OutputTransformType);
  itkGetModifiableObjectMacro(MovingToMiddleTransform, OutputTransformType);
  /** @ITKEndGrouping */
  /** Set FixedToMiddle and MovingToMiddle transforms to restore the registration from a saved state. */
  /** @ITKStartGrouping */
  itkSetObjectMacro(FixedToMiddleTransform, OutputTransformType);
  itkSetObjectMacro(MovingToMiddleTransform, OutputTransformType);
  /** @ITKEndGrouping */
protected:
  SyNImageRegistrationMethod();
  ~SyNImageRegistrationMethod() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** Perform the registration. */
  void
  GenerateData() override;

  /** Handle optimization internally.
   * Starts the optimization at each level. Performs a basic gradient descent operation.
   */
  virtual void
  StartOptimization();

  /**
   * Initialize by setting the interconnects between the components. Need to override
   * in the SyN class since we need to "adapt" the \c m_InverseTransform
   */
  void
  InitializeRegistrationAtEachLevel(const SizeValueType) override;

  virtual DisplacementFieldPointer
  ComputeUpdateField(const FixedImagesContainerType,
                     const PointSetsContainerType,
                     const TransformBaseType *,
                     const MovingImagesContainerType,
                     const PointSetsContainerType,
                     const TransformBaseType *,
                     const FixedImageMasksContainerType,
                     const MovingImageMasksContainerType,
                     MeasureType &);
  virtual DisplacementFieldPointer
  ComputeMetricGradientField(const FixedImagesContainerType,
                             const PointSetsContainerType,
                             const TransformBaseType *,
                             const MovingImagesContainerType,
                             const PointSetsContainerType,
                             const TransformBaseType *,
                             const FixedImageMasksContainerType,
                             const MovingImageMasksContainerType,
                             MeasureType &);

  virtual DisplacementFieldPointer
  ScaleUpdateField(const DisplacementFieldType *);
  virtual DisplacementFieldPointer
  GaussianSmoothDisplacementField(const DisplacementFieldType *, const RealType);
  virtual DisplacementFieldPointer
  InvertDisplacementField(const DisplacementFieldType *, const DisplacementFieldType * = nullptr);

  RealType m_LearningRate{ 0.25 };

  OutputTransformPointer m_MovingToMiddleTransform{ nullptr };
  OutputTransformPointer m_FixedToMiddleTransform{ nullptr };

  RealType     m_ConvergenceThreshold{ static_cast<RealType>(1.0e-6) };
  unsigned int m_ConvergenceWindowSize{ 10 };

  NumberOfIterationsArrayType m_NumberOfIterationsPerLevel{};
  bool                        m_DownsampleImagesForMetricDerivatives{ true };
  bool                        m_AverageMidPointGradients{ false };

private:
  RealType m_GaussianSmoothingVarianceForTheUpdateField{ 3.0 };
  RealType m_GaussianSmoothingVarianceForTheTotalField{ 0.5 };
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkSyNImageRegistrationMethod.hxx"
#endif

#endif
