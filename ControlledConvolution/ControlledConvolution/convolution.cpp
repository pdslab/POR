#include "stdafx.h"
#include <cmath>
#include "convolution.h"

///////////////////////////////////////////////////////////////////////////////
// 1D convolution
// We assume Sample and kernel signal start from t=0.
///////////////////////////////////////////////////////////////////////////////
bool Convolution::Convolve1D(float* in, float* out, const int data_size, float* kernel, const int kernel_size)
{
	int i, j, k;

	// check validity of params
	if (!in || !out || !kernel) return false;
	if (data_size <= 0 || kernel_size <= 0) return false;

	// start convolution from out[kernelSize-1] to out[dataSize-1] (last)
	for (i = kernel_size - 1; i < data_size; ++i)
	{
		out[i] = 0;                             // init to 0 before accumulate

		for (j = i, k = 0; k < kernel_size; --j, ++k)
			out[i] += in[j] * kernel[k];
	}

	// convolution from out[0] to out[kernelSize-2]
	for (i = 0; i < kernel_size - 1; ++i)
	{
		out[i] = 0;                             // init to 0 before sum

		for (j = i, k = 0; j >= 0; --j, ++k)
			out[i] += in[j] * kernel[k];
	}

	return true;
}

// Simplest 2D convolution routine. It is easy to understand how convolution
// works, but is very slow, because of no optimization.
bool Convolution::Convolve2DSlow(unsigned char* in, unsigned char* out, const int data_size_x, const int data_size_y,
	float* kernel, const int kernel_size_x, const int kernel_size_y)
{
	// check validity of params
	if (!in || !out || !kernel) return false;
	if (data_size_x <= 0 || kernel_size_x <= 0) return false;

	// find center position of kernel (half of kernel size)
	const auto kCenterX = kernel_size_x / 2;
	const auto kCenterY = kernel_size_y / 2;

	for (auto i = 0; i < data_size_y; ++i)                // rows
	{
		for (auto j = 0; j < data_size_x; ++j)            // columns
		{
			float sum = 0;                            // init to 0 before sum
			for (auto m = 0; m < kernel_size_y; ++m)      // kernel rows
			{
				const auto mm = kernel_size_y - 1 - m;       // row index of flipped kernel

				for (auto n = 0; n < kernel_size_x; ++n)  // kernel columns
				{
					const auto nn = kernel_size_x - 1 - n;   // column index of flipped kernel

					// index of Sample signal, used for checking boundary
					const auto rowIndex = i + m - kCenterY;
					const auto colIndex = j + n - kCenterX;

					// ignore Sample samples which are out of bound
					if (rowIndex >= 0 && rowIndex < data_size_y && colIndex >= 0 && colIndex < data_size_x)
						sum += in[data_size_x * rowIndex + colIndex] * kernel[kernel_size_x * mm + nn];
				}
			}
			out[data_size_x * i + j] = static_cast<unsigned char>(static_cast<float>(fabs(sum)) + 0.5f);
		}
	}

	return true;
}

// 2D convolution
// 2D data are usually stored in computer memory as contiguous 1D array.
// So, we are using 1D array for 2D data.
// 2D convolution assumes the kernel is center originated, which means, if
// kernel size 3 then, k[-1], k[0], k[1]. The middle of index is always 0.
// The following programming logics are somewhat complicated because of using
// pointer indexing in order to minimize the number of multiplications.

// unsigned char version (8bit): Note that the output is always positive number
bool Convolution::Convolve2D(unsigned char* in, unsigned char* out, const int data_size_x, const int data_size_y,
	float* kernel, const int kernel_size_x, const int kernel_size_y)
{
	unsigned char *inPtr2;

	// check validity of params
	if (!in || !out || !kernel) return false;
	if (data_size_x <= 0 || kernel_size_x <= 0) return false;

	// find center position of kernel (half of kernel size)
	const auto k_center_x = kernel_size_x >> 1;
	const auto k_center_y = kernel_size_y >> 1;

	// init working  pointers
	auto in_ptr = inPtr2 = &in[data_size_x * k_center_y + k_center_x];  // note that  it is shifted (kCenterX, kCenterY),
	auto outPtr = out;
	auto kPtr = kernel;

	// start convolution
	for (int i = 0; i < data_size_y; ++i)                   // number of rows
	{
		// compute the range of convolution, the current row of kernel should be between these
		int rowMax = i + k_center_y;
		int rowMin = i - data_size_y + k_center_y;

		for (int j = 0; j < data_size_x; ++j)              // number of columns
		{
			// compute the range of convolution, the current column of kernel should be between these
			int colMax = j + k_center_x;
			int colMin = j - data_size_x + k_center_x;

			float sum = 0;                                // set to 0 before accumulate

			// flip the kernel and traverse all the kernel values
			// multiply each kernel value with underlying Sample data
			for (int m = 0; m < kernel_size_y; ++m)        // kernel rows
			{
				// check if the index is out of bound of Sample array
				if (m <= rowMax && m > rowMin)
				{
					for (auto n = 0; n < kernel_size_x; ++n)
					{
						// check the boundary of array
						if (n <= colMax && n > colMin)
							sum += *(in_ptr - n) * *kPtr;

						++kPtr;                     // next kernel
					}
				}
				else
					kPtr += kernel_size_x;            // out of bound, move to next row of kernel

				in_ptr -= data_size_x;                 // move Sample data 1 raw up
			}

			// convert negative number to positive
			*outPtr = static_cast<unsigned char>(static_cast<float>(fabs(sum)) + 0.5f);

			kPtr = kernel;                          // reset kernel to (0,0)
			in_ptr = ++inPtr2;                       // next Sample
			++outPtr;                               // next output
		}
	}

	return true;
}

