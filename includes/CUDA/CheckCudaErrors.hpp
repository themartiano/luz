#ifndef CHECKCUDAERRORS_HPP
#define CHECKCUDAERRORS_HPP

#define checkCudaErrors(val) __checkCudaErrors((val), #val, __FILE__, __LINE__)

#endif