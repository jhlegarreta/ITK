itk_module_test()
set(
  ITKDiffusionTensorImageTests
  itkDiffusionTensor3DTest.cxx
  itkDiffusionTensor3DReconstructionImageFilterTest.cxx
  itkTensorRelativeAnisotropyImageFilterTest.cxx
  itkTensorFractionalAnisotropyImageFilterTest.cxx
)

createtestdriver(ITKDiffusionTensorImage "${ITKDiffusionTensorImage-Test_LIBRARIES}" "${ITKDiffusionTensorImageTests}")

itk_add_test(
  NAME itkDiffusionTensor3DTest
  COMMAND
    ITKDiffusionTensorImageTestDriver
    itkDiffusionTensor3DTest
)
itk_add_test(
  NAME itkDiffusionTensor3DReconstructionImageFilterTest
  COMMAND
    ITKDiffusionTensorImageTestDriver
    itkDiffusionTensor3DReconstructionImageFilterTest
    1.0
)
itk_add_test(
  NAME itkTensorRelativeAnisotropyImageFilterTest
  COMMAND
    ITKDiffusionTensorImageTestDriver
    itkTensorRelativeAnisotropyImageFilterTest
)
itk_add_test(
  NAME itkTensorFractionalAnisotropyImageFilterTest
  COMMAND
    ITKDiffusionTensorImageTestDriver
    itkTensorFractionalAnisotropyImageFilterTest
)
