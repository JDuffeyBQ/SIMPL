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

#include <QtCore/QMetaType>                   // for Q_DECLARE_METATYPE
#include <QtCore/QObject>                 // for Q_OBJECT
#include <QtCore/QString>
#include <QtCore/QVector>

#include <list>
#include <set>
#include <tuple>

#include "SIMPLib/Common/SIMPLibDLLExport.h"     // for SIMPLib_EXPORT
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"  // for SIMPL_PIMPL_PROPERTY_DECL

class DataContainerArray;
using DataContainerArrayShPtr = std::shared_ptr<DataContainerArray>;

/**
 * @brief The DataArrayPath class holds a complete or partial path to a data array starting at the DataContainer
 * level. The class is implemented using the PIMPL design pattern.
 */
class SIMPLib_EXPORT DataArrayPath : public QObject
{
  Q_OBJECT
  
  PYB11_CREATE_BINDINGS(DataArrayPath)
  PYB11_CREATION(ARGS QString QString QString)
  PYB11_PROPERTY(QString DataContainerName READ getDataContainerName WRITE setDataContainerName)
  PYB11_PROPERTY(QString AttributeMatrixName READ getAttributeMatrixName WRITE setAttributeMatrixName)
  PYB11_PROPERTY(QString DataArrayName READ getDataArrayName WRITE setDataArrayName)

  PYB11_METHOD(bool isEmpty)
  PYB11_METHOD(bool isValid)
  PYB11_METHOD(void update ARGS dcName amName daName)

  public:
    // tuple <oldPath, newPath>
    using RenameType = std::tuple<DataArrayPath, DataArrayPath>;
    using RenameContainer = std::list<RenameType>;

    enum class DataType
    {
      DataContainer,
      AttributeMatrix,
      DataArray,
      None
    };
    Q_ENUM(DataType)

    DataArrayPath();

    /**
     * @brief DataArrayPath
     * @param dcName
     * @param amName
     * @param daName
     */
    DataArrayPath(const QString& dcName, const QString& amName, const QString& daName);

    /**
     * @brief DataArrayPath
     * @param path A path with the '|' delimeters
     */
    explicit DataArrayPath(const QString& path);

    /**
     * @brief DataArrayPath Copy Constructor
     */
    DataArrayPath(const DataArrayPath& rhs);

    /**
     * @brief ~DataArrayPath
     */
    ~DataArrayPath() override;

        /**
    * @brief checks that a vector of paths have the same data container and attribute matrix
    * @return true if the paths in the vector have the same data container and attribute matrix, false otherwise
    */
    static bool ValidateVector(const QVector<DataArrayPath>& other);

    /**
    * @brief Gets the data array names from a QVector of DataArrayPaths.
    * @return Returns the data array names from a QVector of DataArrayPaths, in a QList.
    */
    static QList<QString> GetDataArrayNames(const QVector<DataArrayPath>& paths);

    /**
    * @brief Gets the attribute matrix path from a QVector of DataArrayPaths.
    * @return Returns the attribute matrix path as a DataArrayPath from a QVector
    * of DataArrayPaths.
    */
    static DataArrayPath GetAttributeMatrixPath(const QVector<DataArrayPath>& paths);

    /**
     * @brief ConvertToQVector Converts a QStringList of DataArrayPaths to a QVector of DataArrayPaths
     * @param paths
     * @return
     */
    static QVector<DataArrayPath> ConvertToQVector(QStringList& paths);

    /**
    * @brief serialize Deserializes the string into a DataArrayPath, using the specified delimiter.
    * @param delimiter
    * @return
    */
    static DataArrayPath Deserialize(QString str, QString delimiter);

    /**
     * @brief checks for and returns any updated DataArrayPaths between two sets
     * @param oldPaths
     * @param newPaths
     * @return
     */
    static RenameContainer CheckForRenamedPaths(DataContainerArrayShPtr oldDca, DataContainerArrayShPtr newDca, std::list<DataArrayPath> oldPaths, std::list<DataArrayPath> newPaths);

