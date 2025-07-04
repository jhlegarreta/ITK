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
#ifndef itkGrayscaleFunctionErodeImageFilter_hxx
#define itkGrayscaleFunctionErodeImageFilter_hxx


namespace itk
{
template <typename TInputImage, typename TOutputImage, typename TKernel>
GrayscaleFunctionErodeImageFilter<TInputImage, TOutputImage, TKernel>::GrayscaleFunctionErodeImageFilter()
{
  m_ErodeBoundaryCondition.SetConstant(NumericTraits<PixelType>::max());
  this->OverrideBoundaryCondition(&m_ErodeBoundaryCondition);
}

template <typename TInputImage, typename TOutputImage, typename TKernel>
auto
GrayscaleFunctionErodeImageFilter<TInputImage, TOutputImage, TKernel>::Evaluate(const NeighborhoodIteratorType & nit,
                                                                                const KernelIteratorType kernelBegin,
                                                                                const KernelIteratorType kernelEnd)
  -> PixelType
{
  PixelType min = NumericTraits<PixelType>::max();

  KernelIteratorType kernel_it = kernelBegin;

  for (unsigned int i = 0; kernel_it < kernelEnd; ++kernel_it, ++i)
  {
    // if structuring element is positive, use the pixel under that element
    // in the image minus the structuring element value
    if (*kernel_it > KernelPixelType{})
    {
      // subtract the structuring element value to the pixel value,
      // note we use GetPixel() on SmartNeighborhoodIterator to respect
      // boundary condition
      const PixelType temp = nit.GetPixel(i) - (PixelType)*kernel_it;
      if (temp < min)
      {
        min = temp;
      }
    }
  }

  return min;
}
} // end namespace itk
#endif
