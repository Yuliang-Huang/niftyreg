#include "NormaliseGradient.hpp"
#include "_reg_tools_gpu.h"

/* *************************************************************** */
__global__ static void GetMaximalLengthKernel(float *dists,
                                              cudaTextureObject_t imageTexture,
                                              const size_t nVoxels,
                                              const bool optimiseX,
                                              const bool optimiseY,
                                              const bool optimiseZ) {
    const size_t tid = ((size_t)blockIdx.y * gridDim.x + blockIdx.x) * blockDim.x + threadIdx.x;
    if (tid < nVoxels) {
        float4 gradValue = tex1Dfetch<float4>(imageTexture, tid);
        dists[tid] = sqrtf((optimiseX ? gradValue.x * gradValue.x : 0) +
                           (optimiseY ? gradValue.y * gradValue.y : 0) +
                           (optimiseZ ? gradValue.z * gradValue.z : 0));
    }
}
/* *************************************************************** */
float NiftyReg::Cuda::GetMaximalLength(const float4 *imageCuda,
                                       const size_t& nVoxels,
                                       const bool& optimiseX,
                                       const bool& optimiseY,
                                       const bool& optimiseZ) {
    // Create a texture object for the imageCuda
    auto imageTexture = cudaCommon_createTextureObject(imageCuda, cudaResourceTypeLinear, false, nVoxels * sizeof(float4),
                                                       cudaChannelFormatKindFloat, 4, cudaFilterModePoint);

    float *dists = nullptr;
    NR_CUDA_SAFE_CALL(cudaMalloc(&dists, nVoxels * sizeof(float)));

    const unsigned int blocks = static_cast<unsigned int>(NiftyReg_CudaBlock::GetInstance(0)->Block_GetMaximalLength);
    const unsigned int grids = static_cast<unsigned int>(reg_ceil(sqrtf(static_cast<float>(nVoxels) / static_cast<float>(blocks))));
    dim3 blockDims(blocks, 1, 1);
    dim3 gridDims(grids, grids, 1);
    GetMaximalLengthKernel<<<gridDims, blockDims>>>(dists, *imageTexture, nVoxels, optimiseX, optimiseY, optimiseZ);
    NR_CUDA_CHECK_KERNEL(gridDims, blockDims);

    const float maxDistance = reg_maxReduction_gpu(dists, nVoxels);
    NR_CUDA_SAFE_CALL(cudaFree(dists));

    return maxDistance;
}
/* *************************************************************** */
__global__ static void NormaliseGradientKernel(float4 *imageCuda,
                                               const size_t nVoxels,
                                               const float maxGradLenInv,
                                               const bool optimiseX,
                                               const bool optimiseY,
                                               const bool optimiseZ) {
    const size_t tid = ((size_t)blockIdx.y * gridDim.x + blockIdx.x) * blockDim.x + threadIdx.x;
    if (tid < nVoxels) {
        float4 grad = imageCuda[tid];
        imageCuda[tid] = make_float4(optimiseX ? grad.x * maxGradLenInv : 0,
                                     optimiseY ? grad.y * maxGradLenInv : 0,
                                     optimiseZ ? grad.z * maxGradLenInv : 0,
                                     grad.w);
    }
}
/* *************************************************************** */
void NiftyReg::Cuda::NormaliseGradient(float4 *imageCuda,
                                       const size_t& nVoxels,
                                       const float& maxGradLength,
                                       const bool& optimiseX,
                                       const bool& optimiseY,
                                       const bool& optimiseZ) {
    const unsigned int blocks = static_cast<unsigned int>(NiftyReg_CudaBlock::GetInstance(0)->Block_reg_arithmetic);
    const unsigned int grids = static_cast<unsigned int>(ceil(sqrtf(static_cast<float>(nVoxels) / static_cast<float>(blocks))));
    const dim3 gridDims(grids, grids, 1);
    const dim3 blockDims(blocks, 1, 1);
    NormaliseGradientKernel<<<gridDims, blockDims>>>(imageCuda, nVoxels, 1 / maxGradLength, optimiseX, optimiseY, optimiseZ);
    NR_CUDA_CHECK_KERNEL(gridDims, blockDims);
}
/* *************************************************************** */
