#include "CUDA/CudaRender.hpp"
#include "CUDA/CheckCudaErrors.hpp"
#include "Renderer.hpp"

void    cudaRender(Scene& scene)
{
    int threadBlockWidth = 8;
    int threadBlockHeight = 8;

    dim3 blocks(scene.getXResolution() / threadBlockWidth + 1, scene.getYResolution() / threadBlockHeight + 1);
    dim3 threads(threadBlockWidth, threadBlockHeight);

    double *frameBuffer = allocateFrameBuffer(scene);

    render<<blocks, threads>>(scene, frameBuffer);

    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    // Writes BMP image file
	BMP bmp("render");
	bmp.writeFile(scene, frameBuffer);

    checkCudaErrors(cudaFree(frameBuffer));
}