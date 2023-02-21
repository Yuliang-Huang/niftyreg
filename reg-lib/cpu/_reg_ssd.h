/**
 * @file _reg_ssd.h
 * @brief File that contains sum squared difference related function
 * @author Marc Modat
 * @date 19/05/2009
 *
 *  Created by Marc Modat on 19/05/2009.
 *  Copyright (c) 2009-2018, University College London
 *  Copyright (c) 2018, NiftyReg Developers.
 *  All rights reserved.
 *  See the LICENSE.txt file in the nifty_reg root folder
 *
 */

#pragma once

#include "_reg_measure.h"

/* *************************************************************** */
/* *************************************************************** */
/// @brief SSD measure of similarity class
class reg_ssd: public reg_measure {
public:
    /// @brief reg_ssd class constructor
    reg_ssd();
    /// @brief reg_ssd class destructor
    virtual ~reg_ssd() {}

    /// @brief Initialise the reg_ssd object
    virtual void InitialiseMeasure(nifti_image *refImgPtr,
                                   nifti_image *floImgPtr,
                                   int *maskRefPtr,
                                   nifti_image *warFloImgPtr,
                                   nifti_image *warFloGraPtr,
                                   nifti_image *forVoxBasedGraPtr,
                                   nifti_image *localWeightSimPtr = nullptr,
                                   int *maskFloPtr = nullptr,
                                   nifti_image *warRefImgPtr = nullptr,
                                   nifti_image *warRefGraPtr = nullptr,
                                   nifti_image *bckVoxBasedGraPtr = nullptr) override;
    /// @brief Define if the specified time point should be normalised
    void SetNormaliseTimepoint(int timepoint, bool normalise);
    /// @brief Returns the ssd value
    virtual double GetSimilarityMeasureValue() override;
    /// @brief Compute the voxel based ssd gradient
    virtual void GetVoxelBasedSimilarityMeasureGradient(int current_timepoint) override;
    /// @brief Here
    virtual void GetDiscretisedValue(nifti_image *controlPointGridImage,
                                     float *discretisedValue,
                                     int discretise_radius,
                                     int discretise_step) override;
protected:
    float currentValue[255];

private:
    bool normaliseTimePoint[255];
};
/* *************************************************************** */

/** @brief Computes and returns the SSD between two input images
 * @param referenceImage First input image to use to compute the metric
 * @param warpedImage Second input image to use to compute the metric
 * @param activeTimePoint Specified which time point volumes have to be considered
 * @param jacobianDeterminantImage Image that contains the Jacobian
 * determinant of a transformation at every voxel position. This
 * image is used to modulate the SSD. The argument is ignored if the
 * pointer is set to nullptr
 * @param mask Array that contains a mask to specify which voxel
 * should be considered. If set to nullptr, all voxels are considered
 * @return Returns the computed sum squared difference
 */
extern "C++" template <class DataType>
double reg_getSSDValue(nifti_image *referenceImage,
                       nifti_image *warpedImage,
                       double *timePointWeight,
                       nifti_image *jacobianDeterminantImage,
                       int *mask,
                       float *currentValue,
                       nifti_image *localWeightImage);

/** @brief Compute a voxel based gradient of the sum squared difference.
 * @param referenceImage First input image to use to compute the metric
 * @param warpedImage Second input image to use to compute the metric
 * @param activeTimePoint Specified which time point volumes have to be considered
 * @param warpedImageGradient Spatial gradient of the input warped image
 * @param ssdGradientImage Output image that will be updated with the
 * value of the SSD gradient
 * @param jacobianDeterminantImage Image that contains the Jacobian
 * determinant of a transformation at every voxel position. This
 * image is used to modulate the SSD. The argument is ignored if the
 * pointer is set to nullptr
 * @param mask Array that contains a mask to specify which voxel
 * should be considered. If set to nullptr, all voxels are considered
 */
extern "C++" template <class DataType>
void reg_getVoxelBasedSSDGradient(nifti_image *referenceImage,
                                  nifti_image *warpedImage,
                                  nifti_image *warpedImageGradient,
                                  nifti_image *ssdGradientImage,
                                  nifti_image *jacobianDeterminantImage,
                                  int *mask,
                                  int current_timepoint,
                                  double timepoint_weight,
                                  nifti_image *localWeightImage);
