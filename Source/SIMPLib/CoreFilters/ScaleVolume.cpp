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

#include "ScaleVolume.h"

#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/partitioner.h>
#include <tbb/task_scheduler_init.h>
#endif

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/SIMPLibVersion.h"

#include "SIMPLib/FilterParameters/DataContainerSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatVec3FilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/Geometry/IGeometry2D.h"
#include "SIMPLib/Geometry/ImageGeom.h"

/**
 * @brief The CalculateCentroidsImpl class implements a threaded algorithm that scales the
 * positions of a set of nodes
 */
class ScaleVolumeUpdateVerticesImpl
{
  float* m_Nodes;
  float* m_Min;
  FloatVec3_t m_ScaleFactor;

public:
  ScaleVolumeUpdateVerticesImpl(float* nodes, float* min, FloatVec3_t scale)
  : m_Nodes(nodes)
  , m_Min(min)
  , m_ScaleFactor(scale)
  {
  }
  ~ScaleVolumeUpdateVerticesImpl() = default;

  void generate(size_t start, size_t end) const
  {
    for(size_t i = start; i < end; i++)
    {
      m_Nodes[3 * i] = m_Min[0] + (m_Nodes[3 * i] - m_Min[0]) * m_ScaleFactor.x;
      m_Nodes[3 * i + 1] = m_Min[1] + (m_Nodes[3 * i + 1] - m_Min[1]) * m_ScaleFactor.y;
      m_Nodes[3 * i + 2] = m_Min[2] + (m_Nodes[3 * i + 2] - m_Min[2]) * m_ScaleFactor.z;
    }
  }

#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
  void operator()(const tbb::blocked_range<size_t>& r) const
  {
    generate(r.begin(), r.end());
  }
#endif
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ScaleVolume::ScaleVolume()
: m_DataContainerName("")
, m_SurfaceDataContainerName("")
, m_ApplyToVoxelVolume(true)
, m_ApplyToSurfaceMesh(true)
{
  m_ScaleFactor.x = 1.0f;
  m_ScaleFactor.y = 1.0f;
  m_ScaleFactor.z = 1.0f;

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ScaleVolume::~ScaleVolume() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ScaleVolume::setupFilterParameters()
{
  FilterParameterVector parameters;

  parameters.push_back(SIMPL_NEW_FLOAT_VEC3_FP("Scaling Factor", ScaleFactor, FilterParameter::Parameter, ScaleVolume));

  QStringList linkedProps("DataContainerName");
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Apply to Image Geometry", ApplyToVoxelVolume, FilterParameter::Parameter, ScaleVolume, linkedProps));
  {
    DataContainerSelectionFilterParameter::RequirementType req;
    req.dcGeometryTypes = IGeometry::Types(1, IGeometry::Type::Image);
    parameters.push_back(SIMPL_NEW_DC_SELECTION_FP("Data Container Image Geometry to Scale", DataContainerName, FilterParameter::RequiredArray, ScaleVolume, req));
  }
  linkedProps.clear();
  linkedProps << "SurfaceDataContainerName";
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Apply to Surface Geometry", ApplyToSurfaceMesh, FilterParameter::Parameter, ScaleVolume, linkedProps));
  {
    DataContainerSelectionFilterParameter::RequirementType req;
    IGeometry::Types dcGeometryTypes;
    dcGeometryTypes.push_back(IGeometry::Type::Triangle);
    dcGeometryTypes.push_back(IGeometry::Type::Quad);
    req.dcGeometryTypes = dcGeometryTypes;
    parameters.push_back(SIMPL_NEW_DC_SELECTION_FP("Data Container Surface Geometry to Scale", SurfaceDataContainerName, FilterParameter::RequiredArray, ScaleVolume, req));
  }

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ScaleVolume::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setApplyToVoxelVolume(reader->readValue("ApplyToVoxelVolume", getApplyToVoxelVolume()));
  setApplyToSurfaceMesh(reader->readValue("ApplyToSurfaceMesh", getApplyToSurfaceMesh()));
  setScaleFactor(reader->readFloatVec3("ScaleFactor", getScaleFactor()));
  setDataContainerName(reader->readString("DataContainerName", getDataContainerName()));
  setSurfaceDataContainerName(reader->readString("SurfaceDataContainerName", getSurfaceDataContainerName()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ScaleVolume::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ScaleVolume::dataCheck()
{
  setErrorCondition(0);
  setWarningCondition(0);

  if(m_ApplyToVoxelVolume == true)
  {
    getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom, AbstractFilter>(this, getDataContainerName());
  }

  if(m_ApplyToSurfaceMesh == true)
  {
    getDataContainerArray()->getPrereqGeometryFromDataContainer<IGeometry2D, AbstractFilter>(this, getSurfaceDataContainerName());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ScaleVolume::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ScaleVolume::updateSurfaceMesh()
{
  setErrorCondition(0);
  setWarningCondition(0);

#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
  tbb::task_scheduler_init init;
  bool doParallel = true;
#endif

  IGeometry2D::Pointer geom2D = getDataContainerArray()->getDataContainer(getSurfaceDataContainerName())->getGeometryAs<IGeometry2D>();
  float* nodes = geom2D->getVertexPointer(0);

  // First get the min/max coords.

  float min[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
  float max[3] = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

  int64_t count = geom2D->getNumberOfVertices();
  for(int64_t i = 0; i < count; i++)
  {
    if(nodes[3 * i] > max[0])
    {
      max[0] = nodes[3 * i];
    }
    if(nodes[3 * i + 1] > max[1])
    {
      max[1] = nodes[3 * i + 1];
    }
    if(nodes[3 * i + 2] > max[2])
    {
      max[2] = nodes[3 * i + 2];
    }

    if(nodes[3 * i] < min[0])
    {
      min[0] = nodes[3 * i];
    }
    if(nodes[3 * i + 1] < min[1])
    {
      min[1] = nodes[3 * i + 1];
    }
    if(nodes[3 * i + 2] < min[2])
    {
      min[2] = nodes[3 * i + 2];
    }
  }

#ifdef SIMPL_USE_PARALLEL_ALGORITHMS
  if(doParallel == true)
  {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, count), ScaleVolumeUpdateVerticesImpl(nodes, min, m_ScaleFactor), tbb::auto_partitioner());
  }
  else
#endif
  {
    ScaleVolumeUpdateVerticesImpl serial(nodes, min, m_ScaleFactor);
    serial.generate(0, count);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ScaleVolume::execute()
{
  setErrorCondition(0);
  setWarningCondition(0);
  dataCheck();
  if(getErrorCondition() < 0)
  {
    return;
  }

  if(m_ApplyToVoxelVolume == true)
  {
    DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());
    ImageGeom::Pointer image = m->getGeometryAs<ImageGeom>();

    float resolution[3] = {0.0f, 0.0f, 0.0f};
    image->getResolution(resolution);
    resolution[0] *= m_ScaleFactor.x;
    resolution[1] *= m_ScaleFactor.y;
    resolution[2] *= m_ScaleFactor.z;
    image->setResolution(resolution);
  }

  if(m_ApplyToSurfaceMesh == true)
  {
    updateSurfaceMesh();
  }

  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ScaleVolume::newFilterInstance(bool copyFilterParameters) const
{
  ScaleVolume::Pointer filter = ScaleVolume::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ScaleVolume::getCompiledLibraryName() const
{
  return Core::CoreBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ScaleVolume::getBrandingString() const
{
  return "SIMPLib Core Filter";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ScaleVolume::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << SIMPLib::Version::Major() << "." << SIMPLib::Version::Minor() << "." << SIMPLib::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ScaleVolume::getGroupName() const
{
  return SIMPL::FilterGroups::CoreFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QUuid ScaleVolume::getUuid()
{
  return QUuid("{3cf44c27-9149-5548-945a-deef1dc994a8}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ScaleVolume::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::SpatialFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ScaleVolume::getHumanLabel() const
{
  return "Change Scaling of Volume";
}
