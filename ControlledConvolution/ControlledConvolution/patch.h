#pragma once
#include <iostream>
#include "coordinate.h"
#include <list>
#include <map>

/*A Patch is a unique , 4-tuple subsection of the original input <w,h> identified by its start and end cooridinates
 * w = <0,x_end>
 * h = <0,y_end>
 * p = <x0,y0,x1,y1> where 0 <= x0 <= x_end and 0 <= y <= y_end
 *
 * Properties:
 * 1. A Patch has a bounding zone (area)
 * 2. No Patch can intersect any other Patch: P1 intersection P2 <= 1
 * 3. A Patch is unique
 *
 */

class Patch
{
public:
	Patch() : start_row_(0), end_row_(0), start_column_(0), end_column_(0), rows_(0), columns_(0), entropy_(0)
	{
	}

	Patch(const int s_row, const int e_row, const int s_col, const int e_col) : entropy_(0)
	{
		start_row_ = s_row;
		end_row_ = e_row;
		start_column_ = s_col;
		end_column_ = e_col;
		rows_ = end_row_ - start_row_;
		columns_ = end_column_ - start_column_;
	}

	explicit Patch(const Coordinate c) : start_row_(0), end_row_(0), start_column_(0), end_column_(0), rows_(0),
		columns_(0), entropy_(0)
	{
		coo_ = c;
	}

	explicit Patch(const cv::Mat& mat, const Coordinate &c);

	void Release() { patch_mat_.release(); }
	~Patch()
	{
		//delete corrdinates_;
	}

#pragma region utils
	void SetName(const std::string &name) { name_ = name; }
	void WriteToFile(const std::string& file) const;
	void Save(const std::string& path, const std::string &format) const;
	void ToPixel();
	void SetMat(const cv::Mat& mat) { patch_mat_ = mat; }
#pragma endregion

#pragma region patch measures
	void ComputeHisogram();
	void ComputeEntropy();
	void ComputeMutualInformationGain();
#pragma endregion

#pragma region setters
	std::initializer_list<cv::Mat> Hist() const { return patch_histogram_; };
	cv::Mat RChannelHist()const { return r_channel_hist_; }
	cv::Mat GChannelHist() const { return g_channel_hist_; }
	cv::Mat BChannelHist() const { return b_channel_hist_; }
	float Entropy() const { return entropy_; }
	void SetBgrPlanes(std::vector<cv::Mat> &bgr_planes) { patch_bgr_planes_ = bgr_planes; }
#pragma endregion

#pragma region getters
	Coordinate GetPatchCoordinates() const
	{
		return coo_;
	}

	std::string Name() const { return name_; }
	cv::Mat GetMat() const { return patch_mat_; }
	std::vector<std::vector<float>> Pixels()const { return patch_pixels_float_; }
	std::vector<cv::Mat> BgrPlanes() const { return patch_bgr_planes_; }
#pragma endregion

private:
	std::vector<std::vector<float>> ExtractPixels();
	int start_row_;
	int end_row_;
	int start_column_;
	int end_column_;
	int corrdinates_[4];
	int rows_;
	int columns_;
	std::string file_;
	std::string name_;
	Coordinate coo_;
	cv::Mat patch_mat_;
	std::vector<std::vector<float>> patch_pixels_float_;
	std::vector<uchar> patch_pixels_uchar_;
	std::vector<cv::Mat> patch_bgr_planes_;
	std::initializer_list<cv::Mat> patch_histogram_;
	cv::Mat r_channel_hist_;
	cv::Mat g_channel_hist_;
	cv::Mat b_channel_hist_;
	float entropy_;
	std::map<Patch, float> mutual_information_;
};

