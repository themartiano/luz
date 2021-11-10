#include <iostream>

void    __checkCudaErrors(cudaError_t result, char const *const func, char const *const file, int const line)
{
    if (result)
    {
        std::cerr << "CUDA error: " <<
            static_cast<unsigned int>(result) << " at " << file << ":" << line << " '" << func << "'" << std::endl;

        // Resets CUDA device
        cudaDeviceReset();
        exit(99);
    }
}