// unsigned short (16bit)
bool Convolution::Convolve2D(unsigned short* in, unsigned short* out, const int data_size_x, const int data_size_y,
	float* kernel, const int kernel_size_x, const int kernel_size_y)
{
	unsigned short *in_ptr2;

	// check validity of params
	if (!in || !out || !kernel) return false;
	if (data_size_x <= 0 || kernel_size_x <= 0) return false;

	// find center position of kernel (half of kernel size)
	const int k_center_x = kernel_size_x >> 1;
	const auto k_center_y = kernel_size_y >> 1;

	// init working  pointers
	auto in_ptr = in_ptr2 = &in[data_size_x * k_center_y + k_center_x];  // note that  it is shifted (kCenterX, kCenterY),
	auto out_ptr = out;
	auto k_ptr = kernel;

	// start convolution
	for (auto i = 0; i < data_size_y; ++i)                   // number of rows
	{
		// compute the range of convolution, the current row of kernel should be between these
		const auto row_max = i + k_center_y;
		const auto row_min = i - data_size_y + k_center_y;

		for (auto j = 0; j < data_size_x; ++j)              // number of columns
		{
			// compute the range of convolution, the current column of kernel should be between these
			int colMax = j + k_center_x;
			int colMin = j - data_size_x + k_center_x;

			float sum = 0;                                // set to 0 before accumulate

			// flip the kernel and traverse all the kernel values
			// multiply each kernel value with underlying Sample data
			for (int m = 0; m < kernel_size_y; ++m)        // kernel rows
			{
				// check if the index is out of bound of Sample array
				if (m <= row_max && m > row_min)
				{
					for (int n = 0; n < kernel_size_x; ++n)
					{
						// check the boundary of array
						if (n <= colMax && n > colMin)
							sum += *(in_ptr - n) * *k_ptr;

						++k_ptr;                     // next kernel
					}
				}
				else
					k_ptr += kernel_size_x;            // out of bound, move to next row of kernel

				in_ptr -= data_size_x;                 // move Sample data 1 raw up
			}

			// convert negative number to positive
			*out_ptr = static_cast<unsigned short>(static_cast<float>(fabs(sum)) + 0.5f);

			k_ptr = kernel;                          // reset kernel to (0,0)
			in_ptr = ++in_ptr2;                       // next Sample
			++out_ptr;                               // next output
		}
	}

	return true;
}

// signed integer (32bit) version:
bool Convolution::Convolve2D(int* in, int* out, const int dataSizeX, const int dataSizeY,
	float* kernel, const int kernelSizeX, const int kernelSizeY)
{
	int *inPtr2;

	// check validity of params
	if (!in || !out || !kernel) return false;
	if (dataSizeX <= 0 || kernelSizeX <= 0) return false;

	// find center position of kernel (half of kernel size)
	const auto kCenterX = kernelSizeX >> 1;
	const auto kCenterY = kernelSizeY >> 1;

	// init working  pointers
	auto inPtr = inPtr2 = &in[dataSizeX * kCenterY + kCenterX];  // note that  it is shifted (kCenterX, kCenterY),
	auto outPtr = out;
	auto kPtr = kernel;

	// start convolution
	for (auto i = 0; i < dataSizeY; ++i)                   // number of rows
	{
		// compute the range of convolution, the current row of kernel should be between these
		const auto rowMax = i + kCenterY;
		const auto rowMin = i - dataSizeY + kCenterY;

		for (auto j = 0; j < dataSizeX; ++j)              // number of columns
		{
			// compute the range of convolution, the current column of kernel should be between these
			const auto colMax = j + kCenterX;
			const auto colMin = j - dataSizeX + kCenterX;

			float sum = 0;                                // set to 0 before accumulate

			// flip the kernel and traverse all the kernel values
			// multiply each kernel value with underlying Sample data
			for (auto m = 0; m < kernelSizeY; ++m)        // kernel rows
			{
				// check if the index is out of bound of Sample array
				if (m <= rowMax && m > rowMin)
				{
					for (auto n = 0; n < kernelSizeX; ++n)
					{
						// check the boundary of array
						if (n <= colMax && n > colMin)
							sum += *(inPtr - n) * *kPtr;

						++kPtr;                     // next kernel
					}
				}
				else
					kPtr += kernelSizeX;            // out of bound, move to next row of kernel

				inPtr -= dataSizeX;                 // move Sample data 1 raw up
			}

			// convert integer number
			if (sum >= 0) *outPtr = static_cast<int>(sum + 0.5f);
			else *outPtr = static_cast<int>(sum - 0.5f);

			kPtr = kernel;                          // reset kernel to (0,0)
			inPtr = ++inPtr2;                       // next Sample
			++outPtr;                               // next output
		}
	}

	return true;
}

// single float precision version:
bool Convolution::Convolve2D(float* in, float* out, const int dataSizeX, const int dataSizeY,
	float* kernel, const int kernelSizeX, const int kernelSizeY)
{
	float *inPtr2;

	// check validity of params
	if (!in || !out || !kernel) return false;
	if (dataSizeX <= 0 || kernelSizeX <= 0) return false;

	// find center position of kernel (half of kernel size)
	const auto kCenterX = kernelSizeX >> 1;
	const auto kCenterY = kernelSizeY >> 1;

	// init working  pointers
	auto inPtr = inPtr2 = &in[dataSizeX * kCenterY + kCenterX];  // note that  it is shifted (kCenterX, kCenterY),
	auto outPtr = out;
	auto kPtr = kernel;

	// start convolution
	for (auto i = 0; i < dataSizeY; ++i)                   // number of rows
	{
		// compute the range of convolution, the current row of kernel should be between these
		const auto rowMax = i + kCenterY;
		const auto rowMin = i - dataSizeY + kCenterY;

		for (auto j = 0; j < dataSizeX; ++j)              // number of columns
		{
			// compute the range of convolution, the current column of kernel should be between these
			const auto colMax = j + kCenterX;
			const auto colMin = j - dataSizeX + kCenterX;

			*outPtr = 0;                            // set to 0 before accumulate

			// flip the kernel and traverse all the kernel values
			// multiply each kernel value with underlying Sample data
			for (auto m = 0; m < kernelSizeY; ++m)        // kernel rows
			{
				// check if the index is out of bound of Sample array
				if (m <= rowMax && m > rowMin)
				{
					for (auto n = 0; n < kernelSizeX; ++n)
					{
						// check the boundary of array
						if (n <= colMax && n > colMin)
							*outPtr += *(inPtr - n) * *kPtr;
						++kPtr;                     // next kernel
					}
				}
				else
					kPtr += kernelSizeX;            // out of bound, move to next row of kernel

				inPtr -= dataSizeX;                 // move Sample data 1 raw up
			}

			kPtr = kernel;                          // reset kernel to (0,0)
			inPtr = ++inPtr2;                       // next Sample
			++outPtr;                               // next output
		}
	}

	return true;
}

