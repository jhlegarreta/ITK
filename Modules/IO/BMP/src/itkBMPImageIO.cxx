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
#include "itkBMPImageIO.h"
#include "itkByteSwapper.h"
#include "itkMakeUniqueForOverwrite.h"
#include "itksys/SystemTools.hxx"
#include <iostream>

namespace itk
{
BMPImageIO::BMPImageIO()
  : m_ColorPalette(0) // palette has no element by default
{
  this->SetNumberOfDimensions(2);

  m_ByteOrder = IOByteOrderEnum::BigEndian;
  m_ComponentType = IOComponentEnum::UCHAR;
  m_PixelType = IOPixelEnum::SCALAR;

  m_Spacing[0] = 1.0;
  m_Spacing[1] = 1.0;

  m_Origin[0] = 0.0;
  m_Origin[1] = 0.0;

  const char * extensions[] = {
    ".bmp",
    ".BMP",
  };

  for (auto ext : extensions)
  {
    this->AddSupportedWriteExtension(ext);
    this->AddSupportedReadExtension(ext);
  }
}

BMPImageIO::~BMPImageIO() = default;

bool
BMPImageIO::CanReadFile(const char * filename)
{
  // First check the filename
  const std::string fname = filename;

  if (fname.empty())
  {
    itkDebugMacro("No filename specified.");
  }


  const bool extensionFound = this->HasSupportedReadExtension(filename, false);

  if (!extensionFound)
  {
    itkDebugMacro("The filename extension is not recognized");
  }

  // Now check the content
  std::ifstream inputStream;
  try
  {
    this->OpenFileForReading(inputStream, fname);
  }
  catch (const ExceptionObject &)
  {
    return false;
  }
  {
    char magic_number1 = 0;
    inputStream.read(&magic_number1, sizeof(char));
    char magic_number2 = 0;
    inputStream.read(&magic_number2, sizeof(char));

    if ((magic_number1 != 'B') || (magic_number2 != 'M'))
    {
      inputStream.close();
      return false;
    }
  }


  // get the size of the file
  constexpr size_t sizeLong = sizeof(long);
  if (sizeLong == 4)
  {
    long tmp = 0;
    inputStream.read(reinterpret_cast<char *>(&tmp), 4);
    // skip 4 bytes
    inputStream.read(reinterpret_cast<char *>(&tmp), 4);
    // read the offset
    inputStream.read(reinterpret_cast<char *>(&tmp), 4);
  }
  else
  {
    int itmp = 0; // in case we are on a 64bit machine
    inputStream.read(reinterpret_cast<char *>(&itmp), 4);
    // skip 4 bytes
    inputStream.read(reinterpret_cast<char *>(&itmp), 4);
    // read the offset
    inputStream.read(reinterpret_cast<char *>(&itmp), 4);
  }

  // get size of header
  if (sizeLong == 4) // if we are on a 32 bit machine
  {
    long infoSize = 0;
    inputStream.read(reinterpret_cast<char *>(&infoSize), sizeof(long));
    ByteSwapper<long>::SwapFromSystemToLittleEndian(&infoSize);
    // error checking
    if ((infoSize != 40) && (infoSize != 12))
    {
      inputStream.close();
      return false;
    }
  }
  else // else we are on a 64bit machine
  {
    int iinfoSize = 0; // in case we are on a 64bit machine
    inputStream.read(reinterpret_cast<char *>(&iinfoSize), 4);
    ByteSwapper<int>::SwapFromSystemToLittleEndian(&iinfoSize);
    const long infoSize = iinfoSize;

    // error checking
    if ((infoSize != 40) && (infoSize != 12))
    {
      inputStream.close();
      return false;
    }
  }

  inputStream.close();
  return true;
}

bool
BMPImageIO::CanWriteFile(const char * name)
{
  const std::string filename = name;

  if (filename.empty())
  {
    itkDebugMacro("No filename specified.");
  }

  const bool extensionFound = this->HasSupportedWriteExtension(name, false);

  if (!extensionFound)
  {
    itkDebugMacro("The filename extension is not recognized");
    return false;
  }

  return true;
}

void
BMPImageIO::Read(void * buffer)
{
  auto *        p = static_cast<char *>(buffer);
  unsigned long l = 0;

  this->OpenFileForReading(m_Ifstream, m_FileName);

  // If the file is RLE compressed
  // RLE-compressed files are lower-left
  // About the RLE compression algorithm:
  // https://msdn.microsoft.com/en-us/library/windows/desktop/dd183383%28v=vs.85%29.aspx
  if (m_BMPCompression == 1 && (this->GetNumberOfComponents() == 3 || this->GetIsReadAsScalarPlusPalette()))
  {
    const auto value = make_unique_for_overwrite<char[]>(m_BMPDataSize + 1);
    m_Ifstream.seekg(m_BitMapOffset, std::ios::beg);
    m_Ifstream.read(value.get(), m_BMPDataSize);

    SizeValueType posLine = 0;
    SizeValueType line = m_Dimensions[1] - 1;
    for (unsigned int i = 0; i < m_BMPDataSize; ++i)
    {
      const unsigned char byte1 = value[i];
      ++i;
      const unsigned char byte2 = value[i];
      if (byte1 == 0)
      {
        if (byte2 == 0)
        {
          // End of line
          --line;
          posLine = 0;
          continue;
        }
        if (byte2 == 1)
        {
          // End of bitmap data
          break;
        }
        else if (byte2 == 2)
        {
          // Delta
          ++i;
          const unsigned char dx = value[i];
          ++i;
          const unsigned char dy = value[i];
          posLine += dx;
          line -= dy;
          continue;
        }
        else
        {
          // Unencoded run
          if (!this->GetIsReadAsScalarPlusPalette())
          {
            for (unsigned long j = 0; j < byte2; ++j)
            {
              ++i;
              const RGBPixelType rgb = this->GetColorPaletteEntry(value[i]);
              l = 3 * (line * m_Dimensions[0] + posLine);
              p[l] = rgb.GetBlue();
              p[l + 1] = rgb.GetGreen();
              p[l + 2] = rgb.GetRed();
              ++posLine;
            }
          }
          else
          {
            for (unsigned long j = 0; j < byte2; ++j)
            {
              ++i;
              l = (line * m_Dimensions[0] + posLine);
              p[l] = value[i];
              ++posLine;
            }
          }
          // If a run's length is odd, the it is padded with 0
          if (byte2 % 2)
          {
            ++i;
          }
        }
      }
      else
      {
        // Encoded run
        if (!this->GetIsReadAsScalarPlusPalette())
        {
          const RGBPixelType rgb = this->GetColorPaletteEntry(byte2);
          for (unsigned long j = 0; j < byte1; ++j)
          {
            l = 3 * (line * m_Dimensions[0] + posLine);
            p[l] = rgb.GetBlue();
            p[l + 1] = rgb.GetGreen();
            p[l + 2] = rgb.GetRed();
            ++posLine;
          }
        }
        else
        {
          for (unsigned long j = 0; j < byte1; ++j)
          {
            l = (line * m_Dimensions[0] + posLine);
            p[l] = byte2;
            ++posLine;
          }
        }
      }
    }
  }
  else
  {
    // File is not compressed
    // Read one row at a time
    const long          streamRead = m_Dimensions[0] * m_Depth / 8;
    long                paddedStreamRead = streamRead;
    const unsigned long step = this->GetNumberOfComponents();
    if (streamRead % 4)
    {
      paddedStreamRead = ((streamRead / 4) + 1) * 4;
    }
    const auto value = make_unique_for_overwrite<char[]>(paddedStreamRead + 1);

    for (unsigned int id = 0; id < m_Dimensions[1]; ++id)
    {
      const unsigned int line_id = m_FileLowerLeft ? (m_Dimensions[1] - id - 1) : id;
      m_Ifstream.seekg(m_BitMapOffset + paddedStreamRead * line_id, std::ios::beg);
      m_Ifstream.read(value.get(), paddedStreamRead);
      for (long i = 0; i < streamRead; ++i)
      {
        if (this->GetNumberOfComponents() == 1)
        {
          p[l++] = value[i];
        }
        else
        {
          if (m_ColorPaletteSize == 0)
          {
            if (this->GetNumberOfComponents() == 3)
            {
              p[l++] = value[i + 2]; // red
              p[l++] = value[i + 1]; // green
              p[l++] = value[i];     // blue
            }
            if (this->GetNumberOfComponents() == 4)
            {
              p[l++] = value[i + 2]; // red
              p[l++] = value[i + 1]; // green
              p[l++] = value[i];     // blue
              p[l++] = value[i + 3]; // alpha
            }
            i += step - 1;
          }
          else
          {
            const RGBPixelType rgb = this->GetColorPaletteEntry(value[i]);
            p[l++] = rgb.GetBlue();
            p[l++] = rgb.GetGreen();
            p[l++] = rgb.GetRed();
          }
        }
      }
    }
  }
  m_Ifstream.close();
}

/**
 *  Read Information about the BMP file
 *  and put the cursor of the stream just before the first data pixel
 */
void
BMPImageIO::ReadImageInformation()
{
  // Now check the content
  this->OpenFileForReading(m_Ifstream, m_FileName);

  {
    char magic_number1 = 0;
    m_Ifstream.read(&magic_number1, sizeof(char));
    char magic_number2 = 0;
    m_Ifstream.read(&magic_number2, sizeof(char));

    if ((magic_number1 != 'B') || (magic_number2 != 'M'))
    {
      m_Ifstream.close();
      itkExceptionMacro("BMPImageIO : Magic Number Fails = " << magic_number1 << " : " << magic_number2);
    }
  }

  // get the size of the file
  constexpr size_t sizeLong = sizeof(long);
  if (sizeLong == 4)
  {
    long tmp = 0;
    m_Ifstream.read(reinterpret_cast<char *>(&tmp), 4);
    // skip 4 bytes
    m_Ifstream.read(reinterpret_cast<char *>(&tmp), 4);
    // read the offset
    m_Ifstream.read(reinterpret_cast<char *>(&tmp), 4);
    m_BitMapOffset = tmp;
    ByteSwapper<long>::SwapFromSystemToLittleEndian(&m_BitMapOffset);
  }
  else
  {
    int itmp = 0; // in case we are on a 64bit machine
    m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
    // skip 4 bytes
    m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
    // read the offset
    m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
    ByteSwapper<int>::SwapFromSystemToLittleEndian(&itmp);
    m_BitMapOffset = static_cast<long>(itmp);
  }

  int  xsize = 0;
  int  ysize = 0;
  long infoSize = 0;
  // get size of header
  if (sizeLong == 4) // if we are on a 32 bit machine
  {
    m_Ifstream.read(reinterpret_cast<char *>(&infoSize), 4);
    ByteSwapper<long>::SwapFromSystemToLittleEndian(&infoSize);
    // error checking
    if ((infoSize != 40) && (infoSize != 12))
    {
      itkExceptionMacro("Unknown file type! " << m_FileName << " is not a Windows BMP file!");
    }

    // there are two different types of BMP files
    if (infoSize == 40)
    {
      // now get the dimensions
      m_Ifstream.read(reinterpret_cast<char *>(&xsize), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&xsize);
      m_Ifstream.read(reinterpret_cast<char *>(&ysize), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&ysize);
    }
    else
    {
      short stmp = 0;
      m_Ifstream.read(reinterpret_cast<char *>(&stmp), sizeof(short));
      ByteSwapper<short>::SwapFromSystemToLittleEndian(&stmp);
      xsize = stmp;
      m_Ifstream.read(reinterpret_cast<char *>(&stmp), sizeof(short));
      ByteSwapper<short>::SwapFromSystemToLittleEndian(&stmp);
      ysize = stmp;
    }
  }
  else // else we are on a 64bit machine
  {
    int iinfoSize = 0; // in case we are on a 64bit machine

    m_Ifstream.read(reinterpret_cast<char *>(&iinfoSize), sizeof(int));
    ByteSwapper<int>::SwapFromSystemToLittleEndian(&iinfoSize);
    infoSize = iinfoSize;

    // error checking
    if ((infoSize != 40) && (infoSize != 12))
    {
      itkExceptionMacro("Unknown file type! " << m_FileName << " is not a Windows BMP file!");
    }

    // there are two different types of BMP files
    if (infoSize == 40)
    {
      // now get the dimensions
      m_Ifstream.read(reinterpret_cast<char *>(&xsize), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&xsize);
      m_Ifstream.read(reinterpret_cast<char *>(&ysize), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&ysize);
    }
    else
    {
      short stmp = 0;
      m_Ifstream.read(reinterpret_cast<char *>(&stmp), 2);
      ByteSwapper<short>::SwapFromSystemToLittleEndian(&stmp);
      xsize = stmp;
      m_Ifstream.read(reinterpret_cast<char *>(&stmp), 2);
      ByteSwapper<short>::SwapFromSystemToLittleEndian(&stmp);
      ysize = stmp;
    }
  }

  // is corner in upper left or lower left
  if (ysize < 0)
  {
    ysize *= -1;
    m_FileLowerLeft = false;
  }
  else
  {
    m_FileLowerLeft = true;
  }

  this->SetNumberOfDimensions(2);
  m_Dimensions[0] = xsize;
  m_Dimensions[1] = ysize;

  // ignore planes
  short stmp = 0;
  m_Ifstream.read(reinterpret_cast<char *>(&stmp), 2);
  // read depth
  m_Ifstream.read(reinterpret_cast<char *>(&m_Depth), 2);
  ByteSwapper<short>::SwapFromSystemToLittleEndian(&m_Depth);

  if ((m_Depth != 8) && (m_Depth != 24) && (m_Depth != 32))
  {
    m_Ifstream.close();
    itkExceptionMacro("Only BMP depths of (8,24,32) are supported. Not " << m_Depth);
  }

  if (infoSize == 40)
  {
    if (sizeLong == 4)
    {
      // Compression
      m_Ifstream.read(reinterpret_cast<char *>(&m_BMPCompression), 4);
      ByteSwapper<long>::SwapFromSystemToLittleEndian(&m_BMPCompression);
      // Image Data Size
      m_Ifstream.read(reinterpret_cast<char *>(&m_BMPDataSize), 4);
      ByteSwapper<unsigned long>::SwapFromSystemToLittleEndian(&m_BMPDataSize);
      // Horizontal Resolution
      long tmp = 0;
      m_Ifstream.read(reinterpret_cast<char *>(&tmp), 4);
      // Vertical Resolution
      m_Ifstream.read(reinterpret_cast<char *>(&tmp), 4);
      // Number of colors
      m_Ifstream.read(reinterpret_cast<char *>(&tmp), 4);
      m_NumberOfColors = static_cast<unsigned short>(tmp);
      // Number of important colors
      m_Ifstream.read(reinterpret_cast<char *>(&tmp), 4);
    }
    else
    {
      int itmp = 0; // in case we are on a 64bit machine
      // Compression
      m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&itmp);
      m_BMPCompression = static_cast<long>(itmp);
      // Image Data Size
      m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&itmp);
      m_BMPDataSize = static_cast<unsigned long>(itmp);
      // Horizontal Resolution
      m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&itmp);
      // Vertical Resolution
      m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&itmp);
      // Number of colors
      m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
      ByteSwapper<int>::SwapFromSystemToLittleEndian(&itmp);
      m_NumberOfColors = static_cast<unsigned short>(itmp);
      // Number of important colors
      m_Ifstream.read(reinterpret_cast<char *>(&itmp), 4);
    }
  }

  // https://msdn.microsoft.com/en-us/library/windows/desktop/dd183376%28v=vs.85%29.aspx
  if (m_BMPCompression == 1 && !m_FileLowerLeft)
  {
    m_Ifstream.close();
    itkExceptionMacro("Compressed BMP are not supposed to be upper-left.");
  }

  // Read the color palette. Only used for 1,4 and 8 bit images.
  if (m_Depth <= 8)
  {
    if (m_NumberOfColors)
    {
      m_ColorPaletteSize = ((1 << m_Depth) < m_NumberOfColors) ? (1 << m_Depth) : m_NumberOfColors;
    }
    else
    {
      m_ColorPaletteSize = (1 << m_Depth);
    }
  }
  else
  {
    m_ColorPaletteSize = 0;
  }
  unsigned char uctmp = 0;
  m_ColorPalette.resize(m_ColorPaletteSize);
  for (unsigned long i = 0; i < m_ColorPaletteSize; ++i)
  {
    RGBPixelType p;
    m_Ifstream.read(reinterpret_cast<char *>(&uctmp), 1);
    p.SetRed(uctmp);
    m_Ifstream.read(reinterpret_cast<char *>(&uctmp), 1);
    p.SetGreen(uctmp);
    m_Ifstream.read(reinterpret_cast<char *>(&uctmp), 1);
    p.SetBlue(uctmp);
    long tmp = 0;
    m_Ifstream.read(reinterpret_cast<char *>(&tmp), 1);
    m_ColorPalette[i] = p;
  }

  m_IsReadAsScalarPlusPalette = false;
  switch (m_Depth)
  {
    case 1:
    case 4:
    case 8:
    {
      if (this->GetExpandRGBPalette())
      {
        this->SetNumberOfComponents(3);
        m_PixelType = IOPixelEnum::RGB;
      }
      else
      {
        this->SetNumberOfComponents(1);
        m_PixelType = IOPixelEnum::SCALAR;
        m_IsReadAsScalarPlusPalette = true;
      }
      break;
    }
    case 24:
    {
      this->SetNumberOfComponents(3);
      m_PixelType = IOPixelEnum::RGB;
      break;
    }
    case 32:
    {
      this->SetNumberOfComponents(4);
      m_PixelType = IOPixelEnum::RGBA;
      break;
    }
  }

  m_Ifstream.close();
}

