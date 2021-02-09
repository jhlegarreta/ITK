/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
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
#include "itkLSMImageIO.h"
#include "itkTestingMacros.h"


// Specific ImageIO test

int
itkLSMImageIOTest(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Missing Parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " inputFileName(*.lsm) ouputFileName(*.lsm)"
              << std::endl;
    return EXIT_FAILURE;
  }

  const unsigned int Dimensions = 2;

  using InputPixelType = itk::RGBPixel<unsigned char>;
  using InputImageType = itk::Image<InputPixelType, Dimensions>;
  using ReaderType = itk::ImageFileReader<InputImageType>;
  using ImageIOType = itk::LSMImageIO;

  ReaderType::Pointer reader = ReaderType::New();
  const char *        inputFileName = argv[1];
  reader->SetFileName(inputFileName);

  ImageIOType::Pointer lsmImageIO = ImageIOType::New();

  EXERCISE_BASIC_OBJECT_METHODS(lsmImageIO, LSMImageIO, TIFFImageIO);


  reader->SetImageIO(lsmImageIO);

  ITK_TRY_EXPECT_NO_EXCEPTION(reader->Update());


  void * buffer = nullptr;

  lsmImageIO->Read(buffer);


  lsmImageIO->SetFileName(outFileName);
  lsmImageIO->Write(buffer);

  using WriterType = itk::ImageFileWriter<InputImageType>;
  WriterType::Pointer writer = WriterType::New();
  const char *        outFileName = argv[2];
  writer->SetFileName(outFileName);
  writer->SetInput(reader->GetOutput());

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());


  std::cout << "Test finished." << std::endl;
  return EXIT_SUCCESS;
}
