//
// ProteinVariantMatch.h
// Copyright (C) 2013 by University of Stuttgart (VISUS).
// All rights reserved.
//
// Created on: Jun 11, 2013
//     Author: scharnkn
//

#if defined(WITH_CUDA)

#ifndef MMPROTEINPLUGIN_PROTEINVARIANTMATCH_H_INCLUDED
#define MMPROTEINPLUGIN_PROTEINVARIANTMATCH_H_INCLUDED
#if (defined(_MSC_VER) && (_MSC_VER > 1000))
#pragma once
#endif // (defined(_MSC_VER) && (_MSC_VER > 1000))

#include "Module.h"
#include "CallerSlot.h"
#include "CalleeSlot.h"
#include "param/ParamSlot.h"
#include "vislib/Array.h"
#include "HostArr.h"
#include "DiagramCall.h"
#include "MolecularDataCall.h"
#include "vislib/Array.h"
//#include "vislib_vector_typedefs.h"
#include "vislib/Vector.h"
#include "vislib/Matrix.h"
typedef vislib::math::Cuboid<float> Cubef;
typedef vislib::math::Matrix<float, 3, vislib::math::COLUMN_MAJOR> Mat3f;
typedef vislib::math::Vector<float, 3> Vec3f;


#include "CUDAQuickSurf.h"
#include "gridParams.h"
#include "cuda_error_check.h"
#include "CudaDevArr.h"
#include "DeformableGPUSurfaceMT.h"

typedef unsigned int uint;

namespace megamol {
namespace protein {


class ProteinVariantMatch : public core::Module {

public:

    /// Enum defining the differend heuristics
    enum Heuristic {RMS_VALUE=0, SURFACE_POTENTIAL, SURFACE_POTENTIAL_SIGN, MEAN_HAUSDORFF_DIST, HAUSDORFF_DIST};

    /// Enum describing different ways of using RMS fitting
    enum RMSFittingMode {RMS_ALL=0, RMS_BACKBONE, RMS_C_ALPHA};

    /** CTor */
    ProteinVariantMatch(void);

    /** DTor */
    virtual ~ProteinVariantMatch(void);

    /**
     * Answer the name of this module.
     *
     * @return The name of this module.
     */
    static const char *ClassName(void) {
        return "ProteinVariantMatch";
    }

    /**
     * Answer a human readable description of this module.
     *
     * @return A human readable description of this module.
     */
    static const char *Description(void) {
        return "Matches a series of protein variants against each other based on different heuristics.";
    }

    /**
     * Answers whether this module is available on the current system.
     *
     * @return 'true' if the module is available, 'false' otherwise.
     */
    static bool IsAvailable(void) {
        return true;
    }

protected:

    /**
     * Implementation of 'Create'.
     *
     * @return 'true' on success, 'false' otherwise.
     */
    virtual bool create(void);

    /**
     * Call callback to get the diagram data
     *
     * @param c The calling call
     *
     * @return True on success
     */
    bool getDiagData(core::Call& call);

    /**
     * Call callback to get the matrix data
     *
     * @param c The calling call
     *
     * @return True on success
     */
    bool getMatrixData(core::Call& call);

    /**
     * Implementation of 'release'.
     */
    virtual void release(void);

    /**
     * Translate and rotate an array of positions according to the current
     * transformation obtained by RMS fitting (a translation vector and
     * rotation matrix).
     *
     * @param mol                A Molecular data call containing the particle
     *                           positions of the corresponding data set. This
     *                           is necessary to compute the centroid of the
     *                           particles.
     * @param surf  The surface object
     * @return 'True' on success, 'false' otherwise
     */
//    bool applyRMSFitting(MolecularDataCall *mol, DeformableGPUSurfaceMT *surf);

    /** TODO */
    void computeDensityBBox(const float *atomPos1, const float *atomPos2,
            size_t atomCnt1, size_t atomCnt2);

private:

