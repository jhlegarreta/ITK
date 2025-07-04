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
#ifndef itkMaskFeaturePointSelectionFilter_hxx
#define itkMaskFeaturePointSelectionFilter_hxx


#include <map>
#include "vnl/vnl_trace.h"
#include "itkNeighborhood.h"
#include "itkNumericTraits.h"
#include "itkImageRegionIterator.h"
#include "itkCompensatedSummation.h"
#include "itkPrintHelper.h"


namespace itk
{
template <typename TImage, typename TMask, typename TFeatures>
MaskFeaturePointSelectionFilter<TImage, TMask, TFeatures>::MaskFeaturePointSelectionFilter()
  : m_NonConnectivity(Self::VERTEX_CONNECTIVITY)
  , m_SelectFraction(0.1)
  , m_ComputeStructureTensors(true)
{
  // default parameters
  m_BlockRadius.Fill(2);
}

template <typename TImage, typename TMask, typename TFeatures>
void
MaskFeaturePointSelectionFilter<TImage, TMask, TFeatures>::PrintSelf(std::ostream & os, Indent indent) const
{
  using namespace print_helper;

  Superclass::PrintSelf(os, indent);

  os << indent << "NonConnectivity: " << m_NonConnectivity << std::endl;
  os << indent << "NonConnectivityOffsets: " << m_NonConnectivityOffsets << std::endl;
  os << indent << "BlockRadius: " << static_cast<typename NumericTraits<SizeType>::PrintType>(m_BlockRadius)
     << std::endl;
  os << indent << "SelectFraction: " << m_SelectFraction << std::endl;
  itkPrintSelfBooleanMacro(ComputeStructureTensors);
}

template <typename TImage, typename TMask, typename TFeatures>
void
MaskFeaturePointSelectionFilter<TImage, TMask, TFeatures>::ComputeConnectivityOffsets()
{
  if (m_NonConnectivity < ImageDimension)
  {
    m_NonConnectivityOffsets.clear();
    // use Neighbourhood to compute all offsets in radius 1
    Neighborhood<unsigned int, ImageDimension> neighborhood;
    neighborhood.SetRadius(NumericTraits<SizeValueType>::OneValue());
    for (SizeValueType i = 0, n = neighborhood.Size(); i < n; ++i)
    {
      OffsetType off = neighborhood.GetOffset(i);

      // count 0s offsets in each dimension
      unsigned int numberOfZeros = 0;
      for (unsigned int j = 0; j < ImageDimension; ++j)
      {
        if (off[j] == 0)
        {
          ++numberOfZeros;
        }
      }

      if (m_NonConnectivity <= numberOfZeros && numberOfZeros < ImageDimension)
      {
        m_NonConnectivityOffsets.push_back(off);
      }
    }
  }
  else
  {
    itkExceptionMacro("Cannot use non-connectivity of value "
                      << m_NonConnectivity << ", expected a value in the range 0.." << ImageDimension - 1 << '.');
  }
}

template <typename TImage, typename TMask, typename TFeatures>
void
MaskFeaturePointSelectionFilter<TImage, TMask, TFeatures>::GenerateData()
{
  // generate non-connectivity offsets
  this->ComputeConnectivityOffsets();

  // fill inputs / outputs / misc
  const TImage *                  image = this->GetInput();
  RegionType                      region = image->GetLargestPossibleRegion();
  typename ImageType::SpacingType voxelSpacing = image->GetSpacing();

  const FeaturePointsPointer pointSet = this->GetOutput();

  using PointsContainer = typename FeaturePointsType::PointsContainer;
  using PointsContainerPointer = typename PointsContainer::Pointer;

  const PointsContainerPointer points = PointsContainer::New();

  using PointDataContainer = typename FeaturePointsType::PointDataContainer;
  using PointDataContainerPointer = typename PointDataContainer::Pointer;

  const PointDataContainerPointer pointData = PointDataContainer::New();

  // initialize selectionMap
  using MapPixelType = unsigned char;
  using SelectionMapType = Image<MapPixelType, ImageDimension>;
  auto selectionMap = SelectionMapType::New();

  // The selectionMap only needs to have the same pixel grid of the input image,
  // but do not have to care about origin, spacing or orientation.
  selectionMap->SetRegions(region);
  selectionMap->Allocate();

  const TMask * mask = this->GetMaskImage();

  if (mask == nullptr)
  {
    // create all 1s selectionMap
    selectionMap->FillBuffer(NumericTraits<MapPixelType>::OneValue());
  }
  else
  {
    // copy mask into selectionMap
    ImageRegionConstIterator<MaskType>    maskItr(mask, region);
    ImageRegionIterator<SelectionMapType> mapItr(selectionMap, region);
    for (maskItr.GoToBegin(), mapItr.GoToBegin(); !maskItr.IsAtEnd(); ++maskItr, ++mapItr)
    {
      mapItr.Set(static_cast<MapPixelType>(maskItr.Get()));
    }
  }

  // set safe region for picking feature points depending on whether tensors are computed
  IndexType safeIndex = region.GetIndex();
  SizeType  safeSize = region.GetSize();

  if (m_ComputeStructureTensors)
  {
    // tensor calculations access points in 2 X m_BlockRadius + 1 radius
    constexpr auto onesSize = SizeType::Filled(1);
    // Define the area in which tensors are going to be computed.
    const SizeType blockSize = m_BlockRadius + m_BlockRadius + onesSize;
    safeIndex += blockSize;
    safeSize -= blockSize + blockSize;
  }
  else
  {
    // variance calculations access points in m_BlockRadius radius
    safeIndex += m_BlockRadius;
    safeSize -= m_BlockRadius + m_BlockRadius;
  }

  region.SetIndex(safeIndex);
  region.SetSize(safeSize);

  // iterators for variance computing loop
  ImageRegionIterator<SelectionMapType> mapItr(selectionMap, region);
  ConstNeighborhoodIterator<ImageType>  imageItr(m_BlockRadius, image, region);
  using NeighborSizeType = typename ConstNeighborhoodIterator<ImageType>::NeighborIndexType;
  const NeighborSizeType numPixelsInNeighborhood = imageItr.Size();

  // sorted container for feature points, stores pair(variance, index)
  using MultiMapType = std::multimap<double, IndexType>;
  MultiMapType pointMap;

  // compute variance for eligible points
  for (imageItr.GoToBegin(), mapItr.GoToBegin(); !imageItr.IsAtEnd(); ++imageItr, ++mapItr)
  {
    if (mapItr.Get())
    {
      CompensatedSummation<double> sum;
      CompensatedSummation<double> sumOfSquares;
      for (NeighborSizeType i = 0; i < numPixelsInNeighborhood; ++i)
      {
        const ImagePixelType pixel = imageItr.GetPixel(i);
        sum += pixel;
        sumOfSquares += pixel * pixel;
      }

      const double mean = sum.GetSum() / numPixelsInNeighborhood;
      const double squaredMean = mean * mean;
      const double meanOfSquares = sumOfSquares.GetSum() / numPixelsInNeighborhood;

      const double variance = meanOfSquares - squaredMean;
      using PairType = typename MultiMapType::value_type;

      // we only insert blocks with variance > 0
      if (itk::NumericTraits<double>::IsPositive(variance))
      {
        pointMap.insert(PairType(variance, imageItr.GetIndex()));
      }
    }
  }

  // number of points to select
  IndexValueType       numberOfPointsInserted = -1; // initialize to -1
  const IndexValueType maxNumberPointsToInserted = Math::Floor<SizeValueType>(0.5 + pointMap.size() * m_SelectFraction);
  constexpr double     TRACE_EPSILON = 1e-8;

  // pick points with highest variance first (inverse iteration)
  auto rit = pointMap.rbegin();
  while (rit != pointMap.rend() && numberOfPointsInserted < maxNumberPointsToInserted)
  {
    // if point is not marked off in selection map and there are still points to be picked
    const IndexType & indexOfPointToPick = rit->second;

    // index should be inside the mask image (GetPixel = 1)
    if (selectionMap->GetPixel(indexOfPointToPick) && region.IsInside(indexOfPointToPick))
    {
      ++numberOfPointsInserted;
      // compute and add structure tensor into pointData
      if (m_ComputeStructureTensors)
      {
        StructureTensorType tensor{};

        Matrix<SpacePrecisionType, ImageDimension, 1> gradI; // vector declared as column matrix

        constexpr auto radius = SizeType::Filled(1); // iterate over neighbourhood of a voxel

        RegionType center;
        center.SetSize(radius);
        center.SetIndex(indexOfPointToPick);

        const SizeType                       neighborRadiusForTensor = m_BlockRadius + m_BlockRadius;
        ConstNeighborhoodIterator<ImageType> gradientItr(neighborRadiusForTensor, image, center);

        gradientItr.GoToBegin();

        // iterate over voxels in the neighbourhood
        for (SizeValueType i = 0; i < gradientItr.Size(); ++i)
        {
          const OffsetType off = gradientItr.GetOffset(i);

          for (unsigned int j = 0; j < ImageDimension; ++j)
          {
            OffsetType left = off;
            left[j] -= 1;

            OffsetType right = off;
            right[j] += 1;

            const ImagePixelType     leftPixelValue = image->GetPixel(gradientItr.GetIndex(left));
            const ImagePixelType     rightPixelValue = image->GetPixel(gradientItr.GetIndex(right));
            const SpacePrecisionType doubleSpacing = voxelSpacing[j] * 2.0;

            // using image GetPixel instead of iterator GetPixel since offsets might be outside of neighbourhood
            gradI(j, 0) = (leftPixelValue - rightPixelValue) / doubleSpacing;
          }

          // Compute tensor product of gradI with itself
          const vnl_matrix<SpacePrecisionType> tnspose{ gradI.GetTranspose().as_matrix() };
          const StructureTensorType            product(gradI * tnspose);
          tensor += product;
        }

        const double trace = vnl_trace(tensor.GetVnlMatrix());

        // trace should be non-zero
        if (itk::Math::abs(trace) < TRACE_EPSILON)
        {
          ++rit;
          --numberOfPointsInserted;
          continue;
        }

        tensor /= trace;
        pointData->InsertElement(numberOfPointsInserted, tensor);

      } // end of compute structure tensor

      // add the point to the container
      PointType point;
      image->TransformIndexToPhysicalPoint(indexOfPointToPick, point);
      points->InsertElement(numberOfPointsInserted, point);

      // mark off connected points
      constexpr MapPixelType ineligeblePointCode = 0;
      for (const auto & m_NonConnectivityOffset : m_NonConnectivityOffsets)
      {
        IndexType idx = rit->second;
        idx += m_NonConnectivityOffset;
        selectionMap->SetPixel(idx, ineligeblePointCode);
      }
    }
    ++rit;
  }
  // set points
  pointSet->SetPoints(points);
  pointSet->SetPointData(pointData);
}
} // end namespace itk

#endif