void
BMPImageIO::SwapBytesIfNecessary(void * buffer, SizeValueType numberOfPixels)
{
  switch (m_ComponentType)
  {
    case IOComponentEnum::CHAR:
    case IOComponentEnum::UCHAR:
    {
      // For CHAR and UCHAR, it is not necessary to swap bytes.
      break;
    }
    case IOComponentEnum::SHORT:
    {
      if (m_ByteOrder == IOByteOrderEnum::LittleEndian)
      {
        ByteSwapper<short>::SwapRangeFromSystemToLittleEndian(static_cast<short *>(buffer), numberOfPixels);
      }
      else if (m_ByteOrder == IOByteOrderEnum::BigEndian)
      {
        ByteSwapper<short>::SwapRangeFromSystemToBigEndian(static_cast<short *>(buffer), numberOfPixels);
      }
      break;
    }
    case IOComponentEnum::USHORT:
    {
      if (m_ByteOrder == IOByteOrderEnum::LittleEndian)
      {
        ByteSwapper<unsigned short>::SwapRangeFromSystemToLittleEndian(static_cast<unsigned short *>(buffer),
                                                                       numberOfPixels);
      }
      else if (m_ByteOrder == IOByteOrderEnum::BigEndian)
      {
        ByteSwapper<unsigned short>::SwapRangeFromSystemToBigEndian(static_cast<unsigned short *>(buffer),
                                                                    numberOfPixels);
      }
      break;
    }
    default:
      itkExceptionMacro("Pixel Type Unknown");
  }
}

