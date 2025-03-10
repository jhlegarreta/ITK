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

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSimpleFilterWatcher.h"

#include "itkSaltAndPepperNoiseImageFilter.h"
#include "itkTestingMacros.h"

int
itkSaltAndPepperNoiseImageFilterTest(int argc, char * argv[])
{

  if (argc < 3)
  {
    std::cerr << "Missing Parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " inputImage outputImage Probability" << std::endl;
    return EXIT_FAILURE;
  }

  constexpr unsigned int Dimension = 2;

  using PixelType = unsigned char;
  using ImageType = itk::Image<PixelType, Dimension>;

  using ReaderType = itk::ImageFileReader<ImageType>;
  auto reader = ReaderType::New();
  reader->SetFileName(argv[1]);

  using SaltAndPepperNoiseImageFilterType = itk::SaltAndPepperNoiseImageFilter<ImageType, ImageType>;
  auto saltAndPepperNoiseImageFilter = SaltAndPepperNoiseImageFilterType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(saltAndPepperNoiseImageFilter, SaltAndPepperNoiseImageFilter, NoiseBaseImageFilter);

  double probability = 0.01;
  if (argc >= 4)
  {
    probability = std::stod(argv[3]);
  }
  saltAndPepperNoiseImageFilter->SetProbability(probability);
  ITK_TEST_SET_GET_VALUE(probability, saltAndPepperNoiseImageFilter->GetProbability());

  // change the default values and then set back to defaults so that
  // the original test image is still valid.
  constexpr PixelType saltValue = 245;
  saltAndPepperNoiseImageFilter->SetSaltValue(saltValue);
  ITK_TEST_SET_GET_VALUE(saltValue, saltAndPepperNoiseImageFilter->GetSaltValue());
  constexpr PixelType pepperValue = 10;
  saltAndPepperNoiseImageFilter->SetPepperValue(pepperValue);
  ITK_TEST_SET_GET_VALUE(pepperValue, saltAndPepperNoiseImageFilter->GetPepperValue());
  saltAndPepperNoiseImageFilter->SetSaltValue(itk::NumericTraits<PixelType>::max());
  saltAndPepperNoiseImageFilter->SetPepperValue(itk::NumericTraits<PixelType>::NonpositiveMin());

  saltAndPepperNoiseImageFilter->SetInput(reader->GetOutput());

  const itk::SimpleFilterWatcher watcher(saltAndPepperNoiseImageFilter, "SaltAndPepperNoiseImageFilter");

  using WriterType = itk::ImageFileWriter<ImageType>;
  auto writer = WriterType::New();
  writer->SetInput(saltAndPepperNoiseImageFilter->GetOutput());
  writer->SetFileName(argv[2]);

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());


  std::cout << "Test finished" << std::endl;
  return EXIT_SUCCESS;
}
