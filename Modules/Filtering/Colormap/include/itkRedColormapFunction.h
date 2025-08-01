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
#ifndef itkRedColormapFunction_h
#define itkRedColormapFunction_h

#include "itkColormapFunction.h"

namespace itk::Function
{
/**
 * \class RedColormapFunction
 * \brief Function object which maps a scalar value into an RGB colormap value.
 *
 * \image html RedColormapFunction.png "Red colormap."
 *
 * \author Nicholas Tustison, Hui Zhang, Gaetan Lehmann, Paul Yushkevich
 * and James C. Gee
 *
 * This code was contributed in the Insight Journal paper:
 *
 * "Meeting Andy Warhol Somewhere Over the Rainbow: RGB Colormapping and ITK"
 * https://doi.org/10.54294/tmojbk
 *
 * \ingroup ITKColormap
 */
template <typename TScalar, typename TRGBPixel>
class ITK_TEMPLATE_EXPORT RedColormapFunction : public ColormapFunction<TScalar, TRGBPixel>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(RedColormapFunction);

  using Self = RedColormapFunction;
  using Superclass = ColormapFunction<TScalar, TRGBPixel>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  using typename Superclass::RGBPixelType;
  using typename Superclass::ScalarType;
  using typename Superclass::RealType;

  RGBPixelType
  operator()(const TScalar &) const override;

protected:
  RedColormapFunction() = default;
  ~RedColormapFunction() override = default;
};
} // namespace itk::Function

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkRedColormapFunction.hxx"
#endif

#endif
