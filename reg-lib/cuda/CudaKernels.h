#pragma once
#include "Kernels.h"
#include "CudaContext.h"

//Kernel functions for affine deformation field 
class CudaAffineDeformationFieldKernel: public AffineDeformationFieldKernel {
public:
	CudaAffineDeformationFieldKernel(Context* conIn, std::string nameIn);
	void execute(bool compose = false);

	mat44 *affineTransformation;
	nifti_image *deformationFieldImage;

	float *deformationFieldArray_d;
	int* mask_d;
	CudaContext* con;

};
//Kernel functions for block matching
class CudaBlockMatchingKernel: public BlockMatchingKernel {
public:

	CudaBlockMatchingKernel(Context* conIn, std::string name);
	void execute();

	nifti_image* target;
	_reg_blockMatchingParam* params;

	CudaContext* con;

	float *targetImageArray_d, *resultImageArray_d, *targetPosition_d, *resultPosition_d, *targetMat_d;
	int *activeBlock_d, *mask_d;

};
//a kernel function for convolution (gaussian smoothing?)
class CudaConvolutionKernel: public ConvolutionKernel {
public:

	CudaConvolutionKernel(std::string name) :
			ConvolutionKernel(name) {
	}

	void execute(nifti_image *image, float *sigma, int kernelType, int *mask = NULL, bool *timePoints = NULL, bool *axis = NULL);

};

//kernel functions for numerical optimisation
class CudaOptimiseKernel: public OptimiseKernel {
public:

	CudaOptimiseKernel(Context* conIn, std::string name);
	void execute(bool affine);

	_reg_blockMatchingParam *blockMatchingParams;
	mat44 *transformationMatrix;
	CudaContext *con;
};

//kernel functions for image resampling with three interpolation variations
class CudaResampleImageKernel: public ResampleImageKernel {
public:
	CudaResampleImageKernel(Context* conIn, std::string name);
	void execute(int interp, float paddingValue, bool *dti_timepoint = NULL, mat33 * jacMat = NULL);

	nifti_image *floatingImage;
	nifti_image *warpedImage;


	//cuda ptrs
	float* floatingImageArray_d;
	float* floIJKMat_d;
	float* warpedImageArray_d;
	float* deformationFieldImageArray_d;
	int* mask_d;

	CudaContext *con;


};

