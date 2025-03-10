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
#include "itkGDCMImageIO.h"
#include "itkVersor.h"
#include "itkTestingMacros.h"

// Specific ImageIO test

/** This test verifies that the direction cosines
 *  computed in itkGDCMImageIO are orthogonal
 */
int
itkGDCMImageIOOrthoDirTest(int argc, char * argv[])
{

  if (argc < 2)
  {
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " DicomImage\n";
    return EXIT_FAILURE;
  }

  using InputPixelType = short;
  using InputImageType = itk::Image<InputPixelType, 3>;
  using ReaderType = itk::ImageFileReader<InputImageType>;
  using ImageIOType = itk::GDCMImageIO;

  auto dcmImageIO = ImageIOType::New();

  auto reader = ReaderType::New();
  reader->SetFileName(argv[1]);
  reader->SetImageIO(dcmImageIO);

  try
  {
    reader->Update();
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cerr << "exception in file reader " << std::endl;
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  const InputImageType::DirectionType directionCosines = reader->GetOutput()->GetDirection();

  std::cout << "Dir Cosines " << directionCosines;

  itk::Versor<itk::SpacePrecisionType> rotation;

  try
  {
    rotation.Set(directionCosines);
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cerr << "exception setting matrix" << std::endl;
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
