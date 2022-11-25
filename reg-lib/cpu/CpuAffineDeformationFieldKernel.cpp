#include "CpuAffineDeformationFieldKernel.h"
#include "_reg_globalTrans.h"

/* *************************************************************** */
CpuAffineDeformationFieldKernel::CpuAffineDeformationFieldKernel(Content *conIn) : AffineDeformationFieldKernel() {
    AladinContent *con = static_cast<AladinContent*>(conIn);
    deformationFieldImage = con->GetCurrentDeformationField();
    affineTransformation = con->GetTransformationMatrix();
    mask = con->GetCurrentReferenceMask();
}
/* *************************************************************** */
void CpuAffineDeformationFieldKernel::Calculate(bool compose) {
    reg_affine_getDeformationField(affineTransformation,
                                   deformationFieldImage,
                                   compose,
                                   mask);
}
/* *************************************************************** */
