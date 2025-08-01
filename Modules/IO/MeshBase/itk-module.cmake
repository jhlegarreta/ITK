set(
  DOCUMENTATION
  "This module contains base classes for IO, helper classes for
IO, and classes that function as an itk::MeshSource in an ITK pipeline. Classes for
specific file formats, found in other modules in the IO group, should inherit
from itk::MeshIOBase. For a mesh source or sink in the ITK
pipeline that handles all available file formats, see itk::MeshFileReader,
or itk::MeshFileWriter."
)

itk_module(
  ITKIOMeshBase
  ENABLE_SHARED
  DEPENDS
    ITKCommon
    ITKIOImageBase
    ITKQuadEdgeMesh
    ITKMesh
    ITKVoronoi
  TEST_DEPENDS
    ITKTestKernel
  DESCRIPTION "${DOCUMENTATION}"
)
