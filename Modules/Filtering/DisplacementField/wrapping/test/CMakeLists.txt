if(ITK_WRAP_PYTHON)
  itk_python_add_test(
    NAME itkDisplacementFieldTransformPythonTest
    COMMAND
      ${CMAKE_CURRENT_SOURCE_DIR}/itkDisplacementFieldTransformTest.py
  )
  itk_python_add_test(
    NAME itkTransformToDisplacementFieldPythonTest
    COMMAND
      ${CMAKE_CURRENT_SOURCE_DIR}/itkTransformToDisplacementFieldTest.py
  )
endif()
