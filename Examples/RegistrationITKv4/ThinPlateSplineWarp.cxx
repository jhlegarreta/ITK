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

//  Command Line Arguments:
//  Insight/Testing/Data/LandmarkWarping3Landmarks1.txt
//                          inputImage  deformedImage deformationField
//
//  Software Guide : BeginLatex
//
//  This example deforms a 3D volume with the Thin plate spline.
//  \index{ThinPlateSplineKernelTransform}
//
//  Software Guide : EndLatex


#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkResampleImageFilter.h"

// Software Guide : BeginCodeSnippet
#include "itkThinPlateSplineKernelTransform.h"
// Software Guide : EndCodeSnippet

#include "itkPointSet.h"
#include <fstream>

namespace
{
int
ExampleMain(int argc, const char * const argv[])
{
  if (argc < 4)
  {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " landmarksFile inputImage ";
    std::cerr << "DeformedImage " << std::endl;
    std::cerr << "deformationField" << std::endl;
    return EXIT_FAILURE;
  }

  constexpr unsigned int ImageDimension = 3;

  using PixelType = unsigned char;
  using InputImageType = itk::Image<PixelType, ImageDimension>;
  using FieldVectorType = itk::Vector<float, ImageDimension>;
  using DisplacementFieldType = itk::Image<FieldVectorType, ImageDimension>;
  using CoordinateRepType = double;
  using TransformType =
    itk::ThinPlateSplineKernelTransform<CoordinateRepType, ImageDimension>;
  using PointType = itk::Point<CoordinateRepType, ImageDimension>;
  using PointSetType = TransformType::PointSetType;
  using PointIdType = PointSetType::PointIdentifier;
  using ResamplerType =
    itk::ResampleImageFilter<InputImageType, InputImageType>;
  using InterpolatorType =
    itk::LinearInterpolateImageFunction<InputImageType, double>;

  InputImageType::ConstPointer inputImage;

  inputImage = itk::ReadImage<InputImageType>(argv[2]);


  // Software Guide : BeginLatex
  //
  // Landmarks correspondences may be associated with the
  // SplineKernelTransforms via Point Set containers. Let us define containers
  // for the landmarks.
  //
  // Software Guide : EndLatex

  // Define container for landmarks

  // Software Guide : BeginCodeSnippet
  auto      sourceLandMarks = PointSetType::New();
  auto      targetLandMarks = PointSetType::New();
  PointType p1;
  PointType p2;
  const PointSetType::PointsContainer::Pointer sourceLandMarkContainer =
    sourceLandMarks->GetPoints();
  const PointSetType::PointsContainer::Pointer targetLandMarkContainer =
    targetLandMarks->GetPoints();
  // Software Guide : EndCodeSnippet

  PointIdType id{};

  // Read in the list of landmarks
  std::ifstream infile;
  infile.open(argv[1]);
  while (!infile.eof())
  {
    infile >> p1[0] >> p1[1] >> p1[2] >> p2[0] >> p2[1] >> p2[2];

    // Software Guide : BeginCodeSnippet
    sourceLandMarkContainer->InsertElement(id, p1);
    targetLandMarkContainer->InsertElement(id++, p2);
    // Software Guide : EndCodeSnippet
  }
  infile.close();

  // Software Guide : BeginCodeSnippet
  auto tps = TransformType::New();
  tps->SetSourceLandmarks(sourceLandMarks);
  tps->SetTargetLandmarks(targetLandMarks);
  tps->ComputeWMatrix();
  // Software Guide : EndCodeSnippet

  // Software Guide : BeginLatex
  //
  // The image is then resampled to produce an output image as defined by the
  // transform. Here we use a LinearInterpolator.
  //
  // Software Guide : EndLatex

  // Set the resampler params
  auto resampler = ResamplerType::New();
  auto interpolator = InterpolatorType::New();
  resampler->SetInterpolator(interpolator);
  const InputImageType::SpacingType   spacing = inputImage->GetSpacing();
  const InputImageType::PointType     origin = inputImage->GetOrigin();
  const InputImageType::DirectionType direction = inputImage->GetDirection();
  const InputImageType::RegionType region = inputImage->GetBufferedRegion();
  const InputImageType::SizeType   size = region.GetSize();

  // Software Guide : BeginCodeSnippet
  resampler->SetOutputSpacing(spacing);
  resampler->SetOutputDirection(direction);
  resampler->SetOutputOrigin(origin);
  resampler->SetSize(size);
  resampler->SetTransform(tps);
  // Software Guide : EndCodeSnippet

  resampler->SetOutputStartIndex(region.GetIndex());
  resampler->SetInput(inputImage);

  // Set and write deformed image

  itk::WriteImage(resampler->GetOutput(), argv[3]);


  // Software Guide : BeginLatex
  //
  // The deformation field is computed as the difference between the input and
  // the deformed image by using an iterator.
  //
  // Software Guide : EndLatex

  // Compute the deformation field

  auto field = DisplacementFieldType::New();
  field->SetRegions(region);
  field->SetOrigin(origin);
  field->SetSpacing(spacing);
  field->Allocate();

  using FieldIterator = itk::ImageRegionIterator<DisplacementFieldType>;
  FieldIterator fi(field, region);
  fi.GoToBegin();
  TransformType::InputPointType    point1;
  TransformType::OutputPointType   point2;
  DisplacementFieldType::IndexType index;

  FieldVectorType displacement;
  while (!fi.IsAtEnd())
  {
    index = fi.GetIndex();
    field->TransformIndexToPhysicalPoint(index, point1);
    point2 = tps->TransformPoint(point1);
    for (unsigned int i = 0; i < ImageDimension; ++i)
    {
      displacement[i] = point2[i] - point1[i];
    }
    fi.Set(displacement);
    ++fi;
  }

  // Write computed deformation field

  itk::WriteImage(field, argv[4]);


  return EXIT_SUCCESS;
}
} // namespace
int
main(int argc, char * argv[])
{
  try
  {
    return ExampleMain(argc, argv);
  }
  catch (const itk::ExceptionObject & exceptionObject)
  {
    std::cerr << "ITK exception caught:\n" << exceptionObject << '\n';
  }
  catch (const std::exception & stdException)
  {
    std::cerr << "std exception caught:\n" << stdException.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Unhandled exception!\n";
  }
  return EXIT_FAILURE;
}
