itk_module_test()
set(
  ITKIONIFTITests
  itkNiftiImageIOTest.cxx
  itkNiftiImageIOTest2.cxx
  itkNiftiImageIOTest3.cxx
  itkNiftiImageIOTest4.cxx
  itkNiftiImageIOTest5.cxx
  itkNiftiImageIOTest6.cxx
  itkNiftiImageIOTest7.cxx
  itkNiftiImageIOTest8.cxx
  itkNiftiImageIOTest9.cxx
  itkNiftiImageIOTest10.cxx
  itkNiftiImageIOTest11.cxx
  itkNiftiImageIOTest12.cxx
  itkNiftiImageIOTest13.cxx
  itkNiftiImageIOTest14.cxx
  itkNiftiLargeImageRegionReadTest.cxx
  itkNiftiReadAnalyzeTest.cxx
  itkNiftiReadWriteDirectionTest.cxx
  itkExtractSlice.cxx
  itkNiftiWriteCoerceOrthogonalDirectionTest.cxx
)

# For itkNiftiImageIOTest.h.
include_directories(${ITKIONIFTI_SOURCE_DIR}/test)

add_library(ITKIONIFTI-TestSupport itkNiftiImageIOTestHelper.cxx)
itk_module_target_label(ITKIONIFTI-TestSupport)
target_link_libraries(
  ITKIONIFTI-TestSupport
  LINK_PUBLIC
    ${ITKIONIFTI-Test_LIBRARIES}
)

list(APPEND ITKIONIFTI-Test_LIBRARIES ITKIONIFTI-TestSupport)

createtestdriver(ITKIONIFTI "${ITKIONIFTI-Test_LIBRARIES}" "${ITKIONIFTITests}")

itk_add_test(
  NAME itkNiftisform2DirectionDef.nii.gz
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftisform2DirectionDefNiiGzTest.nii.gz
    DATA{${ITK_DATA_ROOT}/Input/itkNiftisform2DirectionDef.nii.gz}
)

itk_add_test(
  NAME itkNiftiIOBigEndianCompressed.hdr
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftiIOBigEndianCompressedHdrTest.nii.gz
    itkNiftiIOBigEndianCompressed.hdr
    DATA{${ITK_DATA_ROOT}/Input/BigEndianCompressed.hdr}
)
itk_add_test(
  NAME itkNiftiIOBigEndianCompressed.img
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftiIOBigEndianCompressedImgTest.nii.gz
    itkNiftiIOBigEndianCompressed.img
    DATA{${ITK_DATA_ROOT}/Input/BigEndianCompressed.img}
)
itk_add_test(
  NAME itkNiftiIOBigEndianCompressed.img.gz
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftiIOBigEndianCompressedImgGz.nii.gz
    itkNiftiIOBigEndianCompressed.img.gz
    DATA{${ITK_DATA_ROOT}/Input/BigEndianCompressed.img.gz}
)
itk_add_test(
  NAME itkNiftiIOBigEndian
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftiIOBigEndianTest.nii.gz
    itkNiftiIOBigEndian
    DATA{${ITK_DATA_ROOT}/Input/BigEndian.hdr,BigEndian.mhd,BigEndian.img}
)
itk_add_test(
  NAME itkNiftiIOLittleEndianCompressed
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftiIOLittleEndianCompressedTest.nii.gz
    itkNiftiImageIOTest
    DATA{${ITK_DATA_ROOT}/Input/LittleEndianCompressed.hdr,LittleEndianCompressed.img.gz}
)
itk_add_test(
  NAME itkNiftiIOLittleEndian
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftiIOLittleEndianCompressedTest.nii.gz
    itkNiftiIOLittleEndian
    DATA{${ITK_DATA_ROOT}/Input/LittleEndian.hdr,LittleEndian.mhd,LittleEndian.img}
)
itk_add_test(
  NAME itkNiftiIOInternalTests
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftiIOInternalTestsTest.nii.gz
)
itk_add_test(
  NAME itkNiftiIOShouldFail
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest2
    ${ITK_TEST_OUTPUT_DIR}
    itkNiftiIOShouldFail
    false
    DATA{${ITK_DATA_ROOT}/Input/ramp.gipl}
)
itk_add_test(
  NAME itkNiftiIOShouldSucceed
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest2
    ${ITK_TEST_OUTPUT_DIR}
    itkNiftiIOShouldSucceed
    true
    DATA{${ITK_DATA_ROOT}/Input/LittleEndian.hdr,LittleEndian.mhd,LittleEndian.img}
)
itk_add_test(
  NAME itkDirCosinesTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest4
    ${ITK_TEST_OUTPUT_DIR}
)
itk_add_test(
  NAME itkNiftiSlopeInterceptTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest5
    ${ITK_TEST_OUTPUT_DIR}
)
itk_add_test(
  NAME itkNiftiVectorImageTest2
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest6
    ${ITK_TEST_OUTPUT_DIR}
)
itk_add_test(
  NAME itkNiftiDiffusionTensor3dImageTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest7
    ${ITK_TEST_OUTPUT_DIR}
)
itk_add_test(
  NAME itkNiftiSymmetricSecondRankTensorImageTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest8
    ${ITK_TEST_OUTPUT_DIR}
)
itk_add_test(
  NAME itkNiftiRGBImageTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest9
    ${ITK_TEST_OUTPUT_DIR}
    RGBImage.nii.gz
)
itk_add_test(
  NAME itkNiftiRGBAImageTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest10
    ${ITK_TEST_OUTPUT_DIR}
    RGBAImage.nii.gz
)
itk_add_test(
  NAME itkNiftiVectorImageTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest3
    ${ITK_TEST_OUTPUT_DIR}
)
itk_add_test(
  NAME itkNiftiDimensionLimitsTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest11
    ${ITK_TEST_OUTPUT_DIR}
    SizeFailure.nii.gz
)
itk_add_test(
  NAME itkNiftiLargeRGBTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest12
    ${ITK_TEST_OUTPUT_DIR}
    LargeRGBImage.nii.gz
)
itk_add_test(
  NAME itkNiftiReadAnalyzeTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiReadAnalyzeTest
    ${ITK_TEST_OUTPUT_DIR}
)
itk_add_test(
  NAME itkExtractSliceSlopeInterceptUCHAR
  COMMAND
    ITKIONIFTITestDriver
    --compare
    DATA{Baseline/SlopeInterceptUCHAR-midSlice.nrrd}
    ${ITK_TEST_OUTPUT_DIR}/SlopeInterceptUCHAR-midSlice.nrrd
    itkExtractSlice
    DATA{Input/SlopeInterceptUCHAR.nii.gz}
    ${ITK_TEST_OUTPUT_DIR}/SlopeInterceptUCHAR-midSlice.nrrd
)

