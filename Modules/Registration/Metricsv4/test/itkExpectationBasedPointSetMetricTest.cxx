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

#include "itkExpectationBasedPointSetToPointSetMetricv4.h"
#include "itkTranslationTransform.h"

#include <fstream>
#include "itkMath.h"
#include "itkTestingMacros.h"

template <unsigned int Dimension>
int
itkExpectationBasedPointSetMetricTestRun()
{
  using PointSetType = itk::PointSet<unsigned char, Dimension>;

  using PointType = typename PointSetType::PointType;

  auto fixedPoints = PointSetType::New();

  auto movingPoints = PointSetType::New();

  // Produce two simple point sets of 1) a circle and 2) the same circle with an offset;
  PointType offset;
  for (unsigned int d = 0; d < Dimension; ++d)
  {
    offset[d] = 2;
  }
  unsigned long count = 0;
  for (float theta = 0; theta < 2.0 * itk::Math::pi; theta += 0.1)
  {
    PointType       fixedPoint;
    constexpr float radius = 100.0;
    fixedPoint[0] = radius * std::cos(theta);
    fixedPoint[1] = radius * std::sin(theta);
    if constexpr (Dimension > 2)
    {
      fixedPoint[2] = radius * std::sin(theta);
    }
    fixedPoints->SetPoint(count, fixedPoint);

    PointType movingPoint;
    movingPoint[0] = fixedPoint[0] + offset[0];
    movingPoint[1] = fixedPoint[1] + offset[1];
    if constexpr (Dimension > 2)
    {
      movingPoint[2] = fixedPoint[2] + offset[2];
    }
    movingPoints->SetPoint(count, movingPoint);

    count++;
  }

  // Simple translation transform for moving point set
  using TranslationTransformType = itk::TranslationTransform<double, Dimension>;
  auto translationTransform = TranslationTransformType::New();
  translationTransform->SetIdentity();

  // Instantiate the metric
  using PointSetMetricType = itk::ExpectationBasedPointSetToPointSetMetricv4<PointSetType>;
  auto metric = PointSetMetricType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(metric, ExpectationBasedPointSetToPointSetMetricv4, PointSetToPointSetMetricv4);

  const typename PointSetMetricType::CoordinateType pointSetSigma = 1.0;
  metric->SetPointSetSigma(pointSetSigma);
  ITK_TEST_SET_GET_VALUE(pointSetSigma, metric->GetPointSetSigma());

  constexpr unsigned int evaluationKNeighborhood = 50;
  metric->SetEvaluationKNeighborhood(evaluationKNeighborhood);
  ITK_TEST_SET_GET_VALUE(evaluationKNeighborhood, metric->GetEvaluationKNeighborhood());

  metric->SetFixedPointSet(fixedPoints);
  metric->SetMovingPointSet(movingPoints);
  metric->SetMovingTransform(translationTransform);
  metric->Initialize();

  const typename PointSetMetricType::MeasureType value = metric->GetValue();
  typename PointSetMetricType::DerivativeType    derivative;
  metric->GetDerivative(derivative);

  typename PointSetMetricType::MeasureType    value2;
  typename PointSetMetricType::DerivativeType derivative2;
  metric->GetValueAndDerivative(value2, derivative2);

  int result = EXIT_SUCCESS;
  std::cout << "value: " << value << std::endl;
  std::cout << "derivative: " << derivative << std::endl;
  for (unsigned int d = 0; d < metric->GetNumberOfParameters(); ++d)
  {
    if (itk::Math::abs(derivative[d] - offset[d]) / offset[d] > 0.01)
    {
      std::cerr << "derivative does not match expected offset of " << offset << std::endl;
      result = EXIT_FAILURE;
    }
  }

  // Check for the same results from different methods
  if (itk::Math::NotExactlyEquals(value, value2))
  {
    std::cerr << "value does not match between calls to different methods: "
              << "value: " << value << " value2: " << value2 << std::endl;
    result = EXIT_FAILURE;
  }
  if (derivative != derivative2)
  {
    std::cerr << "derivative does not match between calls to different methods: "
              << "derivative: " << derivative << " derivative2: " << derivative2 << std::endl;
    result = EXIT_FAILURE;
  }

  std::ofstream moving_str1("sourceMoving.txt");
  std::ofstream moving_str2("targetMoving.txt");

  count = 0;

  moving_str1 << "0 0 0 0" << std::endl;
  moving_str2 << "0 0 0 0" << std::endl;

  typename PointType::VectorType vector;
  for (unsigned int d = 0; d < metric->GetNumberOfParameters(); ++d)
  {
    vector[d] = derivative[count++];
  }

  typename PointSetType::PointsContainer::ConstIterator ItM = movingPoints->GetPoints()->Begin();
  while (ItM != movingPoints->GetPoints()->End())
  {
    PointType sourcePoint = ItM.Value();
    PointType targetPoint = sourcePoint + vector;

    for (unsigned int d = 0; d < metric->GetNumberOfParameters(); ++d)
    {
      moving_str1 << sourcePoint[d] << ' ';
      moving_str2 << targetPoint[d] << ' ';
    }
    if constexpr (Dimension < 3)
    {
      moving_str1 << "0 ";
      moving_str2 << "0 ";
    }
    moving_str1 << ItM.Index() << std::endl;
    moving_str2 << ItM.Index() << std::endl;

    ++ItM;
  }

  moving_str1 << "0 0 0 0" << std::endl;
  moving_str2 << "0 0 0 0" << std::endl;

  return result;
}

int
itkExpectationBasedPointSetMetricTest(int, char *[])
{
  int result = EXIT_SUCCESS;

  if (itkExpectationBasedPointSetMetricTestRun<2>() == EXIT_FAILURE)
  {
    std::cerr << "Failed for Dimension 2." << std::endl;
    result = EXIT_FAILURE;
  }

  if (itkExpectationBasedPointSetMetricTestRun<3>() == EXIT_FAILURE)
  {
    std::cerr << "Failed for Dimension 3." << std::endl;
    result = EXIT_FAILURE;
  }

  return result;
}
