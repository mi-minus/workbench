
/*LICENSE_START*/
/*
 * Copyright 2013 Washington University,
 * All rights reserved.
 *
 * Connectome DB and Connectome Workbench are part of the integrated Connectome
 * Informatics Platform.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of Washington University nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*LICENSE_END*/

#include <cmath>

#define __SPARSE_VOLUME_INDEXER_DECLARE__
#include "SparseVolumeIndexer.h"
#undef __SPARSE_VOLUME_INDEXER_DECLARE__

#include "CaretLogger.h"
#include "CiftiInterface.h"

using namespace caret;



/**
 * \class caret::SparseVolumeIndexer
 * \brief Maps IJK indices or XYZ coordinates to a sparse volume data
 * \ingroup Files
 *
 * Some data sources (such as CIFTI files) contain data for a subset
 * (or sparse representation) of volume data.  In a CIFTI file, there
 * is a list of IJK indices that indicate the offset of each voxel
 * in the data (and note that the data may be for both surface vertices
 * and volume voxels).  Finding a particular voxel by its IJK indices
 * requires a sequential search through the CIFTI's list of voxels.
 * This class permits a much faster location of a particular CIFTI file's
 * voxel by creating what is essentially a lookup table that maps the
 * full non-sparse volume IJK indices to an offset in the CIFTI data
 * or a negative value if the voxel is not present in the CIFTI data.
 */

/**
 * Constructor from a CIFTI interface.
 *
 * @param ciftiInterface
 *   Interface to CIFTI file.
 * @param ciftiVoxelMapping
 *   The voxel mappings.
 */
SparseVolumeIndexer::SparseVolumeIndexer(const CiftiInterface* ciftiInterface,
                                         const std::vector<CiftiVolumeMap>& ciftiVoxelMapping)
: CaretObject()
{
    m_dataValid = false;
    
    const int32_t numberOfCiftiVolumeVoxels = static_cast<int32_t>(ciftiVoxelMapping.size());
    if (numberOfCiftiVolumeVoxels <= 0) {
        return;
    }
    
    /*
     * Get volume attributes and make sure orthogonal
     */
    VolumeFile::OrientTypes ciftiOrientation[3];
    int64_t ciftiDimensions[3];
    float ciftiOrigin[3];
    float ciftiSpacing[3];
    if (ciftiInterface->getVolumeAttributesForPlumb(ciftiOrientation,
                                                    ciftiDimensions,
                                                    ciftiOrigin,
                                                    ciftiSpacing) == false) {
        CaretLogWarning("CIFTI Volume is no Plumb!");
        return;
    }
    
    m_dimI = ciftiDimensions[0];
    m_dimJ = ciftiDimensions[1];
    m_dimK = ciftiDimensions[2];
    
    m_spacingX = ciftiSpacing[0];
    m_spacingY = ciftiSpacing[1];
    m_spacingZ = ciftiSpacing[2];
    
    m_originX = ciftiOrigin[0];
    m_originY = ciftiOrigin[1];
    m_originZ = ciftiOrigin[2];
    
    /*
     * Move origin from center to 'corner' of voxel
     */
    m_originX -= m_spacingX;
    m_originY -= m_spacingY;
    m_originZ -= m_spacingZ;
    
    const int64_t numberOfVoxels = (m_dimI * m_dimJ * m_dimK);
    if (numberOfVoxels <= 0) {
        return;
    }
    
//    m_voxelOffsets.resize(numberOfVoxels,
//                          -1);
//        const int64_t offset = getOffsetForIndices(vm.m_ijk[0],
//                                                   vm.m_ijk[1],
//                                                   vm.m_ijk[2]);
//        m_voxelOffsets[offset] = vm.m_ciftiIndex;
    
    for (std::vector<CiftiVolumeMap>::const_iterator iter = ciftiVoxelMapping.begin();
         iter != ciftiVoxelMapping.end();
         iter++) {
        const CiftiVolumeMap& vm = *iter;

        m_voxelIndexLookup.at(vm.m_ijk) = vm.m_ciftiIndex;
    }
    
    bool validateFlag = true;
    if (validateFlag) {
        AString validateString;
        for (std::vector<CiftiVolumeMap>::const_iterator iter = ciftiVoxelMapping.begin();
             iter != ciftiVoxelMapping.end();
             iter++) {
            const CiftiVolumeMap& vm = *iter;
            const int64_t* foundOffset = m_voxelIndexLookup.find(vm.m_ijk);
            if (foundOffset != NULL) {
                if (*foundOffset != vm.m_ciftiIndex) {
                    validateString.appendWithNewLine("IJK ("
                                                     + AString::fromNumbers(vm.m_ijk, 3, ",")
                                                     + " should have lookup value "
                                                     + AString::number(vm.m_ciftiIndex)
                                                     + " but has value "
                                                     + AString::number(*foundOffset));
                }
            }
            else {
                validateString.appendWithNewLine("IJK ("
                                                 + AString::fromNumbers(vm.m_ijk, 3, ",")
                                                 + " should have lookup value "
                                                 + AString::number(vm.m_ciftiIndex)
                                                 + " but was not found.");
            }
        }
        if (validateString.isEmpty() == false) {
            CaretLogSevere("Sparse Indexer Errors:\n"
                           + validateString);
        }
    }
    
    m_dataValid = true;
}

