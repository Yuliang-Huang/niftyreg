/**
 * @file  _reg_lncc.cpp
 * @author Aileen Corder
 * @author Marc Modat
 * @date 10/11/2012.
 * @brief CPP file for the LNCC related class and functions
 * Copyright (c) 2012-2018, University College London
 * Copyright (c) 2018, NiftyReg Developers.
 * All rights reserved.
 * See the LICENSE.txt file in the nifty_reg root folder
 */

#include "_reg_lncc.h"

 /* *************************************************************** */
 /* *************************************************************** */
reg_lncc::reg_lncc(): reg_measure() {
    this->forwardCorrelationImage = nullptr;
    this->referenceMeanImage = nullptr;
    this->referenceSdevImage = nullptr;
    this->warpedFloatingMeanImage = nullptr;
    this->warpedFloatingSdevImage = nullptr;
    this->forwardMask = nullptr;

    this->backwardCorrelationImage = nullptr;
    this->floatingMeanImage = nullptr;
    this->floatingSdevImage = nullptr;
    this->warpedReferenceMeanImage = nullptr;
    this->warpedReferenceSdevImage = nullptr;
    this->backwardMask = nullptr;

    // Gaussian kernel is used by default
    this->kernelType = GAUSSIAN_KERNEL;

    for (int i = 0; i < 255; ++i)
        kernelStandardDeviation[i] = -5.f;
#ifndef NDEBUG
    reg_print_msg_debug("reg_lncc constructor called");
#endif
}
/* *************************************************************** */
/* *************************************************************** */
reg_lncc::~reg_lncc() {
    if (this->forwardCorrelationImage != nullptr)
        nifti_image_free(this->forwardCorrelationImage);
    this->forwardCorrelationImage = nullptr;
    if (this->referenceMeanImage != nullptr)
        nifti_image_free(this->referenceMeanImage);
    this->referenceMeanImage = nullptr;
    if (this->referenceSdevImage != nullptr)
        nifti_image_free(this->referenceSdevImage);
    this->referenceSdevImage = nullptr;
    if (this->warpedFloatingMeanImage != nullptr)
        nifti_image_free(this->warpedFloatingMeanImage);
    this->warpedFloatingMeanImage = nullptr;
    if (this->warpedFloatingSdevImage != nullptr)
        nifti_image_free(this->warpedFloatingSdevImage);
    this->warpedFloatingSdevImage = nullptr;
    if (this->forwardMask != nullptr)
        free(this->forwardMask);
    this->forwardMask = nullptr;

    if (this->backwardCorrelationImage != nullptr)
        nifti_image_free(this->backwardCorrelationImage);
    this->backwardCorrelationImage = nullptr;
    if (this->floatingMeanImage != nullptr)
        nifti_image_free(this->floatingMeanImage);
    this->floatingMeanImage = nullptr;
    if (this->floatingSdevImage != nullptr)
        nifti_image_free(this->floatingSdevImage);
    this->floatingSdevImage = nullptr;
    if (this->warpedReferenceMeanImage != nullptr)
        nifti_image_free(this->warpedReferenceMeanImage);
    this->warpedReferenceMeanImage = nullptr;
    if (this->warpedReferenceSdevImage != nullptr)
        nifti_image_free(this->warpedReferenceSdevImage);
    this->warpedReferenceSdevImage = nullptr;
    if (this->backwardMask != nullptr)
        free(this->backwardMask);
    this->backwardMask = nullptr;
}
/* *************************************************************** */
/* *************************************************************** */
template <class DataType>
void reg_lncc::UpdateLocalStatImages(nifti_image *refImage,
                                     nifti_image *warImage,
                                     nifti_image *meanRefImage,
                                     nifti_image *meanWarImage,
                                     nifti_image *stdDevRefImage,
                                     nifti_image *stdDevWarImage,
                                     int *refMask,
                                     int *combinedMask,
                                     int current_timepoint) {
    // Generate the forward mask to ignore all NaN values
#ifdef _WIN32
    long voxel;
    const long voxelNumber = (long)CalcVoxelNumber(*refImage);
#else
    size_t voxel;
    const size_t voxelNumber = CalcVoxelNumber(*refImage);
#endif
    memcpy(combinedMask, refMask, voxelNumber * sizeof(int));
    reg_tools_removeNanFromMask(refImage, combinedMask);
    reg_tools_removeNanFromMask(warImage, combinedMask);

    DataType *origRefPtr = static_cast<DataType*>(refImage->data);
    DataType *meanRefPtr = static_cast<DataType*>(meanRefImage->data);
    DataType *sdevRefPtr = static_cast<DataType*>(stdDevRefImage->data);
    memcpy(meanRefPtr, &origRefPtr[current_timepoint * voxelNumber], voxelNumber * refImage->nbyper);
    memcpy(sdevRefPtr, &origRefPtr[current_timepoint * voxelNumber], voxelNumber * refImage->nbyper);

    reg_tools_multiplyImageToImage(stdDevRefImage, stdDevRefImage, stdDevRefImage);
    reg_tools_kernelConvolution(meanRefImage, this->kernelStandardDeviation, this->kernelType, combinedMask);
    reg_tools_kernelConvolution(stdDevRefImage, this->kernelStandardDeviation, this->kernelType, combinedMask);

    DataType *origWarPtr = static_cast<DataType*>(warImage->data);
    DataType *meanWarPtr = static_cast<DataType*>(meanWarImage->data);
    DataType *sdevWarPtr = static_cast<DataType*>(stdDevWarImage->data);
    memcpy(meanWarPtr, &origWarPtr[current_timepoint * voxelNumber], voxelNumber * warImage->nbyper);
    memcpy(sdevWarPtr, &origWarPtr[current_timepoint * voxelNumber], voxelNumber * warImage->nbyper);

    reg_tools_multiplyImageToImage(stdDevWarImage, stdDevWarImage, stdDevWarImage);
    reg_tools_kernelConvolution(meanWarImage, this->kernelStandardDeviation, this->kernelType, combinedMask);
    reg_tools_kernelConvolution(stdDevWarImage, this->kernelStandardDeviation, this->kernelType, combinedMask);
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(voxelNumber, sdevRefPtr, meanRefPtr, sdevWarPtr, meanWarPtr)
#endif
    for (voxel = 0; voxel < voxelNumber; ++voxel) {
        // G*(I^2) - (G*I)^2
        sdevRefPtr[voxel] = sqrt(sdevRefPtr[voxel] - reg_pow2(meanRefPtr[voxel]));
        sdevWarPtr[voxel] = sqrt(sdevWarPtr[voxel] - reg_pow2(meanWarPtr[voxel]));
        // Stabilise the computation
        if (sdevRefPtr[voxel] < 1.e-06) sdevRefPtr[voxel] = 0;
        if (sdevWarPtr[voxel] < 1.e-06) sdevWarPtr[voxel] = 0;
    }
}
/* *************************************************************** */
/* *************************************************************** */
void reg_lncc::InitialiseMeasure(nifti_image *refImgPtr,
                                 nifti_image *floImgPtr,
                                 int *maskRefPtr,
                                 nifti_image *warFloImgPtr,
                                 nifti_image *warFloGraPtr,
                                 nifti_image *forVoxBasedGraPtr,
                                 nifti_image *localWeightSimPtr,
                                 int *maskFloPtr,
                                 nifti_image *warRefImgPtr,
                                 nifti_image *warRefGraPtr,
                                 nifti_image *bckVoxBasedGraPtr) {
    reg_measure::InitialiseMeasure(refImgPtr,
                                   floImgPtr,
                                   maskRefPtr,
                                   warFloImgPtr,
                                   warFloGraPtr,
                                   forVoxBasedGraPtr,
                                   localWeightSimPtr,
                                   maskFloPtr,
                                   warRefImgPtr,
                                   warRefGraPtr,
                                   bckVoxBasedGraPtr);

    for (int i = 0; i < this->referenceImagePointer->nt; ++i) {
        if (this->timePointWeight[i] > 0) {
            reg_intensityRescale(this->referenceImagePointer, i, 0.f, 1.f);
            reg_intensityRescale(this->floatingImagePointer, i, 0.f, 1.f);
        }
    }

    // Check that no images are already allocated
    if (this->forwardCorrelationImage != nullptr)
        nifti_image_free(this->forwardCorrelationImage);
    this->forwardCorrelationImage = nullptr;
    if (this->referenceMeanImage != nullptr)
        nifti_image_free(this->referenceMeanImage);
    this->referenceMeanImage = nullptr;
    if (this->referenceSdevImage != nullptr)
        nifti_image_free(this->referenceSdevImage);
    this->referenceSdevImage = nullptr;
    if (this->warpedFloatingMeanImage != nullptr)
        nifti_image_free(this->warpedFloatingMeanImage);
    this->warpedFloatingMeanImage = nullptr;
    if (this->warpedFloatingSdevImage != nullptr)
        nifti_image_free(this->warpedFloatingSdevImage);
    this->warpedFloatingSdevImage = nullptr;
    if (this->backwardCorrelationImage != nullptr)
        nifti_image_free(this->backwardCorrelationImage);
    this->backwardCorrelationImage = nullptr;
    if (this->floatingMeanImage != nullptr)
        nifti_image_free(this->floatingMeanImage);
    this->floatingMeanImage = nullptr;
    if (this->floatingSdevImage != nullptr)
        nifti_image_free(this->floatingSdevImage);
    this->floatingSdevImage = nullptr;
    if (this->warpedReferenceMeanImage != nullptr)
        nifti_image_free(this->warpedReferenceMeanImage);
    this->warpedReferenceMeanImage = nullptr;
    if (this->warpedReferenceSdevImage != nullptr)
        nifti_image_free(this->warpedReferenceSdevImage);
    this->warpedReferenceSdevImage = nullptr;
    if (this->forwardMask != nullptr)
        free(this->forwardMask);
    this->forwardMask = nullptr;
    if (this->backwardMask != nullptr)
        free(this->backwardMask);
    this->backwardMask = nullptr;

    size_t voxelNumber = CalcVoxelNumber(*this->referenceImagePointer);

    // Allocate the required image to store the correlation of the forward transformation
    this->forwardCorrelationImage = nifti_copy_nim_info(this->referenceImagePointer);
    this->forwardCorrelationImage->ndim = this->forwardCorrelationImage->dim[0] = this->referenceImagePointer->nz > 1 ? 3 : 2;
    this->forwardCorrelationImage->nt = this->forwardCorrelationImage->dim[4] = 1;
    this->forwardCorrelationImage->nvox = voxelNumber;
    this->forwardCorrelationImage->data = malloc(voxelNumber * this->forwardCorrelationImage->nbyper);

    // Allocate the required images to store mean and stdev of the reference image
    this->referenceMeanImage = nifti_dup(*this->forwardCorrelationImage, false);
    this->referenceSdevImage = nifti_dup(*this->forwardCorrelationImage, false);

    // Allocate the required images to store mean and stdev of the warped floating image
    this->warpedFloatingMeanImage = nifti_dup(*this->forwardCorrelationImage, false);
    this->warpedFloatingSdevImage = nifti_dup(*this->forwardCorrelationImage, false);

    // Allocate the array to store the mask of the forward image
    this->forwardMask = (int*)malloc(voxelNumber * sizeof(int));
    if (this->isSymmetric) {
        voxelNumber = CalcVoxelNumber(*floatingImagePointer);

        // Allocate the required image to store the correlation of the backward transformation
        this->backwardCorrelationImage = nifti_copy_nim_info(this->floatingImagePointer);
        this->backwardCorrelationImage->ndim = this->backwardCorrelationImage->dim[0] = this->floatingImagePointer->nz > 1 ? 3 : 2;
        this->backwardCorrelationImage->nt = this->backwardCorrelationImage->dim[4] = 1;
        this->backwardCorrelationImage->nvox = voxelNumber;
        this->backwardCorrelationImage->data = malloc(voxelNumber * this->backwardCorrelationImage->nbyper);

        // Allocate the required images to store mean and stdev of the floating image
        this->floatingMeanImage = nifti_dup(*this->backwardCorrelationImage, false);
        this->floatingSdevImage = nifti_dup(*this->backwardCorrelationImage, false);

        // Allocate the required images to store mean and stdev of the warped reference image
        this->warpedReferenceMeanImage = nifti_dup(*this->backwardCorrelationImage, false);
        this->warpedReferenceSdevImage = nifti_dup(*this->backwardCorrelationImage, false);

        // Allocate the array to store the mask of the backward image
        this->backwardMask = (int*)malloc(voxelNumber * sizeof(int));
    }
#ifndef NDEBUG
    char text[255];
    reg_print_msg_debug("reg_lncc::InitialiseMeasure().");
    for (int i = 0; i < this->referenceImagePointer->nt; ++i) {
        sprintf(text, "Weight for timepoint %i: %f", i, this->timePointWeight[i]);
        reg_print_msg_debug(text);
    }
#endif
}
/* *************************************************************** */
/* *************************************************************** */
template<class DataType>
double reg_getLNCCValue(nifti_image *referenceImage,
                        nifti_image *referenceMeanImage,
                        nifti_image *referenceSdevImage,
                        nifti_image *warpedImage,
                        nifti_image *warpedMeanImage,
                        nifti_image *warpedSdevImage,
                        int *combinedMask,
                        float *kernelStandardDeviation,
                        nifti_image *correlationImage,
                        int kernelType,
                        int current_timepoint) {
#ifdef _WIN32
    long voxel;
    const long voxelNumber = (long)CalcVoxelNumber(*referenceImage);
#else
    size_t voxel;
    const size_t voxelNumber = CalcVoxelNumber(*referenceImage);
#endif

    // Compute the local correlation
    DataType *refImagePtr = static_cast<DataType*>(referenceImage->data);
    DataType *currentRefPtr = &refImagePtr[current_timepoint * voxelNumber];

    DataType *warImagePtr = static_cast<DataType*>(warpedImage->data);
    DataType *currentWarPtr = &warImagePtr[current_timepoint * voxelNumber];

    DataType *refMeanPtr = static_cast<DataType*>(referenceMeanImage->data);
    DataType *warMeanPtr = static_cast<DataType*>(warpedMeanImage->data);
    DataType *refSdevPtr = static_cast<DataType*>(referenceSdevImage->data);
    DataType *warSdevPtr = static_cast<DataType*>(warpedSdevImage->data);
    DataType *correlaPtr = static_cast<DataType*>(correlationImage->data);

    for (size_t i = 0; i < voxelNumber; ++i)
        correlaPtr[i] = currentRefPtr[i] * currentWarPtr[i];

    reg_tools_kernelConvolution(correlationImage, kernelStandardDeviation, kernelType, combinedMask);

    double lncc_value_sum = 0., lncc_value;
    double activeVoxel_num = 0.;

    // Iteration over all voxels
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(voxelNumber,combinedMask,refMeanPtr,warMeanPtr, \
    refSdevPtr,warSdevPtr,correlaPtr) \
    private(lncc_value) \
    reduction(+:lncc_value_sum) \
    reduction(+:activeVoxel_num)
#endif
    for (voxel = 0; voxel < voxelNumber; ++voxel) {
        // Check if the current voxel belongs to the mask
        if (combinedMask[voxel] > -1) {
            lncc_value = (correlaPtr[voxel] - (refMeanPtr[voxel] * warMeanPtr[voxel])) / (refSdevPtr[voxel] * warSdevPtr[voxel]);
            if (lncc_value == lncc_value && isinf(lncc_value) == 0) {
                lncc_value_sum += fabs(lncc_value);
                ++activeVoxel_num;
            }
        }
    }
    return lncc_value_sum / activeVoxel_num;
}
/* *************************************************************** */
/* *************************************************************** */
double reg_lncc::GetSimilarityMeasureValue() {
    double lncc_value = 0;

    for (int current_timepoint = 0; current_timepoint < this->referenceImagePointer->nt; ++current_timepoint) {
        if (this->timePointWeight[current_timepoint] > 0) {
            double tp_value = 0;
            // Compute the mean and variance of the reference and warped floating
            switch (this->referenceImagePointer->datatype) {
            case NIFTI_TYPE_FLOAT32:
                this->UpdateLocalStatImages<float>(this->referenceImagePointer,
                                                   this->warpedFloatingImagePointer,
                                                   this->referenceMeanImage,
                                                   this->warpedFloatingMeanImage,
                                                   this->referenceSdevImage,
                                                   this->warpedFloatingSdevImage,
                                                   this->referenceMaskPointer,
                                                   this->forwardMask,
                                                   current_timepoint);
                break;
            case NIFTI_TYPE_FLOAT64:
                this->UpdateLocalStatImages<double>(this->referenceImagePointer,
                                                    this->warpedFloatingImagePointer,
                                                    this->referenceMeanImage,
                                                    this->warpedFloatingMeanImage,
                                                    this->referenceSdevImage,
                                                    this->warpedFloatingSdevImage,
                                                    this->referenceMaskPointer,
                                                    this->forwardMask,
                                                    current_timepoint);
                break;
            }

            // Compute the LNCC - Forward
            switch (this->referenceImagePointer->datatype) {
            case NIFTI_TYPE_FLOAT32:
                tp_value += reg_getLNCCValue<float>(this->referenceImagePointer,
                                                    this->referenceMeanImage,
                                                    this->referenceSdevImage,
                                                    this->warpedFloatingImagePointer,
                                                    this->warpedFloatingMeanImage,
                                                    this->warpedFloatingSdevImage,
                                                    this->forwardMask,
                                                    this->kernelStandardDeviation,
                                                    this->forwardCorrelationImage,
                                                    this->kernelType,
                                                    current_timepoint);
                break;
            case NIFTI_TYPE_FLOAT64:
                tp_value += reg_getLNCCValue<double>(this->referenceImagePointer,
                                                     this->referenceMeanImage,
                                                     this->referenceSdevImage,
                                                     this->warpedFloatingImagePointer,
                                                     this->warpedFloatingMeanImage,
                                                     this->warpedFloatingSdevImage,
                                                     this->forwardMask,
                                                     this->kernelStandardDeviation,
                                                     this->forwardCorrelationImage,
                                                     this->kernelType,
                                                     current_timepoint);
                break;
            }
            if (this->isSymmetric) {
                // Compute the mean and variance of the floating and warped reference
                switch (this->floatingImagePointer->datatype) {
                case NIFTI_TYPE_FLOAT32:
                    this->UpdateLocalStatImages<float>(this->floatingImagePointer,
                                                       this->warpedReferenceImagePointer,
                                                       this->floatingMeanImage,
                                                       this->warpedReferenceMeanImage,
                                                       this->floatingSdevImage,
                                                       this->warpedReferenceSdevImage,
                                                       this->floatingMaskPointer,
                                                       this->backwardMask,
                                                       current_timepoint);
                    break;
                case NIFTI_TYPE_FLOAT64:
                    this->UpdateLocalStatImages<double>(this->floatingImagePointer,
                                                        this->warpedReferenceImagePointer,
                                                        this->floatingMeanImage,
                                                        this->warpedReferenceMeanImage,
                                                        this->floatingSdevImage,
                                                        this->warpedReferenceSdevImage,
                                                        this->floatingMaskPointer,
                                                        this->backwardMask,
                                                        current_timepoint);
                    break;
                }
                // Compute the LNCC - Backward
                switch (this->floatingImagePointer->datatype) {
                case NIFTI_TYPE_FLOAT32:
                    tp_value += reg_getLNCCValue<float>(this->floatingImagePointer,
                                                        this->floatingMeanImage,
                                                        this->floatingSdevImage,
                                                        this->warpedReferenceImagePointer,
                                                        this->warpedReferenceMeanImage,
                                                        this->warpedReferenceSdevImage,
                                                        this->backwardMask,
                                                        this->kernelStandardDeviation,
                                                        this->backwardCorrelationImage,
                                                        this->kernelType,
                                                        current_timepoint);
                    break;
                case NIFTI_TYPE_FLOAT64:
                    tp_value += reg_getLNCCValue<double>(this->floatingImagePointer,
                                                         this->floatingMeanImage,
                                                         this->floatingSdevImage,
                                                         this->warpedReferenceImagePointer,
                                                         this->warpedReferenceMeanImage,
                                                         this->warpedReferenceSdevImage,
                                                         this->backwardMask,
                                                         this->kernelStandardDeviation,
                                                         this->backwardCorrelationImage,
                                                         this->kernelType,
                                                         current_timepoint);
                    break;
                }
            }
            lncc_value += tp_value * this->timePointWeight[current_timepoint];
        }
    }
    return lncc_value;
}
/* *************************************************************** */
/* *************************************************************** */
template <class DataType>
void reg_getVoxelBasedLNCCGradient(nifti_image *referenceImage,
                                   nifti_image *referenceMeanImage,
                                   nifti_image *referenceSdevImage,
                                   nifti_image *warpedImage,
                                   nifti_image *warpedMeanImage,
                                   nifti_image *warpedSdevImage,
                                   int *combinedMask,
                                   float *kernelStandardDeviation,
                                   nifti_image *correlationImage,
                                   nifti_image *warpedGradient,
                                   nifti_image *measureGradientImage,
                                   int kernelType,
                                   int current_timepoint,
                                   double timepoint_weight) {
#ifdef _WIN32
    long voxel;
    long voxelNumber = (long)CalcVoxelNumber(*referenceImage);
#else
    size_t voxel;
    size_t voxelNumber = CalcVoxelNumber(*referenceImage);
#endif

    // Compute the local correlation
    DataType *refImagePtr = static_cast<DataType*>(referenceImage->data);
    DataType *currentRefPtr = &refImagePtr[current_timepoint * voxelNumber];

    DataType *warImagePtr = static_cast<DataType*>(warpedImage->data);
    DataType *currentWarPtr = &warImagePtr[current_timepoint * voxelNumber];

    DataType *refMeanPtr = static_cast<DataType*>(referenceMeanImage->data);
    DataType *warMeanPtr = static_cast<DataType*>(warpedMeanImage->data);
    DataType *refSdevPtr = static_cast<DataType*>(referenceSdevImage->data);
    DataType *warSdevPtr = static_cast<DataType*>(warpedSdevImage->data);
    DataType *correlaPtr = static_cast<DataType*>(correlationImage->data);

    for (size_t i = 0; i < voxelNumber; ++i)
        correlaPtr[i] = currentRefPtr[i] * currentWarPtr[i];

    reg_tools_kernelConvolution(correlationImage, kernelStandardDeviation, kernelType, combinedMask);

    double refMeanValue, warMeanValue, refSdevValue, warSdevValue, correlaValue;
    double temp1, temp2, temp3;
    double activeVoxel_num = 0;

    // Iteration over all voxels
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(voxelNumber,combinedMask,refMeanPtr,warMeanPtr, \
    refSdevPtr,warSdevPtr,correlaPtr) \
    private(refMeanValue,warMeanValue,refSdevValue, \
    warSdevValue, correlaValue, temp1, temp2, temp3) \
    reduction(+:activeVoxel_num)
#endif
    for (voxel = 0; voxel < voxelNumber; ++voxel) {
        // Check if the current voxel belongs to the mask
        if (combinedMask[voxel] > -1) {

            refMeanValue = refMeanPtr[voxel];
            warMeanValue = warMeanPtr[voxel];
            refSdevValue = refSdevPtr[voxel];
            warSdevValue = warSdevPtr[voxel];
            correlaValue = correlaPtr[voxel] - (refMeanValue * warMeanValue);

            temp1 = 1.0 / (refSdevValue * warSdevValue);
            temp2 = correlaValue /
                (refSdevValue * warSdevValue * warSdevValue * warSdevValue);
            temp3 = (correlaValue * warMeanValue) /
                (refSdevValue * warSdevValue * warSdevValue * warSdevValue)
                -
                refMeanValue / (refSdevValue * warSdevValue);
            if (temp1 == temp1 && isinf(temp1) == 0 &&
                temp2 == temp2 && isinf(temp2) == 0 &&
                temp3 == temp3 && isinf(temp3) == 0) {
                // Derivative of the absolute function
                if (correlaValue < 0) {
                    temp1 *= -1;
                    temp2 *= -1;
                    temp3 *= -1;
                }
                warMeanPtr[voxel] = static_cast<DataType>(temp1);
                warSdevPtr[voxel] = static_cast<DataType>(temp2);
                correlaPtr[voxel] = static_cast<DataType>(temp3);
                activeVoxel_num++;
            } else warMeanPtr[voxel] = warSdevPtr[voxel] = correlaPtr[voxel] = 0;
        } else warMeanPtr[voxel] = warSdevPtr[voxel] = correlaPtr[voxel] = 0;
    }

    //adjust weight for number of voxels
    double adjusted_weight = timepoint_weight / activeVoxel_num;

    // Smooth the newly computed values
    reg_tools_kernelConvolution(warpedMeanImage, kernelStandardDeviation, kernelType, combinedMask);
    reg_tools_kernelConvolution(warpedSdevImage, kernelStandardDeviation, kernelType, combinedMask);
    reg_tools_kernelConvolution(correlationImage, kernelStandardDeviation, kernelType, combinedMask);
    DataType *measureGradPtrX = static_cast<DataType*>(measureGradientImage->data);
    DataType *measureGradPtrY = &measureGradPtrX[voxelNumber];
    DataType *measureGradPtrZ = nullptr;
    if (referenceImage->nz > 1)
        measureGradPtrZ = &measureGradPtrY[voxelNumber];

    // Create pointers to the spatial gradient of the warped image
    DataType *warpGradPtrX = static_cast<DataType*>(warpedGradient->data);
    DataType *warpGradPtrY = &warpGradPtrX[voxelNumber];
    DataType *warpGradPtrZ = nullptr;
    if (referenceImage->nz > 1)
        warpGradPtrZ = &warpGradPtrY[voxelNumber];

    double common;
    // Iteration over all voxels
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(voxelNumber,combinedMask,currentRefPtr,currentWarPtr, \
    warMeanPtr,warSdevPtr,correlaPtr,measureGradPtrX,measureGradPtrY, \
    measureGradPtrZ, warpGradPtrX, warpGradPtrY, warpGradPtrZ, adjusted_weight) \
    private(common)
#endif
    for (voxel = 0; voxel < voxelNumber; ++voxel) {
        // Check if the current voxel belongs to the mask
        if (combinedMask[voxel] > -1) {
            common = warMeanPtr[voxel] * currentRefPtr[voxel] - warSdevPtr[voxel] * currentWarPtr[voxel] + correlaPtr[voxel];
            common *= adjusted_weight;
            measureGradPtrX[voxel] -= static_cast<DataType>(warpGradPtrX[voxel] * common);
            measureGradPtrY[voxel] -= static_cast<DataType>(warpGradPtrY[voxel] * common);
            if (warpGradPtrZ != nullptr)
                measureGradPtrZ[voxel] -= static_cast<DataType>(warpGradPtrZ[voxel] * common);
        }
    }
    // Check for NaN
    DataType val;
#ifdef _WIN32
    voxelNumber = (long)measureGradientImage->nvox;
#else
    voxelNumber = measureGradientImage->nvox;
#endif
#ifdef _OPENMP
#pragma omp parallel for default(none) \
    shared(voxelNumber,measureGradPtrX) \
    private(val)
#endif
    for (voxel = 0; voxel < voxelNumber; ++voxel) {
        val = measureGradPtrX[voxel];
        if (val != val || isinf(val) != 0)
            measureGradPtrX[voxel] = 0;
    }
}
/* *************************************************************** */
/* *************************************************************** */
void reg_lncc::GetVoxelBasedSimilarityMeasureGradient(int current_timepoint) {
    // Check if the specified time point exists and is active
    reg_measure::GetVoxelBasedSimilarityMeasureGradient(current_timepoint);
    if (this->timePointWeight[current_timepoint] == 0)
        return;

    // Compute the mean and variance of the reference and warped floating
    switch (this->referenceImagePointer->datatype) {
    case NIFTI_TYPE_FLOAT32:
        this->UpdateLocalStatImages<float>(this->referenceImagePointer,
                                           this->warpedFloatingImagePointer,
                                           this->referenceMeanImage,
                                           this->warpedFloatingMeanImage,
                                           this->referenceSdevImage,
                                           this->warpedFloatingSdevImage,
                                           this->referenceMaskPointer,
                                           this->forwardMask,
                                           current_timepoint);
        break;
    case NIFTI_TYPE_FLOAT64:
        this->UpdateLocalStatImages<double>(this->referenceImagePointer,
                                            this->warpedFloatingImagePointer,
                                            this->referenceMeanImage,
                                            this->warpedFloatingMeanImage,
                                            this->referenceSdevImage,
                                            this->warpedFloatingSdevImage,
                                            this->referenceMaskPointer,
                                            this->forwardMask,
                                            current_timepoint);
        break;
    }

    // Compute the LNCC gradient - Forward
    switch (this->referenceImagePointer->datatype) {
    case NIFTI_TYPE_FLOAT32:
        reg_getVoxelBasedLNCCGradient<float>(this->referenceImagePointer,
                                             this->referenceMeanImage,
                                             this->referenceSdevImage,
                                             this->warpedFloatingImagePointer,
                                             this->warpedFloatingMeanImage,
                                             this->warpedFloatingSdevImage,
                                             this->forwardMask,
                                             this->kernelStandardDeviation,
                                             this->forwardCorrelationImage,
                                             this->warpedFloatingGradientImagePointer,
                                             this->forwardVoxelBasedGradientImagePointer,
                                             this->kernelType,
                                             current_timepoint,
                                             this->timePointWeight[current_timepoint]);
        break;
    case NIFTI_TYPE_FLOAT64:
        reg_getVoxelBasedLNCCGradient<double>(this->referenceImagePointer,
                                              this->referenceMeanImage,
                                              this->referenceSdevImage,
                                              this->warpedFloatingImagePointer,
                                              this->warpedFloatingMeanImage,
                                              this->warpedFloatingSdevImage,
                                              this->forwardMask,
                                              this->kernelStandardDeviation,
                                              this->forwardCorrelationImage,
                                              this->warpedFloatingGradientImagePointer,
                                              this->forwardVoxelBasedGradientImagePointer,
                                              this->kernelType,
                                              current_timepoint,
                                              this->timePointWeight[current_timepoint]);
        break;
    }
    if (this->isSymmetric) {
        // Compute the mean and variance of the floating and warped reference
        switch (this->floatingImagePointer->datatype) {
        case NIFTI_TYPE_FLOAT32:
            this->UpdateLocalStatImages<float>(this->floatingImagePointer,
                                               this->warpedReferenceImagePointer,
                                               this->floatingMeanImage,
                                               this->warpedReferenceMeanImage,
                                               this->floatingSdevImage,
                                               this->warpedReferenceSdevImage,
                                               this->floatingMaskPointer,
                                               this->backwardMask,
                                               current_timepoint);
            break;
        case NIFTI_TYPE_FLOAT64:
            this->UpdateLocalStatImages<double>(this->floatingImagePointer,
                                                this->warpedReferenceImagePointer,
                                                this->floatingMeanImage,
                                                this->warpedReferenceMeanImage,
                                                this->floatingSdevImage,
                                                this->warpedReferenceSdevImage,
                                                this->floatingMaskPointer,
                                                this->backwardMask,
                                                current_timepoint);
            break;
        }
        // Compute the LNCC gradient - Backward
        switch (this->floatingImagePointer->datatype) {
        case NIFTI_TYPE_FLOAT32:
            reg_getVoxelBasedLNCCGradient<float>(this->floatingImagePointer,
                                                 this->floatingMeanImage,
                                                 this->floatingSdevImage,
                                                 this->warpedReferenceImagePointer,
                                                 this->warpedReferenceMeanImage,
                                                 this->warpedReferenceSdevImage,
                                                 this->backwardMask,
                                                 this->kernelStandardDeviation,
                                                 this->backwardCorrelationImage,
                                                 this->warpedReferenceGradientImagePointer,
                                                 this->backwardVoxelBasedGradientImagePointer,
                                                 this->kernelType,
                                                 current_timepoint,
                                                 this->timePointWeight[current_timepoint]);
            break;
        case NIFTI_TYPE_FLOAT64:
            reg_getVoxelBasedLNCCGradient<double>(this->floatingImagePointer,
                                                  this->floatingMeanImage,
                                                  this->floatingSdevImage,
                                                  this->warpedReferenceImagePointer,
                                                  this->warpedReferenceMeanImage,
                                                  this->warpedReferenceSdevImage,
                                                  this->backwardMask,
                                                  this->kernelStandardDeviation,
                                                  this->backwardCorrelationImage,
                                                  this->warpedReferenceGradientImagePointer,
                                                  this->backwardVoxelBasedGradientImagePointer,
                                                  this->kernelType,
                                                  current_timepoint,
                                                  this->timePointWeight[current_timepoint]);
            break;
        }
    }
}
/* *************************************************************** */
/* *************************************************************** */
