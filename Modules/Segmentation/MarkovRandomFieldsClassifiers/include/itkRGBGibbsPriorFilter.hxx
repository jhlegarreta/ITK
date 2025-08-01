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
#ifndef itkRGBGibbsPriorFilter_hxx
#define itkRGBGibbsPriorFilter_hxx

#include "itkMakeUniqueForOverwrite.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>

#define RGBGibbsPriorFilterNeedsDebugging 1

namespace itk
{

template <typename TInputImage, typename TClassifiedImage>
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::RGBGibbsPriorFilter()
  : m_InputImage(nullptr)
  , m_TrainingImage(nullptr)
  , m_LabelledImage(nullptr)
  , m_ClassifierPtr(nullptr)
  , m_MediumImage(nullptr)
  , m_LowPoint()

{
  m_StartPoint.Fill(0);
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::SetLabelledImage(LabelledImageType image)
{
  m_LabelledImage = image;
  this->Allocate();
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::GenerateMediumImage()
{
  const InputImageConstPointer input = this->GetInput();

  m_MediumImage = TInputImage::New();
  m_MediumImage->SetLargestPossibleRegion(input->GetLargestPossibleRegion());
  m_MediumImage->SetRequestedRegionToLargestPossibleRegion();
  m_MediumImage->SetBufferedRegion(m_MediumImage->GetRequestedRegion());
  m_MediumImage->Allocate();
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::Allocate()
{
  // Get the image width/height and depth.
  InputImageSizeType inputImageSize = m_InputImage->GetBufferedRegion().GetSize();

  m_ImageWidth = inputImageSize[0];
  m_ImageHeight = inputImageSize[1];
  m_ImageDepth = inputImageSize[2];

  const unsigned int numberOfPixels = m_ImageWidth * m_ImageHeight * m_ImageDepth;
  m_LabelStatus = make_unique_for_overwrite<LabelType[]>(numberOfPixels);
  for (unsigned int index = 0; index < numberOfPixels; ++index)
  {
    m_LabelStatus[index] = 1;
  }
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::GreyScalarBoundary(LabelledImageIndexType Index3D)
{
  int       signs[4];
  LabelType neighbors[4] = { 0, 0, 0, 0 };

  for (unsigned int rgb = 0; rgb < m_VecDim; ++rgb)
  {
    auto         origin = static_cast<LabelType>(m_InputImage->GetPixel(Index3D)[rgb]);
    unsigned int j = 0;

    for (unsigned int i = 0; i < ImageDimension - 1; ++i)
    {
      Index3D[i]--;
      neighbors[j] = static_cast<LabelType>(m_InputImage->GetPixel(Index3D)[rgb]);
      Index3D[i]++;
      ++j;

      Index3D[i]++;
      neighbors[j] = static_cast<LabelType>(m_InputImage->GetPixel(Index3D)[rgb]);
      Index3D[i]--;
      ++j;
    }

    for (auto & sign : signs)
    {
      sign = 0;
    }

    // Compute the minimum points of piecewise smoothness.
    m_LowPoint[rgb] = origin;
    int change = 1;
    int x = origin;
    int numx = 1;

    while (change > 0)
    {
      change = 0;

      for (unsigned int i = 0; i < 4; ++i)
      {
        if (signs[i] == 0)
        {
          const auto difference = static_cast<LabelType>(itk::Math::abs(m_LowPoint[rgb] - neighbors[i]));
          if (difference < m_BoundaryGradient)
          {
            ++numx;
            x += neighbors[i];
            signs[i]++;
            ++change;
          }
        }
      }

      m_LowPoint[rgb] = x / numx;

      for (unsigned int i = 0; i < 4; ++i)
      {
        if (signs[i] == 1)
        {
          const auto difference = static_cast<LabelType>(itk::Math::abs(m_LowPoint[rgb] - neighbors[i]));
          if (difference > m_BoundaryGradient)
          {
            --numx;
            x -= neighbors[i];
            signs[i]--;
            ++change;
          }
        }
      }

      m_LowPoint[rgb] = x / numx;
    }
  }
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::SetClassifier(typename ClassifierType::Pointer ptrToClassifier)
{
  m_ClassifierPtr = ptrToClassifier;
  m_ClassifierPtr->SetNumberOfClasses(m_NumberOfClasses);
}

template <typename TInputImage, typename TClassifiedImage>
int
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::Sim(int a, int b)
{
  if (a == b)
  {
    return 1;
  }
  return 0;
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::GibbsTotalEnergy(int i)
{
  LabelledImageIndexType offsetIndex3D{};

  int size = m_ImageWidth * m_ImageHeight * m_ImageDepth;
  int frame = m_ImageWidth * m_ImageHeight;
  int rowsize = m_ImageWidth;

  double       energy[2];
  LabelType    eightNeighbors[8];
  unsigned int neighborcount = 0;

  offsetIndex3D[2] = i / frame;
  offsetIndex3D[1] = (i % frame) / rowsize;
  offsetIndex3D[0] = (i % frame) % rowsize;

  if ((i > rowsize - 1) && ((i % rowsize) != rowsize - 1) && (i < size - rowsize) && ((i % rowsize) != 0))
  {
    offsetIndex3D[0]--;
    offsetIndex3D[1]--;
    eightNeighbors[neighborcount++] = static_cast<int>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[0]++;
    eightNeighbors[neighborcount++] = static_cast<int>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[0]++;
    eightNeighbors[neighborcount++] = static_cast<int>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[1]++;
    eightNeighbors[neighborcount++] = static_cast<int>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[1]++;
    eightNeighbors[neighborcount++] = static_cast<int>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[0]--;
    eightNeighbors[neighborcount++] = static_cast<int>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[0]--;
    eightNeighbors[neighborcount++] = static_cast<int>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[1]--;
    eightNeighbors[neighborcount] = static_cast<int>(m_LabelledImage->GetPixel(offsetIndex3D));
  }

  unsigned int k = 0;
  for (unsigned int currentELement : eightNeighbors)
  {
    if (currentELement == m_ObjectLabel)
    {
      ++k;
    }
  }

  const bool changeflag = (k > 3);

  for (unsigned int jj = 0; jj < 2; ++jj)
  {
    energy[jj] = 0;
    energy[jj] += this->GibbsEnergy(i, 0, jj);
    energy[jj] += this->GibbsEnergy(i + rowsize + 1, 1, jj);
    energy[jj] += this->GibbsEnergy(i + rowsize, 2, jj);
    energy[jj] += this->GibbsEnergy(i + rowsize - 1, 3, jj);
    energy[jj] += this->GibbsEnergy(i - 1, 4, jj);
    energy[jj] += this->GibbsEnergy(i - rowsize - 1, 5, jj);
    energy[jj] += this->GibbsEnergy(i - rowsize, 6, jj);
    energy[jj] += this->GibbsEnergy(i - rowsize + 1, 7, jj);
    energy[jj] += this->GibbsEnergy(i + 1, 8, jj);
    if (m_LabelStatus[i] == jj)
    {
      energy[jj] += -3;
    }
    else
    {
      energy[jj] += 3;
    }
  }
  LabelType label = 1;
  if (energy[0] < energy[1])
  {
    label = 0;
  }

  LabelType originlabel = m_LabelledImage->GetPixel(offsetIndex3D);
  if (originlabel != label)
  {
    m_LabelledImage->SetPixel(offsetIndex3D, label);
  }

  else
  {
    if (changeflag)
    {
      double       difenergy = energy[label] - energy[1 - label];
      const double rand_num{ rand() / (RAND_MAX + 1.0) };
      double       energy_num{ std::exp(static_cast<double>(difenergy * 0.5 * size / (2 * size - m_Temp))) };
      if (rand_num < energy_num)
      {
        m_LabelledImage->SetPixel(offsetIndex3D, 1 - label);
      }
    }
  }
}

template <typename TInputImage, typename TClassifiedImage>
double
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::GibbsEnergy(unsigned int i, unsigned int k, unsigned int k1)
{
  LabelledImageRegionIterator labelledImageIt(m_LabelledImage, m_LabelledImage->GetBufferedRegion());

  LabelType    f[8];
  unsigned int neighborcount = 0;
  int          simnum = 0;
  int          changenum = 0;
  double       res = 0.0;

  LabelledImageIndexType offsetIndex3D{};

  LabelledImagePixelType labelledPixel = 0;

  const unsigned int size = m_ImageWidth * m_ImageHeight * m_ImageDepth;
  const unsigned int frame = m_ImageWidth * m_ImageHeight;
  const unsigned int rowsize = m_ImageWidth;

  offsetIndex3D[1] = (i % frame) / rowsize;
  offsetIndex3D[0] = (i % frame) % rowsize;

  if (k != 0)
  {
    labelledPixel = (LabelledImagePixelType)m_LabelledImage->GetPixel(offsetIndex3D);
  }

  if ((i > rowsize - 1) && ((i % rowsize) != rowsize - 1) && (i < size - rowsize) && ((i % rowsize) != 0))
  {
    offsetIndex3D[0]--;
    offsetIndex3D[1]--;
    f[neighborcount++] = static_cast<LabelType>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[0]++;
    f[neighborcount++] = static_cast<LabelType>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[0]++;
    f[neighborcount++] = static_cast<LabelType>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[1]++;
    f[neighborcount++] = static_cast<LabelType>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[1]++;
    f[neighborcount++] = static_cast<LabelType>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[0]--;
    f[neighborcount++] = static_cast<LabelType>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[0]--;
    f[neighborcount++] = static_cast<LabelType>(m_LabelledImage->GetPixel(offsetIndex3D));

    offsetIndex3D[1]--;
    f[neighborcount++] = static_cast<LabelType>(m_LabelledImage->GetPixel(offsetIndex3D));
  }

  // Pixels at the edge of image will be dropped.
  if (neighborcount != 8)
  {
    return 0.0;
  }

  if (k != 0)
  {
    f[k - 1] = k1;
  }
  else
  {
    labelledPixel = k1;
  }

  bool changeflag = (f[0] == labelledPixel);

  // Assuming we are segmenting objects with smooth boundaries, we give weight to local characteristics.
  for (unsigned int j = 0; j < 8; ++j)
  {
    if ((f[j] == labelledPixel) != changeflag)
    {
      ++changenum;
      changeflag = !changeflag;
    }

    if (changeflag)
    {
      if (j % 2 == 0)
      {
        res -= 0.7;
      }
      else
      {
        res -= 1.0;
      }
      ++simnum;
    }
  }

  if (changenum < 3)
  {
    if ((simnum == 4) || (simnum == 5))
    {
      return res -= m_CliqueWeight_2;
    }
  }

  if (simnum == 8)
  {
    return res -= m_CliqueWeight_4;
  }

  return res -= m_CliqueWeight_6;
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::GenerateData()
{
  // First run the Gaussian classifier calculator and generate the Gaussian model for the different classes. Then
  // generate the initial labelled dataset.

  m_InputImage = this->GetInput();
  m_VecDim = InputPixelType::GetVectorDimension();

  this->GenerateMediumImage();

  // Pass the input image and training image set to the classifier. For the first iteration, use the original image.
  // In the following loops, you can use the result provided by a segmentation method such as the deformable model.
  m_ClassifierPtr->SetInputImage(m_InputImage);

  // Create the training image using the original image or the output of a segmentation method such as the deformable
  // model
  // m_ClassifierPtr->SetTrainingImage( m_TrainingImage );

  // Run the Gaussian classifier algorithm.
  m_ClassifierPtr->Update();

  this->SetLabelledImage(m_ClassifierPtr->GetClassifiedImage());

  this->ApplyGPImageFilter();

  // Set the output labelled image and allocate the memory.
  const LabelledImageType outputPtr = this->GetOutput();

  if (m_RecursiveNumber == 0)
  {
    outputPtr->SetLargestPossibleRegion(m_InputImage->GetLargestPossibleRegion());
    outputPtr->SetBufferedRegion(m_InputImage->GetLargestPossibleRegion());
  }

  // Allocate the output buffer memory.
  outputPtr->Allocate();

  // Copy labelled result to the output buffer and set the iterators of the processed image.
  LabelledImageRegionIterator labelledImageIt(m_LabelledImage, m_LabelledImage->GetBufferedRegion());

  // Set the iterators of the output image buffer.
  LabelledImageRegionIterator outImageIt(outputPtr, outputPtr->GetBufferedRegion());

  while (!outImageIt.IsAtEnd())
  {
    auto labelvalue = (LabelledImagePixelType)labelledImageIt.Get();
    outImageIt.Set(labelvalue);
    ++labelledImageIt;
    ++outImageIt;
  }

  ++m_RecursiveNumber;
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::ApplyGPImageFilter()
{
  // Minimize f_1 and f_3.
  this->MinimizeFunctional();
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::MinimizeFunctional()
{
  // This implementation uses the SA algorithm.

  this->ApplyGibbsLabeller();

  this->RegionEraser();

#ifndef RGBGibbsPriorFilterNeedsDebugging
  const unsigned int size = m_ImageWidth * m_ImageHeight * m_ImageDepth;
  const unsigned int rowsize = m_ImageWidth;

  m_Temp = 0;
  srand(static_cast<unsigned int>(time(nullptr)));

  while (m_Temp < 2 * size)
  {
    unsigned int randomPixel = static_cast<unsigned int>(size * rand() / RAND_MAX);
    if ((randomPixel > (rowsize - 1)) && (randomPixel < (size - rowsize)) && (randomPixel % rowsize != 0) &&
        (randomPixel % rowsize != rowsize - 1))
    {
      this->GibbsTotalEnergy(randomPixel); // minimize f_2
    }
    ++m_Temp;
  }
#endif
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::ApplyGibbsLabeller()
{
  // Set the iterators and the pixel type definition for the input image.
  InputImageRegionConstIterator inputImageIt(m_InputImage, m_InputImage->GetBufferedRegion());

  InputImageRegionIterator mediumImageIt(m_MediumImage, m_MediumImage->GetBufferedRegion());

  // Set the iterators and the pixel type definition for the classified image.
  LabelledImageRegionIterator labelledImageIt(m_LabelledImage, m_LabelledImage->GetBufferedRegion());

  // Variable to store the origin pixel vector value.
  InputImagePixelType originPixelVec;

  // Variable to store the modified pixel vector value.
  InputImagePixelType changedPixelVec{};

  // Set a variable to store the offset index.
  LabelledImageIndexType offsetIndex3D{};

  const unsigned int size = m_ImageWidth * m_ImageHeight * m_ImageDepth;
  const unsigned int frame = m_ImageWidth * m_ImageHeight;
  const unsigned int rowsize = m_ImageWidth;

  inputImageIt.GoToBegin();
  mediumImageIt.GoToBegin();
  labelledImageIt.GoToBegin();

  unsigned int i = 0;
  while (!inputImageIt.IsAtEnd())
  {
    offsetIndex3D[2] = i / frame;
    offsetIndex3D[1] = (i % frame) / rowsize;
    offsetIndex3D[0] = (i % frame) % rowsize;

    if ((i > (rowsize - 1)) && (i < (size - rowsize)) && (i % rowsize != 0) && (i % rowsize != rowsize - 1))
    {
      originPixelVec = inputImageIt.Get();
      this->GreyScalarBoundary(offsetIndex3D);
      for (unsigned int rgb = 0; rgb < m_VecDim; ++rgb)
      {
        changedPixelVec[rgb] = m_LowPoint[rgb];
      }
      // mediumImageIt.Set(changedPixelVec);
    }
    else
    {
      changedPixelVec = inputImageIt.Get();
    }

    const std::vector<double> & distValue = m_ClassifierPtr->GetPixelMembershipValue(changedPixelVec);

    LabelType pixLabel = 1;
    if (distValue[1] > m_ObjectThreshold)
    {
      pixLabel = 0;
    }
    labelledImageIt.Set(pixLabel);

    ++i;
    ++labelledImageIt;
    ++inputImageIt;
    ++mediumImageIt;
  }
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::RegionEraser()
{
  const unsigned int size = m_ImageWidth * m_ImageHeight * m_ImageDepth;

  m_Region = std::make_unique<unsigned short[]>(size);
  m_RegionCount = make_unique_for_overwrite<unsigned short[]>(size);

  const auto valid_region_counter = std::make_unique<unsigned short[]>(size);

  LabelledImageRegionIterator labelledImageIt(m_LabelledImage, m_LabelledImage->GetBufferedRegion());

  for (unsigned int r = 0; r < size; ++r)
  {
    m_RegionCount[r] = 1;
  }

  LabelType i{};
  LabelType k{};
  LabelType l{};
  while (!labelledImageIt.IsAtEnd())
  {
    if (m_Region[i] == 0)
    {
      LabelType label = labelledImageIt.Get();
      if (this->LabelRegion(i, ++l, label) > m_ClusterSize)
      {
        valid_region_counter[k] = l;
        ++k;
      }
    }

    ++i;
    ++labelledImageIt;
  }

  i = 0;
  unsigned int j = 0;
  labelledImageIt.GoToBegin();

  while (!labelledImageIt.IsAtEnd())
  {
    j = 0;
    while ((j < k) && (m_Region[i] != valid_region_counter[j]))
    {
      ++j;
    }

    if (j == k)
    {
      LabelType label = labelledImageIt.Get();
      labelledImageIt.Set(1 - label);
    }
    ++i;
    ++labelledImageIt;
  }
}

template <typename TInputImage, typename TClassifiedImage>
unsigned int
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::LabelRegion(int i, int l, int change)
{
  unsigned int       count = 1;
  const unsigned int frame = m_ImageWidth * m_ImageHeight;
  const unsigned int rowsize = m_ImageWidth;

  LabelledImageIndexType offsetIndex3D{};

  m_Region[i] = l;

  offsetIndex3D[2] = i / frame;
  offsetIndex3D[1] = (i % frame) / rowsize;
  offsetIndex3D[0] = (i % frame) % rowsize;

  if (offsetIndex3D[0] > 0)
  {
    offsetIndex3D[0]--;
    int m = m_LabelledImage->GetPixel(offsetIndex3D);
    if ((m == change) && (m_Region[i - 1] == 0))
    {
      count += this->LabelRegion(i - 1, l, change);
    }
    offsetIndex3D[0]++;
  }

  if (offsetIndex3D[0] < static_cast<IndexValueType>(m_ImageWidth - 1))
  {
    offsetIndex3D[0]++;
    int m = m_LabelledImage->GetPixel(offsetIndex3D);
    if ((m == change) && (m_Region[i + 1] == 0))
    {
      count += this->LabelRegion(i + 1, l, change);
    }
    offsetIndex3D[0]--;
  }

  if (offsetIndex3D[1] > 0)
  {
    offsetIndex3D[1]--;
    int m = m_LabelledImage->GetPixel(offsetIndex3D);
    if ((m == change) && (m_Region[i - rowsize] == 0))
    {
      count += this->LabelRegion(i - rowsize, l, change);
    }
    offsetIndex3D[1]++;
  }

  if (offsetIndex3D[1] < static_cast<IndexValueType>(m_ImageHeight - 1))
  {
    offsetIndex3D[1]++;
    int m = m_LabelledImage->GetPixel(offsetIndex3D);
    if ((m == change) && (m_Region[i + rowsize] == 0))
    {
      count += this->LabelRegion(i + rowsize, l, change);
    }
    offsetIndex3D[1]--;
  }

  return count;
}

template <typename TInputImage, typename TClassifiedImage>
void
RGBGibbsPriorFilter<TInputImage, TClassifiedImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "NumberOfClasses: " << m_NumberOfClasses << std::endl;
  os << indent << "MaximumNumberOfIterations: " << m_MaximumNumberOfIterations << std::endl;
  os << indent << "ObjectThreshold: " << m_ObjectThreshold << std::endl;
  os << indent << "BoundaryGradient: " << m_BoundaryGradient << std::endl;
  os << indent << "CliqueWeight_1: " << m_CliqueWeight_1 << std::endl;
  os << indent << "CliqueWeight_2: " << m_CliqueWeight_2 << std::endl;
  os << indent << "CliqueWeight_3: " << m_CliqueWeight_3 << std::endl;
  os << indent << "CliqueWeight_4: " << m_CliqueWeight_4 << std::endl;
  os << indent << "CliqueWeight_5: " << m_CliqueWeight_5 << std::endl;
  os << indent << "CliqueWeight_6: " << m_CliqueWeight_6 << std::endl;
  os << indent << "ClusterSize: " << m_ClusterSize << std::endl;
  os << indent << "ObjectLabel: " << m_ObjectLabel << std::endl;
  os << indent << "StartPoint: " << m_StartPoint << std::endl;

  itkPrintSelfObjectMacro(TrainingImage);
  itkPrintSelfObjectMacro(LabelledImage);
  itkPrintSelfObjectMacro(ClassifierPtr);
}
} /* end namespace itk. */

#endif
