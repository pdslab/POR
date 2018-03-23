#pragma once
#ifndef COMMON_H
#define COMMON_H
#include <opencv2/highgui.hpp>

#include "coordinate.h"

static const std::string PATCH_ROOT_DIR = "../../patches";
static const std::string PATCH_PIXELS = PATCH_ROOT_DIR + "\\pixel data";
static const std::string PATCH_IMAGES = PATCH_ROOT_DIR + "\\image_patches";
static const std::string SORTED_PATCHES_DIR= PATCH_ROOT_DIR + "\\sorted";
static const std::string RONSTRUCTED_SAMPLES = "../../reconstructed";

static const std::string INPUT_SAMPLES = "../../input";
static const std::string CIFAR10 = R"(C:\data\cifar-10-binary.tar\cifar-10-binary)";
#define ATD at<double>
#define ELIF else if

class Common
{
public:
	static std::string GeneratePatchName(const int startRow, const int endRow,const int startCol,const int endCol);
	static std::string GeneratePatchName(const Coordinate &c);
	static bool IsSquareImage(const cv::Mat &mat);
	static bool IsPower2(const int &number);
	static void Show(const cv::Mat& image, const std::string& title);
	static void ShowMultiple(const std::string &title, const int nArgs, ...);
	static void PrintVector(const std::vector<std::vector<float>> &vec);
	static void WriteToFile(const std::vector<std::vector<float>>& vec, const std::string &file);
	static void Resize(const cv::Mat& input, cv::Mat& output, const unsigned int& width, const unsigned int& height);
	static void ReadCifar10(cv::Mat &trainX, cv::Mat &testX, cv::Mat &trainY, cv::Mat &testY);
	static cv::Mat ConcatenateMat(std::vector<cv::Mat> &vec);
	static void	ReadBatch(std::string filename, std::vector<cv::Mat> &vec, cv::Mat &label);
	static void SaveImage(const cv::Mat &mat, const std::string& filename, const std::string& format);
	
};
#endif

