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
#include "itkMINCImageIO.h"

#include <cstdio>
#include <cctype>
#include "vnl/vnl_vector.h"
#include "itkMetaDataObject.h"
#include "itkArray.h"
#include "itkPrintHelper.h"
#include "itkMakeUniqueForOverwrite.h"

#include "itk_minc2.h"

#include <memory> // For unique_ptr.


extern "C"
{
  void
  MINCIOFreeTmpDimHandle(unsigned int size, midimhandle_t * ptr)
  {
    if (!ptr)
    {
      /*
       * Should never happen.
       */
#ifndef NDEBUG
      printf("MINCIOFreeTmpDimHandle: ptr is null pointer");
#endif
      return;
    }
    for (unsigned int x = 0; x < size; ++x)
    {
      [[maybe_unused]] const int error = mifree_dimension_handle(ptr[x]);
#ifndef NDEBUG
      if (error != MI_NOERROR)
      {
        printf("MINCIOFreeTmpDimHandle: mifree_dimension_handle(ptr[%u]) returned %d", x, error);
      }
#endif
    }
  }
}

namespace itk
{

struct ITKIOMINC_HIDDEN MINCImageIOPImpl
{
  int m_NDims; /*Number of dimensions*/

  // dimension size and start and step, in FILE ORDER!

  char **    m_DimensionName;
  misize_t * m_DimensionSize;
  double *   m_DimensionStart;
  double *   m_DimensionStep;
  int        m_DimensionIndices[5];

  midimhandle_t * m_MincFileDims;
  midimhandle_t * m_MincApparentDims;
  mitype_t        m_Volume_type;
  miclass_t       m_Volume_class;

