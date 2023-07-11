/**
 * @file _reg_base.cpp
 * @author Marc Modat
 * @date 15/11/2012
 *
 *  Copyright (c) 2012-2018, University College London
 *  Copyright (c) 2018, NiftyReg Developers.
 *  All rights reserved.
 * See the LICENSE.txt file in the nifty_reg root folder
 *
 */

#include "_reg_base.h"

/* *************************************************************** */
template<class T>
reg_base<T>::reg_base(int refTimePoint, int floTimePoint) {
    SetPlatformType(PlatformType::Cpu);

    maxIterationNumber = 150;
    optimiseX = true;
    optimiseY = true;
    optimiseZ = true;
    perturbationNumber = 0;
    useConjGradient = true;
    useApproxGradient = false;

    similarityWeight = 0; // automatically set depending of the penalty term weights

    executableName = (char*)"NiftyReg BASE";
    referenceTimePoint = refTimePoint;
    floatingTimePoint = floTimePoint;
    referenceSmoothingSigma = 0;
    floatingSmoothingSigma = 0;

    referenceThresholdLow.reset(new T[referenceTimePoint]);
    std::fill(referenceThresholdLow.get(), referenceThresholdLow.get() + referenceTimePoint, std::numeric_limits<T>::lowest());
    referenceThresholdUp.reset(new T[referenceTimePoint]);
    std::fill(referenceThresholdUp.get(), referenceThresholdUp.get() + referenceTimePoint, std::numeric_limits<T>::max());
    floatingThresholdLow.reset(new T[floatingTimePoint]);
    std::fill(floatingThresholdLow.get(), floatingThresholdLow.get() + floatingTimePoint, std::numeric_limits<T>::lowest());
    floatingThresholdUp.reset(new T[floatingTimePoint]);
    std::fill(floatingThresholdUp.get(), floatingThresholdUp.get() + floatingTimePoint, std::numeric_limits<T>::max());

    robustRange = false;
    warpedPaddingValue = std::numeric_limits<T>::quiet_NaN();
    levelNumber = 3;
    levelToPerform = 0;
    gradientSmoothingSigma = 0;
    verbose = true;
    usePyramid = true;

    initialised = false;

    interpolation = 1;  // linear

    landmarkRegWeight = 0;
    landmarkRegNumber = 0;
    landmarkReference = nullptr;
    landmarkFloating = nullptr;

#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::reg_base");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetReferenceImage(NiftiImage inputReferenceIn) {
    inputReference = inputReferenceIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetReferenceImage");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetFloatingImage(NiftiImage inputFloatingIn) {
    inputFloating = inputFloatingIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetFloatingImage");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetMaximalIterationNumber(unsigned iter) {
    maxIterationNumber = iter;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetMaximalIterationNumber");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetReferenceMask(NiftiImage maskImageIn) {
    maskImage = maskImageIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetReferenceMask");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetAffineTransformation(const mat44& affineTransformationIn) {
    affineTransformation.reset(new mat44(affineTransformationIn));
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetAffineTransformation");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetReferenceSmoothingSigma(T referenceSmoothingSigmaIn) {
    referenceSmoothingSigma = referenceSmoothingSigmaIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetReferenceSmoothingSigma");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetFloatingSmoothingSigma(T floatingSmoothingSigmaIn) {
    floatingSmoothingSigma = floatingSmoothingSigmaIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetFloatingSmoothingSigma");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetReferenceThresholdUp(unsigned i, T t) {
    referenceThresholdUp[i] = t;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetReferenceThresholdUp");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetReferenceThresholdLow(unsigned i, T t) {
    referenceThresholdLow[i] = t;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetReferenceThresholdLow");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetFloatingThresholdUp(unsigned i, T t) {
    floatingThresholdUp[i] = t;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetFloatingThresholdUp");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetFloatingThresholdLow(unsigned i, T t) {
    floatingThresholdLow[i] = t;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetFloatingThresholdLow");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseRobustRange() {
    robustRange = true;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseRobustRange");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::DoNotUseRobustRange() {
    robustRange = false;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseRobustRange");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetWarpedPaddingValue(float warpedPaddingValueIn) {
    warpedPaddingValue = warpedPaddingValueIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetWarpedPaddingValue");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetLevelNumber(unsigned levelNumberIn) {
    levelNumber = levelNumberIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetLevelNumber");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetLevelToPerform(unsigned levelToPerformIn) {
    levelToPerform = levelToPerformIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetLevelToPerform");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetGradientSmoothingSigma(T gradientSmoothingSigmaIn) {
    gradientSmoothingSigma = gradientSmoothingSigmaIn;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetGradientSmoothingSigma");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseConjugateGradient() {
    useConjGradient = true;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseConjugateGradient");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::DoNotUseConjugateGradient() {
    useConjGradient = false;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::DoNotUseConjugateGradient");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseApproximatedGradient() {
    useApproxGradient = true;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseApproximatedGradient");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::DoNotUseApproximatedGradient() {
    useApproxGradient = false;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::DoNotUseApproximatedGradient");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::PrintOutInformation() {
    verbose = true;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::PrintOutInformation");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::DoNotPrintOutInformation() {
    verbose = false;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::DoNotPrintOutInformation");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::DoNotUsePyramidalApproach() {
    usePyramid = false;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::DoNotUsePyramidalApproach");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseNearestNeighborInterpolation() {
    interpolation = 0;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseNearestNeighborInterpolation");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseLinearInterpolation() {
    interpolation = 1;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseLinearInterpolation");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseCubicSplineInterpolation() {
    interpolation = 3;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseCubicSplineInterpolation");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetLandmarkRegularisationParam(size_t n, float *r, float *f, float w) {
    landmarkRegNumber = n;
    landmarkReference = r;
    landmarkFloating = f;
    landmarkRegWeight = w;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetLandmarkRegularisationParam");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::CheckParameters() {
    // CHECK THAT BOTH INPUT IMAGES ARE DEFINED
    if (!inputReference) {
        reg_print_fct_error("reg_base::CheckParameters()");
        reg_print_msg_error("The reference image is not defined");
        reg_exit();
    }
    if (!inputFloating) {
        reg_print_fct_error("reg_base::CheckParameters()");
        reg_print_msg_error("The floating image is not defined");
        reg_exit();
    }

    // CHECK THE MASK DIMENSION IF IT IS DEFINED
    if (maskImage) {
        if (inputReference->nx != maskImage->nx ||
            inputReference->ny != maskImage->ny ||
            inputReference->nz != maskImage->nz) {
            reg_print_fct_error("reg_base::CheckParameters()");
            reg_print_msg_error("The reference and mask images have different dimension");
            reg_exit();
        }
    }

    // CHECK THE NUMBER OF LEVEL TO PERFORM
    if (levelToPerform > 0) {
        levelToPerform = levelToPerform < levelNumber ? levelToPerform : levelNumber;
    } else levelToPerform = levelNumber;
    if (levelToPerform == 0 || levelToPerform > levelNumber)
        levelToPerform = levelNumber;

    // SET THE DEFAULT MEASURE OF SIMILARITY IF NONE HAS BEEN SET
    if (!measure_nmi && !measure_ssd && !measure_dti && !measure_lncc &&
        !measure_kld && !measure_mind && !measure_mindssc) {
        measure_nmi.reset(dynamic_cast<reg_nmi*>(measure->Create(MeasureType::Nmi)));
        for (int i = 0; i < inputReference->nt; ++i)
            measure_nmi->SetTimepointWeight(i, 1.0);
    }

    // CHECK THAT IMAGES HAVE SAME NUMBER OF CHANNELS (TIMEPOINTS)
    // THAT EACH CHANNEL HAS AT LEAST ONE SIMILARITY MEASURE ASSIGNED
    // AND THAT EACH SIMILARITY MEASURE IS USED FOR AT LEAST ONE CHANNEL
    // NORMALISE CHANNEL AND SIMILARITY WEIGHTS SO TOTAL = 1
    //
    // NOTE - DTI currently ignored as needs fixing
    //
    // tests ignored if using MIND or MINDSSC as they are not implemented for multi-channel or weighting
    if (!measure_mind && !measure_mindssc) {
        if (inputFloating->nt != inputReference->nt) {
            reg_print_fct_error("reg_base::CheckParameters()");
            reg_print_msg_error("The reference and floating images have different numbers of channels (timepoints)");
            reg_exit();
        }
        unique_ptr<double[]> chanWeightSum(new double[inputReference->nt]());
        double simWeightSum, totWeightSum = 0.;
        double *nmiWeights = nullptr, *ssdWeights = nullptr, *kldWeights = nullptr, *lnccWeights = nullptr;
        if (measure_nmi) {
            nmiWeights = measure_nmi->GetTimepointsWeights();
            simWeightSum = 0;
            for (int n = 0; n < inputReference->nt; n++) {
                if (nmiWeights[n] < 0) {
                    char text[255];
                    sprintf(text, "The NMI weight for timepoint %d has a negative value - weights must be positive", n);
                    reg_print_fct_error("reg_base::CheckParameters()");
                    reg_print_msg_error(text);
                    reg_exit();
                }
                chanWeightSum[n] += nmiWeights[n];
                simWeightSum += nmiWeights[n];
                totWeightSum += nmiWeights[n];
            }
            if (simWeightSum == 0) {
                reg_print_fct_warn("reg_base::CheckParameters()");
                reg_print_msg_warn("The NMI similarity measure has a weight of 0 for all channels so will be ignored");
            }
        }
        if (measure_ssd) {
            ssdWeights = measure_ssd->GetTimepointsWeights();
            simWeightSum = 0;
            for (int n = 0; n < inputReference->nt; n++) {
                if (ssdWeights[n] < 0) {
                    char text[255];
                    sprintf(text, "The SSD weight for timepoint %d has a negative value - weights must be positive", n);
                    reg_print_fct_error("reg_base::CheckParameters()");
                    reg_print_msg_error(text);
                    reg_exit();
                }
                chanWeightSum[n] += ssdWeights[n];
                simWeightSum += ssdWeights[n];
                totWeightSum += ssdWeights[n];
            }
            if (simWeightSum == 0) {
                reg_print_fct_warn("reg_base::CheckParameters()");
                reg_print_msg_warn("The SSD similarity measure has a weight of 0 for all channels so will be ignored");
            }
        }
        if (measure_kld) {
            kldWeights = measure_kld->GetTimepointsWeights();
            simWeightSum = 0;
            for (int n = 0; n < inputReference->nt; n++) {
                if (kldWeights[n] < 0) {
                    char text[255];
                    sprintf(text, "The KLD weight for timepoint %d has a negative value - weights must be positive", n);
                    reg_print_fct_error("reg_base::CheckParameters()");
                    reg_print_msg_error(text);
                    reg_exit();
                }
                chanWeightSum[n] += kldWeights[n];
                simWeightSum += kldWeights[n];
                totWeightSum += kldWeights[n];
            }
            if (simWeightSum == 0) {
                reg_print_fct_warn("reg_base::CheckParameters()");
                reg_print_msg_warn("The KLD similarity measure has a weight of 0 for all channels so will be ignored");
            }
        }
        if (measure_lncc) {
            lnccWeights = measure_lncc->GetTimepointsWeights();
            simWeightSum = 0;
            for (int n = 0; n < inputReference->nt; n++) {
                if (lnccWeights[n] < 0) {
                    char text[255];
                    sprintf(text, "The LNCC weight for timepoint %d has a negative value - weights must be positive", n);
                    reg_print_fct_error("reg_base::CheckParameters()");
                    reg_print_msg_error(text);
                    reg_exit();
                }
                chanWeightSum[n] += lnccWeights[n];
                simWeightSum += lnccWeights[n];
                totWeightSum += lnccWeights[n];
            }
            if (simWeightSum == 0) {
                reg_print_fct_warn("reg_base::CheckParameters()");
                reg_print_msg_warn("The LNCC similarity measure has a weight of 0 for all channels so will be ignored");
            }
        }
        for (int n = 0; n < inputReference->nt; n++) {
            if (chanWeightSum[n] == 0) {
                char text[255];
                sprintf(text, "Channel %d has a weight of 0 for all similarity measures so will be ignored", n);
                reg_print_fct_warn("reg_base::CheckParameters()");
                reg_print_msg_warn(text);
            }
            if (measure_nmi)
                measure_nmi->SetTimepointWeight(n, nmiWeights[n] / totWeightSum);
            if (measure_ssd)
                measure_ssd->SetTimepointWeight(n, ssdWeights[n] / totWeightSum);
            if (measure_kld)
                measure_kld->SetTimepointWeight(n, kldWeights[n] / totWeightSum);
            if (measure_lncc)
                measure_lncc->SetTimepointWeight(n, lnccWeights[n] / totWeightSum);
        }
    }

#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::CheckParameters");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::InitialiseSimilarity() {
    // TODO Move this function to reg_f3d
    F3dContent& con = dynamic_cast<F3dContent&>(*this->con);

    if (measure_nmi)
        measure->Initialise(*measure_nmi, con);

    if (measure_ssd)
        measure->Initialise(*measure_ssd, con);

    if (measure_kld)
        measure->Initialise(*measure_kld, con);

    if (measure_lncc)
        measure->Initialise(*measure_lncc, con);

    if (measure_dti)
        measure->Initialise(*measure_dti, con);

    if (measure_mind)
        measure->Initialise(*measure_mind, con);

    if (measure_mindssc)
        measure->Initialise(*measure_mindssc, con);

#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::InitialiseSimilarity");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::Initialise() {
    if (initialised) return;

    CheckParameters();

    // CREATE THE PYRAMID IMAGES
    const unsigned imageCount = usePyramid ? levelToPerform : 1;
    referencePyramid = vector<NiftiImage>(imageCount);
    floatingPyramid = vector<NiftiImage>(imageCount);
    maskPyramid = vector<unique_ptr<int[]>>(imageCount);

    // Update the input images threshold if required
    if (robustRange) {
        // Create a copy of the reference image to extract the robust range
        NiftiImage tmpReference = inputReference;
        reg_tools_changeDatatype<T>(tmpReference);
        // Extract the robust range of the reference image
        T *refDataPtr = static_cast<T *>(tmpReference->data);
        reg_heapSort(refDataPtr, tmpReference->nvox);
        // Update the reference threshold values if no value has been setup by the user
        if (referenceThresholdLow[0] == std::numeric_limits<T>::lowest())
            referenceThresholdLow[0] = refDataPtr[(int)reg_round((float)tmpReference->nvox * 0.02f)];
        if (referenceThresholdUp[0] == std::numeric_limits<T>::max())
            referenceThresholdUp[0] = refDataPtr[(int)reg_round((float)tmpReference->nvox * 0.98f)];

        // Create a copy of the floating image to extract the robust range
        NiftiImage tmpFloating = inputFloating;
        reg_tools_changeDatatype<T>(tmpFloating);
        // Extract the robust range of the floating image
        T *floDataPtr = static_cast<T *>(tmpFloating->data);
        reg_heapSort(floDataPtr, tmpFloating->nvox);
        // Update the floating threshold values if no value has been setup by the user
        if (floatingThresholdLow[0] == std::numeric_limits<T>::lowest())
            floatingThresholdLow[0] = floDataPtr[(int)reg_round((float)tmpFloating->nvox * 0.02f)];
        if (floatingThresholdUp[0] == std::numeric_limits<T>::max())
            floatingThresholdUp[0] = floDataPtr[(int)reg_round((float)tmpFloating->nvox * 0.98f)];
    }

    // FINEST LEVEL OF REGISTRATION
    const unsigned levelCount = usePyramid ? levelNumber : 1;
    reg_createImagePyramid<T>(inputReference, referencePyramid, levelCount, imageCount);
    reg_createImagePyramid<T>(inputFloating, floatingPyramid, levelCount, imageCount);
    if (maskImage)
        reg_createMaskPyramid<T>(maskImage, maskPyramid, levelCount, imageCount);
    else
        for (unsigned l = 0; l < imageCount; ++l)
            maskPyramid[l].reset(new int[referencePyramid[l].nVoxelsPerVolume()]());

    // SMOOTH THE INPUT IMAGES IF REQUIRED
    for (unsigned l = 0; l < levelToPerform; l++) {
        if (referenceSmoothingSigma != 0) {
            unique_ptr<bool[]> active(new bool[referencePyramid[l]->nt]);
            unique_ptr<float[]> sigma(new float[referencePyramid[l]->nt]);
            active[0] = true;
            for (int i = 1; i < referencePyramid[l]->nt; ++i)
                active[i] = false;
            sigma[0] = referenceSmoothingSigma;
            reg_tools_kernelConvolution(referencePyramid[l], sigma.get(), GAUSSIAN_KERNEL, nullptr, active.get());
        }
        if (floatingSmoothingSigma != 0) {
            // Only the first image is smoothed
            unique_ptr<bool[]> active(new bool[floatingPyramid[l]->nt]);
            unique_ptr<float[]> sigma(new float[floatingPyramid[l]->nt]);
            active[0] = true;
            for (int i = 1; i < floatingPyramid[l]->nt; ++i)
                active[i] = false;
            sigma[0] = floatingSmoothingSigma;
            reg_tools_kernelConvolution(floatingPyramid[l], sigma.get(), GAUSSIAN_KERNEL, nullptr, active.get());
        }
    }

    // THRESHOLD THE INPUT IMAGES IF REQUIRED
    for (unsigned l = 0; l < imageCount; l++) {
        reg_thresholdImage<T>(referencePyramid[l], referenceThresholdLow[0], referenceThresholdUp[0]);
        reg_thresholdImage<T>(floatingPyramid[l], referenceThresholdLow[0], referenceThresholdUp[0]);
    }

    initialised = true;
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::Initialise");
#endif
}
/* *************************************************************** */
template<class T>
double reg_base<T>::ComputeSimilarityMeasure() {
    double measure = 0;
    if (measure_nmi)
        measure += measure_nmi->GetSimilarityMeasureValue();

    if (measure_ssd)
        measure += measure_ssd->GetSimilarityMeasureValue();

    if (measure_kld)
        measure += measure_kld->GetSimilarityMeasureValue();

    if (measure_lncc)
        measure += measure_lncc->GetSimilarityMeasureValue();

    if (measure_dti)
        measure += measure_dti->GetSimilarityMeasureValue();

    if (measure_mind)
        measure += measure_mind->GetSimilarityMeasureValue();

    if (measure_mindssc)
        measure += measure_mindssc->GetSimilarityMeasureValue();

#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::ComputeSimilarityMeasure");
#endif
    return similarityWeight * measure;
}
/* *************************************************************** */
template<class T>
void reg_base<T>::GetVoxelBasedGradient() {
    // The voxel based gradient image is filled with zeros
    // TODO Temporarily call F3dContent. This function will be moved to reg_f3d
    dynamic_cast<F3dContent&>(*con).ZeroVoxelBasedMeasureGradient();

    // The intensity gradient is first computed
    //   if(measure_nmi || measure_ssd ||
    //         measure_kld || measure_lncc ||
    //         measure_dti)
    //   {
    //    if(measure_dti){
    //        reg_getImageGradient(floating,
    //                             warpedGradient,
    //                             deformationFieldImage,
    //                             currentMask,
    //                             interpolation,
    //                             warpedPaddingValue,
    //                             measure_dti->GetActiveTimepoints(),
    //		 					   forwardJacobianMatrix,
    //							   warped);
    //    }
    //    else{
    //    }
    //   }

    //   if(measure_dti)
    //      measure_dti->GetVoxelBasedSimilarityMeasureGradient();

    for (int t = 0; t < con->Content::GetReference()->nt; ++t) {
        compute->GetImageGradient(interpolation, warpedPaddingValue, t);

        // The gradient of the various measures of similarity are computed
        if (measure_nmi)
            measure_nmi->GetVoxelBasedSimilarityMeasureGradient(t);

        if (measure_ssd)
            measure_ssd->GetVoxelBasedSimilarityMeasureGradient(t);

        if (measure_kld)
            measure_kld->GetVoxelBasedSimilarityMeasureGradient(t);

        if (measure_lncc)
            measure_lncc->GetVoxelBasedSimilarityMeasureGradient(t);

        if (measure_mind)
            measure_mind->GetVoxelBasedSimilarityMeasureGradient(t);

        if (measure_mindssc)
            measure_mindssc->GetVoxelBasedSimilarityMeasureGradient(t);
    }

#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::GetVoxelBasedGradient");
#endif
}
/* *************************************************************** */
//template<class T>
//void reg_base<T>::ApproximateParzenWindow()
//{
//    if(!measure_nmi)
//        measure_nmi.reset(dynamic_cast<reg_nmi*>(measure->Create(MeasureType::Nmi)));
//    measure_nmi=approxParzenWindow = true;
//}
///* *************************************************************** */
//template<class T>
//void reg_base<T>::DoNotApproximateParzenWindow()
//{
//    if(!measure_nmi)
//        measure_nmi.reset(dynamic_cast<reg_nmi*>(measure->Create(MeasureType::Nmi)));
//    measure_nmi=approxParzenWindow = false;
//}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseNMISetReferenceBinNumber(int timepoint, int refBinNumber) {
    if (!measure_nmi)
        measure_nmi.reset(dynamic_cast<reg_nmi*>(measure->Create(MeasureType::Nmi)));
    measure_nmi->SetTimepointWeight(timepoint, 1.0);//weight initially set to default value of 1.0
    // I am here adding 4 to the specified bin number to accommodate for
    // the spline support
    measure_nmi->SetReferenceBinNumber(refBinNumber + 4, timepoint);
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseNMISetReferenceBinNumber");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseNMISetFloatingBinNumber(int timepoint, int floBinNumber) {
    if (!measure_nmi)
        measure_nmi.reset(dynamic_cast<reg_nmi*>(measure->Create(MeasureType::Nmi)));
    measure_nmi->SetTimepointWeight(timepoint, 1.0);//weight initially set to default value of 1.0
    // I am here adding 4 to the specified bin number to accommodate for
    // the spline support
    measure_nmi->SetFloatingBinNumber(floBinNumber + 4, timepoint);
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseNMISetFloatingBinNumber");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseSSD(int timepoint, bool normalise) {
    if (!measure_ssd)
        measure_ssd.reset(dynamic_cast<reg_ssd*>(measure->Create(MeasureType::Ssd)));
    measure_ssd->SetTimepointWeight(timepoint, 1.0);//weight initially set to default value of 1.0
    measure_ssd->SetNormaliseTimepoint(timepoint, normalise);
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseSSD");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseMIND(int timepoint, int offset) {
    if (!measure_mind)
        measure_mind.reset(dynamic_cast<reg_mind*>(measure->Create(MeasureType::Mind)));
    measure_mind->SetTimepointWeight(timepoint, 1.0);//weight set to 1.0 to indicate timepoint is active
    measure_mind->SetDescriptorOffset(offset);
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseMIND");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseMINDSSC(int timepoint, int offset) {
    if (!measure_mindssc)
        measure_mindssc.reset(dynamic_cast<reg_mindssc*>(measure->Create(MeasureType::Mindssc)));
    measure_mindssc->SetTimepointWeight(timepoint, 1.0);//weight set to 1.0 to indicate timepoint is active
    measure_mindssc->SetDescriptorOffset(offset);
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseMINDSSC");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseKLDivergence(int timepoint) {
    if (!measure_kld)
        measure_kld.reset(dynamic_cast<reg_kld*>(measure->Create(MeasureType::Kld)));
    measure_kld->SetTimepointWeight(timepoint, 1.0);//weight initially set to default value of 1.0
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseKLDivergence");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseLNCC(int timepoint, float stddev) {
    if (!measure_lncc)
        measure_lncc.reset(dynamic_cast<reg_lncc*>(measure->Create(MeasureType::Lncc)));
    measure_lncc->SetKernelStandardDeviation(timepoint, stddev);
    measure_lncc->SetTimepointWeight(timepoint, 1.0); // weight initially set to default value of 1.0
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseLNCC");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetLNCCKernelType(int type) {
    if (!measure_lncc) {
        reg_print_fct_error("reg_base<T>::SetLNCCKernelType");
        reg_print_msg_error("The LNCC object has to be created first");
        reg_exit();
    }
    measure_lncc->SetKernelType(type);
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::SetLNCCKernelType");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::UseDTI(bool *timepoint) {
    reg_print_msg_error("The use of DTI has been deactivated as it requires some refactoring");
    reg_exit();

    if (!measure_dti)
        measure_dti.reset(dynamic_cast<reg_dti*>(measure->Create(MeasureType::Dti)));
    for (int i = 0; i < inputReference->nt; ++i) {
        if (timepoint[i])
            measure_dti->SetTimepointWeight(i, 1.0);  // weight set to 1.0 to indicate timepoint is active
    }
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::UseDTI");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetNMIWeight(int timepoint, double weight) {
    if (!measure_nmi) {
        reg_print_fct_error("reg_base<T>::SetNMIWeight");
        reg_print_msg_error("The NMI object has to be created before the timepoint weights can be set");
        reg_exit();
    }
    measure_nmi->SetTimepointWeight(timepoint, weight);
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetLNCCWeight(int timepoint, double weight) {
    if (!measure_lncc) {
        reg_print_fct_error("reg_base<T>::SetLNCCWeight");
        reg_print_msg_error("The LNCC object has to be created before the timepoint weights can be set");
        reg_exit();
    }
    measure_lncc->SetTimepointWeight(timepoint, weight);
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetSSDWeight(int timepoint, double weight) {
    if (!measure_ssd) {
        reg_print_fct_error("reg_base<T>::SetSSDWeight");
        reg_print_msg_error("The SSD object has to be created before the timepoint weights can be set");
        reg_exit();
    }
    measure_ssd->SetTimepointWeight(timepoint, weight);
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetKLDWeight(int timepoint, double weight) {
    if (!measure_kld) {
        reg_print_fct_error("reg_base<T>::SetKLDWeight");
        reg_print_msg_error("The KLD object has to be created before the timepoint weights can be set");
        reg_exit();
    }
    measure_kld->SetTimepointWeight(timepoint, weight);
}
/* *************************************************************** */
template<class T>
void reg_base<T>::SetLocalWeightSim(NiftiImage localWeightSimInputIn) {
    localWeightSimInput = localWeightSimInputIn;
    reg_tools_changeDatatype<T>(localWeightSimInput);
}
/* *************************************************************** */
template<class T>
void reg_base<T>::WarpFloatingImage(int inter) {
    // Compute the deformation field
    GetDeformationField();

    if (!measure_dti) {
        // Resample the floating image
        compute->ResampleImage(inter, warpedPaddingValue);
    } else {
        // reg_defField_getJacobianMatrix(deformationFieldImage, forwardJacobianMatrix);
        /*DTI needs fixing!
       reg_resampleImage(floating,
                          warped,
                          deformationFieldImage,
                          currentMask,
                          inter,
                          warpedPaddingValue,
                          measure_dti->GetActiveTimepoints(),
                          forwardJacobianMatrix);*/
    }
#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::WarpFloatingImage");
#endif
}
/* *************************************************************** */
template<class T>
void reg_base<T>::DeinitCurrentLevel(int currentLevel) {
    optimiser = nullptr;
    if (currentLevel >= 0) {
        if (usePyramid) {
            referencePyramid[currentLevel] = nullptr;
            floatingPyramid[currentLevel] = nullptr;
            maskPyramid[currentLevel] = nullptr;
        } else if (currentLevel == levelToPerform - 1) {
            referencePyramid[0] = nullptr;
            floatingPyramid[0] = nullptr;
            maskPyramid[0] = nullptr;
        }
    }
}
/* *************************************************************** */
template<class T>
void reg_base<T>::Run() {
#ifndef NDEBUG
    char text[255];
    sprintf(text, "%s::Run() called", executableName);
    reg_print_msg_debug(text);
#endif

    Initialise();
#ifdef NDEBUG
    if (verbose) {
#endif
        reg_print_info(executableName, "***********************************************************");
#ifdef NDEBUG
    }
#endif

    // Update the maximal number of iteration to perform per level
    maxIterationNumber = maxIterationNumber * pow(2, levelToPerform - 1);

    // Loop over the different resolution level to perform
    for (int currentLevel = 0; currentLevel < levelToPerform; currentLevel++) {
        // The grid is refined if necessary
        T maxStepSize = InitCurrentLevel(currentLevel);
        T currentSize = maxStepSize;
        T smallestSize = maxStepSize / (T)100.0;

        DisplayCurrentLevelParameters(currentLevel);

        // Initialise the measures of similarity
        InitialiseSimilarity();

        // Initialise the optimiser
        SetOptimiser();

        // Loop over the number of perturbation to do
        for (size_t perturbation = 0; perturbation <= perturbationNumber; ++perturbation) {
            // Evaluate the objective function value
            UpdateBestObjFunctionValue();
            PrintInitialObjFunctionValue();

            // Iterate until convergence or until the max number of iteration is reach
            while (currentSize) {
                if (optimiser->GetCurrentIterationNumber() >= optimiser->GetMaxIterationNumber()) {
                    reg_print_msg_warn("The current level reached the maximum number of iteration");
                    break;
                }

                // Compute the objective function gradient
                GetObjectiveFunctionGradient();

                // Normalise the gradient
                NormaliseGradient();

                // Initialise the line search initial step size
                currentSize = std::min(currentSize, maxStepSize);

                // A line search is performed
                optimiser->Optimise(maxStepSize, smallestSize, currentSize);

                // Update the objective function variables and print some information
                PrintCurrentObjFunctionValue(currentSize);
            }

            if (perturbation < perturbationNumber) {
                optimiser->Perturbation(smallestSize);
                currentSize = maxStepSize;
#ifdef NDEBUG
                if (verbose) {
#endif
                    char text[255];
                    reg_print_info(executableName, "Perturbation Step - The number of iteration is reset to 0");
                    sprintf(text, "Perturbation Step - Every control point positions is altered by [-%g %g]",
                            smallestSize, smallestSize);
                    reg_print_info(executableName, text);

#ifdef NDEBUG
                }
#endif
            }
        } // perturbation loop

        // Final folding correction
        CorrectTransformation();

        // Some cleaning is performed
        DeinitCurrentLevel(currentLevel);

#ifdef NDEBUG
        if (verbose) {
#endif
            reg_print_info(executableName, "Current registration level done");
            reg_print_info(executableName, "***********************************************************");
#ifdef NDEBUG
        }
#endif
        // Update the number of level for the next level
        maxIterationNumber /= 2;
    } // level levelToPerform

#ifndef NDEBUG
    reg_print_fct_debug("reg_base<T>::Run");
#endif
}
/* *************************************************************** */
template class reg_base<float>;