// double float precision version:
bool Convolution::Convolve2D(double* in, double* out, const int dataSizeX, const int dataSizeY,
	double* kernel, const int kernelSizeX, const int kernelSizeY)
{
	double *inPtr2;

	// check validity of params
	if (!in || !out || !kernel) return false;
	if (dataSizeX <= 0 || kernelSizeX <= 0) return false;

	// find center position of kernel (half of kernel size)
	const auto kCenterX = kernelSizeX >> 1;
	const auto kCenterY = kernelSizeY >> 1;

	// init working  pointers
	auto inPtr = inPtr2 = &in[dataSizeX * kCenterY + kCenterX];  // note that  it is shifted (kCenterX, kCenterY),
	auto outPtr = out;
	auto kPtr = kernel;

	// start convolution
	for (auto i = 0; i < dataSizeY; ++i)                   // number of rows
	{
		// compute the range of convolution, the current row of kernel should be between these
		const auto rowMax = i + kCenterY;
		const auto rowMin = i - dataSizeY + kCenterY;

		for (auto j = 0; j < dataSizeX; ++j)              // number of columns
		{
			// compute the range of convolution, the current column of kernel should be between these
			const auto colMax = j + kCenterX;
			const auto colMin = j - dataSizeX + kCenterX;

			*outPtr = 0;                            // set to 0 before accumulate

			// flip the kernel and traverse all the kernel values
			// multiply each kernel value with underlying Sample data
			for (int m = 0; m < kernelSizeY; ++m)        // kernel rows
			{
				// check if the index is out of bound of Sample array
				if (m <= rowMax && m > rowMin)
				{
					for (int n = 0; n < kernelSizeX; ++n)
					{
						// check the boundary of array
						if (n <= colMax && n > colMin)
							*outPtr += *(inPtr - n) * *kPtr;
						++kPtr;                     // next kernel
					}
				}
				else
					kPtr += kernelSizeX;            // out of bound, move to next row of kernel

				inPtr -= dataSizeX;                 // move Sample data 1 raw up
			}

			kPtr = kernel;                          // reset kernel to (0,0)
			inPtr = ++inPtr2;                       // next Sample
			++outPtr;                               // next output
		}
	}

	return true;
}

// Separable 2D Convolution
// If the MxN kernel can be separable to (Mx1) and (1xN) matrices, the
// multiplication can be reduced to M+N comapred to MxN in normal convolution.
// It does not check the output is excceded max for performance reason. And we
// assume the kernel contains good(valid) data, therefore, the result cannot be
// larger than max.

// unsigned char (8-bit) version
bool Convolution::Convolve2DSeparable(unsigned char* in, unsigned char* out, int dataSizeX, int dataSizeY,
	float* kernelX, int kSizeX, float* kernelY, int kSizeY)
{
	int i, j, k, m, n;
	float*tmpPtr2;                        // working pointers
	int kOffset;                 // kernel indice

	// check validity of params
	if (!in || !out || !kernelX || !kernelY) return false;
	if (dataSizeX <= 0 || kSizeX <= 0) return false;

	// allocate temp storage to keep intermediate result
	float *tmp = new float[dataSizeX * dataSizeY];
	if (!tmp) return false;  // memory allocation error

	// store accumulated sum
	float *sum = new float[dataSizeX];
	if (!sum) return false;  // memory allocation error

	// covolve horizontal direction ///////////////////////

	// find center position of kernel (half of kernel size)
	int kCenter = kSizeX >> 1;                          // center index of kernel array
	int endIndex = dataSizeX - kCenter;                 // index for full kernel convolution

	// init working pointers
	unsigned char *inPtr = in;
	float *tmpPtr = tmp;                                   // store intermediate results from 1D horizontal convolution

	// start horizontal convolution (x-direction)
	for (i = 0; i < dataSizeY; ++i)                    // number of rows
	{
		kOffset = 0;                                // starting index of partial kernel varies for each Sample

		// COLUMN FROM index=0 TO index=kCenter-1
		for (j = 0; j < kCenter; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kCenter + kOffset, m = 0; k >= 0; --k, ++m) // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++tmpPtr;                               // next output
			++kOffset;                              // increase starting index of kernel
		}

		// COLUMN FROM index=kCenter TO index=(dataSizeX-kCenter-1)
		for (j = kCenter; j < endIndex; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulate

			for (k = kSizeX - 1, m = 0; k >= 0; --k, ++m)  // full kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
		}

		kOffset = 1;                                // ending index of partial kernel varies for each Sample

		// COLUMN FROM index=(dataSizeX-kCenter) TO index=(dataSizeX-1)
		for (j = endIndex; j < dataSizeX; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kSizeX - 1, m = 0; k >= kOffset; --k, ++m)   // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
			++kOffset;                              // increase ending index of partial kernel
		}

		inPtr += kCenter;                           // next row
	}
	// END OF HORIZONTAL CONVOLUTION //////////////////////

	// start vertical direction ///////////////////////////

	// find center position of kernel (half of kernel size)
	kCenter = kSizeY >> 1;                          // center index of vertical kernel
	endIndex = dataSizeY - kCenter;                 // index where full kernel convolution should stop

	// set working pointers
	tmpPtr = tmpPtr2 = tmp;
	unsigned char *outPtr = out;

	// clear out array before accumulation
	for (i = 0; i < dataSizeX; ++i)
		sum[i] = 0;

	// start to convolve vertical direction (y-direction)

	// ROW FROM index=0 TO index=(kCenter-1)
	kOffset = 0;                                    // starting index of partial kernel varies for each Sample
	for (i = 0; i < kCenter; ++i)
	{
		for (k = kCenter + kOffset; k >= 0; --k)     // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			// covert negative to positive
			*outPtr = (unsigned char)((float)fabs(sum[n]) + 0.5f);
			sum[n] = 0;                             // reset to zero for next summing
			++outPtr;                               // next element of output
		}

		tmpPtr = tmpPtr2;                           // reset Sample pointer
		++kOffset;                                  // increase starting index of kernel
	}

	// ROW FROM index=kCenter TO index=(dataSizeY-kCenter-1)
	for (i = kCenter; i < endIndex; ++i)
	{
		for (k = kSizeY - 1; k >= 0; --k)             // convolve with full kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			// covert negative to positive
			*outPtr = (unsigned char)((float)fabs(sum[n]) + 0.5f);
			sum[n] = 0;                             // reset for next summing
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;
	}

	// ROW FROM index=(dataSizeY-kCenter) TO index=(dataSizeY-1)
	kOffset = 1;                                    // ending index of partial kernel varies for each Sample
	for (i = endIndex; i < dataSizeY; ++i)
	{
		for (k = kSizeY - 1; k >= kOffset; --k)        // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			// covert negative to positive
			*outPtr = (unsigned char)((float)fabs(sum[n]) + 0.5f);
			sum[n] = 0;                             // reset for next summing
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;                           // next Sample
		++kOffset;                                  // increase ending index of kernel
	}
	// END OF VERTICAL CONVOLUTION ////////////////////////

	// deallocate temp buffers
	delete[] tmp;
	delete[] sum;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// unsigned short (16-bit) version