/**
 * Destructor.
 */
SparseVolumeIndexer::~SparseVolumeIndexer()
{
    
}

/**
 * @return True if this instance is valid.
 */
bool
SparseVolumeIndexer::isValid() const
{
    return m_dataValid;
}


/**
 * Get the offset for the given IJK indices.
 *
 * @param i
 *   I index.
 * @param j
 *   J index.
 * @param k
 *   K index.
 * @return
 *   Offset for given indices or -1 if no data for the given indices.
 */
int64_t
SparseVolumeIndexer::getOffsetForIndices(const int64_t i,
                                         const int64_t j,
                                         const int64_t k) const
{
    const int64_t* offset = m_voxelIndexLookup.find(i, j, k);
    if (offset != NULL) {
        return *offset;
    }
    
    return -1;
}

/**
 * Convert the coordinates to volume indices.  Any coordinates are accepted
 * and output indices are not necessarily within the volume.
 *
 * @param x
 *   X coordinate.
 * @param y
 *   y coordinate.
 * @param z
 *   z coordinate.
 * @param iOut
 *   I index.
 * @param jOut
 *   J index.
 * @param kOut
 *   K index.
 * @return
 *   True if volume attributes (origin/spacing/dimensions) are valid.
 */
bool
SparseVolumeIndexer::coordinateToIndices(const float x,
                                         const float y,
                                         const float z,
                                         int64_t& iOut,
                                         int64_t& jOut,
                                         int64_t& kOut) const
{
    if (m_dataValid) {
        iOut = static_cast<int64_t>(std::floor((x - m_originX) / m_spacingX));
        jOut = static_cast<int64_t>(std::floor((y - m_originY) / m_spacingY));
        kOut = static_cast<int64_t>(std::floor((z - m_originZ) / m_spacingZ));
        
        return true;
    }
    
    return false;
}


/**
 * Get the offset for the given XYZ coordinates.
 *
 * @param x
 *   X coordinate.
 * @param y
 *   y coordinate.
 * @param z
 *   z coordinate.
 * @return
 *   Offset for given coordinates or -1 if no data for the given coordinates.
 */
int64_t
SparseVolumeIndexer::getOffsetForCoordinate(const float x,
                                            const float y,
                                            const float z) const
{
    int64_t i, j, k;
    if (coordinateToIndices(x, y, z, i, j, k)) {
        return getOffsetForIndices(i, j, k);
    }
    
    return -1;
}

/**
 * Get the XYZ coordinate for the given indices.
 * Any indices are accepted.
 *
 * @param i
 *   I index.
 * @param j
 *   J index.
 * @param k
 *   K index.
 * @param xOut
 *   X coordinate.
 * @param yOut
 *   y coordinate.
 * @param zOut
 *   z coordinate.
 * @return
 *   True if volume attributes (origin/spacing/dimensions) are valid.
 */
bool
SparseVolumeIndexer::indicesToCoordinate(const int64_t i,
                                         const int64_t j,
                                         const int64_t k,
                                         float& xOut,
                                         float& yOut,
                                         float& zOut) const
{
    if (m_dataValid) {
        xOut = m_originX + i * m_spacingX;
        yOut = m_originY + j * m_spacingY;
        zOut = m_originZ + k * m_spacingZ;
        
        return true;
    }
    
    return false;
}