void
BMPImageIO::Write32BitsInteger(unsigned int value)
{
  auto tmp = static_cast<char>(value % 256);
  m_Ofstream.write(&tmp, sizeof(char));
  tmp = static_cast<char>((value % 65536L) / 256);
  m_Ofstream.write(&tmp, sizeof(char));
  tmp = static_cast<char>((value / 65536L) % 256);
  m_Ofstream.write(&tmp, sizeof(char));
  tmp = static_cast<char>((value / 65536L) / 256);
  m_Ofstream.write(&tmp, sizeof(char));
}

void
BMPImageIO::Write16BitsInteger(unsigned short value)
{
  auto tmp = static_cast<char>(value % 256);
  m_Ofstream.write(&tmp, sizeof(char));
  tmp = static_cast<char>((value % 65536L) / 256);
  m_Ofstream.write(&tmp, sizeof(char));
}

BMPImageIO::RGBPixelType
BMPImageIO::GetColorPaletteEntry(const unsigned char entry) const
{
  if (entry < m_ColorPalette.size())
  {
    return m_ColorPalette[entry];
  }

  RGBPixelType p;
  p.SetRed(0);
  p.SetGreen(0);
  p.SetBlue(0);
  return p;
}

void
BMPImageIO::WriteImageInformation()
{}