  // MINC2 volume handle , currently opened
  mihandle_t m_Volume;
};


bool
MINCImageIO::CanReadFile(const char * name)
{
  if (*name == 0)
  {
    itkDebugMacro("No filename specified.");
    return false;
  }

  return this->HasSupportedReadExtension(name, false);
}

void
MINCImageIO::Read(void * buffer)
{
  const unsigned int nDims = this->GetNumberOfDimensions();
  const unsigned int nComp = this->GetNumberOfComponents();

  const auto start = make_unique_for_overwrite<misize_t[]>(nDims + (nComp > 1 ? 1 : 0));
  const auto count = make_unique_for_overwrite<misize_t[]>(nDims + (nComp > 1 ? 1 : 0));

  for (unsigned int i = 0; i < nDims; ++i)
  {
    if (i < m_IORegion.GetImageDimension())
    {
      start[nDims - i - 1] = m_IORegion.GetIndex()[i];
      count[nDims - i - 1] = m_IORegion.GetSize()[i];
    }
    else
    {
      start[nDims - i - 1] = 0;
      count[nDims - i - 1] = 1;
    }
  }
  if (nComp > 1)
  {
    start[nDims] = 0;
    count[nDims] = nComp;
  }
  mitype_t volume_data_type = MI_TYPE_UBYTE;

  switch (this->GetComponentType())
  {
    case IOComponentEnum::UCHAR:
      volume_data_type = MI_TYPE_UBYTE;
      break;
    case IOComponentEnum::CHAR:
      volume_data_type = MI_TYPE_BYTE;
      break;
    case IOComponentEnum::USHORT:
      volume_data_type = MI_TYPE_USHORT;
      break;
    case IOComponentEnum::SHORT:
      volume_data_type = MI_TYPE_SHORT;
      break;
    case IOComponentEnum::UINT:
      volume_data_type = MI_TYPE_UINT;
      break;
    case IOComponentEnum::INT:
      volume_data_type = MI_TYPE_INT;
      break;
    case IOComponentEnum::ULONG: // TODO: make sure we are cross-platform here!
      volume_data_type = MI_TYPE_UINT;
      break;
    case IOComponentEnum::LONG: // TODO: make sure we are cross-platform here!
      volume_data_type = MI_TYPE_INT;
      break;
    case IOComponentEnum::FLOAT:
      volume_data_type = MI_TYPE_FLOAT;
      break;
    case IOComponentEnum::DOUBLE:
      volume_data_type = MI_TYPE_DOUBLE;
      break;
    default:
      itkDebugMacro("Could read datatype " << this->GetComponentType());
      return;
  }

  if (miget_real_value_hyperslab(m_MINCPImpl->m_Volume, volume_data_type, start.get(), count.get(), buffer) < 0)
  {
    itkExceptionMacro(" Can not get real value hyperslab!!\n");
  }
}

void
MINCImageIO::CleanupDimensions()
{
  if (m_MINCPImpl->m_DimensionName)
  {
    for (int i = 0; i < m_MINCPImpl->m_NDims; ++i)
    {
      mifree_name(m_MINCPImpl->m_DimensionName[i]);
      m_MINCPImpl->m_DimensionName[i] = nullptr;
    }
  }

  delete[] m_MINCPImpl->m_DimensionName;
  delete[] m_MINCPImpl->m_DimensionSize;
  delete[] m_MINCPImpl->m_DimensionStart;
  delete[] m_MINCPImpl->m_DimensionStep;
  delete[] m_MINCPImpl->m_MincFileDims;
  delete[] m_MINCPImpl->m_MincApparentDims;

  m_MINCPImpl->m_DimensionName = nullptr;
  m_MINCPImpl->m_DimensionSize = nullptr;
  m_MINCPImpl->m_DimensionStart = nullptr;
  m_MINCPImpl->m_DimensionStep = nullptr;
  m_MINCPImpl->m_MincFileDims = nullptr;
  m_MINCPImpl->m_MincApparentDims = nullptr;
}

void
MINCImageIO::AllocateDimensions(int nDims)
{
  this->CleanupDimensions();

  m_MINCPImpl->m_NDims = nDims;

  m_MINCPImpl->m_DimensionName = new char *[m_MINCPImpl->m_NDims];
  m_MINCPImpl->m_DimensionSize = new misize_t[m_MINCPImpl->m_NDims];
  m_MINCPImpl->m_DimensionStart = new double[m_MINCPImpl->m_NDims];
  m_MINCPImpl->m_DimensionStep = new double[m_MINCPImpl->m_NDims];
  m_MINCPImpl->m_MincFileDims = new midimhandle_t[m_MINCPImpl->m_NDims];
  m_MINCPImpl->m_MincApparentDims = new midimhandle_t[m_MINCPImpl->m_NDims];

  for (int i = 0; i < m_MINCPImpl->m_NDims; ++i)
  {
    m_MINCPImpl->m_DimensionName[i] = nullptr;
    m_MINCPImpl->m_DimensionSize[i] = 0;
    m_MINCPImpl->m_DimensionStart[i] = 0.0;
    m_MINCPImpl->m_DimensionStep[i] = 0.0;
  }

  for (auto & dimensionIndex : m_MINCPImpl->m_DimensionIndices)
  {
    dimensionIndex = -1;
  }
}

void
MINCImageIO::CloseVolume()
{
  this->CleanupDimensions();

  if (m_MINCPImpl->m_Volume)
  {
    miclose_volume(m_MINCPImpl->m_Volume);
  }
  m_MINCPImpl->m_Volume = nullptr;
}

MINCImageIO::MINCImageIO()
  : m_MINCPImpl(std::make_unique<MINCImageIOPImpl>())

{
  for (auto & dimensionIndex : m_MINCPImpl->m_DimensionIndices)
  {
    dimensionIndex = -1;
  }

  const char * readExtensions[] = { ".mnc",    ".MNC",
#ifdef HAVE_MINC1
                                    ".mnc.gz", ".MNC.GZ",
#endif // HAVE_MINC1
                                    ".mnc2",   ".MNC2" };


  for (auto ext : readExtensions)
  {
    this->AddSupportedReadExtension(ext);
  }


  const char * writeExtensions[] = { ".mnc", ".MNC", ".mnc2", ".MNC2" };

  for (auto ext : writeExtensions)
  {
    this->AddSupportedWriteExtension(ext);
  }

  this->m_UseCompression = true;
  this->Self::SetMaximumCompressionLevel(9);
  this->Self::SetCompressionLevel(4); // Range 0-9; 0 = no file compression, 9 =
                                      // maximum file compression
  m_MINCPImpl->m_Volume_type = MI_TYPE_FLOAT;
  m_MINCPImpl->m_Volume_class = MI_CLASS_REAL;
}

MINCImageIO::~MINCImageIO() { this->CloseVolume(); }

void
MINCImageIO::PrintSelf(std::ostream & os, Indent indent) const
{
  using namespace print_helper;

  Superclass::PrintSelf(os, indent);

  os << indent << "MINCPImpl: " << m_MINCPImpl.get() << std::endl;
  os << indent << "DirectionCosines: " << m_DirectionCosines << std::endl;
  os << indent << "RAStoLPS: " << m_RAStoLPS << std::endl;
}

void
MINCImageIO::ReadImageInformation()
{
  std::string dimension_order;

  this->CloseVolume();
  // call to minc2.0 function to open the file
  if (miopen_volume(m_FileName.c_str(), MI2_OPEN_READ, &m_MINCPImpl->m_Volume) < 0)
  {
    // Error opening the volume
    itkExceptionMacro("Could not open file \"" << m_FileName << "\".");
  }

  // find out how many dimensions are there regularly sampled
  // dimensions only
  int ndims = 0;
  if (miget_volume_dimension_count(m_MINCPImpl->m_Volume, MI_DIMCLASS_ANY, MI_DIMATTR_ALL, &ndims) < 0)
  {
    itkDebugMacro("Could not get the number of dimensions in the volume!");
    return;
  }
  this->AllocateDimensions(ndims);

  // get dimension handles in FILE ORDER (i.e, the order as they are
  // submitted to file)
  if (miget_volume_dimensions(m_MINCPImpl->m_Volume,
                              MI_DIMCLASS_ANY,
                              MI_DIMATTR_ALL,
                              MI_DIMORDER_FILE,
                              m_MINCPImpl->m_NDims,
                              m_MINCPImpl->m_MincFileDims) < 0)
  {
    itkExceptionMacro("Could not get dimension handles!");
  }

  for (int i = 0; i < m_MINCPImpl->m_NDims; ++i)
  {
    char * name = nullptr;
    if (miget_dimension_name(m_MINCPImpl->m_MincFileDims[i], &name) < 0)
    {
      // Error getting dimension name
      itkExceptionMacro("Could not get dimension name!");
    }
    double       _sep = NAN;
    const char * _sign = "+";
    if (miget_dimension_separation(m_MINCPImpl->m_MincFileDims[i], MI_ORDER_FILE, &_sep) == MI_NOERROR && _sep < 0)
    {
      _sign = "-";
    }
    m_MINCPImpl->m_DimensionName[i] = name;
    if (!strcmp(name, MIxspace) || !strcmp(name, MIxfrequency)) // this is X space
    {
      m_MINCPImpl->m_DimensionIndices[1] = i;
      dimension_order += _sign;
      dimension_order += "X";
    }
    else if (!strcmp(name, MIyspace) || !strcmp(name, MIyfrequency)) // this is Y space
    {
      m_MINCPImpl->m_DimensionIndices[2] = i;
      dimension_order += _sign;
      dimension_order += "Y";
    }
    else if (!strcmp(name, MIzspace) || !strcmp(name, MIzfrequency)) // this is Z space
    {
      m_MINCPImpl->m_DimensionIndices[3] = i;
      dimension_order += _sign;
      dimension_order += "Z";
    }
    else if (!strcmp(name, MIvector_dimension)) // this is vector space
    {
      m_MINCPImpl->m_DimensionIndices[0] = i;
      dimension_order += "+"; // vector dimension is always positive
      dimension_order += "V";
    }
    else if (!strcmp(name, MItime) || !strcmp(name, MItfrequency)) // this is time space
    {
      m_MINCPImpl->m_DimensionIndices[4] = i;
      dimension_order += _sign;
      dimension_order += "T";
    }
    else
    {
      itkExceptionMacro("Unsupported MINC dimension:" << name);
    }
  }

  // fill the DimensionSize by calling the following MINC2.0 function
  if (miget_dimension_sizes(m_MINCPImpl->m_MincFileDims, m_MINCPImpl->m_NDims, m_MINCPImpl->m_DimensionSize) < 0)
  {
    // Error getting dimension sizes
    itkExceptionMacro("Could not get dimension sizes!");
  }

  if (miget_dimension_separations(
        m_MINCPImpl->m_MincFileDims, MI_ORDER_FILE, m_MINCPImpl->m_NDims, m_MINCPImpl->m_DimensionStep) < 0)
  {
    itkExceptionMacro(" Could not dimension sizes");
  }

  if (miget_dimension_starts(
        m_MINCPImpl->m_MincFileDims, MI_ORDER_FILE, m_MINCPImpl->m_NDims, m_MINCPImpl->m_DimensionStart) < 0)
  {
    itkExceptionMacro(" Could not dimension sizes");
  }

  mitype_t volume_data_type;
  if (miget_data_type(m_MINCPImpl->m_Volume, &volume_data_type) < 0)
  {
    itkExceptionMacro(" Can not get volume data type!!\n");
  }

  // find out whether the data has slice scaling
  miboolean_t slice_scaling_flag = 0;
  miboolean_t global_scaling_flag = 0;

  if (miget_slice_scaling_flag(m_MINCPImpl->m_Volume, &slice_scaling_flag) < 0)
  {
    itkExceptionMacro(" Can not get slice scaling flag!!\n");
  }

  // voxel valid range
  double valid_min = NAN;
  double valid_max = NAN;
  // get the voxel valid range
  if (miget_volume_valid_range(m_MINCPImpl->m_Volume, &valid_max, &valid_min) < 0)
  {
    itkExceptionMacro(" Can not get volume valid range!!\n");
  }

  // real volume range, only awailable when slice scaling is off
  double volume_min = 0.0;
  double volume_max = 1.0;
  if (!slice_scaling_flag)
  {
    if (miget_volume_range(m_MINCPImpl->m_Volume, &volume_max, &volume_min) < 0)
    {
      itkExceptionMacro(" Can not get volume range!!\n");
    }
    global_scaling_flag = !(volume_min == valid_min && volume_max == valid_max);
  }

  int spatial_dimension_count = 0;

  // extract direction cosines
  for (int i = 1; i < 4; ++i)
  {
    if (m_MINCPImpl->m_DimensionIndices[i] != -1) // this dimension is present
    {
      ++spatial_dimension_count;
    }
  }

  if (spatial_dimension_count == 0) // sorry, this is metaphysical question
  {
    itkExceptionMacro(" minc files without spatial dimensions are not supported!");
  }

  if (m_MINCPImpl->m_DimensionIndices[0] != -1 && m_MINCPImpl->m_DimensionIndices[4] != -1)
  {
    itkExceptionMacro(" 4D minc files vector dimension are not supported currently");
  }

  this->SetNumberOfDimensions(spatial_dimension_count);

  int          numberOfComponents = 1;
  unsigned int usableDimensions = 0;

  auto dir_cos = Matrix<double, 3, 3>::GetIdentity();

  // Conversion matrix for MINC PositiveCoordinateOrientation RAS (LeftToRight, PosteriorToAnterior, InferiorToSuperior)
  // to ITK PositiveCoordinateOrientation LPS (RightToLeft, AnteriorToPosterior, InferiorToSuperior)
  auto RAStofromLPS = Matrix<double, 3, 3>::GetIdentity();
  RAStofromLPS(0, 0) = -1.0;
  RAStofromLPS(1, 1) = -1.0;
  std::vector<double> dir_cos_temp(3);

  Vector<double, 3> origin{};
  Vector<double, 3> oOrigin{};

  // minc api uses inverse order of dimensions , fastest varying are last
  Vector<double, 3> sep;
  for (int i = 3; i > 0; i--)
  {
    if (m_MINCPImpl->m_DimensionIndices[i] != -1)
    {
      // MINC2: bad design!
      // micopy_dimension(hdim[m_MINCPImpl->m_DimensionIndices[i]],&apparent_dimension_order[usable_dimensions]);
      m_MINCPImpl->m_MincApparentDims[usableDimensions] =
        m_MINCPImpl->m_MincFileDims[m_MINCPImpl->m_DimensionIndices[i]];
      // always use positive
      miset_dimension_apparent_voxel_order(m_MINCPImpl->m_MincApparentDims[usableDimensions], MI_POSITIVE);
      misize_t _sz = 0;
      miget_dimension_size(m_MINCPImpl->m_MincApparentDims[usableDimensions], &_sz);

      double _sep = NAN;
      miget_dimension_separation(m_MINCPImpl->m_MincApparentDims[usableDimensions], MI_ORDER_APPARENT, &_sep);
      std::vector<double> _dir(3);
      miget_dimension_cosines(m_MINCPImpl->m_MincApparentDims[usableDimensions], &_dir[0]);
      double _start = NAN;
      miget_dimension_start(m_MINCPImpl->m_MincApparentDims[usableDimensions], MI_ORDER_APPARENT, &_start);

      for (int j = 0; j < 3; ++j)
      {
        dir_cos[j][i - 1] = _dir[j];
      }

      origin[i - 1] = _start;
      sep[i - 1] = _sep;

      this->SetDimensions(i - 1, static_cast<unsigned int>(_sz));
      this->SetSpacing(i - 1, _sep);

      ++usableDimensions;
    }
  }


  // Transform MINC PositiveCoordinateOrientation RAS coordinates to
  // internal ITK PositiveCoordinateOrientation LPS Coordinates
  if (this->m_RAStoLPS)
    dir_cos = RAStofromLPS * dir_cos;

  // Transform origin coordinates
  oOrigin = dir_cos * origin;

  for (int i = 0; i < spatial_dimension_count; ++i)
  {
    this->SetOrigin(i, oOrigin[i]);
    for (unsigned int j = 0; j < 3; j++)
    {
      dir_cos_temp[j] = dir_cos[j][i];
    }
    this->SetDirection(i, dir_cos_temp);
  }

  if (m_MINCPImpl->m_DimensionIndices[0] != -1) // have vector dimension
  {
    // micopy_dimension(m_MINCPImpl->m_MincFileDims[m_MINCPImpl->m_DimensionIndices[0]],&apparent_dimension_order[usable_dimensions]);
    m_MINCPImpl->m_MincApparentDims[usableDimensions] = m_MINCPImpl->m_MincFileDims[m_MINCPImpl->m_DimensionIndices[0]];
    // always use positive, vector dimension does not supposed to have notion of positive step size, so leaving as is
    // miset_dimension_apparent_voxel_order(m_MINCPImpl->m_MincApparentDims[usable_dimensions],MI_POSITIVE);
    misize_t _sz = 0;
    miget_dimension_size(m_MINCPImpl->m_MincApparentDims[usableDimensions], &_sz);
    numberOfComponents = _sz;
    ++usableDimensions;
  }

  if (m_MINCPImpl->m_DimensionIndices[4] != -1) // have time dimension
  {
    // micopy_dimension(hdim[m_MINCPImpl->m_DimensionIndices[4]],&apparent_dimension_order[usable_dimensions]);
    m_MINCPImpl->m_MincApparentDims[usableDimensions] = m_MINCPImpl->m_MincFileDims[m_MINCPImpl->m_DimensionIndices[4]];
    // always use positive
    miset_dimension_apparent_voxel_order(m_MINCPImpl->m_MincApparentDims[usableDimensions], MI_POSITIVE);
    misize_t _sz = 0;
    miget_dimension_size(m_MINCPImpl->m_MincApparentDims[usableDimensions], &_sz);
    numberOfComponents = _sz;
    ++usableDimensions;
  }

  // Set apparent dimension order to the MINC2 api
  if (miset_apparent_dimension_order(m_MINCPImpl->m_Volume, usableDimensions, m_MINCPImpl->m_MincApparentDims) < 0)
  {
    itkExceptionMacro(" Can't set apparent dimension order!");
  }

  miclass_t volume_data_class;

  if (miget_data_class(m_MINCPImpl->m_Volume, &volume_data_class) < 0)
  {
    itkExceptionMacro(" Could not get data class");
  }

  // set the file data type
  if (slice_scaling_flag || global_scaling_flag)
  {
    switch (volume_data_type)
    {
      case MI_TYPE_FLOAT:
        this->SetComponentType(IOComponentEnum::FLOAT);
        break;
      case MI_TYPE_DOUBLE:
        this->SetComponentType(IOComponentEnum::DOUBLE);
        break;
      case MI_TYPE_FCOMPLEX:
        this->SetComponentType(IOComponentEnum::FLOAT);
        break;
      case MI_TYPE_DCOMPLEX:
        this->SetComponentType(IOComponentEnum::DOUBLE);
        break;
      default:
        this->SetComponentType(IOComponentEnum::FLOAT);
        break;
    } // end of switch
      // file will have do
  }
  else
  {
    switch (volume_data_type)
    {
      case MI_TYPE_BYTE:
        this->SetComponentType(IOComponentEnum::CHAR);
        break;
      case MI_TYPE_UBYTE:
        this->SetComponentType(IOComponentEnum::UCHAR);
        break;
      case MI_TYPE_SHORT:
        this->SetComponentType(IOComponentEnum::SHORT);
        break;
      case MI_TYPE_USHORT:
        this->SetComponentType(IOComponentEnum::USHORT);
        break;
      case MI_TYPE_INT:
        this->SetComponentType(IOComponentEnum::INT);
        break;
      case MI_TYPE_UINT:
        this->SetComponentType(IOComponentEnum::UINT);
        break;
      case MI_TYPE_FLOAT:
        this->SetComponentType(IOComponentEnum::FLOAT);
        break;
      case MI_TYPE_DOUBLE:
        this->SetComponentType(IOComponentEnum::DOUBLE);
        break;
      case MI_TYPE_SCOMPLEX:
        this->SetComponentType(IOComponentEnum::SHORT);
        break;
      case MI_TYPE_ICOMPLEX:
        this->SetComponentType(IOComponentEnum::INT);
        break;
      case MI_TYPE_FCOMPLEX:
        this->SetComponentType(IOComponentEnum::FLOAT);
        break;
      case MI_TYPE_DCOMPLEX:
        this->SetComponentType(IOComponentEnum::DOUBLE);
        break;
      default:
        itkExceptionMacro("Bad data type ");
    } // end of switch
  }

  switch (volume_data_class)
  {
    case MI_CLASS_REAL:
      if (numberOfComponents == 1)
      {
        this->SetPixelType(IOPixelEnum::SCALAR);
      }
      else
      {
        this->SetPixelType(IOPixelEnum::VECTOR); // TODO: handle more types (i.e matrix, tensor etc)
      }
      break;
    case MI_CLASS_INT:
      if (numberOfComponents == 1)
      {
        this->SetPixelType(IOPixelEnum::SCALAR);
      }
      else
      {
        this->SetPixelType(IOPixelEnum::VECTOR); // TODO: handle more types (i.e matrix, tensor etc)
      }
      break;
    case MI_CLASS_LABEL:
      if (numberOfComponents == 1)
      {
        this->SetPixelType(IOPixelEnum::SCALAR);
      }
      else
      {
        this->SetPixelType(IOPixelEnum::VECTOR);
      }
      // create an array of label names and values
      // not sure how to pass this to itk yet!
      break;
    case MI_CLASS_COMPLEX:
      // m_Complex = 1;
      this->SetPixelType(IOPixelEnum::COMPLEX);
      numberOfComponents *= 2;
      break;
    default:
      itkExceptionMacro("Bad data class ");
  } // end of switch

  this->SetNumberOfComponents(numberOfComponents);
  this->ComputeStrides();

  // create metadata object to store useful additional information
  MetaDataDictionary & thisDic = GetMetaDataDictionary();
  thisDic.Clear();

  const std::string classname(GetNameOfClass());
  //  EncapsulateMetaData<std::string>(thisDic,ITK_InputFilterName,
  // classname);
  // preserve information if the volume was PositiveCoordinateOrientation RAS to PositiveCoordinateOrientation LPS
  // converted
  EncapsulateMetaData<bool>(thisDic, "RAStoLPS", m_RAStoLPS);

  // store history
  size_t minc_history_length = 0;
  if (miget_attr_length(m_MINCPImpl->m_Volume, "", "history", &minc_history_length) == MI_NOERROR)
  {
    const auto minc_history = make_unique_for_overwrite<char[]>(minc_history_length + 1);
    if (miget_attr_values(
          m_MINCPImpl->m_Volume, MI_TYPE_STRING, "", "history", minc_history_length + 1, minc_history.get()) ==
        MI_NOERROR)
    {
      EncapsulateMetaData<std::string>(thisDic, "history", std::string(minc_history.get()));
    }
  }

  if (m_MINCPImpl->m_DimensionIndices[4] != -1) // have time dimension
  {
    // store time dimension start and step in metadata for preservation
    double _sep = NAN;
    miget_dimension_separation(
      m_MINCPImpl->m_MincFileDims[m_MINCPImpl->m_DimensionIndices[4]], MI_ORDER_APPARENT, &_sep);
    double _start = NAN;
    miget_dimension_start(m_MINCPImpl->m_MincFileDims[m_MINCPImpl->m_DimensionIndices[4]], MI_ORDER_APPARENT, &_start);
    EncapsulateMetaData<double>(thisDic, "tstart", _start);
    EncapsulateMetaData<double>(thisDic, "tstep", _sep);
  }

  EncapsulateMetaData<std::string>(thisDic, "dimension_order", dimension_order);

  // preserve original volume storage data type
  switch (volume_data_type)
  {
    case MI_TYPE_BYTE:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(char).name());
      break;
    case MI_TYPE_UBYTE:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(unsigned char).name());
      break;
    case MI_TYPE_SHORT:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(short).name());
      break;
    case MI_TYPE_USHORT:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(unsigned short).name());
      break;
    case MI_TYPE_INT:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(int).name());
      break;
    case MI_TYPE_UINT:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(unsigned int).name());
      break;
    case MI_TYPE_FLOAT:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(float).name());
      break;
    case MI_TYPE_DOUBLE:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(double).name());
      break;
    case MI_TYPE_SCOMPLEX:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(short).name());
      break;
    case MI_TYPE_ICOMPLEX:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(int).name());
      break;
    case MI_TYPE_FCOMPLEX:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(float).name());
      break;
    case MI_TYPE_DCOMPLEX:
      EncapsulateMetaData<std::string>(thisDic, "storage_data_type", typeid(double).name());
      break;
    default:
      break;
      // don't store this storage data type
  } // end of switch

  // iterate over all root level groups , and extract all underlying attributes
  // unfortunately more complicated attribute structure of MINC2 is not supported
  // at least it is not used anywhere
  milisthandle_t grplist = nullptr;

  if ((milist_start(m_MINCPImpl->m_Volume, "", 0, &grplist)) == MI_NOERROR)
  {
    char group_name[256];
    while (milist_grp_next(grplist, group_name, sizeof(group_name)) == MI_NOERROR)
    {
      milisthandle_t attlist = nullptr;
      if ((milist_start(m_MINCPImpl->m_Volume, group_name, 1, &attlist)) == MI_NOERROR)
      {
        char attribute[256];

        while (milist_attr_next(
                 m_MINCPImpl->m_Volume, attlist, group_name, sizeof(group_name), attribute, sizeof(attribute)) ==
               MI_NOERROR)
        {
          mitype_t    att_data_type;
          size_t      att_length = 0;
          std::string entry_key = group_name;
          entry_key += ":";
          entry_key += attribute;

          if (miget_attr_type(m_MINCPImpl->m_Volume, group_name, attribute, &att_data_type) == MI_NOERROR &&
              miget_attr_length(m_MINCPImpl->m_Volume, group_name, attribute, &att_length) == MI_NOERROR)
          {
            switch (att_data_type)
            {
              case MI_TYPE_STRING:
              {
                const auto tmp = make_unique_for_overwrite<char[]>(att_length + 1);
                if (miget_attr_values(
                      m_MINCPImpl->m_Volume, att_data_type, group_name, attribute, att_length + 1, tmp.get()) ==
                    MI_NOERROR)
                {
                  EncapsulateMetaData<std::string>(thisDic, entry_key, std::string(tmp.get()));
                }
              }
              break;
              case MI_TYPE_FLOAT:
              {
                Array<float> tmp(att_length);
                if (miget_attr_values(
                      m_MINCPImpl->m_Volume, att_data_type, group_name, attribute, att_length, tmp.data_block()) ==
                    MI_NOERROR)
                {
                  if (att_length == 1)
                  {
                    EncapsulateMetaData<float>(thisDic, entry_key, tmp[0]);
                  }
                  else
                  {
                    EncapsulateMetaData<Array<float>>(thisDic, entry_key, tmp);
                  }
                }
                else
                {
                  itkExceptionMacro(" Error getting float attribute! ");
                }
              }
              break;
              case MI_TYPE_DOUBLE:
              {
                Array<double> tmp(att_length);
                if (miget_attr_values(
                      m_MINCPImpl->m_Volume, att_data_type, group_name, attribute, att_length, tmp.data_block()) ==
                    MI_NOERROR)
                {
                  if (att_length == 1)
                  {
                    EncapsulateMetaData<double>(thisDic, entry_key, tmp[0]);
                  }
                  else
                  {
                    EncapsulateMetaData<Array<double>>(thisDic, entry_key, tmp);
                  }
                }
              }
              break;
              case MI_TYPE_INT:
              {
                Array<int> tmp(att_length);
                if (miget_attr_values(
                      m_MINCPImpl->m_Volume, att_data_type, group_name, attribute, att_length, tmp.data_block()) ==
                    MI_NOERROR)
                {
                  if (att_length == 1)
                  {
                    EncapsulateMetaData<int>(thisDic, entry_key, tmp[0]);
                  }
                  else
                  {
                    EncapsulateMetaData<Array<int>>(thisDic, entry_key, tmp);
                  }
                }
              }
              break;
              default:
                itkExceptionMacro("Unsupported Attribute data type ");
            }
          }
          else
          {
            itkExceptionMacro("Problem reading attribute info ");
          }
        }
        milist_finish(attlist);
      }
    }
    milist_finish(grplist);
  }
}