///////////////////////////////////////////////////////////////////////////////
bool Convolution::Convolve2DSeparable(unsigned short* in, unsigned short* out, int dataSizeX, int dataSizeY,
	float* kernelX, int kSizeX, float* kernelY, int kSizeY)
{
	int i, j, k, m, n;
	float*tmpPtr2;                        // working pointers
	int kOffset;                 // kernel indice

	// check validity of params
	if (!in || !out || !kernelX || !kernelY) return false;
	if (dataSizeX <= 0 || kSizeX <= 0) return false;

	// allocate temp storage to keep intermediate result
	float *tmp = new float[dataSizeX * dataSizeY];
	if (!tmp) return false;  // memory allocation error

	// store accumulated sum
	float *sum = new float[dataSizeX];
	if (!sum) return false;  // memory allocation error

	// covolve horizontal direction ///////////////////////

	// find center position of kernel (half of kernel size)
	int kCenter = kSizeX >> 1;                          // center index of kernel array
	int endIndex = dataSizeX - kCenter;                 // index for full kernel convolution

	// init working pointers
	unsigned short *inPtr = in;
	float *tmpPtr = tmp;                                   // store intermediate results from 1D horizontal convolution

	// start horizontal convolution (x-direction)
	for (i = 0; i < dataSizeY; ++i)                    // number of rows
	{
		kOffset = 0;                                // starting index of partial kernel varies for each Sample

		// COLUMN FROM index=0 TO index=kCenter-1
		for (j = 0; j < kCenter; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kCenter + kOffset, m = 0; k >= 0; --k, ++m) // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++tmpPtr;                               // next output
			++kOffset;                              // increase starting index of kernel
		}

		// COLUMN FROM index=kCenter TO index=(dataSizeX-kCenter-1)
		for (j = kCenter; j < endIndex; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulate

			for (k = kSizeX - 1, m = 0; k >= 0; --k, ++m)  // full kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
		}

		kOffset = 1;                                // ending index of partial kernel varies for each Sample

		// COLUMN FROM index=(dataSizeX-kCenter) TO index=(dataSizeX-1)
		for (j = endIndex; j < dataSizeX; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kSizeX - 1, m = 0; k >= kOffset; --k, ++m)   // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
			++kOffset;                              // increase ending index of partial kernel
		}

		inPtr += kCenter;                           // next row
	}
	// END OF HORIZONTAL CONVOLUTION //////////////////////

	// start vertical direction ///////////////////////////

	// find center position of kernel (half of kernel size)
	kCenter = kSizeY >> 1;                          // center index of vertical kernel
	endIndex = dataSizeY - kCenter;                 // index where full kernel convolution should stop

	// set working pointers
	tmpPtr = tmpPtr2 = tmp;
	unsigned short *outPtr = out;

	// clear out array before accumulation
	for (i = 0; i < dataSizeX; ++i)
		sum[i] = 0;

	// start to convolve vertical direction (y-direction)

	// ROW FROM index=0 TO index=(kCenter-1)
	kOffset = 0;                                    // starting index of partial kernel varies for each Sample
	for (i = 0; i < kCenter; ++i)
	{
		for (k = kCenter + kOffset; k >= 0; --k)     // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			// covert negative to positive
			*outPtr = (unsigned short)((float)fabs(sum[n]) + 0.5f);
			sum[n] = 0;                             // reset to zero for next summing
			++outPtr;                               // next element of output
		}

		tmpPtr = tmpPtr2;                           // reset Sample pointer
		++kOffset;                                  // increase starting index of kernel
	}

	// ROW FROM index=kCenter TO index=(dataSizeY-kCenter-1)
	for (i = kCenter; i < endIndex; ++i)
	{
		for (k = kSizeY - 1; k >= 0; --k)             // convolve with full kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			// covert negative to positive
			*outPtr = (unsigned short)((float)fabs(sum[n]) + 0.5f);
			sum[n] = 0;                             // reset before next summing
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;
	}

	// ROW FROM index=(dataSizeY-kCenter) TO index=(dataSizeY-1)
	kOffset = 1;                                    // ending index of partial kernel varies for each Sample
	for (i = endIndex; i < dataSizeY; ++i)
	{
		for (k = kSizeY - 1; k >= kOffset; --k)        // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			// covert negative to positive
			*outPtr = (unsigned short)((float)fabs(sum[n]) + 0.5f);
			sum[n] = 0;                             // reset before next summing
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;                           // next Sample
		++kOffset;                                  // increase ending index of kernel
	}
	// END OF VERTICAL CONVOLUTION ////////////////////////

	// deallocate temp buffers
	delete[] tmp;
	delete[] sum;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// integer (32-bit) version
