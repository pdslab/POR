// ControlledConvolution.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
// main.cpp
// ========
// It convolves 2D image with Gaussian smoothing kernel,
// and compare the performance between normal 2D convolution and separable
// convolution.
// The result images will be displayed on the screen using OpenGL.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2005-08-31
// UPDATED: 2005-09-12
///////////////////////////////////////////////////////////////////////////////

#include <GL/glut.h>
#include "convolution.h"
#include <stdio.h>
#include <fstream>

// image data for openGL
struct image_data {
	int x;      // resolution X
	int y;      // resolution Y
	GLenum  format; // data format (RGB or INDEX..)
	GLenum  type;   // data type (8bit, 16bit or 32bit..)
	GLvoid  *buf;   // image pixel bits
};

void show_image(const char * img, const string& title);
void show_image(const cv::Mat &img, const string& title);
cv::Mat extract_image_patch(const cv::Mat &buffer, const int start_x, const int start_y, const int end_x, const int end_y);
bool init_shared_mem();
void clear_shared_mem();

/**
 * \brief 
 * \param file_name 
 * \param x 
 * \param y 
 * \param data 
 * \return 
 */
bool load_raw_image(char *file_name, int x, int y, unsigned char *data);
cv::Mat load_image(const char * img, const cv::ImreadModes mode, const bool show_image);
string exe_path();
// constants ////////////////////////
const char *file_name_default = "img.jpg";
const int  image_x_default = 256;
const int  image_y_default = 256;
const int  max_name_default = 1024;

// global variables ////////////////
image_data       *image;
unsigned char   *in_buf;
unsigned char   *out_buf1;
unsigned char   *out_buf2;
char            file_name[max_name_default];
int             image_x;
int             image_y;
void            *font = GLUT_BITMAP_8_BY_13;
int             font_width = 8;
int             font_height = 13;
int             main_win, sub_win1, sub_win2, sub_win3, sub_win4;
double          time1, time2;

FILE *stream;

///////////////////////////////////////////////////////////////////////////////
int main(const int argc, char **argv)
{
	// use default image file if not specified
	if (argc == 4)
	{
		strcpy_s(file_name, argv[1]);
		image_x = atoi(argv[2]);
		image_y = atoi(argv[3]);
	}
	else 
	{
		printf("Usage: %s <image-file> <width> <height>\n", argv[0]);
		strcpy_s(file_name, file_name_default);
		image_x = image_x_default;
		image_y = image_y_default;
	}
	//------------------------------------------Read \ display------------------------------------------------------------
	// allocate memory for global variables
	if (!init_shared_mem()) return 0;

	auto image_cv_mat = load_image(file_name, cv::IMREAD_COLOR, true);
	// open raw image file
	if (image_cv_mat.empty())
	{
		clear_shared_mem();                       // exit program if failed to load image
		return 0;
	}

	const auto patch = extract_image_patch(image_cv_mat,35,35,40,40);
	show_image(patch, "0x0_25x25_Patch");

	//---------------------------------------------------------------------------------------------------------------------------

	// define 5x5 Gaussian kernel
	float kernel[25] = { 1 / 256.0f,  4 / 256.0f,  6 / 256.0f,  4 / 256.0f, 1 / 256.0f,
						 4 / 256.0f, 16 / 256.0f, 24 / 256.0f, 16 / 256.0f, 4 / 256.0f,
						 6 / 256.0f, 24 / 256.0f, 36 / 256.0f, 24 / 256.0f, 6 / 256.0f,
						 4 / 256.0f, 16 / 256.0f, 24 / 256.0f, 16 / 256.0f, 4 / 256.0f,
						 1 / 256.0f,  4 / 256.0f,  6 / 256.0f,  4 / 256.0f, 1 / 256.0f };

	// Separable kernel
	float kernel_x[5] = { 1 / 16.0f,  4 / 16.0f,  6 / 16.0f,  4 / 16.0f, 1 / 16.0f };
	float kernel_y[5] = { 1 / 16.0f,  4 / 16.0f,  6 / 16.0f,  4 / 16.0f, 1 / 16.0f };

	// integer kernel
	auto kernel_factor = 1 / 256.0f;
	int kernel_int[25] = { 1,  4,  6,  4, 1,
						   4, 16, 24, 16, 4,
						   6, 24, 36, 24, 6,
						   4, 16, 24, 16, 4,
						   1,  4,  6,  4, 1 };

	convolve_2d(in_buf, out_buf1, image_x, image_y, kernel, 5, 5);
	convolve_2d_separable(in_buf, out_buf2, image_x, image_y, kernel_x, 5, kernel_y, 5);

	// compare the results of separable convolution with normal convolution, they should be equal.
	for (auto i = 0; i < image_x*image_y; ++i)
	{
		if (out_buf1[i] != out_buf2[i]) 
			printf("different at %d (%d, %d), out1:%d, out2:%d\n", i, i%image_x, i / image_x, out_buf1[i], out_buf2[i]);
	}

	return 0;
}

