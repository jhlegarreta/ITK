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

#include <iostream>

#include "itkAffineTransform.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkNearestNeighborExtrapolateImageFunction.h"
#include "itkPipelineMonitorImageFilter.h"
#include "itkTestingMacros.h"

/* Further testing of itkResampleImageFilter
 * Output is compared with baseline image using the cmake itk_add_test
 * '--compare' option.
 */

namespace
{

template <typename TCoordinateType, unsigned int VDimension>
class NonlinearAffineTransform : public itk::AffineTransform<TCoordinateType, VDimension>
{
public:
  /** Standard class type aliases.   */
  using Self = NonlinearAffineTransform;
  using Superclass = itk::AffineTransform<TCoordinateType, VDimension>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** New macro for creation of through a smart pointer. */
  itkSimpleNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(NonlinearAffineTransform);

  /** Override this so not linear. See test below. */
  typename itk::TransformBaseTemplate<TCoordinateType>::TransformCategoryEnum
  GetTransformCategory() const override
  {
    return itk::TransformBaseTemplate<TCoordinateType>::TransformCategoryEnum::UnknownTransformCategory;
  }
};
} // namespace

int
itkResampleImageTest2Streaming(int argc, char * argv[])
{
  if (argc < 7)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv);
    std::cerr << "inputImage referenceImage resampledImageLinear resampledImageNonLinear "
                 "resampledImageLinearNearestExtrapolate resampledImageNonLinearNearestExtrapolate";
    std::cerr << std::endl;
    return EXIT_FAILURE;
  }

  constexpr unsigned int VDimension = 2;

  using PixelType = unsigned char;
  using ImageType = itk::Image<PixelType, VDimension>;
  using CoordinateType = double;

  using AffineTransformType = itk::AffineTransform<CoordinateType, VDimension>;
  using NonlinearAffineTransformType = NonlinearAffineTransform<CoordinateType, VDimension>;
  using InterpolatorType = itk::LinearInterpolateImageFunction<ImageType, CoordinateType>;
  using ExtrapolatorType = itk::NearestNeighborExtrapolateImageFunction<ImageType, CoordinateType>;

  using ReaderType = itk::ImageFileReader<ImageType>;
  using WriterType = itk::ImageFileWriter<ImageType>;

  auto reader1 = ReaderType::New();
  auto reader2 = ReaderType::New();

  auto writer1 = WriterType::New();
  auto writer2 = WriterType::New();
  auto writer3 = WriterType::New();
  auto writer4 = WriterType::New();

  reader1->SetFileName(argv[1]);
  reader2->SetFileName(argv[2]);

  writer1->SetFileName(argv[3]);
  writer2->SetFileName(argv[4]);
  writer3->SetFileName(argv[5]);
  writer4->SetFileName(argv[6]);

  // Create an affine transformation
  auto affineTransform = AffineTransformType::New();
  affineTransform->Scale(2.0);

  // Create a linear interpolation image function
  auto interpolator = InterpolatorType::New();

  // Create a nearest neighbor extrapolate image function
  auto extrapolator = ExtrapolatorType::New();

  // Create and configure a resampling filter
  using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
  auto resample = ResampleFilterType::New();

  using MonitorFilterType = itk::PipelineMonitorImageFilter<ImageType>;
  auto monitor = MonitorFilterType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(resample, ResampleImageFilter, ImageToImageFilter);

  monitor->SetInput(reader1->GetOutput());
  ITK_TEST_SET_GET_VALUE(reader1->GetOutput(), monitor->GetInput());

  resample->SetReferenceImage(reader2->GetOutput());
  ITK_TEST_SET_GET_VALUE(reader2->GetOutput(), resample->GetReferenceImage());

  resample->UseReferenceImageOn();
  ITK_TEST_EXPECT_TRUE(resample->GetUseReferenceImage());

  resample->SetTransform(affineTransform);
  ITK_TEST_SET_GET_VALUE(affineTransform, resample->GetTransform());

  resample->SetInterpolator(interpolator);
  ITK_TEST_SET_GET_VALUE(interpolator, resample->GetInterpolator());

  resample->SetInput(monitor->GetOutput());
  ITK_TEST_SET_GET_VALUE(monitor->GetOutput(), resample->GetInput());
  writer1->SetInput(resample->GetOutput());

  // Check GetReferenceImage
  if (resample->GetReferenceImage() != reader2->GetOutput())
  {
    std::cerr << "GetReferenceImage() failed ! " << std::endl;
    std::cerr << "Test failed." << std::endl;
    return EXIT_FAILURE;
  }

  // Run the resampling filter with the normal, linear, affine transform.
  // This will use ResampleImageFilter::LinearThreadedGenerateData().
  std::cout << "Test with normal AffineTransform." << std::endl;
  writer1->SetNumberOfStreamDivisions(8); // split into 8 pieces for streaming.
  monitor->ClearPipelineSavedInformation();
  ITK_TRY_EXPECT_NO_EXCEPTION(writer1->Update());

  if (!monitor->VerifyInputFilterExecutedStreaming(8))
  {
    std::cerr << "Streaming failed to execute as expected!" << std::endl;
    std::cerr << monitor;
    std::cerr << "Test failed." << std::endl;
    return EXIT_FAILURE;
  }

  // Assign an affine transform that returns
  // false for IsLinear() instead of true, to force
  // the filter to use the NonlinearThreadedGenerateData method
  // instead of LinearThreadedGenerateData. This will test that
  // we get the same results for both methods.
  std::cout << "Test with NonlinearAffineTransform." << std::endl;
  auto nonlinearAffineTransform = NonlinearAffineTransformType::New();

  nonlinearAffineTransform->Scale(2.0);
  resample->SetTransform(nonlinearAffineTransform);
  monitor->ClearPipelineSavedInformation();
  writer2->SetInput(resample->GetOutput());
  writer2->SetNumberOfStreamDivisions(8);
  ITK_TRY_EXPECT_NO_EXCEPTION(writer2->Update());

  std::cout << "We demanded splitting into 8 pieces for streaming, but faked non-linearity should disable streaming."
            << std::endl;
  if (monitor->VerifyInputFilterExecutedStreaming(8))
  {
    std::cerr << "Streaming succeeded for non-linear transform which should not be the case!" << std::endl;
    std::cerr << monitor;
    std::cerr << "Test failed." << std::endl;
    return EXIT_FAILURE;
  }

  // Instead of using the default pixel when sampling outside the input image,
  // we use a nearest neighbor extrapolator.
  resample->SetTransform(affineTransform);
  resample->SetExtrapolator(extrapolator);
  writer3->SetInput(resample->GetOutput());
  std::cout << "Test with nearest neighbor extrapolator, affine transform." << std::endl;
  writer3->SetNumberOfStreamDivisions(8); // split into 8 pieces for streaming.
  ITK_TRY_EXPECT_NO_EXCEPTION(writer3->Update());

  // Instead of using the default pixel when sampling outside the input image,
  // we use a nearest neighbor extrapolator.
  resample->SetTransform(nonlinearAffineTransform);
  writer4->SetInput(resample->GetOutput());
  std::cout << "Test with nearest neighbor extrapolator, nonlinear transform." << std::endl;
  writer4->SetNumberOfStreamDivisions(
    8); // demand splitting into 8 pieces for streaming, but faked non-linearity will disable streaming
  ITK_TRY_EXPECT_NO_EXCEPTION(writer4->Update());

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
