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
#ifndef itkRedColormapFunction_hxx
#define itkRedColormapFunction_hxx

namespace itk::Function
{
template <typename TScalar, typename TRGBPixel>
auto
RedColormapFunction<TScalar, TRGBPixel>::operator()(const TScalar & v) const -> RGBPixelType
{
  // Map the input scalar between [0, 1].
  const RealType value = this->RescaleInputValue(v);

  // Set the rgb components after rescaling the values.
  RGBPixelType pixel;
  NumericTraits<TRGBPixel>::SetLength(pixel, 3);

  pixel[0] = this->RescaleRGBComponentValue(value);
  pixel[1] = 0;
  pixel[2] = 0;

  return pixel;
}
} // namespace itk::Function

#endif
