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
#ifndef itkRegularizedHeavisideStepFunction_h
#define itkRegularizedHeavisideStepFunction_h

#include "itkHeavisideStepFunctionBase.h"

namespace itk
{
/** \class RegularizedHeavisideStepFunction
 *
 * \brief Base class of the Regularized (smoothed) Heaviside functions.
 *
 * \author Mosaliganti K., Smith B., Gelas A., Gouaillard A., Megason S.
 *
 *  This code was taken from the Insight Journal paper \cite Mosaliganti_2009_c
 *  that is based on the papers \cite Mosaliganti_2009_a and
 *  \cite  Mosaliganti_2009_b.
 *
 * \ingroup ITKCommon
 */
template <typename TInput = float, typename TOutput = double>
class ITK_TEMPLATE_EXPORT RegularizedHeavisideStepFunction : public HeavisideStepFunctionBase<TInput, TOutput>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(RegularizedHeavisideStepFunction);

  using Self = RegularizedHeavisideStepFunction;
  using Superclass = HeavisideStepFunctionBase<TInput, TOutput>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using typename Superclass::InputType;
  using typename Superclass::OutputType;

  using RealType = typename NumericTraits<InputType>::RealType;

  void
  SetEpsilon(const RealType & ieps);

  itkGetConstMacro(Epsilon, RealType);
  itkGetConstMacro(OneOverEpsilon, RealType);

protected:
  RegularizedHeavisideStepFunction() = default;
  ~RegularizedHeavisideStepFunction() override = default;

private:
  RealType m_Epsilon{ NumericTraits<RealType>::OneValue() };
  RealType m_OneOverEpsilon{ NumericTraits<RealType>::OneValue() };
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkRegularizedHeavisideStepFunction.hxx"
#endif

#endif
