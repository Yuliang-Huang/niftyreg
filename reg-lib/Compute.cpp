#include "Compute.h"
#include "F3dContent.h"
#include "_reg_resampling.h"
#include "_reg_localTrans_jac.h"
#include "_reg_localTrans_regul.h"

/* *************************************************************** */
void Compute::ResampleImage(int inter, float paddingValue) {
    reg_resampleImage(con.GetFloating(),
                      con.GetWarped(),
                      con.GetDeformationField(),
                      con.GetReferenceMask(),
                      inter,
                      paddingValue);
}
/* *************************************************************** */
double Compute::GetJacobianPenaltyTerm(bool approx) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    return reg_spline_getJacobianPenaltyTerm(con.GetControlPointGrid(),
                                             con.GetReference(),
                                             approx);
}
/* *************************************************************** */
void Compute::JacobianPenaltyTermGradient(float weight, bool approx) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    reg_spline_getJacobianPenaltyTermGradient(con.GetControlPointGrid(),
                                              con.GetReference(),
                                              con.GetTransformationGradient(),
                                              weight,
                                              approx);
}
/* *************************************************************** */
double Compute::CorrectFolding(bool approx) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    return reg_spline_correctFolding(con.GetControlPointGrid(),
                                     con.GetReference(),
                                     approx);
}
/* *************************************************************** */
double Compute::ApproxBendingEnergy() {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    return reg_spline_approxBendingEnergy(con.GetControlPointGrid());
}
/* *************************************************************** */
void Compute::ApproxBendingEnergyGradient(float weight) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    reg_spline_approxBendingEnergyGradient(con.GetControlPointGrid(),
                                           con.GetTransformationGradient(),
                                           weight);
}
/* *************************************************************** */
double Compute::ApproxLinearEnergy() {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    return reg_spline_approxLinearEnergy(con.GetControlPointGrid());
}
/* *************************************************************** */
void Compute::ApproxLinearEnergyGradient(float weight) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    reg_spline_approxLinearEnergyGradient(con.F3dContent::GetControlPointGrid(),
                                          con.GetTransformationGradient(),
                                          weight);
}
/* *************************************************************** */
double Compute::GetLandmarkDistance(size_t landmarkNumber, float *landmarkReference, float *landmarkFloating) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    return reg_spline_getLandmarkDistance(con.F3dContent::GetControlPointGrid(),
                                          landmarkNumber,
                                          landmarkReference,
                                          landmarkFloating);
}
/* *************************************************************** */
void Compute::LandmarkDistanceGradient(size_t landmarkNumber, float *landmarkReference, float *landmarkFloating, float weight) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    reg_spline_getLandmarkDistanceGradient(con.F3dContent::GetControlPointGrid(),
                                           con.GetTransformationGradient(),
                                           landmarkNumber,
                                           landmarkReference,
                                           landmarkFloating,
                                           weight);
}
/* *************************************************************** */
void Compute::GetDeformationField(bool composition, bool bspline) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    reg_spline_getDeformationField(con.GetControlPointGrid(),
                                   con.GetDeformationField(),
                                   con.GetReferenceMask(),
                                   composition,
                                   bspline);
}
/* *************************************************************** */
void Compute::UpdateControlPointPosition(float *currentDOF, float *bestDOF, float *gradient, float scale, bool optimiseX, bool optimiseY, bool optimiseZ) {
    nifti_image *controlPointGrid = dynamic_cast<F3dContent&>(con).GetControlPointGrid();
    if (optimiseX && optimiseY && optimiseZ) {
        // Update the values for all axis displacement
        for (size_t i = 0; i < controlPointGrid->nvox; ++i)
            currentDOF[i] = bestDOF[i] + scale * gradient[i];
    } else {
        size_t voxNumber = controlPointGrid->nvox / (controlPointGrid->nz > 1 ? 3 : 2);
        // Update the values for the x-axis displacement
        if (optimiseX) {
            for (size_t i = 0; i < voxNumber; ++i)
                currentDOF[i] = bestDOF[i] + scale * gradient[i];
        }
        // Update the values for the y-axis displacement
        if (optimiseY) {
            float *currentDOFY = &currentDOF[voxNumber];
            float *bestDOFY = &bestDOF[voxNumber];
            float *gradientY = &gradient[voxNumber];
            for (size_t i = 0; i < voxNumber; ++i)
                currentDOFY[i] = bestDOFY[i] + scale * gradientY[i];
        }
        // Update the values for the z-axis displacement
        if (optimiseZ && controlPointGrid->nz > 1) {
            float *currentDOFZ = &currentDOF[2 * voxNumber];
            float *bestDOFZ = &bestDOF[2 * voxNumber];
            float *gradientZ = &gradient[2 * voxNumber];
            for (size_t i = 0; i < voxNumber; ++i)
                currentDOFZ[i] = bestDOFZ[i] + scale * gradientZ[i];
        }
    }
}
/* *************************************************************** */
void Compute::GetImageGradient(int interpolation, float paddingValue, int activeTimepoint) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    reg_getImageGradient(con.GetFloating(),
                         con.GetWarpedGradient(),
                         con.GetDeformationField(),
                         con.GetReferenceMask(),
                         interpolation,
                         paddingValue,
                         activeTimepoint);
}
/* *************************************************************** */
double Compute::GetMaximalLength(size_t nodeNumber, bool optimiseX, bool optimiseY, bool optimiseZ) {
    nifti_image *transformationGradient = dynamic_cast<F3dContent&>(con).GetTransformationGradient();
    switch (transformationGradient->datatype) {
    case NIFTI_TYPE_FLOAT32:
        return reg_getMaximalLength<float>(transformationGradient, optimiseX, optimiseY, optimiseZ);
    case NIFTI_TYPE_FLOAT64:
        return reg_getMaximalLength<double>(transformationGradient, optimiseX, optimiseY, optimiseZ);
    }
    return 0;
}
/* *************************************************************** */
void Compute::NormaliseGradient(size_t nodeNumber, double maxGradLength, bool optimiseX, bool optimiseY, bool optimiseZ) {
    // TODO Fix reg_tools_multiplyValueToImage to accept optimiseX, optimiseY, optimiseZ
    nifti_image *transformationGradient = dynamic_cast<F3dContent&>(con).GetTransformationGradient();
    reg_tools_multiplyValueToImage(transformationGradient, transformationGradient, 1 / maxGradLength);
}
/* *************************************************************** */
void Compute::SmoothGradient(float sigma) {
    if (sigma != 0) {
        sigma = fabs(sigma);
        reg_tools_kernelConvolution(dynamic_cast<F3dContent&>(con).GetTransformationGradient(), &sigma, GAUSSIAN_KERNEL);
    }
}
/* *************************************************************** */
template<typename Type>
void Compute::GetApproximatedGradient(InterfaceOptimiser& opt) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    nifti_image *controlPointGrid = con.GetControlPointGrid();
    nifti_image *transformationGradient = con.GetTransformationGradient();

    // Loop over every control point
    Type *gridPtr = static_cast<Type*>(controlPointGrid->data);
    Type *gradPtr = static_cast<Type*>(transformationGradient->data);
    const Type eps = controlPointGrid->dx / Type(100);
    for (size_t i = 0; i < controlPointGrid->nvox; ++i) {
        const Type currentValue = gridPtr[i];
        gridPtr[i] = currentValue + eps;
        // Update the changes for GPU
        con.UpdateControlPointGrid();
        double valPlus = opt.GetObjectiveFunctionValue();
        gridPtr[i] = currentValue - eps;
        // Update the changes for GPU
        con.UpdateControlPointGrid();
        double valMinus = opt.GetObjectiveFunctionValue();
        gridPtr[i] = currentValue;
        gradPtr[i] = -Type((valPlus - valMinus) / (2 * eps));
    }

    // Update the changes for GPU
    con.UpdateControlPointGrid();
    con.UpdateTransformationGradient();
}
/* *************************************************************** */
void Compute::GetApproximatedGradient(InterfaceOptimiser& opt) {
    switch (dynamic_cast<F3dContent&>(con).F3dContent::GetControlPointGrid()->datatype) {
    case NIFTI_TYPE_FLOAT32:
        GetApproximatedGradient<float>(opt);
        break;
    case NIFTI_TYPE_FLOAT64:
        GetApproximatedGradient<double>(opt);
        break;
    }
}
/* *************************************************************** */
void Compute::GetDefFieldFromVelocityGrid(bool updateStepNumber) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    reg_spline_getDefFieldFromVelocityGrid(con.GetControlPointGrid(),
                                           con.GetDeformationField(),
                                           updateStepNumber);
}
/* *************************************************************** */
void Compute::ConvolveImage(nifti_image *image) {
    const nifti_image *controlPointGrid = dynamic_cast<F3dContent&>(con).F3dContent::GetControlPointGrid();
    const int kernelType = CUBIC_SPLINE_KERNEL;
    float currentNodeSpacing[3];
    currentNodeSpacing[0] = currentNodeSpacing[1] = currentNodeSpacing[2] = controlPointGrid->dx;
    bool activeAxis[3] = {1, 0, 0};
    reg_tools_kernelConvolution(image,
                                currentNodeSpacing,
                                kernelType,
                                nullptr, // mask
                                nullptr, // all volumes are considered as active
                                activeAxis);
    // Convolution along the y axis
    currentNodeSpacing[0] = currentNodeSpacing[1] = currentNodeSpacing[2] = controlPointGrid->dy;
    activeAxis[0] = 0;
    activeAxis[1] = 1;
    reg_tools_kernelConvolution(image,
                                currentNodeSpacing,
                                kernelType,
                                nullptr, // mask
                                nullptr, // all volumes are considered as active
                                activeAxis);
    // Convolution along the z axis if required
    if (image->nz > 1) {
        currentNodeSpacing[0] = currentNodeSpacing[1] = currentNodeSpacing[2] = controlPointGrid->dz;
        activeAxis[1] = 0;
        activeAxis[2] = 1;
        reg_tools_kernelConvolution(image,
                                    currentNodeSpacing,
                                    kernelType,
                                    nullptr, // mask
                                    nullptr, // all volumes are considered as active
                                    activeAxis);
    }
}
/* *************************************************************** */
void Compute::ConvolveVoxelBasedMeasureGradient(float weight) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    ConvolveImage(con.GetVoxelBasedMeasureGradient());

    // The node-based NMI gradient is extracted
    mat44 *reorientation = Content::GetIJKMatrix(*con.GetFloating());
    reg_voxelCentric2NodeCentric(con.GetTransformationGradient(),
                                 con.GetVoxelBasedMeasureGradient(),
                                 weight,
                                 false, // no update
                                 reorientation);
}
/* *************************************************************** */
void Compute::ExponentiateGradient(Content& conBwIn) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    F3dContent& conBw = dynamic_cast<F3dContent&>(conBwIn);
    const nifti_image *deformationField = con.Content::GetDeformationField();
    nifti_image *voxelBasedMeasureGradient = con.GetVoxelBasedMeasureGradient();
    nifti_image *controlPointGridBw = conBw.GetControlPointGrid();
    mat44 *affineTransformationBw = conBw.GetTransformationMatrix();
    const size_t compNum = size_t(fabs(controlPointGridBw->intent_p2)); // The number of composition

    /* Allocate a temporary gradient image to store the backward gradient */
    nifti_image *tempGrad = nifti_dup(*voxelBasedMeasureGradient, false);

    // Create all deformation field images needed for resampling
    nifti_image **tempDef = (nifti_image**)malloc((compNum + 1) * sizeof(nifti_image*));
    for (size_t i = 0; i <= compNum; ++i)
        tempDef[i] = nifti_dup(*deformationField, false);

    // Generate all intermediate deformation fields
    reg_spline_getIntermediateDefFieldFromVelGrid(controlPointGridBw, tempDef);

    // Remove the affine component
    nifti_image *affineDisp = nullptr;
    if (affineTransformationBw) {
        affineDisp = nifti_dup(*deformationField, false);
        reg_affine_getDeformationField(affineTransformationBw, affineDisp);
        reg_getDisplacementFromDeformation(affineDisp);
    }

    for (size_t i = 0; i < compNum; ++i) {
        if (affineDisp)
            reg_tools_subtractImageFromImage(tempDef[i], affineDisp, tempDef[i]);
        reg_resampleGradient(voxelBasedMeasureGradient, // floating
                             tempGrad,   // warped - out
                             tempDef[i], // deformation field
                             1,  // interpolation type - linear
                             0); // padding value
        reg_tools_addImageToImage(tempGrad, // in
                                  voxelBasedMeasureGradient,  // in
                                  voxelBasedMeasureGradient); // out
    }

    // Normalise the forward gradient
    reg_tools_divideValueToImage(voxelBasedMeasureGradient, // in
                                 voxelBasedMeasureGradient, // out
                                 pow(2, compNum)); // value

    for (size_t i = 0; i <= compNum; ++i)
        nifti_image_free(tempDef[i]);
    free(tempDef);
    nifti_image_free(tempGrad);
    if (affineDisp)
        nifti_image_free(affineDisp);
}
/* *************************************************************** */
nifti_image* Compute::ScaleGradient(const nifti_image& transformationGradient, float scale) {
    nifti_image *scaledGradient = nifti_dup(transformationGradient, false);
    reg_tools_multiplyValueToImage(&transformationGradient, scaledGradient, scale);
    return scaledGradient;
}
/* *************************************************************** */
void Compute::UpdateVelocityField(float scale, bool optimiseX, bool optimiseY, bool optimiseZ) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    nifti_image *scaledGradient = ScaleGradient(*con.GetTransformationGradient(), scale);
    nifti_image *controlPointGrid = con.GetControlPointGrid();

    // Reset the gradient along the axes if appropriate
    reg_setGradientToZero(scaledGradient, !optimiseX, !optimiseY, !optimiseZ);

    // Update the velocity field
    reg_tools_addImageToImage(controlPointGrid,  // in
                              scaledGradient,    // in
                              controlPointGrid); // out

    nifti_image_free(scaledGradient);
}
/* *************************************************************** */
void Compute::BchUpdate(float scale, int bchUpdateValue) {
    F3dContent& con = dynamic_cast<F3dContent&>(this->con);
    nifti_image *scaledGradient = ScaleGradient(*con.GetTransformationGradient(), scale);
    nifti_image *controlPointGrid = con.GetControlPointGrid();

    compute_BCH_update(controlPointGrid, scaledGradient, bchUpdateValue);

    nifti_image_free(scaledGradient);
}
/* *************************************************************** */
void Compute::SymmetriseVelocityFields(Content& conBwIn) {
    nifti_image *controlPointGrid = dynamic_cast<F3dContent&>(this->con).GetControlPointGrid();
    nifti_image *controlPointGridBw = dynamic_cast<F3dContent&>(conBwIn).GetControlPointGrid();

    // In order to ensure symmetry, the forward and backward velocity fields
    // are averaged in both image spaces: reference and floating
    nifti_image *warpedTrans = nifti_dup(*controlPointGridBw, false);
    nifti_image *warpedTransBw = nifti_dup(*controlPointGrid, false);

    // Both parametrisations are converted into displacement
    reg_getDisplacementFromDeformation(controlPointGrid);
    reg_getDisplacementFromDeformation(controlPointGridBw);

    // Both parametrisations are copied over
    memcpy(warpedTransBw->data, controlPointGridBw->data, warpedTransBw->nvox * warpedTransBw->nbyper);
    memcpy(warpedTrans->data, controlPointGrid->data, warpedTrans->nvox * warpedTrans->nbyper);

    // and subtracted (sum and negation)
    reg_tools_subtractImageFromImage(controlPointGridBw,  // displacement
                                   warpedTrans,         // displacement
                                   controlPointGridBw); // displacement output
    reg_tools_subtractImageFromImage(controlPointGrid,  // displacement
                                   warpedTransBw,     // displacement
                                   controlPointGrid); // displacement output

    // Divide by 2
    reg_tools_multiplyValueToImage(controlPointGridBw, // displacement
                                   controlPointGridBw, // displacement output
                                   0.5f);
    reg_tools_multiplyValueToImage(controlPointGrid, // displacement
                                   controlPointGrid, // displacement output
                                   0.5f);

    // Convert the velocity field from displacement to deformation
    reg_getDeformationFromDisplacement(controlPointGrid);
    reg_getDeformationFromDisplacement(controlPointGridBw);

    nifti_image_free(warpedTrans);
    nifti_image_free(warpedTransBw);
}
/* *************************************************************** */