///////////////////////////////////////////////////////////////////////////////
bool Convolution::Convolve2DSeparable(int* in, int* out, int dataSizeX, int dataSizeY,
	float* kernelX, int kSizeX, float* kernelY, int kSizeY)
{
	int i, j, k, m, n;
	float*tmpPtr2;                        // working pointers
	int kOffset;                 // kernel indice

	// check validity of params
	if (!in || !out || !kernelX || !kernelY) return false;
	if (dataSizeX <= 0 || kSizeX <= 0) return false;

	// allocate temp storage to keep intermediate result
	float *tmp = new float[dataSizeX * dataSizeY];
	if (!tmp) return false;  // memory allocation error

	// store accumulated sum
	float *sum = new float[dataSizeX];
	if (!sum) return false;  // memory allocation error

	// covolve horizontal direction ///////////////////////

	// find center position of kernel (half of kernel size)
	int kCenter = kSizeX >> 1;                          // center index of kernel array
	int endIndex = dataSizeX - kCenter;                 // index for full kernel convolution

	// init working pointers
	int *inPtr = in;
	float *tmpPtr = tmp;                                   // store intermediate results from 1D horizontal convolution

	// start horizontal convolution (x-direction)
	for (i = 0; i < dataSizeY; ++i)                    // number of rows
	{
		kOffset = 0;                                // starting index of partial kernel varies for each Sample

		// COLUMN FROM index=0 TO index=kCenter-1
		for (j = 0; j < kCenter; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kCenter + kOffset, m = 0; k >= 0; --k, ++m) // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++tmpPtr;                               // next output
			++kOffset;                              // increase starting index of kernel
		}

		// COLUMN FROM index=kCenter TO index=(dataSizeX-kCenter-1)
		for (j = kCenter; j < endIndex; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulate

			for (k = kSizeX - 1, m = 0; k >= 0; --k, ++m)  // full kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
		}

		kOffset = 1;                                // ending index of partial kernel varies for each Sample

		// COLUMN FROM index=(dataSizeX-kCenter) TO index=(dataSizeX-1)
		for (j = endIndex; j < dataSizeX; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kSizeX - 1, m = 0; k >= kOffset; --k, ++m)   // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
			++kOffset;                              // increase ending index of partial kernel
		}

		inPtr += kCenter;                           // next row
	}
	// END OF HORIZONTAL CONVOLUTION //////////////////////

	// start vertical direction ///////////////////////////

	// find center position of kernel (half of kernel size)
	kCenter = kSizeY >> 1;                          // center index of vertical kernel
	endIndex = dataSizeY - kCenter;                 // index where full kernel convolution should stop

	// set working pointers
	tmpPtr = tmpPtr2 = tmp;
	int *outPtr = out;

	// clear out array before accumulation
	for (i = 0; i < dataSizeX; ++i)
		sum[i] = 0;

	// start to convolve vertical direction (y-direction)

	// ROW FROM index=0 TO index=(kCenter-1)
	kOffset = 0;                                    // starting index of partial kernel varies for each Sample
	for (i = 0; i < kCenter; ++i)
	{
		for (k = kCenter + kOffset; k >= 0; --k)     // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			if (sum[n] >= 0)
				*outPtr = (int)(sum[n] + 0.5f);     // store final result to output array
			else
				*outPtr = (int)(sum[n] - 0.5f);     // store final result to output array

			sum[n] = 0;                             // reset to zero for next summing
			++outPtr;                               // next element of output
		}

		tmpPtr = tmpPtr2;                           // reset Sample pointer
		++kOffset;                                  // increase starting index of kernel
	}

	// ROW FROM index=kCenter TO index=(dataSizeY-kCenter-1)
	for (i = kCenter; i < endIndex; ++i)
	{
		for (k = kSizeY - 1; k >= 0; --k)             // convolve with full kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			if (sum[n] >= 0)
				*outPtr = (int)(sum[n] + 0.5f);     // store final result to output array
			else
				*outPtr = (int)(sum[n] - 0.5f);     // store final result to output array
			sum[n] = 0;                             // reset to 0 before next summing
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;
	}

	// ROW FROM index=(dataSizeY-kCenter) TO index=(dataSizeY-1)
	kOffset = 1;                                    // ending index of partial kernel varies for each Sample
	for (i = endIndex; i < dataSizeY; ++i)
	{
		for (k = kSizeY - 1; k >= kOffset; --k)        // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			if (sum[n] >= 0)
				*outPtr = (int)(sum[n] + 0.5f);     // store final result to output array
			else
				*outPtr = (int)(sum[n] - 0.5f);     // store final result to output array
			sum[n] = 0;                             // reset before next summing
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;                           // next Sample
		++kOffset;                                  // increase ending index of kernel
	}
	// END OF VERTICAL CONVOLUTION ////////////////////////

	// deallocate temp buffers
	delete[] tmp;
	delete[] sum;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// single precision float version
///////////////////////////////////////////////////////////////////////////////
bool Convolution::Convolve2DSeparable(float* in, float* out, int dataSizeX, int dataSizeY,
	float* kernelX, int kSizeX, float* kernelY, int kSizeY)
{
	int i, j, k, m, n;
	float*tmpPtr2;                        // working pointers
	int kOffset;                 // kernel indice

	// check validity of params
	if (!in || !out || !kernelX || !kernelY) return false;
	if (dataSizeX <= 0 || kSizeX <= 0) return false;

	// allocate temp storage to keep intermediate result
	float *tmp = new float[dataSizeX * dataSizeY];
	if (!tmp) return false;  // memory allocation error

	// store accumulated sum
	float *sum = new float[dataSizeX];
	if (!sum) return false;  // memory allocation error

	// covolve horizontal direction ///////////////////////

	// find center position of kernel (half of kernel size)
	int kCenter = kSizeX >> 1;                          // center index of kernel array
	int endIndex = dataSizeX - kCenter;                 // index for full kernel convolution

	// init working pointers
	float *inPtr = in;
	float *tmpPtr = tmp;                                   // store intermediate results from 1D horizontal convolution

	// start horizontal convolution (x-direction)
	for (i = 0; i < dataSizeY; ++i)                    // number of rows
	{
		kOffset = 0;                                // starting index of partial kernel varies for each Sample

		// COLUMN FROM index=0 TO index=kCenter-1
		for (j = 0; j < kCenter; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kCenter + kOffset, m = 0; k >= 0; --k, ++m) // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++tmpPtr;                               // next output
			++kOffset;                              // increase starting index of kernel
		}

		// COLUMN FROM index=kCenter TO index=(dataSizeX-kCenter-1)
		for (j = kCenter; j < endIndex; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulate

			for (k = kSizeX - 1, m = 0; k >= 0; --k, ++m)  // full kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
		}

		kOffset = 1;                                // ending index of partial kernel varies for each Sample

		// COLUMN FROM index=(dataSizeX-kCenter) TO index=(dataSizeX-1)
		for (j = endIndex; j < dataSizeX; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kSizeX - 1, m = 0; k >= kOffset; --k, ++m)   // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
			++kOffset;                              // increase ending index of partial kernel
		}

		inPtr += kCenter;                           // next row
	}
	// END OF HORIZONTAL CONVOLUTION //////////////////////

	// start vertical direction ///////////////////////////

	// find center position of kernel (half of kernel size)
	kCenter = kSizeY >> 1;                          // center index of vertical kernel
	endIndex = dataSizeY - kCenter;                 // index where full kernel convolution should stop

	// set working pointers
	tmpPtr = tmpPtr2 = tmp;
	float *outPtr = out;

	// clear out array before accumulation
	for (i = 0; i < dataSizeX; ++i)
		sum[i] = 0;

	// start to convolve vertical direction (y-direction)

	// ROW FROM index=0 TO index=(kCenter-1)
	kOffset = 0;                                    // starting index of partial kernel varies for each Sample
	for (i = 0; i < kCenter; ++i)
	{
		for (k = kCenter + kOffset; k >= 0; --k)     // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			*outPtr = sum[n];                       // store final result to output array
			sum[n] = 0;                             // reset to zero for next summing
			++outPtr;                               // next element of output
		}

		tmpPtr = tmpPtr2;                           // reset Sample pointer
		++kOffset;                                  // increase starting index of kernel
	}

	// ROW FROM index=kCenter TO index=(dataSizeY-kCenter-1)
	for (i = kCenter; i < endIndex; ++i)
	{
		for (k = kSizeY - 1; k >= 0; --k)             // convolve with full kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			*outPtr = sum[n];                       // store final result to output buffer
			sum[n] = 0;                             // reset before next summing
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;
	}

	// ROW FROM index=(dataSizeY-kCenter) TO index=(dataSizeY-1)
	kOffset = 1;                                    // ending index of partial kernel varies for each Sample
	for (i = endIndex; i < dataSizeY; ++i)
	{
		for (k = kSizeY - 1; k >= kOffset; --k)        // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			*outPtr = sum[n];                       // store final result to output array
			sum[n] = 0;                             // reset to 0 for next sum
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;                           // next Sample
		++kOffset;                                  // increase ending index of kernel
	}
	// END OF VERTICAL CONVOLUTION ////////////////////////

	// deallocate temp buffers
	delete[] tmp;
	delete[] sum;
	return true;
}