void
BMPImageIO::Write(const void * buffer)
{
  const unsigned int nDims = this->GetNumberOfDimensions();

  if (nDims != 2)
  {
    itkExceptionMacro("BMPImageIO cannot write images with a dimension != 2");
  }

  if (this->GetComponentType() != IOComponentEnum::UCHAR)
  {
    itkExceptionMacro("BMPImageIO supports unsigned char only");
  }
  if ((this->m_NumberOfComponents != 1) && (this->m_NumberOfComponents != 3) && (this->m_NumberOfComponents != 4))
  {
    itkExceptionMacro("BMPImageIO supports 1,3 or 4 components only");
  }

  this->OpenFileForWriting(m_Ofstream, m_FileName);


  // A BMP file has four sections:
  //
  // * BMP Header                         14 bytes
  // * Bitmap Information (DIB header)    40 bytes (Windows V3)
  // * Color Palette
  // * Bitmap Data
  //
  // For more details:
  //
  //             https://en.wikipedia.org/wiki/BMP_file_format

  // Write the BMP header
  //
  // Header structure is represented by first a 14 byte field, then the bitmap
  // info header.
  //
  // The 14 byte field:
  //
  // Offset Length Description
  //
  //   0      2    Contain the string, "BM", (Hex: 42 4D)
  //   2      4    The length of the entire file.
  //   6      2    Reserved for application data. Usually zero.
  //   8      2    Reserved for application data. Usually zero.
  //  10      4    Provides an offset from the start of the file
  //               to the first byte of image sample data. This
  //               is normally 54 bytes (Hex: 36)
  char tmp = 66;
  m_Ofstream.write(&tmp, sizeof(char));
  tmp = 77;
  m_Ofstream.write(&tmp, sizeof(char));

  const unsigned int bpp = this->GetNumberOfComponents();
  long               bytesPerRow = m_Dimensions[0] * bpp;
  if (bytesPerRow % 4)
  {
    bytesPerRow = ((bytesPerRow / 4) + 1) * 4;
  }
  const unsigned long paddedBytes = bytesPerRow - (m_Dimensions[0] * bpp);

  const auto   rawImageDataSize = static_cast<unsigned int>((bytesPerRow * m_Dimensions[1]));
  unsigned int fileSize = (rawImageDataSize) + 54;
  if (bpp == 1)
  {
    fileSize += 1024; // need colour LUT
  }
  this->Write32BitsInteger(fileSize);

  constexpr unsigned short applicationReservedValue = 0;
  this->Write16BitsInteger(applicationReservedValue);
  this->Write16BitsInteger(applicationReservedValue);

  unsigned int offsetToBinaryDataStart = 54;
  if (bpp == 1) // more space is needed for the LUT
  {
    offsetToBinaryDataStart += 1024;
  }
  this->Write32BitsInteger(offsetToBinaryDataStart);
  // End of BMP header, 14 bytes written so far

  // Write the DIB header
  //
  // Offset Length Description
  //
  //  14      4    Size of the header (40 bytes)(Hex: 28)
  //
  //
  //  Color Palette
  //
  //  If the bit_count is 1, 4 or 8, the structure must be followed by a colour
  //  lookup table, with 4 bytes per entry, the first 3 of which identify the
  //  blue, green and red intensities, respectively.
  //
  //  Finally the pixel data
  constexpr unsigned int bitmapHeaderSize = 40;
  this->Write32BitsInteger(bitmapHeaderSize);

  // image width
  this->Write32BitsInteger(static_cast<unsigned int>(m_Dimensions[0]));

  // image height -ve means top to bottom
  this->Write32BitsInteger(static_cast<unsigned int>(m_Dimensions[1]));

  // Set `planes'=1 (mandatory)
  constexpr unsigned short numberOfColorPlanes = 1;
  this->Write16BitsInteger(numberOfColorPlanes);

  // Set bits per pixel.
  unsigned short numberOfBitsPerPixel = 0;
  switch (bpp)
  {
    case 4:
      numberOfBitsPerPixel = 32;
      break;
    case 3:
      numberOfBitsPerPixel = 24;
      break;
    case 1:
      numberOfBitsPerPixel = 8;
      break;
    default:
      itkExceptionMacro("Number of components not supported.");
  }
  this->Write16BitsInteger(numberOfBitsPerPixel);

  constexpr unsigned int compressionMethod = 0;
  this->Write32BitsInteger(compressionMethod);
  this->Write32BitsInteger(rawImageDataSize);

  // Assuming spacing is in millimeters,
  // the resolution is set here in pixel per meter.
  // The specification calls for a signed integer, but
  // here we force it to be an unsigned integer to avoid
  // dealing with directions in a subterraneous way.
  const auto horizontalResolution = Math::Round<unsigned int>(1000.0 / m_Spacing[0]);
  const auto verticalResolution = Math::Round<unsigned int>(1000.0 / m_Spacing[1]);

  this->Write32BitsInteger(horizontalResolution);
  this->Write32BitsInteger(verticalResolution);

  // zero here defaults to 2^n colors in the palette
  constexpr unsigned int numberOfColorsInPalette = 0;
  this->Write32BitsInteger(numberOfColorsInPalette);

  // zero here indicates that all colors in the palette are important.
  constexpr unsigned int numberOfImportantColorsInPalette = 0;
  this->Write32BitsInteger(numberOfImportantColorsInPalette);
  // End of DIB header, 54 bytes written so far

  // Write down colour LUT
  // only when using 1 byte per pixel
  if (bpp == 1)
  {
    for (unsigned int n = 0; n < 256; ++n)
    {
      const char tmp2 = static_cast<unsigned char>(n);
      m_Ofstream.write(&tmp2, sizeof(char));
      m_Ofstream.write(&tmp2, sizeof(char));
      m_Ofstream.write(&tmp2, sizeof(char));
      m_Ofstream.write(&tmp, sizeof(char));
    }
  }

  // Write down the raw binary pixel data
  for (unsigned int h = 0; h < m_Dimensions[1]; ++h)
  {
    constexpr char paddingValue = 0;
    const auto *   ptr = static_cast<const char *>(buffer);
    ptr += (m_Dimensions[1] - (h + 1)) * m_Dimensions[0] * bpp;
    if (bpp == 1)
    {
      for (unsigned int i = 0; i < m_Dimensions[0]; ++i)
      {
        m_Ofstream.write(ptr, sizeof(char));
        ++ptr;
      }
      for (unsigned int i = 0; i < paddedBytes; ++i)
      {
        m_Ofstream.write(&paddingValue, sizeof(char));
      }
    }
    if (bpp == 3)
    {
      for (unsigned int i = 0; i < m_Dimensions[0]; ++i)
      {
        ptr += 2;
        m_Ofstream.write(ptr, sizeof(char)); // blue
        --ptr;
        m_Ofstream.write(ptr, sizeof(char)); // green
        --ptr;
        m_Ofstream.write(ptr, sizeof(char)); // red
        ptr += 3;
      }
      for (unsigned int i = 0; i < paddedBytes; ++i)
      {
        m_Ofstream.write(&paddingValue, sizeof(char));
      }
    }
    if (bpp == 4)
    {
      for (unsigned int i = 0; i < m_Dimensions[0]; ++i)
      {
        ptr += 2;
        m_Ofstream.write(ptr, sizeof(char)); // blue
        --ptr;
        m_Ofstream.write(ptr, sizeof(char)); // green
        --ptr;
        m_Ofstream.write(ptr, sizeof(char)); // red
        ptr += 3;
        m_Ofstream.write(ptr, sizeof(char)); // alpha
        ++ptr;
      }
      for (unsigned int i = 0; i < paddedBytes; ++i)
      {
        m_Ofstream.write(&paddingValue, sizeof(char));
      }
    }
  }
}

/** Print Self Method */
void
BMPImageIO::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "BitMapOffset: " << m_BitMapOffset << std::endl;
  os << indent << "FileLowerLeft: " << m_FileLowerLeft << std::endl;
  os << indent << "Depth: " << m_Depth << std::endl;
  os << indent << "NumberOfColors: " << m_NumberOfColors << std::endl;
  os << indent << "ColorPaletteSize: " << m_ColorPaletteSize << std::endl;
  os << indent << "BMPCompression: " << m_BMPCompression << std::endl;
  os << indent << "DataSize: " << m_BMPDataSize << std::endl;
  if (m_IsReadAsScalarPlusPalette)
  {
    os << "Read as Scalar Image plus palette" << '\n';
  }
  if (!m_ColorPalette.empty())
  {
    os << indent << "ColorPalette:" << std::endl;
    for (unsigned int i = 0; i < m_ColorPalette.size(); ++i)
    {
      os << indent << '[' << i << ']' << itk::NumericTraits<PaletteType::value_type>::PrintType(m_ColorPalette[i])
         << std::endl;
    }
  }
}
} // end namespace itk
