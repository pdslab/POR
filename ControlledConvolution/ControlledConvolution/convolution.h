#ifndef CONVOLUTION_H
#define CONVOLUTION_H
#include "Filter.h"

class Convolution
{
public:
	// 1D convolution /////////////////////////////////////////////////////////////
	// We assume Sample and kernel signal start from t=0. (The first element of
	// kernel and Sample signal is at t=0)
	// it returns false if parameters are not valid.
	bool Convolve1D(float* in, float* out, int size, float* kernel, int kernelSize);
	// 2D convolution (No Optimization) ///////////////////////////////////////////
	// Simplest 2D convolution routine. It is easy to understand how convolution
	// works, but is very slow, because of no optimization.
	///////////////////////////////////////////////////////////////////////////////
	bool Convolve2DSlow(unsigned char* in, unsigned char* out, int sizeX, int sizeY, float* kernel, int kSizeX, int kSizeY);
	// 2D convolution /////////////////////////////////////////////////////////////
	// 2D data are usually stored as contiguous 1D array in computer memory.
	// So, we are using 1D array for 2D data.
	// 2D convolution assumes the kernel is center originated, which means, if
	// kernel size 3 then, k[-1], k[0], k[1]. The middle of index is always 0.
	// The following programming logics are somewhat complicated because of using
	// pointer indexing in order to minimize the number of multiplications.
	// It returns false if parameters are not valid.
	///////////////////////////////////////////////////////////////////////////////
	bool Convolve2D(unsigned char* in, unsigned char* out, int sizeX, int sizeY, float* kernel, int kSizeX, int kSizeY);
	bool Convolve2D(unsigned short* in, unsigned short* out, int sizeX, int sizeY, float* kernel, int kSizeX, int kSizeY);
	bool Convolve2D(int* in, int* out, int sizeX, int sizeY, float* kernel, int kSizeX, int kSizeY);
	bool Convolve2D(float* in, float* out, int sizeX, int sizeY, float* kernel, int kSizeX, int kSizeY);
	bool Convolve2D(double* in, double* out, int sizeX, int sizeY, double* kernel, int kSizeX, int kSizeY);
	// 2D separable convolution ///////////////////////////////////////////////////
	// If the MxN kernel can be separable to (Mx1) and (1xN) matrices, the
	// multiplication can be reduced to M+N compared to MxN in normal convolution.
	// It does not check the output is exceeded max for performance reason. And we
	// assume the kernel contains good(valid) data, therefore, the result cannot be
	// larger than max.
	// It returns false if parameters are not valid.
	///////////////////////////////////////////////////////////////////////////////
	bool Convolve2DSeparable(unsigned char* in, unsigned char* out, int sizeX, int sizeY, 
		float* xKernel, int kSizeX, float* yKernel, int kSizeY);
	bool Convolve2DSeparable(unsigned short* in, unsigned short* out, int sizeX, int sizeY, 
		float* xKernel, int kSizeX, float* yKernel, int kSizeY);
	bool Convolve2DSeparable(int* in, int* out, int sizeX, int sizeY, float* xKernel, int kSizeX, 
		float* yKernel, int kSizeY);
	bool Convolve2DSeparable(float* in, float* out, int size_x, int size_y, float* x_kernel, 
		int k_size_x, float* y_kernel, int k_size_y);
	bool Convolve2DSeparable(double* in, double* out, int size_x, int size_y, double* x_kernel, 
		int k_size_x, double* y_kernel, int k_size_y);
	//Signature for controlled convolution 
	bool ControlledConvolve2DSeparable(double* in, double* out, int size_x, int size_y, 
		double* x_kernel, int k_size_x, double* y_kernel, int k_size_y);
	// 2D convolution Fast ////////////////////////////////////////////////////////
	// In order to improve the performance, this function uses multple cursors of
	// Sample signal. It avoids indexing Sample array during convolution. And, the
	// Sample signal is partitioned to 9 different sections, so we don't need to
	// check the boundary for every samples.
	///////////////////////////////////////////////////////////////////////////////
	bool Convolve2DFast(unsigned char* in, unsigned char* out, int size_x, int size_y, float* kernel, 
		int k_size_x, int k_size_y);
	bool Convolve2DFast2(unsigned char* in, unsigned char* out, int size_x, int size_y, int* kernel, 
		float factor, int k_size_x, int k_size_y);
	//Composition filter 
	bool OrderByComposition(double* in, double* out, int size_x, int size_y, double* x_kernel, int k_size_x, 
		double* y_kernel, int k_size_y);
	bool Convolve2DSeparable(double* in, double* out, int dataSizeX, int dataSizeY,
		double* kernelX, int kSizeX, float* kernelY, int kSizeY);
};
#endif