bool Convolution::Convolve2DSeparable(double * in, double * out, int sizeX, int sizeY, double * xKernel, int kSizeX, double * yKernel, int kSizeY)
{
	return false;
}

bool Convolution::ControlledConvolve2DSeparable(double * in, double * out, int sizeX, int sizeY, double * xKernel, int kSizeX, double * yKernel, int kSizeY)
{
	return false;
}

bool Convolution::OrderByComposition(double * in, double * out, int sizeX, int sizeY, double * xKernel, int kSizeX, double * yKernel, int kSizeY)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// double precision float version
///////////////////////////////////////////////////////////////////////////////
bool Convolution::Convolve2DSeparable(double* in, double* out, int dataSizeX, int dataSizeY,
	double* kernelX, int kSizeX, float* kernelY, int kSizeY)
{
	int i, j, k, m, n;
	double*tmpPtr2;                       // working pointers
	int kOffset;                 // kernel indice

	// check validity of params
	if (!in || !out || !kernelX || !kernelY) return false;
	if (dataSizeX <= 0 || kSizeX <= 0) return false;

	// allocate temp storage to keep intermediate result
	double *tmp = new double[dataSizeX * dataSizeY];
	if (!tmp) return false;  // memory allocation error

	// store accumulated sum
	double *sum = new double[dataSizeX];
	if (!sum) return false;  // memory allocation error

	// covolve horizontal direction ///////////////////////

	// find center position of kernel (half of kernel size)
	int kCenter = kSizeX >> 1;                          // center index of kernel array
	int endIndex = dataSizeX - kCenter;                 // index for full kernel convolution

	// init working pointers
	double *inPtr = in;
	double *tmpPtr = tmp;                                   // store intermediate results from 1D horizontal convolution

	// start horizontal convolution (x-direction)
	for (i = 0; i < dataSizeY; ++i)                    // number of rows
	{
		kOffset = 0;                                // starting index of partial kernel varies for each Sample

		// COLUMN FROM index=0 TO index=kCenter-1
		for (j = 0; j < kCenter; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kCenter + kOffset, m = 0; k >= 0; --k, ++m) // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++tmpPtr;                               // next output
			++kOffset;                              // increase starting index of kernel
		}

		// COLUMN FROM index=kCenter TO index=(dataSizeX-kCenter-1)
		for (j = kCenter; j < endIndex; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulate

			for (k = kSizeX - 1, m = 0; k >= 0; --k, ++m)  // full kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
		}

		kOffset = 1;                                // ending index of partial kernel varies for each Sample

		// COLUMN FROM index=(dataSizeX-kCenter) TO index=(dataSizeX-1)
		for (j = endIndex; j < dataSizeX; ++j)
		{
			*tmpPtr = 0;                            // init to 0 before accumulation

			for (k = kSizeX - 1, m = 0; k >= kOffset; --k, ++m)   // convolve with partial of kernel
			{
				*tmpPtr += *(inPtr + m) * kernelX[k];
			}
			++inPtr;                                // next Sample
			++tmpPtr;                               // next output
			++kOffset;                              // increase ending index of partial kernel
		}

		inPtr += kCenter;                           // next row
	}
	// END OF HORIZONTAL CONVOLUTION //////////////////////

	// start vertical direction ///////////////////////////

	// find center position of kernel (half of kernel size)
	kCenter = kSizeY >> 1;                          // center index of vertical kernel
	endIndex = dataSizeY - kCenter;                 // index where full kernel convolution should stop

	// set working pointers
	tmpPtr = tmpPtr2 = tmp;
	double *outPtr = out;

	// clear out array before accumulation
	for (i = 0; i < dataSizeX; ++i)
		sum[i] = 0;

	// start to convolve vertical direction (y-direction)

	// ROW FROM index=0 TO index=(kCenter-1)
	kOffset = 0;                                    // starting index of partial kernel varies for each Sample
	for (i = 0; i < kCenter; ++i)
	{
		for (k = kCenter + kOffset; k >= 0; --k)     // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			*outPtr = sum[n];                       // store final result to output array
			sum[n] = 0;                             // reset to zero for next summing
			++outPtr;                               // next element of output
		}

		tmpPtr = tmpPtr2;                           // reset Sample pointer
		++kOffset;                                  // increase starting index of kernel
	}

	// ROW FROM index=kCenter TO index=(dataSizeY-kCenter-1)
	for (i = kCenter; i < endIndex; ++i)
	{
		for (k = kSizeY - 1; k >= 0; --k)             // convolve with full kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			*outPtr = sum[n];                       // store final result to output array
			sum[n] = 0;                             // reset to zero for next summing
			++outPtr;                               // next output
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;
	}

	// ROW FROM index=(dataSizeY-kCenter) TO index=(dataSizeY-1)
	kOffset = 1;                                    // ending index of partial kernel varies for each Sample
	for (i = endIndex; i < dataSizeY; ++i)
	{
		for (k = kSizeY - 1; k >= kOffset; --k)        // convolve with partial kernel
		{
			for (j = 0; j < dataSizeX; ++j)
			{
				sum[j] += *tmpPtr * kernelY[k];
				++tmpPtr;
			}
		}

		for (n = 0; n < dataSizeX; ++n)              // convert and copy from sum to out
		{
			*outPtr = sum[n];                       // store final result to output array
			sum[n] = 0;                             // reset to zero for next summing
			++outPtr;                               // increase ending index of partial kernel
		}

		// move to next row
		tmpPtr2 += dataSizeX;
		tmpPtr = tmpPtr2;                           // next Sample
		++kOffset;                                  // increase ending index of kernel
	}
	// END OF VERTICAL CONVOLUTION ////////////////////////

	// deallocate temp buffers
	delete[] tmp;
	delete[] sum;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// 2D Convolution Fast
