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
#ifndef itkCoxDeBoorBSplineKernelFunction_hxx
#define itkCoxDeBoorBSplineKernelFunction_hxx

#include "itkMath.h"

namespace itk
{
template <unsigned int VSplineOrder, typename TRealValueType>
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::CoxDeBoorBSplineKernelFunction()
  : m_SplineOrder(VSplineOrder)
{
  this->GenerateBSplineShapeFunctions(this->m_SplineOrder + 1);
}

template <unsigned int VSplineOrder, typename TRealValueType>
void
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::SetSplineOrder(const unsigned int order)
{
  if (order != this->m_SplineOrder)
  {
    this->m_SplineOrder = order;
    this->GenerateBSplineShapeFunctions(this->m_SplineOrder + 1);
    this->Modified();
  }
}

template <unsigned int VSplineOrder, typename TRealValueType>
void
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::GenerateBSplineShapeFunctions(const unsigned int order)
{
  const auto numberOfPieces = static_cast<unsigned int>(TRealValueType{ 0.5 } * (order + 1));

  this->m_BSplineShapeFunctions.set_size(numberOfPieces, order);

  VectorType knots(order + 1);
  for (unsigned int i = 0; i < knots.size(); ++i)
  {
    knots[i] = TRealValueType{ -0.5 } * static_cast<TRealValueType>(order) + static_cast<TRealValueType>(i);
  }

  for (unsigned int i = 0; i < numberOfPieces; ++i)
  {
    const PolynomialType poly =
      this->CoxDeBoor(order, knots, 0, static_cast<unsigned int>(TRealValueType{ 0.5 } * (order)) + i);
    this->m_BSplineShapeFunctions.set_row(i, poly.coefficients());
  }
}

template <unsigned int VSplineOrder, typename TRealValueType>
auto
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::CoxDeBoor(const unsigned short order,
                                                                        const VectorType     knots,
                                                                        const unsigned int   whichBasisFunction,
                                                                        const unsigned int whichPiece) -> PolynomialType
{
  VectorType           tmp(2);
  PolynomialType       poly1(TRealValueType{ 0.0 });
  PolynomialType       poly2(TRealValueType{ 0.0 });
  const unsigned short p = order - 1;
  const unsigned short i = whichBasisFunction;

  if (p == 0 && whichBasisFunction == whichPiece)
  {
    PolynomialType poly(TRealValueType{ 1.0 });
    return poly;
  }

  // Term 1
  TRealValueType den = knots(i + p) - knots(i);

  if (itk::Math::AlmostEquals(den, TRealValueType{ 0.0 }))
  {
    const PolynomialType poly(TRealValueType{ 0.0 });
    poly1 = poly;
  }
  else
  {
    tmp(0) = TRealValueType{ 1.0 };
    tmp(1) = -knots(i);
    tmp /= den;
    poly1 = PolynomialType(tmp) * this->CoxDeBoor(order - 1, knots, i, whichPiece);
  }

  // Term 2
  den = knots(i + p + 1) - knots(i + 1);
  if (itk::Math::AlmostEquals(den, TRealValueType{ 0.0 }))
  {
    const PolynomialType poly(TRealValueType{ 0.0 });
    poly2 = poly;
  }
  else
  {
    tmp(0) = TRealValueType{ -1.0 };
    tmp(1) = knots(i + p + 1);
    tmp /= den;
    poly2 = PolynomialType(tmp) * this->CoxDeBoor(order - 1, knots, i + 1, whichPiece);
  }
  return (poly1 + poly2);
}

template <unsigned int VSplineOrder, typename TRealValueType>
auto
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::GetShapeFunctionsInZeroToOneInterval() -> MatrixType
{
  const int  order = this->m_SplineOrder + 1;
  const auto numberOfPieces = static_cast<unsigned int>(order);
  MatrixType shapeFunctions(numberOfPieces, order);

  VectorType knots(2 * order);

  for (unsigned int i = 0; i < knots.size(); ++i)
  {
    knots[i] = -static_cast<TRealValueType>(this->m_SplineOrder) + static_cast<TRealValueType>(i);
  }

  for (unsigned int i = 0; i < numberOfPieces; ++i)
  {
    const PolynomialType poly = this->CoxDeBoor(order, knots, i, order - 1);
    shapeFunctions.set_row(i, poly.coefficients());
  }
  return shapeFunctions;
}

template <unsigned int VSplineOrder, typename TRealValueType>
auto
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::GetShapeFunctions() -> MatrixType
{
  return this->m_BSplineShapeFunctions;
}


template <unsigned int VSplineOrder, typename TRealValueType>
TRealValueType
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::Evaluate(const TRealValueType & u) const
{
  const TRealValueType absValue = itk::Math::abs(u);

  auto which = static_cast<unsigned int>(absValue);
  if (this->m_SplineOrder % 2 == 0)
  {
    which = static_cast<unsigned int>(absValue + TRealValueType{ 0.5 });
  }

  if (which < this->m_BSplineShapeFunctions.rows())
  {
    return PolynomialType(this->m_BSplineShapeFunctions.get_row(which)).evaluate(absValue);
  }

  return TRealValueType{ 0.0 };
}

template <unsigned int VSplineOrder, typename TRealValueType>
TRealValueType
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::EvaluateDerivative(const TRealValueType & u) const
{
  return this->EvaluateNthDerivative(u, 1);
}

template <unsigned int VSplineOrder, typename TRealValueType>
TRealValueType
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::EvaluateNthDerivative(const TRealValueType & u,
                                                                                    const unsigned int     n) const
{
  const TRealValueType absValue = itk::Math::abs(u);

  auto which = static_cast<unsigned int>(absValue);
  if (this->m_SplineOrder % 2 == 0)
  {
    which = static_cast<unsigned int>(absValue + TRealValueType{ 0.5 });
  }

  if (which < this->m_BSplineShapeFunctions.rows())
  {
    PolynomialType polynomial(this->m_BSplineShapeFunctions.get_row(which));
    for (unsigned int i = 0; i < n; ++i)
    {
      polynomial = polynomial.derivative();
    }
    const TRealValueType der = polynomial.evaluate(absValue);
    if (u < TRealValueType{ 0.0 } && n % 2 != 0)
    {
      return -der;
    }

    return der;
  }
  else
  {
    return TRealValueType{ 0.0 };
  }
}

template <unsigned int VSplineOrder, typename TRealValueType>
void
CoxDeBoorBSplineKernelFunction<VSplineOrder, TRealValueType>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "BSplineShapeFunctions: " << std::endl;

  TRealValueType a{ 0.0 };
  TRealValueType b{ 0.0 };

  for (unsigned int i = 0; i < this->m_BSplineShapeFunctions.rows(); ++i)
  {
    os << indent << indent;

    PolynomialType(this->m_BSplineShapeFunctions.get_row(i)).print(os);

    if (i == 0)
    {
      if (this->m_SplineOrder % 2 == 0)
      {
        b = TRealValueType{ 0.5 };
      }
      else
      {
        b = TRealValueType{ 1.0 };
      }
    }
    else
    {
      a = b;
      b += TRealValueType{ 1.0 };
    }

    os << ",  X \\in [" << a << ", " << b << ']' << std::endl;
  }

  os << indent << "SplineOrder: " << m_SplineOrder << std::endl;
}
} // namespace itk

#endif
