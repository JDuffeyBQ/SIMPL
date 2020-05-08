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

#include <hdf5.h>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/PhaseType.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/StatsData/StatsData.h"

typedef QVector<FloatArrayType::Pointer> VectorOfFloatArray;

/**
 * @class MatrixStatsData MatrixStatsData.h DREAM3DLib/StatsData/MatrixStatsData.h
 * @brief This class holds the statistical data for a single phase of a microstructure.
 * There are several statistics that are held by this class in a varying number of
 * storage types. Some types have specific ordering of the sub arrays with in them. Those
 * are enumerated here:@n
 * For the distributions a QVector holds objects of the DataArray<float>::Pointer which is
 * typedef'ed to FloatArrayType @see DataArray.hpp. The order of the FloatArrayType
 * objects in the QVector is as follows:
 *   <b>Beta Distribution</b>@n
 *    @li Alpha
 *    @li Beta
 *    <b>Log Normal Distribution</b>@n
 *    @li Average
 *    @li Standard Deviation
 *    <b>Power Law Distribution</b>@n
 *    @li Alpha
 *    @li K
 *    @li Beta
 *
 * The Feature Diameter Info is an array of 3 floats where the values are encoded as:@n
 * @li [0] = Average
 * @li [1] = Max
 * @li [2] = Min
 *
 * The Feature Size Distribution is an array of 2 floats where the values are encoded as:@n
 * @li [0] = Average
 * @li [1] = Standard Deviation
 *
 *
 * @date Feb 9, 2012
 * @version 1.0
 */
class SIMPLib_EXPORT MatrixStatsData : public StatsData
{
public:
  using Self = MatrixStatsData;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static Pointer New();

  /**
   * @brief Returns the name of the class for MatrixStatsData
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for MatrixStatsData
   */
  static QString ClassName();

  ~MatrixStatsData() override;

  /**
   * @brief GetTypeName Returns a string representation of the type of data that is stored by this class. This
   * can be a primitive like char, float, int or the name of a class.
   * @return
   */
  void getXdmfTypeAndSize(QString& xdmfTypeName, int& precision)
  {
    xdmfTypeName = getNameOfClass();
    precision = 0;
  }

  /**
   * @brief getStatsType
   * @return
   */
  QString getStatsType() override;

  /**
   * @brief getPhaseType
   * @return
   */
  PhaseType::Type getPhaseType() override;

  /**
   * @brief deepCopy
   * @return
   */
  StatsData::Pointer deepCopy() override;

  /**
   * @brief writeHDF5Data
   * @param groupId
   * @return
   */
  int writeHDF5Data(hid_t groupId) override;

  /**
   * @brief readHDF5Data
   * @param groupId
   * @return
   */
  int readHDF5Data(hid_t groupId) override;

  /**
   * @brief writeJson
   * @param json
   */
  void writeJson(QJsonObject& json) override;

  /**
   * @brief readJson
   * @param json
   */
  void readJson(const QJsonObject& json) override;

protected:
  MatrixStatsData();

public:
  MatrixStatsData(const MatrixStatsData&) = delete;            // Copy Constructor Not Implemented
  MatrixStatsData(MatrixStatsData&&) = delete;                 // Move Constructor Not Implemented
  MatrixStatsData& operator=(const MatrixStatsData&) = delete; // Copy Assignment Not Implemented
  MatrixStatsData& operator=(MatrixStatsData&&) = delete;      // Move Assignment Not Implemented

private:
};