// In order to improve the performance, this function uses multple cursors of
// Sample signal. It avoids indexing Sample array during convolution. And, the
// Sample signal is partitioned to 9 different sections, so we don't need to
// check the boundary for every samples.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// unsigned char (8-bit) version
///////////////////////////////////////////////////////////////////////////////
bool Convolution::Convolve2DFast(unsigned char* in, unsigned char* out, int dataSizeX, int dataSizeY,
	float* kernel, int kernelSizeX, int kernelSizeY)
{
	int i, j, m, n, x, t;
	float sum;                                      // temp accumulation buffer
	int k;

	// check validity of params
	if (!in || !out || !kernel) return false;
	if (dataSizeX <= 0 || kernelSizeX <= 0) return false;

	// find center position of kernel (half of kernel size)
	int kCenterX = kernelSizeX >> 1;
	int kCenterY = kernelSizeY >> 1;
	int kSize = kernelSizeX * kernelSizeY;              // total kernel size

	// allocate memeory for multi-cursor
	unsigned char **inPtr = new unsigned char*[kSize];
	if (!inPtr) return false;                        // allocation error

	// set initial position of multi-cursor, NOTE: it is swapped instead of kernel
	unsigned char *ptr = in + (dataSizeX * kCenterY + kCenterX); // the first cursor is shifted (kCenterX, kCenterY)
	for (m = 0, t = 0; m < kernelSizeY; ++m)
	{
		for (n = 0; n < kernelSizeX; ++n, ++t)
		{
			inPtr[t] = ptr - n;
		}
		ptr -= dataSizeX;
	}

	// init working  pointers
	unsigned char *outPtr = out;

	int rowEnd = dataSizeY - kCenterY;                  // bottom row partition divider
	int colEnd = dataSizeX - kCenterX;                  // right column partition divider

	// convolve rows from index=0 to index=kCenterY-1
	int y = kCenterY;
	for (i = 0; i < kCenterY; ++i)
	{
		// partition #1 ***********************************
		x = kCenterX;
		for (j = 0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
		{
			sum = 0;
			t = 0;
			for (m = 0; m <= y; ++m)
			{
				for (n = 0; n <= x; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += (kernelSizeX - x - 1);         // jump to next row
			}

			// store output
			*outPtr = (unsigned char)((float)fabs(sum) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		// partition #2 ***********************************
		for (j = kCenterX; j < colEnd; ++j)            // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
		{
			sum = 0;
			t = 0;
			for (m = 0; m <= y; ++m)
			{
				for (n = 0; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
			}

			// store output
			*outPtr = (unsigned char)((float)fabs(sum) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		// partition #3 ***********************************
		x = 1;
		for (j = colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
		{
			sum = 0;
			t = x;
			for (m = 0; m <= y; ++m)
			{
				for (n = x; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += x;                             // jump to next row
			}

			// store output
			*outPtr = static_cast<unsigned char>(static_cast<float>(fabs(sum)) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		++y;                                        // add one more row to convolve for next run
	}

	// convolve rows from index=kCenterY to index=(dataSizeY-kCenterY-1)
	for (i = kCenterY; i < rowEnd; ++i)               // number of rows
	{
		// partition #4 ***********************************
		x = kCenterX;
		for (j = 0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
		{
			sum = 0;
			t = 0;
			for (m = 0; m < kernelSizeY; ++m)
			{
				for (n = 0; n <= x; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += (kernelSizeX - x - 1);
			}

			// store output
			*outPtr = static_cast<unsigned char>(static_cast<float>(fabs(sum)) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		// partition #5 ***********************************
		for (j = kCenterX; j < colEnd; ++j)          // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
		{
			sum = 0;
			t = 0;
			for (m = 0; m < kernelSizeY; ++m)
			{
				for (n = 0; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++inPtr[t]; // in this partition, all cursors are used to convolve. moving cursors to next is safe here
					++t;
				}
			}

			// store output
			*outPtr = static_cast<unsigned char>(static_cast<float>(fabs(sum)) + 0.5f);
			++outPtr;
			++x;
		}

		// partition #6 ***********************************
		x = 1;
		for (j = colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
		{
			sum = 0;
			t = x;
			for (m = 0; m < kernelSizeY; ++m)
			{
				for (n = x; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += x;
			}

			// store output
			*outPtr = static_cast<unsigned char>(static_cast<float>(fabs(sum)) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}
	}

	// convolve rows from index=(dataSizeY-kCenterY) to index=(dataSizeY-1)
	y = 1;
	for (i = rowEnd; i < dataSizeY; ++i)               // number of rows
	{
		// partition #7 ***********************************
		x = kCenterX;
		for (j = 0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
		{
			sum = 0;
			t = kernelSizeX * y;

			for (m = y; m < kernelSizeY; ++m)
			{
				for (n = 0; n <= x; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += (kernelSizeX - x - 1);
			}

			// store output
			*outPtr = static_cast<unsigned char>(static_cast<float>(fabs(sum)) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		// partition #8 ***********************************
		for (j = kCenterX; j < colEnd; ++j)            // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
		{
			sum = 0;
			t = kernelSizeX * y;
			for (m = y; m < kernelSizeY; ++m)
			{
				for (n = 0; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
			}

			// store output
			*outPtr = (unsigned char)((float)fabs(sum) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];
		}

		// partition #9 ***********************************
		x = 1;
		for (j = colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
		{
			sum = 0;
			t = kernelSizeX * y + x;
			for (m = y; m < kernelSizeY; ++m)
			{
				for (n = x; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += x;
			}

			// store output
			*outPtr = (unsigned char)((float)fabs(sum) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		++y;                                        // the starting row index is increased
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Fast 2D Convolution using integer multiplication instead of float.
// Multiply coefficient(factor) to accumulated sum at last.
// NOTE: IT IS NOT FASTER THAN FLOAT MULTIPLICATION, TRY YOURSELF!!!
///////////////////////////////////////////////////////////////////////////////
bool Convolution::Convolve2DFast2(unsigned char* in, unsigned char* out, int dataSizeX, int dataSizeY,
	int* kernel, float factor, int kernelSizeX, int kernelSizeY)
{
	int i, j, m, n, x, t;
	int sum;                                        // temp accumulation buffer
	int k;

	// check validity of params
	if (!in || !out || !kernel) return false;
	if (dataSizeX <= 0 || kernelSizeX <= 0) return false;

	// find center position of kernel (half of kernel size)
	int kCenterX = kernelSizeX >> 1;
	int kCenterY = kernelSizeY >> 1;
	int kSize = kernelSizeX * kernelSizeY;              // total kernel size

	// allocate memeory for multi-cursor
	unsigned char **inPtr = new unsigned char*[kSize];
	if (!inPtr) return false;                        // allocation error

	// set initial position of multi-cursor, NOTE: it is swapped instead of kernel
	unsigned char *ptr = in + (dataSizeX * kCenterY + kCenterX); // the first cursor is shifted (kCenterX, kCenterY)
	for (m = 0, t = 0; m < kernelSizeY; ++m)
	{
		for (n = 0; n < kernelSizeX; ++n, ++t)
		{
			inPtr[t] = ptr - n;
		}
		ptr -= dataSizeX;
	}

	// init working  pointers
	unsigned char *outPtr = out;

	int rowEnd = dataSizeY - kCenterY;                  // bottom row partition divider
	int colEnd = dataSizeX - kCenterX;                  // right column partition divider

	// convolve rows from index=0 to index=kCenterY-1
	int y = kCenterY;
	for (i = 0; i < kCenterY; ++i)
	{
		// partition #1 ***********************************
		x = kCenterX;
		for (j = 0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
		{
			sum = 0;
			t = 0;
			for (m = 0; m <= y; ++m)
			{
				for (n = 0; n <= x; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += (kernelSizeX - x - 1);         // jump to next row
			}

			// store output
			*outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		// partition #2 ***********************************
		for (j = kCenterX; j < colEnd; ++j)            // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
		{
			sum = 0;
			t = 0;
			for (m = 0; m <= y; ++m)
			{
				for (n = 0; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
			}

			// store output
			*outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		// partition #3 ***********************************
		x = 1;
		for (j = colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
		{
			sum = 0;
			t = x;
			for (m = 0; m <= y; ++m)
			{
				for (n = x; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += x;                             // jump to next row
			}

			// store output
			*outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		++y;                                        // add one more row to convolve for next run
	}

	// convolve rows from index=kCenterY to index=(dataSizeY-kCenterY-1)
	for (i = kCenterY; i < rowEnd; ++i)               // number of rows
	{
		// partition #4 ***********************************
		x = kCenterX;
		for (j = 0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
		{
			sum = 0;
			t = 0;
			for (m = 0; m < kernelSizeY; ++m)
			{
				for (n = 0; n <= x; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += (kernelSizeX - x - 1);
			}

			// store output
			*outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		// partition #5 ***********************************
		for (j = kCenterX; j < colEnd; ++j)          // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
		{
			sum = 0;
			t = 0;
			for (m = 0; m < kernelSizeY; ++m)
			{
				for (n = 0; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++inPtr[t]; // in this partition, all cursors are used to convolve. moving cursors to next is safe here
					++t;
				}
			}

			// store output
			*outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
		}

		// partition #6 ***********************************
		x = 1;
		for (j = colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
		{
			sum = 0;
			t = x;
			for (m = 0; m < kernelSizeY; ++m)
			{
				for (n = x; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += x;
			}

			// store output
			*outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}
	}

	// convolve rows from index=(dataSizeY-kCenterY) to index=(dataSizeY-1)
	y = 1;
	for (i = rowEnd; i < dataSizeY; ++i)               // number of rows
	{
		// partition #7 ***********************************
		x = kCenterX;
		for (j = 0; j < kCenterX; ++j)                 // column from index=0 to index=kCenterX-1
		{
			sum = 0;
			t = kernelSizeX * y;

			for (m = y; m < kernelSizeY; ++m)
			{
				for (n = 0; n <= x; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += (kernelSizeX - x - 1);
			}

			// store output
			*outPtr = static_cast<unsigned char>(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		// partition #8 ***********************************
		for (j = kCenterX; j < colEnd; ++j)            // column from index=kCenterX to index=(dataSizeX-kCenterX-1)
		{
			sum = 0;
			t = kernelSizeX * y;
			for (m = y; m < kernelSizeY; ++m)
			{
				for (n = 0; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
			}

			// store output
			*outPtr = static_cast<unsigned char>(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];
		}

		// partition #9 ***********************************
		x = 1;
		for (j = colEnd; j < dataSizeX; ++j)           // column from index=(dataSizeX-kCenter) to index=(dataSizeX-1)
		{
			sum = 0;
			t = kernelSizeX * y + x;
			for (m = y; m < kernelSizeY; ++m)
			{
				for (n = x; n < kernelSizeX; ++n)
				{
					sum += *inPtr[t] * kernel[t];
					++t;
				}
				t += x;
			}

			// store output
			*outPtr = (unsigned char)(fabs(sum * factor) + 0.5f);
			++outPtr;
			++x;
			for (k = 0; k < kSize; ++k) ++inPtr[k];    // move all cursors to next
		}

		++y;                                        // the starting row index is increased
	}

	return true;
}