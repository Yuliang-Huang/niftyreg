#include "_reg_aladin.h"

/* *************************************************************** */
template<class T>
reg_aladin<T>::reg_aladin() {
    this->executableName = (char*)"Aladin";

    this->transformationMatrix = new mat44;
    this->inputTransformName = nullptr;

    this->blockMatchingParams = nullptr;

    this->verbose = true;

    this->maxIterations = 5;

    this->numberOfLevels = 3;
    this->levelsToPerform = 3;

    this->performRigid = 1;
    this->performAffine = 1;

    this->blockStepSize = 1;
    this->blockPercentage = 50;
    this->inlierLts = 50;

    this->alignCentre = 1;
    this->alignCentreMass = 0;

    this->interpolation = 1;    // linear

    this->floatingSigma = 0;
    this->referenceSigma = 0;

    this->referenceUpperThreshold = std::numeric_limits<T>::max();
    this->referenceLowerThreshold = std::numeric_limits<T>::min();

    this->floatingUpperThreshold = std::numeric_limits<T>::max();
    this->floatingLowerThreshold = std::numeric_limits<T>::min();

    this->warpedPaddingValue = std::numeric_limits<T>::quiet_NaN();

    this->funcProgressCallback = nullptr;
    this->paramsProgressCallback = nullptr;

    this->platformType = PlatformType::Cpu;
    this->currentLevel = 0;
    this->gpuIdx = 999;

#ifndef NDEBUG
    reg_print_msg_debug("reg_aladin constructor called");
#endif
}
/* *************************************************************** */
template<class T>
reg_aladin<T>::~reg_aladin() {
    if (this->transformationMatrix)
        delete this->transformationMatrix;

#ifndef NDEBUG
    reg_print_msg_debug("reg_aladin destructor called");
#endif
}
/* *************************************************************** */
template<class T>
bool reg_aladin<T>::TestMatrixConvergence(mat44 *mat) {
    bool convergence = true;
    if ((fabsf(mat->m[0][0]) - 1.0f) > CONVERGENCE_EPS)
        convergence = false;
    if ((fabsf(mat->m[1][1]) - 1.0f) > CONVERGENCE_EPS)
        convergence = false;
    if ((fabsf(mat->m[2][2]) - 1.0f) > CONVERGENCE_EPS)
        convergence = false;

    if ((fabsf(mat->m[0][1]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;
    if ((fabsf(mat->m[0][2]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;
    if ((fabsf(mat->m[0][3]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;

    if ((fabsf(mat->m[1][0]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;
    if ((fabsf(mat->m[1][2]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;
    if ((fabsf(mat->m[1][3]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;

    if ((fabsf(mat->m[2][0]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;
    if ((fabsf(mat->m[2][1]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;
    if ((fabsf(mat->m[2][3]) - 0.0f) > CONVERGENCE_EPS)
        convergence = false;

    return convergence;
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::SetVerbose(bool _verbose) {
    this->verbose = _verbose;
}
/* *************************************************************** */
template<class T>
int reg_aladin<T>::Check() {
    //This does all the initial checking
    if (!this->inputReference) {
        reg_print_fct_error("reg_aladin<T>::Check()");
        reg_print_msg_error("No reference image has been specified or it can not be read");
        return EXIT_FAILURE;
    }

    if (!this->inputFloating) {
        reg_print_fct_error("reg_aladin<T>::Check()");
        reg_print_msg_error("No floating image has been specified or it can not be read");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
/* *************************************************************** */
template<class T>
int reg_aladin<T>::Print() {
    if (!this->inputReference) {
        reg_print_fct_error("reg_aladin<T>::Print()");
        reg_print_msg_error("No reference image has been specified");
        return EXIT_FAILURE;
    }
    if (!this->inputFloating) {
        reg_print_fct_error("reg_aladin<T>::Print()");
        reg_print_msg_error("No floating image has been specified");
        return EXIT_FAILURE;
    }

    /* *********************************** */
    /* DISPLAY THE REGISTRATION PARAMETERS */
    /* *********************************** */
#ifdef NDEBUG
    if (this->verbose) {
#endif
        std::string text;
        reg_print_info(this->executableName, "Parameters");
        text = stringFormat("Platform: %s", this->platform->GetName().c_str());
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("Reference image name: %s", this->inputReference->fname);
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("\t%ix%ix%i voxels", this->inputReference->nx, this->inputReference->ny, this->inputReference->nz);
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("\t%gx%gx%g mm", this->inputReference->dx, this->inputReference->dy, this->inputReference->dz);
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("Floating image name: %s", this->inputFloating->fname);
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("\t%ix%ix%i voxels", this->inputFloating->nx, this->inputFloating->ny, this->inputFloating->nz);
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("\t%gx%gx%g mm", this->inputFloating->dx, this->inputFloating->dy, this->inputFloating->dz);
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("Maximum iteration number: %i", this->maxIterations);
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("\t(%i during the first level)", 2 * this->maxIterations);
        reg_print_info(this->executableName, text.c_str());
        text = stringFormat("Percentage of blocks: %i %%", this->blockPercentage);
        reg_print_info(this->executableName, text.c_str());
        reg_print_info(this->executableName, "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
#ifdef NDEBUG
    }
#endif
    return EXIT_SUCCESS;
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::SetInputTransform(const char *filename) {
    this->inputTransformName = (char*)filename;
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::InitialiseRegistration() {
#ifndef NDEBUG
    reg_print_fct_debug("reg_aladin::InitialiseRegistration()");
#endif

    this->platform.reset(new Platform(this->platformType));
    this->platform->SetGpuIdx(this->gpuIdx);

    this->Print();

    // CREATE THE PYRAMID IMAGES
    this->referencePyramid = vector<NiftiImage>(this->levelsToPerform);
    this->floatingPyramid = vector<NiftiImage>(this->levelsToPerform);
    this->referenceMaskPyramid = vector<unique_ptr<int[]>>(this->levelsToPerform);

    // FINEST LEVEL OF REGISTRATION
    reg_createImagePyramid<T>(this->inputReference,
                              this->referencePyramid,
                              this->numberOfLevels,
                              this->levelsToPerform);
    reg_createImagePyramid<T>(this->inputFloating,
                              this->floatingPyramid,
                              this->numberOfLevels,
                              this->levelsToPerform);

    if (this->inputReferenceMask)
        reg_createMaskPyramid<T>(this->inputReferenceMask,
                                 this->referenceMaskPyramid,
                                 this->numberOfLevels,
                                 this->levelsToPerform);
    else
        for (unsigned l = 0; l < this->levelsToPerform; ++l)
            this->referenceMaskPyramid[l].reset(new int[this->referencePyramid[l].nVoxelsPerVolume()]());

    unique_ptr<Kernel> convolutionKernel(this->platform->CreateKernel(ConvolutionKernel::GetName(), nullptr));
    // SMOOTH THE INPUT IMAGES IF REQUIRED
    for (unsigned l = 0; l < this->levelsToPerform; l++) {
        if (this->referenceSigma != 0) {
            // Only the first image is smoothed
            unique_ptr<bool[]> active(new bool[this->referencePyramid[l]->nt]);
            unique_ptr<float[]> sigma(new float[this->referencePyramid[l]->nt]);
            active[0] = true;
            for (int i = 1; i < this->referencePyramid[l]->nt; ++i)
                active[i] = false;
            sigma[0] = this->referenceSigma;
            convolutionKernel->castTo<ConvolutionKernel>()->Calculate(this->referencePyramid[l], sigma.get(), 0, nullptr, active.get());
        }
        if (this->floatingSigma != 0) {
            // Only the first image is smoothed
            unique_ptr<bool[]> active(new bool[this->floatingPyramid[l]->nt]);
            unique_ptr<float[]> sigma(new float[this->floatingPyramid[l]->nt]);
            active[0] = true;
            for (int i = 1; i < this->floatingPyramid[l]->nt; ++i)
                active[i] = false;
            sigma[0] = this->floatingSigma;
            convolutionKernel->castTo<ConvolutionKernel>()->Calculate(this->floatingPyramid[l], sigma.get(), 0, nullptr, active.get());
        }
    }

    // THRESHOLD THE INPUT IMAGES IF REQUIRED
    for (unsigned l = 0; l < this->levelsToPerform; l++) {
        reg_thresholdImage<T>(this->referencePyramid[l], this->referenceLowerThreshold, this->referenceUpperThreshold);
        reg_thresholdImage<T>(this->floatingPyramid[l], this->floatingLowerThreshold, this->floatingUpperThreshold);
    }

    // Initialise the transformation
    if (this->inputTransformName != nullptr) {
        if (FILE *aff = fopen(this->inputTransformName, "r")) {
            fclose(aff);
        } else {
            std::string text = stringFormat("The specified input affine file (%s) can not be read", this->inputTransformName);
            reg_print_fct_error("reg_aladin<T>::InitialiseRegistration()");
            reg_print_msg_error(text.c_str());
            reg_exit();
        }
        reg_tool_ReadAffineFile(this->transformationMatrix, this->inputTransformName);
    } else { // No input affine transformation
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                this->transformationMatrix->m[i][j] = 0;
            }
            this->transformationMatrix->m[i][i] = 1;
        }
        if (this->alignCentre && this->alignCentreMass == 0) {
            const mat44 *floatingMatrix = (this->inputFloating->sform_code > 0) ? &(this->inputFloating->sto_xyz) : &(this->inputFloating->qto_xyz);
            const mat44 *referenceMatrix = (this->inputReference->sform_code > 0) ? &(this->inputReference->sto_xyz) : &(this->inputReference->qto_xyz);
            //In pixel coordinates
            float floatingCenter[3];
            floatingCenter[0] = (float)(this->inputFloating->nx) / 2.0f;
            floatingCenter[1] = (float)(this->inputFloating->ny) / 2.0f;
            floatingCenter[2] = (float)(this->inputFloating->nz) / 2.0f;
            float referenceCenter[3];
            referenceCenter[0] = (float)(this->inputReference->nx) / 2.0f;
            referenceCenter[1] = (float)(this->inputReference->ny) / 2.0f;
            referenceCenter[2] = (float)(this->inputReference->nz) / 2.0f;
            //From pixel coordinates to real coordinates
            float floatingRealPosition[3];
            reg_mat44_mul(floatingMatrix, floatingCenter, floatingRealPosition);
            float referenceRealPosition[3];
            reg_mat44_mul(referenceMatrix, referenceCenter, referenceRealPosition);
            //Set translation to the transformation matrix
            this->transformationMatrix->m[0][3] = floatingRealPosition[0] - referenceRealPosition[0];
            this->transformationMatrix->m[1][3] = floatingRealPosition[1] - referenceRealPosition[1];
            this->transformationMatrix->m[2][3] = floatingRealPosition[2] - referenceRealPosition[2];
        } else if (this->alignCentreMass == 2) {
            float referenceCentre[3] = { 0, 0, 0 };
            float referenceCount = 0;
            reg_tools_changeDatatype<float>(this->inputReference);
            float *refPtr = static_cast<float *>(this->inputReference->data);
            size_t refIndex = 0;
            for (int z = 0; z < this->inputReference->nz; ++z) {
                for (int y = 0; y < this->inputReference->ny; ++y) {
                    for (int x = 0; x < this->inputReference->nx; ++x) {
                        float value = refPtr[refIndex];
                        referenceCentre[0] += (float)x * value;
                        referenceCentre[1] += (float)y * value;
                        referenceCentre[2] += (float)z * value;
                        referenceCount += value;
                        refIndex++;
                    }
                }
            }
            referenceCentre[0] /= referenceCount;
            referenceCentre[1] /= referenceCount;
            referenceCentre[2] /= referenceCount;
            float refCOM[3];
            if (this->inputReference->sform_code > 0)
                reg_mat44_mul(&(this->inputReference->sto_xyz), referenceCentre, refCOM);

            float floatingCentre[3] = { 0, 0, 0 };
            float floatingCount = 0;
            reg_tools_changeDatatype<float>(this->inputFloating);
            float *floPtr = static_cast<float *>(this->inputFloating->data);
            size_t floIndex = 0;
            for (int z = 0; z < this->inputFloating->nz; ++z) {
                for (int y = 0; y < this->inputFloating->ny; ++y) {
                    for (int x = 0; x < this->inputFloating->nx; ++x) {
                        float value = floPtr[floIndex];
                        floatingCentre[0] += (float)x * value;
                        floatingCentre[1] += (float)y * value;
                        floatingCentre[2] += (float)z * value;
                        floatingCount += value;
                        floIndex++;
                    }
                }
            }
            floatingCentre[0] /= floatingCount;
            floatingCentre[1] /= floatingCount;
            floatingCentre[2] /= floatingCount;
            float floCOM[3];
            if (this->inputFloating->sform_code > 0)
                reg_mat44_mul(&(this->inputFloating->sto_xyz), floatingCentre, floCOM);
            reg_mat44_eye(this->transformationMatrix);
            this->transformationMatrix->m[0][3] = floCOM[0] - refCOM[0];
            this->transformationMatrix->m[1][3] = floCOM[1] - refCOM[1];
            this->transformationMatrix->m[2][3] = floCOM[2] - refCOM[2];
        }
    }
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::DeallocateCurrentInputImage() {
    this->referencePyramid[this->currentLevel] = nullptr;
    this->floatingPyramid[this->currentLevel] = nullptr;
    this->referenceMaskPyramid[this->currentLevel] = nullptr;
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::CreateKernels() {
    this->affineTransformation3DKernel.reset(platform->CreateKernel(AffineDeformationFieldKernel::GetName(), this->con.get()));
    this->resamplingKernel.reset(platform->CreateKernel(ResampleImageKernel::GetName(), this->con.get()));
    if (this->blockMatchingParams) {
        this->blockMatchingKernel.reset(platform->CreateKernel(BlockMatchingKernel::GetName(), this->con.get()));
        this->optimiseKernel.reset(platform->CreateKernel(OptimiseKernel::GetName(), this->con.get()));
    } else {
        this->blockMatchingKernel = nullptr;
        this->optimiseKernel = nullptr;
    }
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::DeallocateKernels() {
    this->affineTransformation3DKernel = nullptr;
    this->resamplingKernel = nullptr;
    this->blockMatchingKernel = nullptr;
    this->optimiseKernel = nullptr;
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::GetDeformationField() {
    this->affineTransformation3DKernel->template castTo<AffineDeformationFieldKernel>()->Calculate();
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::GetWarpedImage(int interp, float padding) {
    this->GetDeformationField();
    this->resamplingKernel->template castTo<ResampleImageKernel>()->Calculate(interp, padding);
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::UpdateTransformationMatrix(int type) {
    this->blockMatchingKernel->template castTo<BlockMatchingKernel>()->Calculate();
    this->optimiseKernel->template castTo<OptimiseKernel>()->Calculate(type);

#ifndef NDEBUG
    reg_mat44_disp(this->transformationMatrix, (char *)"[NiftyReg DEBUG] updated forward matrix");
#endif
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::InitAladinContent(nifti_image *ref,
                                      nifti_image *flo,
                                      int *mask,
                                      mat44 *transMat,
                                      size_t bytes,
                                      unsigned blockPercentage,
                                      unsigned inlierLts,
                                      unsigned blockStepSize) {
    unique_ptr<AladinContentCreator> contentCreator{ dynamic_cast<AladinContentCreator*>(this->platform->CreateContentCreator(ContentType::Aladin)) };
    this->con.reset(contentCreator->Create(ref, flo, mask, transMat, bytes, blockPercentage, inlierLts, blockStepSize));
    this->blockMatchingParams = this->con->AladinContent::GetBlockMatchingParams();
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::DeinitAladinContent() {
    this->con = nullptr;
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::ResolveMatrix(unsigned iterations, const unsigned optimizationFlag) {
    unsigned iteration = 0;
    while (iteration < iterations) {
#ifndef NDEBUG
        char text[255];
        sprintf(text, "%s - level: %i/%i - iteration %i/%i",
                optimizationFlag ? (char *)"Affine" : (char *)"Rigid",
                this->currentLevel + 1, this->numberOfLevels, iteration + 1, iterations);
        reg_print_msg_debug(text);
#endif
        this->GetWarpedImage(this->interpolation, this->warpedPaddingValue);
        this->UpdateTransformationMatrix(optimizationFlag);

        iteration++;
    }
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::Run() {
    this->InitialiseRegistration();

    //Main loop over the levels:
    for (this->currentLevel = 0; this->currentLevel < this->levelsToPerform; this->currentLevel++) {
        this->InitAladinContent(this->referencePyramid[currentLevel], this->floatingPyramid[currentLevel],
                                this->referenceMaskPyramid[currentLevel].get(), this->transformationMatrix, sizeof(T),
                                this->blockPercentage, this->inlierLts, this->blockStepSize);
        this->CreateKernels();

        // Twice more iterations are performed during the first level
        // All the blocks are used during the first level
        const unsigned maxNumberOfIterationToPerform = (currentLevel == 0) ? this->maxIterations * 2 : this->maxIterations;

#ifdef NDEBUG
        if (this->verbose) {
#endif
            this->DebugPrintLevelInfoStart();
#ifdef NDEBUG
        }
#endif

#ifndef NDEBUG
        if (this->con->GetReference()->sform_code > 0)
            reg_mat44_disp(&this->con->GetReference()->sto_xyz, (char *)"[NiftyReg DEBUG] Reference image matrix (sform sto_xyz)");
        else
            reg_mat44_disp(&this->con->GetReference()->qto_xyz, (char *)"[NiftyReg DEBUG] Reference image matrix (qform qto_xyz)");
        if (this->con->GetFloating()->sform_code > 0)
            reg_mat44_disp(&this->con->GetFloating()->sto_xyz, (char *)"[NiftyReg DEBUG] Floating image matrix (sform sto_xyz)");
        else
            reg_mat44_disp(&this->con->GetFloating()->qto_xyz, (char *)"[NiftyReg DEBUG] Floating image matrix (qform qto_xyz)");
#endif

        /* ****************** */
        /* Rigid registration */
        /* ****************** */
        if ((this->performRigid && !this->performAffine) || (this->performAffine && this->performRigid && this->currentLevel == 0)) {
            const unsigned ratio = (this->performAffine && this->performRigid && this->currentLevel == 0) ? 4 : 1;
            ResolveMatrix(maxNumberOfIterationToPerform * ratio, RIGID);
        }

        /* ******************* */
        /* Affine registration */
        /* ******************* */
        if (this->performAffine)
            ResolveMatrix(maxNumberOfIterationToPerform, AFFINE);

        // SOME CLEANING IS PERFORMED
        this->DeallocateKernels();
        this->DeinitAladinContent();
        this->DeallocateCurrentInputImage();

#ifdef NDEBUG
        if (this->verbose) {
#endif
            this->DebugPrintLevelInfoEnd();
            reg_print_info(this->executableName, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
#ifdef NDEBUG
        }
#endif

    }

#ifndef NDEBUG
    reg_print_msg_debug("reg_aladin::Run() done");
#endif
}
/* *************************************************************** */
template<class T>
NiftiImage reg_aladin<T>::GetFinalWarpedImage() {
    // The initial images are used
    if (!this->inputReference || !this->inputFloating || !this->transformationMatrix) {
        reg_print_fct_error("reg_aladin::GetFinalWarpedImage()");
        reg_print_msg_error("The reference, floating images and the transformation have to be defined");
        reg_exit();
    }

    unique_ptr<int[]> mask(new int[this->inputReference.nVoxelsPerVolume()]());

    reg_aladin<T>::InitAladinContent(this->inputReference,
                                     this->inputFloating,
                                     mask.get(),
                                     this->transformationMatrix,
                                     sizeof(T));
    reg_aladin<T>::CreateKernels();

    reg_aladin<T>::GetWarpedImage(3, this->warpedPaddingValue); // cubic spline interpolation

    NiftiImage warpedImage(this->con->GetWarped(), true);
    warpedImage->cal_min = this->inputFloating->cal_min;
    warpedImage->cal_max = this->inputFloating->cal_max;
    warpedImage->scl_slope = this->inputFloating->scl_slope;
    warpedImage->scl_inter = this->inputFloating->scl_inter;

    reg_aladin<T>::DeallocateKernels();
    reg_aladin<T>::DeinitAladinContent();
    return warpedImage;
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::DebugPrintLevelInfoStart() {
    /* Display some parameters specific to the current level */
    char text[255];
    sprintf(text, "Current level %i / %i", this->currentLevel + 1, this->numberOfLevels);
    reg_print_info(this->executableName, text);
    sprintf(text, "reference image size: \t%ix%ix%i voxels\t%gx%gx%g mm",
            this->con->GetReference()->nx,
            this->con->GetReference()->ny,
            this->con->GetReference()->nz,
            this->con->GetReference()->dx,
            this->con->GetReference()->dy,
            this->con->GetReference()->dz);
    reg_print_info(this->executableName, text);
    sprintf(text, "floating image size: \t%ix%ix%i voxels\t%gx%gx%g mm",
            this->con->GetFloating()->nx,
            this->con->GetFloating()->ny,
            this->con->GetFloating()->nz,
            this->con->GetFloating()->dx,
            this->con->GetFloating()->dy,
            this->con->GetFloating()->dz);
    reg_print_info(this->executableName, text);
    if (this->con->GetReference()->nz == 1) {
        reg_print_info(this->executableName, "Block size = [4 4 1]");
    } else reg_print_info(this->executableName, "Block size = [4 4 4]");
    reg_print_info(this->executableName, "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
    sprintf(text, "Block number = [%i %i %i]", this->blockMatchingParams->blockNumber[0],
            this->blockMatchingParams->blockNumber[1], this->blockMatchingParams->blockNumber[2]);
    reg_print_info(this->executableName, text);
    reg_mat44_disp(this->transformationMatrix, (char *)"[reg_aladin] Initial transformation matrix:");
}
/* *************************************************************** */
template<class T>
void reg_aladin<T>::DebugPrintLevelInfoEnd() {
    reg_mat44_disp(this->transformationMatrix, (char *)"[reg_aladin] Final transformation matrix:");
}
/* *************************************************************** */
template class reg_aladin<float>;
