/* ============================================================================
 * Copyright (c) 2011, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
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
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "H5ReconVolumeReader.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5ReconVolumeReader::H5ReconVolumeReader()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5ReconVolumeReader::~H5ReconVolumeReader()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5ReconVolumeReader::getSizeAndResolution(int volDims[3], float spacing[3])
{
  int err = 0;

  if (m_Filename.empty() == true)
  {
    std::cout << "H5ReconVolumeReader Error; Filename was empty" << std::endl;
    return -1;
  }

  OPEN_HDF5_FILE(fileId, m_Filename)
  OPEN_RECONSTRUCTION_GROUP(reconGid, AIM::Reconstruction::VoxelDataName.c_str(), fileId)

  err = H5Lite::readPointerDataset(reconGid, H5_DIMENSIONS, volDims);
  if (err < 0)
  {
    std::cout << "H5ReconVolumeReader Error Reading the Dimensions" << std::endl;
    err = H5Gclose(reconGid);
    err = H5Fclose(fileId);
    return err;
  }
  err = H5Lite::readPointerDataset(reconGid, H5_SPACING, spacing);
  if (err < 0)
  {
    std::cout << "H5ReconVolumeReader Error Reading the Spacing (Resolution)" << std::endl;
    err = H5Gclose(reconGid);
    err = H5Fclose(fileId);
    return err;
  }

  err = H5Gclose(reconGid);
  err = H5Fclose(fileId);

  return err;
}

