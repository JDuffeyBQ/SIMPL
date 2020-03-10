/* ============================================================================
 * Copyright (c) 2009-2016 BlueQuartz Software, LLC
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The code contained herein was partially funded by the followig contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <memory>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Filtering/AbstractFilter.h"

/**
 * @brief The CopyObject class. See [Filter documentation](@ref copyobject) for details.
 */
class SIMPLib_EXPORT CopyObject : public AbstractFilter
{
  Q_OBJECT

#ifdef SIMPL_ENABLE_PYTHON
  PYB11_CREATE_BINDINGS(CopyObject SUPERCLASS AbstractFilter)
  PYB11_SHARED_POINTERS(CopyObject)
  PYB11_FILTER_NEW_MACRO(CopyObject)
  PYB11_FILTER_PARAMETER(int, ObjectToCopy)
  PYB11_FILTER_PARAMETER(DataArrayPath, DataContainerToCopy)
  PYB11_FILTER_PARAMETER(DataArrayPath, AttributeMatrixToCopy)
  PYB11_FILTER_PARAMETER(DataArrayPath, AttributeArrayToCopy)
  PYB11_FILTER_PARAMETER(QString, CopiedObjectName)
  PYB11_PROPERTY(int ObjectToCopy READ getObjectToCopy WRITE setObjectToCopy)
  PYB11_PROPERTY(DataArrayPath DataContainerToCopy READ getDataContainerToCopy WRITE setDataContainerToCopy)
  PYB11_PROPERTY(DataArrayPath AttributeMatrixToCopy READ getAttributeMatrixToCopy WRITE setAttributeMatrixToCopy)
  PYB11_PROPERTY(DataArrayPath AttributeArrayToCopy READ getAttributeArrayToCopy WRITE setAttributeArrayToCopy)
  PYB11_PROPERTY(QString CopiedObjectName READ getCopiedObjectName WRITE setCopiedObjectName)
#endif

public:
  using Self = CopyObject;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  
  /**
   * @brief Returns a NullPointer wrapped by a shared_ptr<>
   * @return
   */
  static Pointer NullPointer();

  /**
   * @brief Creates a new object wrapped in a shared_ptr<>
   * @return
   */
  static Pointer New();

  /**
   * @brief Returns the name of the class for CopyObject
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for CopyObject
   */
  static QString ClassName();

  ~CopyObject() override;

  /**
   * @brief Setter property for ObjectToCopy
   */
  void setObjectToCopy(int value);
  /**
   * @brief Getter property for ObjectToCopy
   * @return Value of ObjectToCopy
   */
  int getObjectToCopy() const;

  Q_PROPERTY(int ObjectToCopy READ getObjectToCopy WRITE setObjectToCopy)

  /**
   * @brief Setter property for DataContainerToCopy
   */
  void setDataContainerToCopy(const DataArrayPath& value);
  /**
   * @brief Getter property for DataContainerToCopy
   * @return Value of DataContainerToCopy
   */
  DataArrayPath getDataContainerToCopy() const;

  Q_PROPERTY(DataArrayPath DataContainerToCopy READ getDataContainerToCopy WRITE setDataContainerToCopy)

  /**
   * @brief Setter property for AttributeMatrixToCopy
   */
  void setAttributeMatrixToCopy(const DataArrayPath& value);
  /**
   * @brief Getter property for AttributeMatrixToCopy
   * @return Value of AttributeMatrixToCopy
   */
  DataArrayPath getAttributeMatrixToCopy() const;

  Q_PROPERTY(DataArrayPath AttributeMatrixToCopy READ getAttributeMatrixToCopy WRITE setAttributeMatrixToCopy)

  /**
   * @brief Setter property for AttributeArrayToCopy
   */
  void setAttributeArrayToCopy(const DataArrayPath& value);
  /**
   * @brief Getter property for AttributeArrayToCopy
   * @return Value of AttributeArrayToCopy
   */
  DataArrayPath getAttributeArrayToCopy() const;

  Q_PROPERTY(DataArrayPath AttributeArrayToCopy READ getAttributeArrayToCopy WRITE setAttributeArrayToCopy)

  /**
   * @brief Setter property for CopiedObjectName
   */
  void setCopiedObjectName(const QString& value);
  /**
   * @brief Getter property for CopiedObjectName
   * @return Value of CopiedObjectName
   */
  QString getCopiedObjectName() const;

  Q_PROPERTY(QString CopiedObjectName READ getCopiedObjectName WRITE setCopiedObjectName)

  /**
   * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
   */
  QString getCompiledLibraryName() const override;

  /**
   * @brief getBrandingString Returns the branding string for the filter, which is a tag
   * used to denote the filter's association with specific plugins
   * @return Branding string
   */
  QString getBrandingString() const override;

  /**
   * @brief getFilterVersion Returns a version string for this filter. Default
   * value is an empty string.
   * @return
   */
  QString getFilterVersion() const override;

  /**
   * @brief newFilterInstance Reimplemented from @see AbstractFilter class
   */
  AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

  /**
   * @brief getGroupName Reimplemented from @see AbstractFilter class
   */
  QString getGroupName() const override;

  /**
   * @brief getSubGroupName Reimplemented from @see AbstractFilter class
   */
  QString getSubGroupName() const override;

  /**
   * @brief getUuid Return the unique identifier for this filter.
   * @return A QUuid object.
   */
  QUuid getUuid() const override;

  /**
   * @brief getHumanLabel Reimplemented from @see AbstractFilter class
   */
  QString getHumanLabel() const override;

  /**
   * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
   */
  void setupFilterParameters() override;

  /**
   * @brief readFilterParameters Reimplemented from @see AbstractFilter class
   */
  void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

  /**
   * @brief execute Reimplemented from @see AbstractFilter class
   */
  void execute() override;


protected:
  CopyObject();
  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck() override;

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

public:
  CopyObject(const CopyObject&) = delete;            // Copy Constructor Not Implemented
  CopyObject(CopyObject&&) = delete;                 // Move Constructor Not Implemented
  CopyObject& operator=(const CopyObject&) = delete; // Copy Assignment Not Implemented
  CopyObject& operator=(CopyObject&&) = delete;      // Move Assignment Not Implemented

private:
  int m_ObjectToCopy = {};
  DataArrayPath m_DataContainerToCopy = {};
  DataArrayPath m_AttributeMatrixToCopy = {};
  DataArrayPath m_AttributeArrayToCopy = {};
  QString m_CopiedObjectName = {};
};