cv::Mat extract_image_patch(const cv::Mat& buffer, const int start_x, const int start_y, const int end_x, const int end_y)
{
	const auto patch_x = end_x - start_x;
	const auto patch_y = end_y - start_y;

	auto patchs_output_dir = exe_path() + "\\patchs\\";

	CreateDirectory(patchs_output_dir.c_str(), nullptr);

	patchs_output_dir += std::to_string(start_x) + to_string(start_y) + "_" + to_string(end_x) + to_string(end_y) + "_patch.txt";
	const auto output_file = patchs_output_dir;
	ofstream output_stream;
	output_stream.open(output_file);

	if (output_stream.is_open())
	{
		//allocate output matrix for patch
		std::vector<uchar> row;
		std::vector<vector<uchar>> matrix;
		cv::Mat patch(patch_x, patch_y, CV_8U);

		for (auto i = start_y; i < end_y; i++)
		{
			row.clear();
			for (auto j = start_x; j < end_x; j++)
			{
				const auto u_char = buffer.at<uchar>(i, j);

				//convert to float 
				const auto pixel_element = static_cast<float>(u_char);

				cout << pixel_element << " ";
				output_stream << pixel_element << " ";
				row.push_back(u_char);
			}

			matrix.push_back(row);
			output_stream << endl;
			cout << endl;
		}

		output_stream.close();

		return patch;
	}
	cout << "Error opening output file\n";
	throw new exception("Error opening file for write");
}

string exe_path() 
{
	char buffer[MAX_PATH];
	GetModuleFileName(nullptr, buffer, MAX_PATH);
	const auto pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}
void show_image(const char * img, const string& title)
{
	const auto image = cv::imread(img, CV_LOAD_IMAGE_COLOR);   // Read the file

	show_image(image, title);
}

void show_image(const cv::Mat &image, const string & title)
{
	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return;
	}

	cv::namedWindow(title, cv::WINDOW_AUTOSIZE);// Create a window for display.
	imshow(title, image);                   // Show our image inside it.

	cv::waitKey();
}

///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool init_shared_mem()
{
	image = new image_data;
	if (!image)
	{
		printf("ERROR: Memory Allocation Failed.\n");
		return false;
	}

	// allocate input/output buffer
	in_buf = new unsigned char[image_x * image_y];
	out_buf1 = new unsigned char[image_x * image_y];
	out_buf2 = new unsigned char[image_x * image_y];

	if (!in_buf || !out_buf1 || !out_buf2)
	{
		printf("ERROR: Memory Allocation Failed.\n");
		return false;
	}

	// set image data
	image->x = image_x;
	image->y = image_y;
	image->format = GL_LUMINANCE;
	image->type = GL_UNSIGNED_BYTE;
	image->buf = static_cast<GLvoid*>(in_buf);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// clean up shared memory
///////////////////////////////////////////////////////////////////////////////
void clear_shared_mem()
{
	delete image;
	delete[] in_buf;
	delete[] out_buf1;
	delete[] out_buf2;
}

///////////////////////////////////////////////////////////////////////////////
// load 8-bit RAW image
///////////////////////////////////////////////////////////////////////////////
bool load_raw_image(char * file_name, const int x, const int y, unsigned char *data)
{
	// check params
	if (!file_name || !data)
		return false;

	//LPCTSTR filePath = TEXT("");

	//auto exists = PathFileExists(reinterpret_cast<LPCTSTR>(file_name));

	///*if (!exists)
	//{
	//	printf("Unable to open %s. PathFileExists returned false", (char*)fileName);
	//	return false;
	//}*/

	FILE *fp = nullptr;
	char buffer[256];
	const auto err = (fopen_s(&fp, file_name, "r"));

	if (err != 0)
	{
		setlocale(LC_ALL, "zh_CN.utf8"); // printf needs CTYPE for multibyte output
		//size_t errmsglen = strerrorlen_s(err) + 1;
		strerror_s(buffer, err);
		printf("Cannot open %s . Fopen returned %s\n", static_cast<char*>(file_name), buffer);
		return false;
	}

	if (fp == nullptr) printf("File didn't open correctly");

	// read pixel data
	try
	{
		fread(data, 1, x*y, fp);
	}
	catch (exception ex)
	{
	}
	fclose(fp);

	return true;
}

cv::Mat load_image(const char * img_path, const cv::ImreadModes mode, const bool showImage)
{
	auto image = imread(img_path, mode); // Read the file
	if (image.empty())                      // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return image;
	}
	if (showImage) show_image(image, "Original Image");
	return image;
}