    /**
     * @brief checks if the targets of the old DataArrayPath and new DataArrayPath are compatible in their given DataContainerArrays
     * @param oldDca
     * @param newDca
     * @param oldPath
     * @param newPath
     */
    static bool CheckRenamePath(DataContainerArrayShPtr oldDca, DataContainerArrayShPtr newDca, DataArrayPath oldPath, DataArrayPath newPath);

    /**
    * @brief Returns the DataType matching the current path
    * @return
    */
    DataType getDataType();

    SIMPL_INSTANCE_STRING_PROPERTY(DataContainerName)
    SIMPL_INSTANCE_STRING_PROPERTY(AttributeMatrixName)
    SIMPL_INSTANCE_STRING_PROPERTY(DataArrayName)


    /**
     * @brief operator =
     */
    void operator=(const DataArrayPath& rhs);

    /**
     * @brief operator ==
     * @param rhs
     * @return
     */
    bool operator==(const DataArrayPath& rhs) const;

    /**
     * @brief serialize Returns the path using the '|' charater by default. This can be over ridden by the programmer
     * @param delimiter
     * @return
     */
    QString serialize(QString delimiter = "|") const;

    /**
     * @brief getAsVector Returns the DataArrayPath represented as a QVector<String> where index[0] = DataContainer Name,
     * index[1] = AttributeMatrix Name and index[2] = DataArray Name
     * @return
     */
    QVector<QString> toQVector();

    /**
     * @brief isEmpty Returns if ALL of the string elements are empty. Note that a class could return FALSE for this
     * function and FALSE for isValid() also so this function is not a true indication of a valid path.
     * @return
     */
    bool isEmpty() const;

    /**
     * @brief isValid Returns if ALL of the string components have some value stored in them so 'valid' in this sense just
     * means that all three components have a non-empty string. This does NOT necessarily mean that those strings, once
     * serialized into a path actually refer to something in your DataContainer.
     * @return
     */
    bool isValid() const;

    /**
     * @brief toQStringList Converts the DataArrayPath to a QStringList
     * @return
     */
    QStringList toQStringList() const;

    /**
     * @brief Updates the DataArrayPath with the values in the arguments
     * @param dcName The DataContainer Name
     * @param amName The AttributeMatrix Name
     * @param daName The DataArray Name
     */
    void update(const QString& dcName, const QString& amName, const QString& daName);

    /**
     * @brief checks that two paths share the same data container
     * @param other The other path
     * @return true if the two paths share the same data container, false otherwise
     */
    bool hasSameDataContainer(const DataArrayPath& other) const;

    /**
    * @brief checks that two paths share the same attribute matrix
    * @param other The other path
    * @return true if the two paths share the same attribute matrix, false otherwise
    */
    bool hasSameAttributeMatrix(const DataArrayPath& other) const;

    /**
     * @brief checks that two paths share the same data container and attribute matrix
     * @param other The other path
     * @return true if the two paths share the same data container and attribute matrix, false otherwise
     */
    bool hasSameAttributeMatrixPath(const DataArrayPath& other) const;

    /**
     * @brief checks that two paths share the same data array
     * @param other The other path
     * @return true if the two paths share the same data array, false otherwise
     */
    bool hasSameDataArray(const DataArrayPath& other) const;

    /**
     * @brief checks if the given DataArrayPath could indicate a possible renamed path.
     * This requires that the given path be no longer than the current path and only one value is changed.
     * Returns true if this is a possible rename and returns false otherwise.
     * @param updated
     * @return
     */
    bool possibleRename(const DataArrayPath& updated) const;

    /**
     * @brief updates the current path based on the newPath if it matches the given previous value
     * @param renamePath
     * return
     */
    bool updatePath(const DataArrayPath::RenameType& renamePath);

    /**
    * @brief Writes the contents of the proxy to the json object 'json'
    * @param json
    * @return
    */
    void writeJson(QJsonObject& json) const;

    /**
    * @brief Reads the contents of the the json object 'json' into the proxy
    * @param json
    * @return
    */
    bool readJson(QJsonObject& json);

    /**
     * @brief Returns a QJsonObject that holds the values of this class.
     * @return
     */
    QJsonObject toJsonObject() const;


  protected:



  private:


};


Q_DECLARE_METATYPE(DataArrayPath)
Q_DECLARE_METATYPE(DataArrayPath::DataType)