    /**
     * Translate and rotate an array of positions according to the current
     * transformation obtained by RMS fitting (a translation vector and
     * rotation matrix).
     *
     * @param mol          A Molecular data call containing the particle
     *                     positions of the corresponding data set. This
     *                     is necessary to compute the centroid of the
     *                     particles.
     * @param vertexPos_D  The device array containing the vertex
     *                     positions to be transformed (device memory)
     * @param vertexCnt    The number of vertices to be transformed
     * @param rotation     The rotation matrix
     * @param translation  The translation matrix
     *
     * @return 'True' on success, 'false' otherwise
     */
    bool applyRMSFittingToPosArray(
            MolecularDataCall *mol,
            CudaDevArr<float> &vertexPos_D,
            uint vertexCnt,
            float rotation[3][3],
            float translation[3]);

    /**
     * (Re-)computes a smooth density map based on an array of givwen particle
     * positions using a CUDA implementation.
     *
     * @param mol           The data call containing the particle positions
     * @param cqs           The CUDAQuickSurf object used to compute the density
     *                      map
     * @param gridDensMap   Grid parameters for the resulting density map TODO
     *
     * @return 'True' on success, 'false' otherwise
     */
    bool computeDensityMap(
            const float* atomPos,
            MolecularDataCall* mol,
            CUDAQuickSurf *cqs);

    /**
     * Computes the result of the chosen heuristic when comparing every variant
     * with every other variant.
     *
     * @return 'True' on success, 'false' otherwise
     */
    bool computeMatch(core::param::ParamSlot& p);

    /**
     * Computes the RMS value between all variants ans stores it to the match
     * matrix
     *
     * @return 'True' on success, 'false' otherwise
     */
    bool computeMatchRMS();
    /**
     * Quantifies the surface difference between all variants and stores the
     * results in the matching matrices.
     *
     * @return 'True' on success, 'false' otherwise
     */
    bool computeMatchSurfMapping();

    /**
     * Computes the RMS (Root Mean Square) value of two variants.
     *
     * @param atomPos0 Array with atom positions for variant 0
     * @param atomPos1 Array with atom positions for variant 1
     * @param cnt      The number of elements in both arrays (must be equal)
     * @param fit      If 'true', the actual fitting of performed, otherwise
     *                 only the RMS value is computed
     * @param flag     If == 0, rms deviation is calculated but no structure
     *                 will move,
     *                 if == 1, rms deviation is calculated, Vec moves back, but
     *                 toFitVec's centroid moved to (0,0,0). Alignment will be
     *                 done in the calling function.
     *                 if == 2, rms deviation is calculated and toFitVec will
     *                 align to Vec.
     *                 (see also RMS.h)
     * @param rotation    Saves the rotation matrix if wanted (may be null)
     * @param translation Saves the translation vector if wanted (may be null)
     *
     * @return The RMS value of the two variants
     */
    float getRMS(float *atomPos0, float *atomPos1, unsigned int cnt, bool fit,
            int flag, float rotation[3][3], float translation[3]);

    /**
     * Extracts all atom positions from a moleculr data call, that are used to
     * compute the RMS value (either all protein atoms, or only backbone atoms,
     * or only C alpha atoms).
     *
     * @param mol    The data call comtaining the particles
     * @param posArr The array for the extracted positions
     * @param cnt    The number of extracted elements
     *
     * @return 'True' on success, 'false' otherwise
     */
    bool getRMSPosArray(MolecularDataCall *mol, HostArr<float> &posArr,
            unsigned int &cnt);

    /**
     * Update parameters slots.
     */
    void updatParams();


    /* Data caller/callee slots */

    /// Caller slot for vertex data
    megamol::core::CallerSlot particleDataCallerSlot;

    /// Caller slot volume data
    megamol::core::CallerSlot volumeDataCallerSlot;

    /// Callee slot for diagram output data
    megamol::core::CalleeSlot diagDataOutCalleeSlot;

    /// Callee slot for matrix output data
    megamol::core::CalleeSlot matrixDataOutCalleeSlot;


    /* Parameters slots */

    /// Param to trigger computation
    core::param::ParamSlot triggerMatchingSlot;

    /// Param to chose the heuristic
    core::param::ParamSlot theheuristicSlot;
    Heuristic theheuristic;

    /// RMS fitting mode
    core::param::ParamSlot fittingModeSlot;
    RMSFittingMode fittingMode;