bool
MINCImageIO::CanWriteFile(const char * name)
{
  if (name[0] == '\0')
  {
    itkDebugMacro("No filename specified.");
    return false;
  }

  return this->HasSupportedWriteExtension(name, true);
}

/*
 * fill out the appropriate header information
 */
void
MINCImageIO::WriteImageInformation()
{
  const unsigned int nDims = this->GetNumberOfDimensions();
  const unsigned int nComp = this->GetNumberOfComponents();

  this->CloseVolume();
  this->AllocateDimensions(nDims + (nComp > 1 ? 1 : 0));

  MetaDataDictionary & thisDic = GetMetaDataDictionary();

  unsigned int minc_dimensions = 0;
  if (nComp > 3) // last dimension will be either vector or time
  {
    micreate_dimension(MItime,
                       MI_DIMCLASS_TIME,
                       MI_DIMATTR_REGULARLY_SAMPLED,
                       nComp,
                       &m_MINCPImpl->m_MincApparentDims[m_MINCPImpl->m_NDims - minc_dimensions - 1]);

    double tstart = 0.0;
    if (!ExposeMetaData<double>(thisDic, "tstart", tstart))
    {
      tstart = 0.0;
    }

    miset_dimension_start(m_MINCPImpl->m_MincApparentDims[m_MINCPImpl->m_NDims - minc_dimensions - 1], tstart);

    double tstep = 1.0;
    if (!ExposeMetaData<double>(thisDic, "tstep", tstep))
    {
      tstep = 1.0;
    }

    miset_dimension_separation(m_MINCPImpl->m_MincApparentDims[m_MINCPImpl->m_NDims - minc_dimensions - 1], tstep);

    ++minc_dimensions;
  }
  else if (nComp > 1)
  {
    micreate_dimension(MIvector_dimension,
                       MI_DIMCLASS_RECORD,
                       MI_DIMATTR_REGULARLY_SAMPLED,
                       nComp,
                       &m_MINCPImpl->m_MincApparentDims[m_MINCPImpl->m_NDims - minc_dimensions - 1]);
    ++minc_dimensions;
  }

  micreate_dimension(MIxspace,
                     MI_DIMCLASS_SPATIAL,
                     MI_DIMATTR_REGULARLY_SAMPLED,
                     this->GetDimensions(0),
                     &m_MINCPImpl->m_MincApparentDims[m_MINCPImpl->m_NDims - minc_dimensions - 1]);
  ++minc_dimensions;

  if (nDims > 1)
  {
    micreate_dimension(MIyspace,
                       MI_DIMCLASS_SPATIAL,
                       MI_DIMATTR_REGULARLY_SAMPLED,
                       this->GetDimensions(1),
                       &m_MINCPImpl->m_MincApparentDims[m_MINCPImpl->m_NDims - minc_dimensions - 1]);
    ++minc_dimensions;
  }

  if (nDims > 2)
  {
    micreate_dimension(MIzspace,
                       MI_DIMCLASS_SPATIAL,
                       MI_DIMATTR_REGULARLY_SAMPLED,
                       this->GetDimensions(2),
                       &m_MINCPImpl->m_MincApparentDims[m_MINCPImpl->m_NDims - minc_dimensions - 1]);
    ++minc_dimensions;
  }

  if (nDims > 3)
  {
    MINCIOFreeTmpDimHandle(minc_dimensions, m_MINCPImpl->m_MincApparentDims);
    itkExceptionMacro("Unfortunately, only up to 3D volume are supported now.");
  }

  // allocating dimensions
  vnl_matrix<double> directionCosineMatrix(nDims, nDims);
  directionCosineMatrix.set_identity();
  vnl_vector<double> origin(nDims);

  // MINC stores direction cosines in PositiveCoordinateOrientation RAS
  // need to convert to PositiveCoordinateOrientation LPS for ITK
  vnl_matrix<double> RAS_tofrom_LPS(nDims, nDims);
  RAS_tofrom_LPS.set_identity();
  RAS_tofrom_LPS(0, 0) = -1.0;
  RAS_tofrom_LPS(1, 1) = -1.0;

  for (unsigned int i = 0; i < nDims; ++i)
  {
    for (unsigned int j = 0; j < nDims; ++j)
    {
      directionCosineMatrix[i][j] = this->GetDirection(i)[j];
    }
    origin[i] = this->GetOrigin(i);
  }

  const vnl_matrix<double> inverseDirectionCosines{ vnl_matrix_inverse<double>(directionCosineMatrix).as_matrix() };
  origin *= inverseDirectionCosines; // transform to minc convention


  // Convert ITK direction cosines from PositiveCoordinateOrientation LPS to PositiveCoordinateOrientation RAS
  if (this->m_RAStoLPS)
    directionCosineMatrix *= RAS_tofrom_LPS;

  for (unsigned int i = 0; i < nDims; ++i)
  {
    const unsigned int j = i + (nComp > 1 ? 1 : 0);
    double             dir_cos[3];
    for (unsigned int k = 0; k < 3; ++k)
    {
      if (k < nDims)
      {
        dir_cos[k] = directionCosineMatrix[i][k];
      }
      else
      {
        dir_cos[k] = 0.0;
      }
    }
    miset_dimension_separation(m_MINCPImpl->m_MincApparentDims[minc_dimensions - j - 1], this->GetSpacing(i));
    miset_dimension_start(m_MINCPImpl->m_MincApparentDims[minc_dimensions - j - 1], origin[i]);
    miset_dimension_cosines(m_MINCPImpl->m_MincApparentDims[minc_dimensions - j - 1], dir_cos);
  }

  // TODO: fix this to appropriate
  m_MINCPImpl->m_Volume_type = MI_TYPE_FLOAT;
  m_MINCPImpl->m_Volume_class = MI_CLASS_REAL;

  switch (this->GetComponentType())
  {
    case IOComponentEnum::UCHAR:
      m_MINCPImpl->m_Volume_type = MI_TYPE_UBYTE;
      break;
    case IOComponentEnum::CHAR:
      m_MINCPImpl->m_Volume_type = MI_TYPE_BYTE;
      break;
    case IOComponentEnum::USHORT:
      m_MINCPImpl->m_Volume_type = MI_TYPE_USHORT;
      break;
    case IOComponentEnum::SHORT:
      m_MINCPImpl->m_Volume_type = MI_TYPE_SHORT;
      break;
    case IOComponentEnum::UINT:
      m_MINCPImpl->m_Volume_type = MI_TYPE_UINT;
      break;
    case IOComponentEnum::INT:
      m_MINCPImpl->m_Volume_type = MI_TYPE_INT;
      break;
      //     case IOComponentEnum::ULONG://TODO: make sure we are cross-platform here!
      //       volume_data_type=MI_TYPE_ULONG;
      //       break;
      //     case IOComponentEnum::LONG://TODO: make sure we are cross-platform here!
      //       volume_data_type=MI_TYPE_LONG;
      //       break;
    case IOComponentEnum::FLOAT:
      m_MINCPImpl->m_Volume_type = MI_TYPE_FLOAT;
      break;
    case IOComponentEnum::DOUBLE:
      m_MINCPImpl->m_Volume_type = MI_TYPE_DOUBLE;
      break;
    default:
      MINCIOFreeTmpDimHandle(minc_dimensions, m_MINCPImpl->m_MincApparentDims);
      itkExceptionMacro("Could read datatype " << this->GetComponentType());
  }

  std::string storage_data_type;

  // perform storage of floating point data using fixed point arithmetics
  if ((this->GetComponentType() == IOComponentEnum::FLOAT || this->GetComponentType() == IOComponentEnum::DOUBLE) &&
      ExposeMetaData<std::string>(thisDic, "storage_data_type", storage_data_type))
  {
    if (storage_data_type == typeid(char).name())
    {
      m_MINCPImpl->m_Volume_type = MI_TYPE_BYTE;
    }
    else if (storage_data_type == typeid(unsigned char).name())
      m_MINCPImpl->m_Volume_type = MI_TYPE_UBYTE;
    else if (storage_data_type == typeid(short).name())
      m_MINCPImpl->m_Volume_type = MI_TYPE_SHORT;
    else if (storage_data_type == typeid(unsigned short).name())
      m_MINCPImpl->m_Volume_type = MI_TYPE_USHORT;
    else if (storage_data_type == typeid(int).name())
      m_MINCPImpl->m_Volume_type = MI_TYPE_INT;
    else if (storage_data_type == typeid(unsigned int).name())
      m_MINCPImpl->m_Volume_type = MI_TYPE_UINT;
    else if (storage_data_type == typeid(float).name())
      m_MINCPImpl->m_Volume_type = MI_TYPE_FLOAT;
    else if (storage_data_type == typeid(double).name())
      m_MINCPImpl->m_Volume_type = MI_TYPE_DOUBLE;
  }
  // now let's create the same dimension order and positive/negative step size as
  // in original image
  std::string dimension_order;
  bool        dimorder_good = false;
  if (ExposeMetaData<std::string>(thisDic, "dimension_order", dimension_order))
  {
    // the format should be ((+|-)(X|Y|Z|V|T))*
    if (dimension_order.length() == (minc_dimensions * 2))
    {
      dimorder_good = true;
      for (unsigned int i = 0; i < minc_dimensions && dimorder_good; ++i)
      {
        const bool positive = (dimension_order[i * 2] == '+');
        int        j = 0;
        switch (dimension_order[i * 2 + 1])
        {
          case 'v':
          case 'V':
            if (nComp <= 1)
            {
              itkDebugMacro("Dimension order is incorrect " << dimension_order);
              dimorder_good = false;
            }
            else
            {
              j = m_MINCPImpl->m_NDims - 1;
            }
            break;
          case 't':
          case 'T':
            if (nComp <= 1)
            {
              itkDebugMacro("Dimension order is incorrect " << dimension_order);
              dimorder_good = false;
            }
            else
            {
              j = m_MINCPImpl->m_NDims - 1;
            }
            break;
          case 'x':
          case 'X':
            j = m_MINCPImpl->m_NDims - 1 - ((nComp > 1 ? 1 : 0));
            break;
          case 'y':
          case 'Y':
            j = m_MINCPImpl->m_NDims - 1 - ((nComp > 1 ? 1 : 0) + 1);
            break;
          case 'z':
          case 'Z':
            j = m_MINCPImpl->m_NDims - 1 - ((nComp > 1 ? 1 : 0) + 2);
            break;
          default:
            itkDebugMacro("Dimension order is incorrect " << dimension_order);
            dimorder_good = false;
            j = 0;
            break;
        }

        if (dimorder_good)
        {
          // flip the sign
          if (!positive && dimension_order[i * 2 + 1] != 'V' &&
              dimension_order[i * 2 + 1] != 'v') // Vector dimension is always positive
          {
            double _sep = NAN;
            miget_dimension_separation(m_MINCPImpl->m_MincApparentDims[j], MI_ORDER_FILE, &_sep);
            double _start = NAN;
            miget_dimension_start(m_MINCPImpl->m_MincApparentDims[j], MI_ORDER_FILE, &_start);
            misize_t _sz = 0;
            miget_dimension_size(m_MINCPImpl->m_MincApparentDims[j], &_sz);

            _start = _start + (_sz - 1) * _sep;
            _sep *= -1;

            miset_dimension_separation(m_MINCPImpl->m_MincApparentDims[j], _sep);
            miset_dimension_start(m_MINCPImpl->m_MincApparentDims[j], _start);

            miset_dimension_apparent_voxel_order(m_MINCPImpl->m_MincApparentDims[j], MI_POSITIVE);
          }
          // Hmmm.... what are we going to get in the end?
          m_MINCPImpl->m_MincFileDims[i] = m_MINCPImpl->m_MincApparentDims[j];
        }
      }
    }
    else
    {
      itkDebugMacro("Dimension order is incorrect " << dimension_order);
    }
  }

  if (!dimorder_good) // use default order!
  {
    for (unsigned int i = 0; i < minc_dimensions; ++i)
    {
      m_MINCPImpl->m_MincFileDims[i] = m_MINCPImpl->m_MincApparentDims[i];
    }
  }

  mivolumeprops_t hprops = nullptr;
  if (minew_volume_props(&hprops) < 0)
  {
    MINCIOFreeTmpDimHandle(minc_dimensions, m_MINCPImpl->m_MincApparentDims);
    itkExceptionMacro("Could not allocate MINC properties");
  }

  if (this->m_UseCompression)
  {
    if (miset_props_compression_type(hprops, MI_COMPRESS_ZLIB) < 0)
    {
      MINCIOFreeTmpDimHandle(minc_dimensions, m_MINCPImpl->m_MincApparentDims);
      mifree_volume_props(hprops);
      itkExceptionMacro("Could not set MINC compression");
    }

    if (miset_props_zlib_compression(hprops, this->GetCompressionLevel()) < 0)
    {
      MINCIOFreeTmpDimHandle(minc_dimensions, m_MINCPImpl->m_MincApparentDims);
      mifree_volume_props(hprops);
      itkExceptionMacro("Could not set MINC compression level");
    }
  }
  else
  {
    if (miset_props_compression_type(hprops, MI_COMPRESS_NONE) < 0)
    {
      MINCIOFreeTmpDimHandle(minc_dimensions, m_MINCPImpl->m_MincApparentDims);
      mifree_volume_props(hprops);
      itkExceptionMacro("Could not set MINC compression");
    }
  }

  if (micreate_volume(m_FileName.c_str(),
                      minc_dimensions,
                      m_MINCPImpl->m_MincFileDims,
                      m_MINCPImpl->m_Volume_type,
                      m_MINCPImpl->m_Volume_class,
                      hprops,
                      &m_MINCPImpl->m_Volume) < 0)
  {
    // Error opening the volume
    MINCIOFreeTmpDimHandle(minc_dimensions, m_MINCPImpl->m_MincApparentDims);
    mifree_volume_props(hprops);
    itkExceptionMacro("Could not open file \"" << m_FileName << "\".");
  }

  if (micreate_volume_image(m_MINCPImpl->m_Volume) < 0)
  {
    // Error opening the volume
    mifree_volume_props(hprops);
    itkExceptionMacro("Could not create image in  file \"" << m_FileName << "\".");
  }

  if (miset_apparent_dimension_order(m_MINCPImpl->m_Volume, minc_dimensions, m_MINCPImpl->m_MincApparentDims) < 0)
  {
    mifree_volume_props(hprops);
    itkExceptionMacro(" Can't set apparent dimension order!");
  }

  if (miset_slice_scaling_flag(m_MINCPImpl->m_Volume, 0) < 0)
  {
    mifree_volume_props(hprops);
    itkExceptionMacro("Could not set slice scaling flag");
  }

  double valid_min = NAN;
  double valid_max = NAN;
  miget_volume_valid_range(m_MINCPImpl->m_Volume, &valid_max, &valid_min);

  // by default valid range will be equal to range, to avoid scaling
  miset_volume_range(m_MINCPImpl->m_Volume, valid_max, valid_min);

  for (auto it = thisDic.Begin(); it != thisDic.End(); ++it)
  {
    // don't store some internal ITK junk
    if (it->first == "ITK_InputFilterName" || it->first == "NRRD_content" || it->first == "NRRD_centerings[0]" ||
        it->first == "NRRD_centerings[1]" || it->first == "NRRD_centerings[2]" || it->first == "NRRD_centerings[3]" ||
        it->first == "NRRD_kinds[0]" || it->first == "NRRD_kinds[1]" || it->first == "NRRD_kinds[2]" ||
        it->first == "NRRD_kinds[3]" || it->first == "NRRD_space")
      continue;

    const char *         d = strchr(it->first.c_str(), ':');
    MetaDataObjectBase * bs = it->second;
    if (d)
    {
      const std::string var(it->first.c_str(), d - it->first.c_str());
      const std::string att(d + 1);

      // VF:THIS is not good OO style at all :(
      const char * tname = bs->GetMetaDataObjectTypeName();
      if (!strcmp(tname, typeid(std::string).name()))
      {
        const std::string & tmp = dynamic_cast<MetaDataObject<std::string> *>(bs)->GetMetaDataObjectValue();
        miset_attr_values(
          m_MINCPImpl->m_Volume, MI_TYPE_STRING, var.c_str(), att.c_str(), tmp.length() + 1, tmp.c_str());
      }
      else if (!strcmp(tname, typeid(Array<double>).name()))
      {
        const Array<double> & tmp = dynamic_cast<MetaDataObject<Array<double>> *>(bs)->GetMetaDataObjectValue();
        miset_attr_values(
          m_MINCPImpl->m_Volume, MI_TYPE_DOUBLE, var.c_str(), att.c_str(), tmp.Size(), tmp.data_block());
      }
      else if (!strcmp(tname, typeid(Array<float>).name()))
      {
        const Array<float> & tmp = dynamic_cast<MetaDataObject<Array<float>> *>(bs)->GetMetaDataObjectValue();
        miset_attr_values(m_MINCPImpl->m_Volume, MI_TYPE_FLOAT, var.c_str(), att.c_str(), tmp.Size(), tmp.data_block());
      }
      else if (!strcmp(tname, typeid(Array<int>).name()))
      {
        const Array<int> & tmp = dynamic_cast<MetaDataObject<Array<int>> *>(bs)->GetMetaDataObjectValue();
        miset_attr_values(m_MINCPImpl->m_Volume, MI_TYPE_INT, var.c_str(), att.c_str(), tmp.Size(), tmp.data_block());
      }
      else if (!strcmp(tname, typeid(double).name()))
      {
        double tmp = dynamic_cast<MetaDataObject<double> *>(bs)->GetMetaDataObjectValue();
        miset_attr_values(m_MINCPImpl->m_Volume, MI_TYPE_DOUBLE, var.c_str(), att.c_str(), 1, &tmp);
      }
      else if (!strcmp(tname, typeid(float).name()))
      {
        float tmp = dynamic_cast<MetaDataObject<float> *>(bs)->GetMetaDataObjectValue();
        miset_attr_values(m_MINCPImpl->m_Volume, MI_TYPE_FLOAT, var.c_str(), att.c_str(), 1, &tmp);
      }
      else if (!strcmp(tname, typeid(int).name()))
      {
        int tmp = dynamic_cast<MetaDataObject<int> *>(bs)->GetMetaDataObjectValue();
        miset_attr_values(m_MINCPImpl->m_Volume, MI_TYPE_INT, var.c_str(), att.c_str(), 1, &tmp);
      }
      else
      {
        itkDebugMacro("Unsupported metadata type:" << tname);
      }
    }
    else if (it->first == "history")
    {
      const std::string & tmp = dynamic_cast<MetaDataObject<std::string> *>(bs)->GetMetaDataObjectValue();
      miset_attr_values(m_MINCPImpl->m_Volume, MI_TYPE_STRING, "", "history", tmp.length() + 1, tmp.c_str());
    }
    else
    { // else we have an attribute without variable name, most likely
      // it comes from another file type
      // TODO: figure out what to do with it
    }
  }

  // preserve information of MINC PositiveCoordinateOrientation RAS to ITK PositiveCoordinateOrientation LPS conversion
  {
    int tmp = static_cast<int>(this->m_RAStoLPS);
    miset_attr_values(m_MINCPImpl->m_Volume, MI_TYPE_INT, "itk", "RAStoLPS", 1, &tmp);
  }

  mifree_volume_props(hprops);
}

