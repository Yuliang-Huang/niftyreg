#pragma once

#include "niftilib/nifti1_io.h"

/*
extern "C++"
void optimize_gpu(_reg_blockMatchingParam *blockMatchingParams,
                    mat44 *updateAffineMatrix,
                    float **targetPosition_d,
                    float **resultPosition_d,
                    bool affine = true);

extern "C++"
void affineLocalSearch3DCuda(mat44 *cpuMat, float* final_d, float *A_d, float* Sigma_d, float* U_d, float* VT_d, float * newResultPos_d, float* targetPos_d, float* resultPos_d, float* lengths_d, const unsigned numBlocks, const unsigned num_to_keep, const unsigned m, const unsigned n);
*/
extern "C++"
void cusolverSVD(float* A_d, unsigned m, unsigned n, float* S_d, float* VT_d, float* U_d);

extern "C++"
void optimize_affine3D_cuda(mat44* cpuMat, float* final_d, float* A_d, float* U_d, float* Sigma_d, float* VT_d, float* lengths_d, float* reference_d, float* warped_d, float* newWarped_d, unsigned m, unsigned n, const unsigned numToKeep, bool ilsIn, bool isAffine);
/*
extern "C++"
void getAffineMat3D(float* A_d, float* Sigma_d, float* VT_d, float* U_d, float* target_d, float* result_d, float* r_d, float *transformation, const unsigned numBlocks, unsigned m, unsigned n);

extern "C++"
void downloadMat44(mat44 *lastTransformation, float* transform_d);

extern "C++"
void uploadMat44(mat44 lastTransformation, float* transform_d);
*/