    /// Single frame to be compared with all other frames when using the 1d
    /// function plot
    core::param::ParamSlot singleFrameIdxSlot;
    uint singleFrameIdx;

    /* Hardcoded parameters for the 'quicksurf' class */

    /// Parameter for assumed radius of density grid data
    static const float qsParticleRad;

    /// Parameter for the cutoff radius for the gaussian kernel
    static const float qsGaussLim;

    /// Parameter for assumed radius of density grid data
    static const float qsGridSpacing;

    /// Parameter to toggle scaling by van der Waals radius
    static const bool qsSclVanDerWaals;

    /// Parameter for iso value for volume rendering
    static const float qsIsoVal;

    /* Parameter slots for surface mapping */

    /// Maximum number of iterations when mapping the mesh
    core::param::ParamSlot surfMapMaxItSlot;
    uint surfMapMaxIt;

    /// Maximum number of iterations when regularizing the mesh
    core::param::ParamSlot surfMapRegMaxItSlot;
    uint surfMapRegMaxIt;

    /// Interpolation method used when computing external forces based on
    /// gradient of the scalar field
    core::param::ParamSlot surfMapInterpolModeSlot;
    DeformableGPUSurfaceMT::InterpolationMode surfMapInterpolMode;

    /// Stiffness of the springs defining the spring forces in surface
    core::param::ParamSlot surfMapSpringStiffnessSlot;
    float surfMapSpringStiffness;

    /// Weighting of the external forces, note that the weight
    /// of the internal forces is implicitely defined by
    /// 1.0 - surf0ExternalForcesWeight
    core::param::ParamSlot surfMapExternalForcesWeightSlot;
    float surfMapExternalForcesWeight;

    /// Overall scaling for the forces acting upon the surface
    core::param::ParamSlot surfMapForcesSclSlot;
    float surfMapForcesScl;


    /* Variant matching */

    /// TODO
    vislib::Array<float> matchRMSD;

    /// The minimum match value
    float minMatchRMSDVal;

    /// The maximum match value
    float maxMatchRMSDVal;

    /// TODO
    vislib::Array<float> matchSurfacePotential;

    /// The minimum match value
    float minMatchSurfacePotentialVal;

    /// The maximum match value
    float maxMatchSurfacePotentialVal;

    /// TODO
    vislib::Array<float> matchSurfacePotentialSign;

    /// The minimum match value
    float minMatchSurfacePotentialSignVal;

    /// The maximum match value
    float maxMatchSurfacePotentialSignVal;

    /// TODO
    vislib::Array<float> matchMeanHausdorffDistance;

    /// The minimum match value
    float minMatchMeanHausdorffDistanceVal;

    /// The maximum match value
    float maxMatchMeanHausdorffDistanceVal;

    /// TODO
    vislib::Array<float> matchHausdorffDistance;

    /// The minimum match value
    float minMatchHausdorffDistanceVal;

    /// The maximum match value
    float maxMatchHausdorffDistanceVal;


    /// The number of variants to be compared
    unsigned int nVariants;


    /* Diagram data */

    /// Diagram series that contains data series for DiagramCall
    vislib::PtrArray<DiagramCall::DiagramSeries> featureList;


    /* RMS fitting */

    HostArr<float> rmsPosVec0;  ///> Position vector #0 for rms fitting
    HostArr<float> rmsPosVec1;  ///> Position vector #1 for rms fitting
    HostArr<float> rmsWeights;  ///> Particle weights
    HostArr<int> rmsMask;       ///> Mask for particles

    /* Volume generation */

    void *cudaqsurf0, *cudaqsurf1;   ///> Pointer to CUDAQuickSurf objects
    HostArr<float> gridDataPos;      ///> Data array for intermediate calculations

    /* Surface mapping */

    /// Device pointer to external forces for every vertex
    CudaDevArr<float> vertexExternalForcesScl_D;

    /// Array containing activity information for all grid cells (device memory)
    CudaDevArr<uint> cubeStates_D;

    /// Array containing activity information for all grid cells (device memory)
    CudaDevArr<uint> cubeOffsets_D;

    /// Mapping from list of active cells to generall cell list (and vice versa)
    CudaDevArr<uint> cubeMap0_D, cubeMapInv0_D;
    CudaDevArr<uint> cubeMap1_D, cubeMapInv1_D;