template <typename T>
void
get_buffer_min_max(const void * _buffer, size_t len, double & buf_min, double & buf_max)
{
  const auto * buf = static_cast<const T *>(_buffer);

  buf_min = buf_max = buf[0];
  for (size_t i = 0; i < len; ++i)
  {
    if (buf[i] < static_cast<double>(buf_min))
    {
      buf_min = static_cast<double>(buf[i]);
    }
    if (buf[i] > static_cast<double>(buf_max))
    {
      buf_max = static_cast<double>(buf[i]);
    }
  }
}

void
MINCImageIO::Write(const void * buffer)
{
  const unsigned int nDims = this->GetNumberOfDimensions();
  const unsigned int nComp = this->GetNumberOfComponents();
  size_t             buffer_length = 1;

  const auto start = make_unique_for_overwrite<misize_t[]>(nDims + (nComp > 1 ? 1 : 0));
  const auto count = make_unique_for_overwrite<misize_t[]>(nDims + (nComp > 1 ? 1 : 0));

  for (unsigned int i = 0; i < nDims; ++i)
  {
    if (i < m_IORegion.GetImageDimension())
    {
      start[nDims - i - 1] = m_IORegion.GetIndex()[i];
      count[nDims - i - 1] = m_IORegion.GetSize()[i];
      buffer_length *= m_IORegion.GetSize()[i];
    }
    else
    {
      start[nDims - i - 1] = 0;
      count[nDims - i - 1] = 1;
    }
  }

  if (nComp > 1)
  {
    start[nDims] = 0;
    count[nDims] = nComp;
    buffer_length *= nComp;
  }

  double   buffer_min = NAN;
  double   buffer_max = NAN;
  mitype_t volume_data_type = MI_TYPE_UBYTE;

  switch (this->GetComponentType())
  {
    case IOComponentEnum::UCHAR:
      volume_data_type = MI_TYPE_UBYTE;
      get_buffer_min_max<unsigned char>(buffer, buffer_length, buffer_min, buffer_max);
      break;
    case IOComponentEnum::CHAR:
      volume_data_type = MI_TYPE_BYTE;
      get_buffer_min_max<signed char>(buffer, buffer_length, buffer_min, buffer_max);
      break;
    case IOComponentEnum::USHORT:
      volume_data_type = MI_TYPE_USHORT;
      get_buffer_min_max<unsigned short>(buffer, buffer_length, buffer_min, buffer_max);
      break;
    case IOComponentEnum::SHORT:
      volume_data_type = MI_TYPE_SHORT;
      get_buffer_min_max<short>(buffer, buffer_length, buffer_min, buffer_max);
      break;
    case IOComponentEnum::UINT:
      volume_data_type = MI_TYPE_UINT;
      get_buffer_min_max<unsigned int>(buffer, buffer_length, buffer_min, buffer_max);
      break;
    case IOComponentEnum::INT:
      volume_data_type = MI_TYPE_INT;
      get_buffer_min_max<int>(buffer, buffer_length, buffer_min, buffer_max);
      break;
      //     case IOComponentEnum::ULONG://TODO: make sure we are cross-platform here!
      //       volume_data_type=MI_TYPE_ULONG;
      //       break;
      //     case IOComponentEnum::LONG://TODO: make sure we are cross-platform here!
      //       volume_data_type=MI_TYPE_LONG;
      //       break;
    case IOComponentEnum::FLOAT: // TODO: make sure we are cross-platform here!
      volume_data_type = MI_TYPE_FLOAT;
      get_buffer_min_max<float>(buffer, buffer_length, buffer_min, buffer_max);
      break;
    case IOComponentEnum::DOUBLE: // TODO: make sure we are cross-platform here!
      volume_data_type = MI_TYPE_DOUBLE;
      get_buffer_min_max<double>(buffer, buffer_length, buffer_min, buffer_max);
      break;
    default:
      itkExceptionMacro("Could not read datatype " << this->GetComponentType());
  }

  this->WriteImageInformation();

  // by default valid range will be equal to range, to avoid scaling
  if (volume_data_type == m_MINCPImpl->m_Volume_type)
  {
    miset_volume_valid_range(m_MINCPImpl->m_Volume, buffer_max, buffer_min);
    miset_volume_range(m_MINCPImpl->m_Volume, buffer_max, buffer_min);
  }
  else // we are using scaling
  {
    // Special hack to deal with rounding errors
    // unfortunately the dynamic range will be smaller
    // but it's ok since it matches float
    if (this->GetComponentType() == IOComponentEnum::FLOAT)
    {
      if (m_MINCPImpl->m_Volume_type == MI_TYPE_INT)
      {
        miset_volume_valid_range(m_MINCPImpl->m_Volume, INT_MAX / 2, INT_MIN / 2);
      }
      else if (m_MINCPImpl->m_Volume_type == MI_TYPE_UINT)
      {
        miset_volume_valid_range(m_MINCPImpl->m_Volume, UINT_MAX / 2, 0);
      }
    }
    miset_volume_range(m_MINCPImpl->m_Volume, buffer_max, buffer_min);
  }

  if (miset_real_value_hyperslab(
        m_MINCPImpl->m_Volume, volume_data_type, start.get(), count.get(), const_cast<void *>(buffer)) < 0)
  {
    itkExceptionMacro(" Can not set real value hyperslab!!\n");
  }
  // TODO: determine what to do if we are streaming
  this->CloseVolume();
}

} // end namespace itk
