#include "CUDA/CudaUtils.hpp"
#include "CUDA/CheckCudaErrors.hpp"

double* allocateFrameBuffer(Scene& scene)
{
    double* frameBuffer;

    int bufferSize = 3 * (scene.getXResolution() * scene.getYResolution()) * sizeof(double);

    checkCudaErrors(cudaMallocManaged((void **)&frameBuffer, bufferSize));
}
