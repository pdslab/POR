#include "stdafx.h"
#include "Patch.h"
#include <fstream>
#include <Windows.h>

Patch::Patch(const Mat& mat, const Coordinate& c): start_row_(0), end_row_(0), start_column_(0), end_column_(0),
													   rows_(0), columns_(0), entropy_(0)
{
	patch_mat_ = mat;
	coo_ = c;
	start_row_ = coo_.Start()[0];
	start_column_ = coo_.Start()[1];
	end_column_ = coo_.End()[1];
	end_row_ = coo_.End()[0];
}

void Patch::WriteToFile(const string & file_name) const
{
	cout << "Entering write_to_file ... filename = " << file_name << endl;

	ofstream output;
	const auto f = PATCH_PIXELS + "\\" + file_name + ".txt";
	output.open(f, fstream::out);

	for (auto row = Pixels().begin(); row != Pixels().end(); ++row)
	{
		for (auto col = row->begin(); col != row->end(); ++col)
		{
			output << *col << " ";
		}
		output << endl;
	}
	output.flush();
	output.close();
}

void Patch::Save(const string& path, const string& format) const
{
	const auto outputFile = path + "\\" +  name_ + "." + format;
	//std::cout << "Saving " << outputFile << endl;
	imwrite(outputFile, patch_mat_);
}

void Patch::ToPixel()
{
	for(auto i = 0; i < patch_mat_.rows; i++)
	{
		const float* p = patch_mat_.ptr<float>(i);
		const vector<float> vec(p, p + patch_mat_.cols);
		patch_pixels_float_.push_back(vec);
	}
}

void Patch::ComputeHisogram()
{
	vector<Mat> bgrPlanes;
	split(patch_mat_, bgrPlanes);
	auto histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 };
	const float* histRange = { range };

	const auto uniform = true;
	const auto accumulate = false;

	Mat bHist, gHist, rHist;

	/// Compute the histograms:
	calcHist(&bgrPlanes[0], 1, nullptr, Mat(), bHist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgrPlanes[1], 1, nullptr, Mat(), gHist, 1, &histSize, &histRange, uniform, accumulate);
	calcHist(&bgrPlanes[2], 1, nullptr, Mat(), rHist, 1, &histSize, &histRange, uniform, accumulate);

	// Draw the histograms for B, G and R
	const auto histW = 512;
	const auto histH = 400;
	const auto binW = cvRound(static_cast<double>(histW) / histSize);

	Mat histImage(histH, histW, CV_8UC3, Scalar(0, 0, 0));

	/// Normalize the result to [ 0, histImage.rows ]
	normalize(bHist, bHist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(gHist, gHist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(rHist, rHist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	/// Draw for each channel
	for (auto i = 1; i < histSize; i++)
	{
		line(histImage, Point(binW*(i - 1), histH - cvRound(bHist.at<float>(i - 1))),
			Point(binW*(i), histH - cvRound(bHist.at<float>(i))),
			Scalar(255, 0, 0), 2, 8, 0);
		line(histImage, Point(binW*(i - 1), histH - cvRound(gHist.at<float>(i - 1))),
			Point(binW*(i), histH - cvRound(gHist.at<float>(i))),
			Scalar(0, 255, 0), 2, 8, 0);
		line(histImage, Point(binW*(i - 1), histH - cvRound(rHist.at<float>(i - 1))),
			Point(binW*(i), histH - cvRound(rHist.at<float>(i))),
			Scalar(0, 0, 255), 2, 8, 0);
	}

	r_channel_hist_ = rHist;
	g_channel_hist_ = gHist;
	b_channel_hist_ = bHist;

	patch_histogram_ = { rHist,gHist, bHist };
}

void Patch::ComputeEntropy()
{

}

void Patch::ComputeMutualInformationGain()
{

}

vector<vector<float>> Patch::ExtractPixels()
{
	GetPatchCoordinates();
	rows_ = coo_.GetHeight();
	columns_ = coo_.GetWidth();

	vector<vector<float>> tmp(rows_, vector<float>(columns_));

	for (auto i = 0; i < patch_mat_.rows; i++)
	{
		for (auto j = 0; j < patch_mat_.cols; j++)
		{
			const auto item = patch_mat_.at<uchar>(i, j);
			tmp[i][j] = item;
		}
	}
	
	return tmp;
}