itk_add_test(
  NAME itkNiftiReadWriteDirectionTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiReadWriteDirectionTest
    DATA{${ITK_DATA_ROOT}/Input/LPSLabels.nii.gz}
    DATA{${ITK_DATA_ROOT}/Input/LPSLabels_noqform.nii.gz}
    DATA{${ITK_DATA_ROOT}/Input/LPSLabels_nosform.nii.gz}
    DATA{${ITK_DATA_ROOT}/Input/NonOrthoSform.nii.gz}
    ${ITK_TEST_OUTPUT_DIR}
    "test_filled_qform_itkNiftiReadWriteDirectionTest.nii.gz"
    8.0 # Tolerance in degrees for direction recovered from non-orthogonal sform
)

itk_add_test(
  NAME itkNiftiReadWriteDirectionSmallVoxelTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiReadWriteDirectionTest
    DATA{Input/SmallVoxels.nii.gz}
    DATA{Input/SmallVoxels_noqform.nii.gz}
    DATA{Input/SmallVoxels_nosform.nii.gz}
    DATA{Input/SmallVoxelsNonOrthoSform.nii.gz}
    ${ITK_TEST_OUTPUT_DIR}
    "test_filled_qform_itkNiftiReadWriteDirectionSmallVoxelTest.nii.gz"
    4.0 # Tolerance in degrees for direction recovered from non-orthogonal sform
)

itk_add_test(
  NAME itkNiftiSmallVoxelsAffinePrecisionTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest13
    DATA{Input/SmallVoxels_AffinePrecision.nii.gz}
)

itk_add_test(
  NAME itkNiftiSpatialTemporalUnitsTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest14
    ${ITK_TEST_OUTPUT_DIR}/xyzt_units_output_check.nii
    DATA{Input/xyzt_units_test_scl_mm_s.nii.gz}
    DATA{Input/xyzt_units_test_scl.nii.gz}
)

itk_add_test(
  NAME itkNiftiSpatialTemporalUnitsTest2
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest14
    ${ITK_TEST_OUTPUT_DIR}/xyzt_units_output_check2.nii
    DATA{Input/xyzt_units_test_scl_mm_s.nii.gz}
    DATA{Input/xyzt_units_test_scl_mm_s.nii.gz}
)

itk_add_test(
  NAME itkNiftiQFormSFormDifferentSpacingTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiImageIOTest13
    DATA{Input/ChickenEgg-zeros.nii.gz}
)

itk_add_test(
  NAME itkNiftiLargeImageRegionReadTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiLargeImageRegionReadTest
    ${ITK_TEST_OUTPUT_DIR}/itkNiftiLargeImageRegionReadTest.nii.gz
)

itk_add_test(
  NAME itkNiftiWriteCoerceOrthogonalDirectionTest
  COMMAND
    ITKIONIFTITestDriver
    itkNiftiWriteCoerceOrthogonalDirectionTest
    ${ITK_TEST_OUTPUT_DIR}
)
