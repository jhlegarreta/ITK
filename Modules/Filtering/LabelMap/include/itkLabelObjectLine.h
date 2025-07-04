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
#ifndef itkLabelObjectLine_h
#define itkLabelObjectLine_h

#include "itkMacro.h"
#include "itkIndex.h"
#include "itkIndent.h"

namespace itk
{
/**
 * \class LabelObjectLine
 * LabelObjectLine is the line object used in the LabelObject class
 * to store the line which are part of the object.
 * A line is formed of and index and a length in the dimension 0.
 * It is used in a run-length encoding
 *
 * \author Gaetan Lehmann. Biologie du Developpement et de la Reproduction, INRA de Jouy-en-Josas, France.
 *
 * This implementation was taken from the Insight Journal paper:
 * https://doi.org/10.54294/q6auw4
 *
 * \ingroup LabeledImageObject
 * \ingroup ITKLabelMap
 */
template <unsigned int VImageDimension>
class ITK_TEMPLATE_EXPORT LabelObjectLine
{
public:
  ITK_DEFAULT_COPY_AND_MOVE(LabelObjectLine);

  static constexpr unsigned int ImageDimension = VImageDimension;

  using IndexType = Index<VImageDimension>;
  using LengthType = SizeValueType;

  LabelObjectLine();
  LabelObjectLine(const IndexType & idx, const LengthType & length);
  virtual ~LabelObjectLine() = default;

  /**
   * Set/Get Index
   */
  void
  SetIndex(const IndexType & idx);

  [[nodiscard]] const IndexType &
  GetIndex() const;

  /**
   * SetGet Length
   */
  void
  SetLength(const LengthType length);

  [[nodiscard]] const LengthType &
  GetLength() const;

  /**
   *  Check for index
   */
  [[nodiscard]] bool
  HasIndex(const IndexType idx) const;

  [[nodiscard]] bool
  IsNextIndex(const IndexType & idx) const;

  /** Cause the object to print itself out. */
  void
  Print(std::ostream & os, Indent indent = 0) const;

protected:
  /** Methods invoked by Print() to print information about the object
   * including superclasses. Typically not called by the user (use Print()
   * instead) but used in the hierarchical print process to combine the
   * output of several classes.  */
  virtual void
  PrintSelf(std::ostream & os, Indent indent) const;

  virtual void
  PrintHeader(std::ostream & os, Indent indent) const;

  virtual void
  PrintTrailer(std::ostream & itkNotUsed(os), Indent itkNotUsed(indent)) const
  {}

private:
  IndexType  m_Index{};
  LengthType m_Length{};
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkLabelObjectLine.hxx"
#endif

#endif
