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

/**
 *
 *  This program illustrates the use of VersorsRigid3DTransform
 *
 *  Versors are Unit Quaternions used to represent rotations.
 *  VersorTransform is a Rigid 3D Transform that support
 *  Versors and Vectors in its interface.
 *
 */

#include "itkVersorTransform.h"
#include <iostream>

// -------------------------
//
//   Main code
//
// -------------------------
int
itkVersorTransformTest(int, char *[])
{

  using ValueType = double;

  constexpr ValueType epsilon = 1e-12;

  //  Versor Transform type
  using TransformType = itk::VersorTransform<ValueType>;

  //  Versor type
  using VersorType = TransformType::VersorType;

  //  Vector type
  using VectorType = TransformType::InputVectorType;

  //  Parameters type
  using ParametersType = TransformType::ParametersType;

  //  Jacobian type
  using JacobianType = TransformType::JacobianType;

  //  Rotation Matrix type
  using MatrixType = TransformType::MatrixType;

  {
    std::cout << "Test default constructor... ";

    auto transform = TransformType::New();

    auto axis = itk::MakeFilled<VectorType>(1.5);

    const ValueType angle = 120.0 * std::atan(1.0) / 45.0;

    VersorType versor;
    versor.Set(axis, angle);

    ParametersType parameters(transform->GetNumberOfParameters()); // Number of parameters

    parameters[0] = versor.GetX();
    parameters[1] = versor.GetY();
    parameters[2] = versor.GetZ();

    transform->SetParameters(parameters);

    std::cout << " PASSED !" << std::endl;
  }

  {
    std::cout << "Test initial rotation matrix " << std::endl;
    auto             transform = TransformType::New();
    const MatrixType matrix = transform->GetMatrix();
    std::cout << "Matrix = " << std::endl;
    std::cout << matrix << std::endl;
  }

  /* Create a Rigid 3D transform with rotation */

  {
    bool Ok = true;

    auto rotation = TransformType::New();

    auto axis = itk::MakeFilled<itk::Vector<double, 3>>(1);

    const double angle = (std::atan(1.0) / 45.0) * 120.0; // turn 120 degrees

    // this rotation will permute the axis x->y, y->z, z->x
    rotation->SetRotation(axis, angle);

    TransformType::OffsetType offset = rotation->GetOffset();
    std::cout << "pure Rotation test:  ";
    std::cout << offset << std::endl;
    for (unsigned int i = 0; i < 3; ++i)
    {
      if (itk::Math::abs(offset[i] - 0.0) > epsilon)
      {
        Ok = false;
        break;
      }
    }

    if (!Ok)
    {
      std::cerr << "Get Offset  differs from null in rotation " << std::endl;
      return EXIT_FAILURE;
    }

    VersorType versor;
    versor.Set(axis, angle);

    {
      // Rotate an itk::Point
      constexpr TransformType::InputPointType::ValueType pInit[3] = { 1, 4, 9 };
      const TransformType::InputPointType                p = pInit;
      TransformType::OutputPointType                     q;
      q = versor.Transform(p);

      TransformType::OutputPointType r;
      r = rotation->TransformPoint(p);
      for (unsigned int i = 0; i < 3; ++i)
      {
        if (itk::Math::abs(q[i] - r[i]) > epsilon)
        {
          Ok = false;
          break;
        }
      }
      if (!Ok)
      {
        std::cerr << "Error rotating point : " << p << std::endl;
        std::cerr << "Result should be     : " << q << std::endl;
        std::cerr << "Reported Result is   : " << r << std::endl;
        return EXIT_FAILURE;
      }

      std::cout << "Ok rotating an itk::Point " << std::endl;
    }

    {
      // Translate an itk::Vector
      TransformType::InputVectorType::ValueType pInit[3] = { 1, 4, 9 };
      const TransformType::InputVectorType      p = pInit;
      TransformType::OutputVectorType           q;
      q = versor.Transform(p);

      TransformType::OutputVectorType r;
      r = rotation->TransformVector(p);
      for (unsigned int i = 0; i < 3; ++i)
      {
        if (itk::Math::abs(q[i] - r[i]) > epsilon)
        {
          Ok = false;
          break;
        }
      }
      if (!Ok)
      {
        std::cerr << "Error rotating vector : " << p << std::endl;
        std::cerr << "Result should be      : " << q << std::endl;
        std::cerr << "Reported Result is    : " << r << std::endl;
        return EXIT_FAILURE;
      }

      std::cout << "Ok rotating an itk::Vector " << std::endl;
    }

    {
      // Translate an itk::CovariantVector
      TransformType::InputCovariantVectorType::ValueType pInit[3] = { 1, 4, 9 };
      const TransformType::InputCovariantVectorType      p = pInit;
      TransformType::OutputCovariantVectorType           q;
      q = versor.Transform(p);

      TransformType::OutputCovariantVectorType r;
      r = rotation->TransformCovariantVector(p);
      for (unsigned int i = 0; i < 3; ++i)
      {
        if (itk::Math::abs(q[i] - r[i]) > epsilon)
        {
          Ok = false;
          break;
        }
      }
      if (!Ok)
      {
        std::cerr << "Error rotating covariant vector : " << p << std::endl;
        std::cerr << "Result should be                : " << q << std::endl;
        std::cerr << "Reported Result is              : " << r << std::endl;
        return EXIT_FAILURE;
      }

      std::cout << "Ok rotating an itk::CovariantVector " << std::endl;
    }

    {
      // Translate a vnl_vector
      TransformType::InputVnlVectorType p;
      p[0] = 1;
      p[1] = 4;
      p[2] = 9;

      TransformType::OutputVnlVectorType q;
      q = versor.Transform(p);

      TransformType::OutputVnlVectorType r;
      r = rotation->TransformVector(p);
      for (unsigned int i = 0; i < 3; ++i)
      {
        if (itk::Math::abs(q[i] - r[i]) > epsilon)
        {
          Ok = false;
          break;
        }
      }
      if (!Ok)
      {
        std::cerr << "Error rotating vnl_vector : " << p << std::endl;
        std::cerr << "Result should be          : " << q << std::endl;
        std::cerr << "Reported Result is        : " << r << std::endl;
        return EXIT_FAILURE;
      }

      std::cout << "Ok rotating an vnl_Vector " << std::endl;
    }
  }

  /**  Exercise the SetCenter method  */
  {
    bool Ok = true;

    auto transform = TransformType::New();

    auto axis = itk::MakeFilled<itk::Vector<double, 3>>(1);

    const double angle = (std::atan(1.0) / 45.0) * 30.0; // turn 30 degrees

    transform->SetRotation(axis, angle);

    TransformType::InputPointType center;
    center[0] = 31;
    center[1] = 62;
    center[2] = 93;

    transform->SetCenter(center);

    TransformType::OutputPointType transformedPoint;
    transformedPoint = transform->TransformPoint(center);
    for (unsigned int i = 0; i < 3; ++i)
    {
      if (itk::Math::abs(center[i] - transformedPoint[i]) > epsilon)
      {
        Ok = false;
        break;
      }
    }

    if (!Ok)
    {
      std::cerr << "The center point was not invariant to rotation " << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << "Ok center is invariant to rotation." << std::endl;


    const unsigned int np = transform->GetNumberOfParameters();

    ParametersType parameters(np); // Number of parameters

    constexpr VersorType versor;

    parameters[0] = versor.GetX(); // Rotation axis * std::sin(t/2)
    parameters[1] = versor.GetY();
    parameters[2] = versor.GetZ();

    transform->SetParameters(parameters);

    ParametersType parameters2 = transform->GetParameters();

    constexpr double tolerance = 1e-8;
    for (unsigned int p = 0; p < np; ++p)
    {
      if (itk::Math::abs(parameters[p] - parameters2[p]) > tolerance)
      {
        std::cerr << "Output parameter does not match input " << std::endl;
        return EXIT_FAILURE;
      }
    }
    std::cout << "Input/Output parameter check Passed !" << std::endl;

    // Try the ComputeJacobianWithRespectToParameters method
    TransformType::InputPointType aPoint;
    aPoint[0] = 10.0;
    aPoint[1] = 20.0;
    aPoint[2] = -10.0;
    JacobianType jacobian;
    transform->ComputeJacobianWithRespectToParameters(aPoint, jacobian);
    std::cout << "Jacobian: " << std::endl;
    std::cout << jacobian << std::endl;

    // copy the read one just for getting the right matrix size
    JacobianType TheoreticalJacobian = jacobian;

    TheoreticalJacobian[0][0] = 0.0;
    TheoreticalJacobian[1][0] = 206.0;
    TheoreticalJacobian[2][0] = -84.0;

    TheoreticalJacobian[0][1] = -206.0;
    TheoreticalJacobian[1][1] = 0.0;
    TheoreticalJacobian[2][1] = 42.0;

    TheoreticalJacobian[0][2] = 84.0;
    TheoreticalJacobian[1][2] = -42.0;
    TheoreticalJacobian[2][2] = 0.0;
    for (unsigned int ii = 0; ii < 3; ++ii)
    {
      for (unsigned int jj = 0; jj < 3; ++jj)
      {
        if (itk::Math::abs(TheoreticalJacobian[ii][jj] - jacobian[ii][jj]) > 1e-5)
        {
          std::cerr << "Jacobian components differ from expected values ";
          std::cerr << std::endl << std::endl;
          std::cerr << "Expected Jacobian = " << std::endl;
          std::cerr << TheoreticalJacobian << std::endl << std::endl;
          std::cerr << "Computed Jacobian = " << std::endl;
          std::cerr << jacobian << std::endl << std::endl;
          std::cerr << std::endl << "Test FAILED ! " << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
  }

  {
    // Testing SetMatrix()
    std::cout << "Testing SetMatrix() ... ";
    MatrixType matrix;

    auto t = TransformType::New();

    // attempt to set an non-orthogonal matrix
    unsigned int par = 0;
    for (unsigned int row = 0; row < 3; ++row)
    {
      for (unsigned int col = 0; col < 3; ++col)
      {
        matrix[row][col] = static_cast<double>(par + 1);
        ++par;
      }
    }

    bool Ok = false;
    try
    {
      t->SetMatrix(matrix);
    }
    catch (const itk::ExceptionObject & itkNotUsed(err))
    {
      Ok = true;
    }
    catch (...)
    {
      std::cout << "Caught unknown exception" << std::endl;
    }

    if (!Ok)
    {
      std::cerr << "Error: expected to catch an exception when attempting";
      std::cerr << " to set an non-orthogonal matrix." << std::endl;
      return EXIT_FAILURE;
    }

    t = TransformType::New();

    // attempt to set an orthogonal matrix
    matrix.GetVnlMatrix().set_identity();

    constexpr double a = 1.0 / 180.0 * itk::Math::pi;
    matrix[0][0] = std::cos(a);
    matrix[0][1] = -1.0 * std::sin(a);
    matrix[1][0] = std::sin(a);
    matrix[1][1] = std::cos(a);

    Ok = true;
    try
    {
      t->SetMatrix(matrix);
    }
    catch (const itk::ExceptionObject & err)
    {
      std::cout << err << std::endl;
      Ok = false;
    }
    catch (...)
    {
      std::cout << "Caught unknown exception" << std::endl;
      Ok = false;
    }

    if (!Ok)
    {
      std::cerr << "Error: caught unexpected exception" << std::endl;
      return EXIT_FAILURE;
    }

    // Check the computed parameters

    VectorType axis{};
    axis[2] = 1.0;
    VersorType v;
    v.Set(axis, a);

    ParametersType e(t->GetNumberOfParameters());
    e[0] = v.GetX();
    e[1] = v.GetY();
    e[2] = v.GetZ();

    t = TransformType::New();
    t->SetParameters(e);

    auto t2 = TransformType::New();
    t2->SetMatrix(t->GetMatrix());

    ParametersType p = t2->GetParameters();
    for (unsigned int k = 0; k < e.GetSize(); ++k)
    {
      if (itk::Math::abs(e[k] - p[k]) > epsilon)
      {
        std::cout << " [ FAILED ] " << std::endl;
        std::cout << "Expected parameters: " << e << std::endl;
        std::cout << "but got: " << p << std::endl;
        return EXIT_FAILURE;
      }
    }

    {
      // Check that setting parameters updates the transformation offset
      t = TransformType::New();

      const TransformType::InputPointType center = itk::MakePoint(2, 4, 8);

      // 90 degree rotation around x axis
      ParametersType q(3);
      q[0] = itk::Math::sqrt1_2;
      q[1] = 0.0;
      q[2] = 0.0;

      t->SetCenter(center);
      t->SetParameters(q);

      constexpr double expectedOffset[] = { 0.0, 12.0, 4.0 };

      TransformType::OffsetType offset = t->GetOffset();
      for (unsigned int k = 0; k < 3; ++k)
      {
        if (itk::Math::abs(expectedOffset[k] - offset[k]) > epsilon)
        {
          std::cerr << " [ FAILED ] " << std::endl;
          std::cerr << "Expected offset: " << expectedOffset << std::endl;
          std::cerr << "but got: " << offset << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
    {
      auto tInverse = TransformType::New();
      if (!t->GetInverse(tInverse))
      {
        std::cout << "Cannot create inverse transform" << std::endl;
        return EXIT_FAILURE;
      }
      std::cout << "translation: " << t;
      std::cout << "translationInverse: " << tInverse;
    }
  }
  std::cout << std::endl << "Test PASSED ! " << std::endl;
  return EXIT_SUCCESS;
}
