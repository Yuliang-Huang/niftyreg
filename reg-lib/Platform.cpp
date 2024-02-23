#include "Platform.h"
#include "CpuKernelFactory.h"
#ifdef USE_CUDA
#include "CudaContext.hpp"
#include "CudaF3dContent.h"
#include "CudaComputeFactory.h"
#include "CudaContentCreatorFactory.h"
#include "CudaKernelFactory.h"
#include "CudaMeasureCreatorFactory.hpp"
#include "CudaOptimiser.hpp"
#endif
#ifdef USE_OPENCL
#include "ClContextSingleton.h"
#include "ClComputeFactory.h"
#include "ClContentCreatorFactory.h"
#include "ClKernelFactory.h"
#endif

/* *************************************************************** */
Platform::Platform(const PlatformType platformTypeIn) {
    platformType = platformTypeIn;
    if (platformType == PlatformType::Cpu) {
        platformName = "CPU";
        computeFactory = new ComputeFactory();
        contentCreatorFactory = new ContentCreatorFactory();
        kernelFactory = new CpuKernelFactory();
        measureCreatorFactory = new MeasureCreatorFactory();
    }
#ifdef USE_CUDA
    else if (platformType == PlatformType::Cuda) {
        platformName = "CUDA";
        SetGpuIdx(999);
        computeFactory = new CudaComputeFactory();
        contentCreatorFactory = new CudaContentCreatorFactory();
        kernelFactory = new CudaKernelFactory();
        measureCreatorFactory = new CudaMeasureCreatorFactory();
    }
#endif
#ifdef USE_OPENCL
    else if (platformType == PlatformType::OpenCl) {
        platformName = "OpenCL";
        SetGpuIdx(999);
        computeFactory = new ClComputeFactory();
        contentCreatorFactory = new ClContentCreatorFactory();
        kernelFactory = new ClKernelFactory();
    }
#endif
    else NR_FATAL_ERROR("Unsupported platform type");
}
/* *************************************************************** */
Platform::~Platform() {
    delete computeFactory;
    delete contentCreatorFactory;
    delete kernelFactory;
    delete measureCreatorFactory;
}
/* *************************************************************** */
std::string Platform::GetName() const {
    return platformName;
}
/* *************************************************************** */
PlatformType Platform::GetPlatformType() const {
    return platformType;
}
/* *************************************************************** */
void Platform::SetGpuIdx(unsigned gpuIdxIn) {
    if (platformType == PlatformType::Cpu) {
        gpuIdx = 999;
    }
#ifdef USE_CUDA
    else if (platformType == PlatformType::Cuda) {
        CudaContext& cudaContext = CudaContext::GetInstance();
        if (gpuIdxIn != 999) {
            gpuIdx = gpuIdxIn;
            cudaContext.SetCudaIdx(gpuIdxIn);
        }
    }
#endif
#ifdef USE_OPENCL
    else if (platformType == PlatformType::OpenCl) {
        ClContextSingleton& clContext = ClContextSingleton::GetInstance();
        if (gpuIdxIn != 999) {
            gpuIdx = gpuIdxIn;
            clContext.SetClIdx(gpuIdxIn);
        }

        cl_device_type field;
        clContext.CheckErrNum(clGetDeviceInfo(clContext.GetDeviceId(), CL_DEVICE_TYPE, sizeof(field), &field, nullptr), "Failed to find OpenCL device info");
        if (CL_DEVICE_TYPE_CPU == field)
            NR_FATAL_ERROR("The OpenCL kernels only support GPU devices for now");
    }
#endif
}
/* *************************************************************** */
Compute* Platform::CreateCompute(Content& con) const {
    return computeFactory->Produce(con);
}
/* *************************************************************** */
ContentCreator* Platform::CreateContentCreator(const ContentType conType) const {
    return contentCreatorFactory->Produce(conType);
}
/* *************************************************************** */
Kernel* Platform::CreateKernel(const std::string& name, Content *con) const {
    return kernelFactory->Produce(name, con);
}
/* *************************************************************** */
MeasureCreator* Platform::CreateMeasureCreator() const {
    return measureCreatorFactory->Produce();
}
/* *************************************************************** */
template<typename Type>
Optimiser<Type>* Platform::CreateOptimiser(F3dContent& con,
                                           InterfaceOptimiser& opt,
                                           size_t maxIterationNumber,
                                           bool useConjGradient,
                                           bool optimiseX,
                                           bool optimiseY,
                                           bool optimiseZ,
                                           F3dContent *conBw) const {
    Optimiser<Type> *optimiser;
    nifti_image *controlPointGrid = con.F3dContent::GetControlPointGrid();
    nifti_image *controlPointGridBw = conBw ? conBw->F3dContent::GetControlPointGrid() : nullptr;
    Type *controlPointGridData, *transformationGradientData;
    Type *controlPointGridDataBw = nullptr, *transformationGradientDataBw = nullptr;

    if (platformType == PlatformType::Cpu) {
        optimiser = useConjGradient ? new ConjugateGradient<Type>() : new Optimiser<Type>();
        controlPointGridData = (Type*)controlPointGrid->data;
        transformationGradientData = (Type*)con.GetTransformationGradient()->data;
        if (conBw) {
            controlPointGridDataBw = (Type*)controlPointGridBw->data;
            transformationGradientDataBw = (Type*)conBw->GetTransformationGradient()->data;
        }
    }
#ifdef USE_CUDA
    else if (platformType == PlatformType::Cuda) {
        optimiser = dynamic_cast<Optimiser<Type>*>(useConjGradient ? new CudaConjugateGradient() : new CudaOptimiser());
        controlPointGridData = (Type*)dynamic_cast<CudaF3dContent&>(con).GetControlPointGridCuda();
        transformationGradientData = (Type*)dynamic_cast<CudaF3dContent&>(con).GetTransformationGradientCuda();
        if (conBw) {
            controlPointGridDataBw = (Type*)dynamic_cast<CudaF3dContent*>(conBw)->GetControlPointGridCuda();
            transformationGradientDataBw = (Type*)dynamic_cast<CudaF3dContent*>(conBw)->GetTransformationGradientCuda();
        }
    }
#endif

    optimiser->Initialise(controlPointGrid->nvox,
                          controlPointGrid->nz > 1 ? 3 : 2,
                          optimiseX,
                          optimiseY,
                          optimiseZ,
                          maxIterationNumber,
                          0,
                          &opt,
                          controlPointGridData,
                          transformationGradientData,
                          controlPointGridBw ? controlPointGridBw->nvox : 0,
                          controlPointGridDataBw,
                          transformationGradientDataBw);

    return optimiser;
}
template Optimiser<float>* Platform::CreateOptimiser(F3dContent&, InterfaceOptimiser&, size_t, bool, bool, bool, bool, F3dContent*) const;
template Optimiser<double>* Platform::CreateOptimiser(F3dContent&, InterfaceOptimiser&, size_t, bool, bool, bool, bool, F3dContent*) const;
/* *************************************************************** */
