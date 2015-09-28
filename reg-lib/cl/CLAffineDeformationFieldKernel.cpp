#include "CLAffineDeformationFieldKernel.h"
#include "config.h"

#include "_reg_tools.h"

CLAffineDeformationFieldKernel::CLAffineDeformationFieldKernel(Content *conIn, std::string nameIn) :
        AffineDeformationFieldKernel(nameIn) {
    //populate the CLContent object ptr
    con = static_cast<ClContent*>(conIn);

    //path to kernel files
    const char* niftyreg_install_dir = getenv("NIFTYREG_INSTALL_DIR");
    std::string clInstallPath;
    if(niftyreg_install_dir!=NULL){
        char opencl_kernel_path[255];
        sprintf(opencl_kernel_path, "%s/include/cl/", niftyreg_install_dir);
        clInstallPath = opencl_kernel_path;
    }
    else clInstallPath = CL_KERNELS_PATH;
    std::string clKernel("affineDeformationKernel.cl");

    //get opencl context params
    sContext = &CLContextSingletton::Instance();
    clContext = sContext->getContext();
    commandQueue = sContext->getCommandQueue();
    program = sContext->CreateProgram((clInstallPath + clKernel).c_str());

    //get cpu ptrs
    this->deformationFieldImage = con->Content::getCurrentDeformationField();
    this->affineTransformation = con->Content::getTransformationMatrix();
    this->ReferenceMatrix = (this->deformationFieldImage->sform_code > 0) ? &(this->deformationFieldImage->sto_xyz) : &(this->deformationFieldImage->qto_xyz);

    cl_int errNum;
    // Create OpenCL kernel
    if(this->deformationFieldImage->nz>1)
       kernel = clCreateKernel(program, "affineKernel3D", &errNum);
    else kernel = clCreateKernel(program, "affineKernel2D", &errNum);
    sContext->checkErrNum(errNum, "Error setting kernel CLAffineDeformationFieldKernel.");

    //get cl ptrs
    clDeformationField = con->getDeformationFieldArrayClmem();
    clMask = con->getMaskClmem();

    //set some final kernel args
    errNum = clSetKernelArg(this->kernel, 2, sizeof(cl_mem), &this->clMask);
    sContext->checkErrNum(errNum, "Error setting clMask.");

}
/* *************************************************************** */
void CLAffineDeformationFieldKernel::calculate(bool compose) {
    const unsigned int xThreads = 8;
    const unsigned int yThreads = 8;
    const unsigned int zThreads = 8;

    const unsigned int xBlocks = ((this->deformationFieldImage->nx % xThreads) == 0) ?
             (this->deformationFieldImage->nx / xThreads) : (this->deformationFieldImage->nx / xThreads) + 1;
    const unsigned int yBlocks = ((this->deformationFieldImage->ny % yThreads) == 0) ?
             (this->deformationFieldImage->ny / yThreads) : (this->deformationFieldImage->ny / yThreads) + 1;
    const unsigned int zBlocks = ((this->deformationFieldImage->nz % zThreads) == 0) ?
             (this->deformationFieldImage->nz / zThreads) : (this->deformationFieldImage->nz / zThreads) + 1;
    const cl_uint dims = this->deformationFieldImage->nz>1?3:2;
    size_t globalWorkSize[dims], localWorkSize[dims];
    globalWorkSize[0]=xBlocks * xThreads;
    globalWorkSize[1]=yBlocks * yThreads;
    localWorkSize[0]=xThreads;
    localWorkSize[1]=yThreads;
    if(dims==3){
       globalWorkSize[2]=zBlocks * zThreads;
       localWorkSize[2]=zThreads;
    }

    mat44 transformationMatrix = (compose == true) ?
             *this->affineTransformation : reg_mat44_mul(this->affineTransformation, ReferenceMatrix);

    float* trans = (float *) malloc(16 * sizeof(float));
    mat44ToCptr(transformationMatrix, trans);

    cl_int errNum;

    cl_uint3 pms_d = {{ (cl_uint)this->deformationFieldImage->nx,
                        (cl_uint)this->deformationFieldImage->ny,
                        (cl_uint)this->deformationFieldImage->nz,
                        (cl_uint)0 }};

    cl_mem cltransMat = clCreateBuffer(this->clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                       sizeof(float) * 16, trans, &errNum);
    this->sContext->checkErrNum(errNum,
                                "CLAffineDeformationFieldKernel::calculate failed to allocate memory (cltransMat): ");

    cl_uint composition = compose;
    errNum = clSetKernelArg(this->kernel, 0, sizeof(cl_mem), &cltransMat);
    sContext->checkErrNum(errNum, "Error setting cltransMat.");
    errNum |= clSetKernelArg(this->kernel, 1, sizeof(cl_mem), &this->clDeformationField);
    sContext->checkErrNum(errNum, "Error setting clDeformationField.");
    errNum |= clSetKernelArg(this->kernel, 3, sizeof(cl_uint3), &pms_d);
    sContext->checkErrNum(errNum, "Error setting kernel arguments.");
    errNum |= clSetKernelArg(this->kernel, 4, sizeof(cl_uint), &composition);
    sContext->checkErrNum(errNum, "Error setting kernel arguments.");

    errNum = clEnqueueNDRangeKernel(this->commandQueue, kernel, dims, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
    sContext->checkErrNum(errNum, "Error queuing CLAffineDeformationFieldKernel for execution");
    clFinish(commandQueue);

    free(trans);
    clReleaseMemObject(cltransMat);
    return;
}
/* *************************************************************** */
CLAffineDeformationFieldKernel::~CLAffineDeformationFieldKernel() {
    if (kernel != 0)
        clReleaseKernel(kernel);
    if (program != 0)
        clReleaseProgram(program);
}