    /// Activity of vertices
    CudaDevArr<uint> vertexStates_D;

    /// Positions of active vertices
    CudaDevArr<float> vertexPos0_D, vertexPos1_D, vertexPosMapped_D, vertexPosRMSTransformed_D;

    /// Positions of active vertices (holds intermediate results)
    CudaDevArr<float3> activeVertexPos_D;

    /// Index offsets for active vertices
    CudaDevArr<uint> vertexIdxOffs_D;

    /// Mapping from active vertex indices to general index list (and vice versa)
    CudaDevArr<uint> vertexMap0_D, vertexMapInv0_D;
    CudaDevArr<uint> vertexMap1_D, vertexMapInv1_D;

    /// Connectivity information for all vertices (at most 18 neighbours per vertex)
    CudaDevArr<int> vertexNeighbours0_D, vertexNeighbours1_D;

    /// Array containing number of vertices for each tetrahedron
    CudaDevArr<uint> verticesPerTetrahedron_D;

    /// Vertex index offsets for all tetrahedrons
    CudaDevArr<uint> tetrahedronVertexOffsets_D;

    /// Array for volume gradient
    CudaDevArr<float4> volGradient_D;

    /// The bounding boxes of the density maps
//    gridParams gridDensMap0, gridDensMap1;
    /// Dimensions of the density volumes
    int3 volDim;

    /// Origin of the density volumes
    float3 volOrg;

    /// maximum coordinate of the density volumes
    float3 volMaxC;

    /// Delta for the density volumes
    float3 volDelta;

    /// The bounding boxes for potential maps
    gridParams gridPotential0, gridPotential1;

    /// Triangle indices
    CudaDevArr<uint> triangleIdx0_D, triangleIdx1_D;

    /// The number of active vertices
    uint vertexCnt0, vertexCnt1;

    /// The number of triangle indices
    uint triangleCnt0, triangleCnt1;

    /// Minimum force to keep going
    static const float minDispl;

    /* Surface area summation */

    /// The area of all triangles
    CudaDevArr<float> trianglesArea_D;

    /// The averaged vertex values for all triangles (potential difference)
    CudaDevArr<float> trianglesAreaWeightedPotentialDiff_D;

    /// The averaged vertex values for all triangles (hausdorff distance)
    CudaDevArr<float> trianglesAreaWeightedHausdorffDist_D;

    /// The averaged vertex values for all triangles (potential sign)
    CudaDevArr<float> trianglesAreaWeightedPotentialSign_D;

    /// Distance between new and old potential values
    CudaDevArr<float> vertexPotentialDiff_D;

    /// Flags whether the sign has changed
    CudaDevArr<float> vertexPotentialSignDiff_D;

    /// Flag for corrupt triangles
    CudaDevArr<float> corruptTriangleFlag_D;

    /// Per-vertex hausdorff distance
    CudaDevArr<float> vtxHausdorffDist_D;


    /* Boolean flags */

    /// Triggers recomputation of the match
    bool triggerComputeMatch;

    /// Triggers recomputation of the rmsd based match
    bool triggerComputeMatchRMSD;

    /* CUDA arrays holding potential textures */

    CudaDevArr<float> potentialTex0_D;
    CudaDevArr<float> potentialTex1_D;

    /// Triggers recomputation of the surface potential based match
    bool triggerComputeMatchSurfMapping;

    /// Array for laplacian
    CudaDevArr<float3> laplacian_D;


    Mat3f rmsRotation;          ///> Rotation matrix for the fitting
    Vec3f rmsTranslation;       ///> Translation vector for the fitting
    HostArr<float> atomPosFitted;

    DeformableGPUSurfaceMT surfStart;
    DeformableGPUSurfaceMT surfEnd;

    /// The bounding boxes for particle data
    Cubef bboxParticles;

    float maxAtomRad;
    float minAtomRad;


};

} // end namespace protein
} // end namespace megamol

#endif // MMPROTEINPLUGIN_PROTEINVARIANTMATCH_H_INCLUDED
#endif // (defined(WITH_CUDA) && (WITH_CUDA))
