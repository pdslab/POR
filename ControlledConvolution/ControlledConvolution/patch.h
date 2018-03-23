#pragma once
#ifndef PATCH_H
#define PATHC_H
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
	Patch(): start_row_(0), end_row_(0), start_column_(0), end_column_(0), rows_(0), columns_(0), entropy_(0)
	{
	}

	Patch(const int s_row, const int e_row, const int s_col, const int e_col): entropy_(0)
	{
		start_row_ = s_row;
		end_row_ = e_row;
		start_column_ = s_col;
		end_column_ = e_col;
		rows_ = end_row_ - start_row_;
		columns_ = end_column_ - start_column_;
	}

	explicit Patch(const Coordinate c): start_row_(0), end_row_(0), start_column_(0), end_column_(0), rows_(0),
	                                    columns_(0), entropy_(0)
	{
		coo_ = c;
	}

	explicit Patch(const Mat& mat, const Coordinate &c);

	void Release() { patch_mat_.release(); }
	~Patch()
	{
		//delete corrdinates_;
	}



#pragma region utils 
	void SetName(const string &name) { name_ = name; }
	void WriteToFile(const string& file) const;
	void Save(const string& path, const string &format) const;
	void ToPixel();
#pragma endregion 

#pragma region patch measures 
	void ComputeHisogram();
	void ComputeEntropy();
	void ComputeMutualInformationGain();
#pragma endregion 

#pragma region setters
	initializer_list<Mat> Hist() const { return patch_histogram_; };
	Mat RChannelHist()const { return r_channel_hist_; }
	Mat GChannelHist() const { return g_channel_hist_; }
	Mat BChannelHist() const { return b_channel_hist_; }
	float Entropy() const { return entropy_; }
	void SetBgrPlanes(vector<Mat> &bgr_planes) { patch_bgr_planes_ = bgr_planes; }
#pragma endregion

#pragma region getters 
	Coordinate GetPatchCoordinates() const
	{
		return coo_;
	}

	string Name() const { return name_; }
	Mat GetMat() const { return patch_mat_; }
	vector<vector<float>> Pixels()const { return patch_pixels_float_; }
	vector<Mat> BgrPlanes() const { return patch_bgr_planes_; }
#pragma endregion 

private:
	vector<vector<float>> ExtractPixels();
	int start_row_;
	int end_row_;
	int start_column_;
	int end_column_;
	int corrdinates_[4] ;
	int rows_;
	int columns_;
	string file_;
	string name_;
	Coordinate coo_;
	Mat patch_mat_;
	vector<vector<float>> patch_pixels_float_;
	vector<uchar> patch_pixels_uchar_;
	vector<Mat> patch_bgr_planes_;
	initializer_list<Mat> patch_histogram_;
	Mat r_channel_hist_;
	Mat g_channel_hist_;
	Mat b_channel_hist_;
	float entropy_;
	map<Patch, float> mutual_information_;
};

#endif

