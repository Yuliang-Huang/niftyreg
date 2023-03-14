// Enable testing
#define NR_TESTING

#include <random>
#include <catch2/catch_test_macros.hpp>
#include "_reg_localTrans.h"
#include "Platform.h"
#include "ResampleImageKernel.h"
#include "AffineDeformationFieldKernel.h"


template <typename T>
void InterpCubicSplineKernel(T relative, T (&basis)[4]) {
    if (relative < 0) relative = 0; //reg_rounding error
    const T relative2 = relative * relative;
    basis[0] = (relative * ((2.f - relative) * relative - 1.f)) / 2.f;
    basis[1] = (relative2 * (3.f * relative - 5.f) + 2.f) / 2.f;
    basis[2] = (relative * ((4.f - 3.f * relative) * relative + 1.f)) / 2.f;
    basis[3] = (relative - 1.f) * relative2 / 2.f;
}

template <typename T>
void InterpCubicSplineKernel(T relative, T (&basis)[4], T (&derivative)[4]) {
    InterpCubicSplineKernel(relative, basis);
    if (relative < 0) relative = 0; //reg_rounding error
    const T relative2 = relative * relative;
    derivative[0] = (4.f * relative - 3.f * relative2 - 1.f) / 2.f;
    derivative[1] = (9.f * relative - 10.f) * relative / 2.f;
    derivative[2] = (8.f * relative - 9.f * relative2 + 1.f) / 2.f;
    derivative[3] = (3.f * relative - 2.f) * relative / 2.f;
}

NiftiImage CreateControlPointGrid(const NiftiImage& reference) {
    // Set the spacing for the control point grid
    float spacingInMillimetre[3] = { reference->dx, reference->dy, reference->dz };

    // Define the spacing for the first level
    float gridSpacing[3];
    gridSpacing[0] = spacingInMillimetre[0];
    gridSpacing[1] = spacingInMillimetre[1];
    gridSpacing[2] = 1;
    if (reference->nz > 1)
        gridSpacing[2] = spacingInMillimetre[2];

    // Create and allocate the control point image
    NiftiImage controlPointGrid;
    reg_createControlPointGrid<float>(controlPointGrid, reference, gridSpacing);

    // The control point position image is initialised with the affine transformation
    reg_getDeformationFromDisplacement(controlPointGrid);

    return controlPointGrid;
}