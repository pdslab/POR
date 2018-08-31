#include "stdafx.h"
#include "ImageMeasure.h"


ImageMeasure::ImageMeasure(const cv::Mat mat)
{
	_image = mat;
}

ImageMeasure::~ImageMeasure()
{
}

void ImageMeasure::CalculateEntropy() 
{
	//const auto image = p.GetMat();

	std::vector<cv::Mat> channels;

	cv::split(_image, channels);

	auto histSize = 255;

	float range[] = { 0,256 };

	const float* histRange = { range };

	const auto uniform = true;
	const auto accumulate = false;

	cv::Mat hist0, hist1, hist2;

	//Calculate histogram of each color channel
	cv::calcHist(&channels[0], 1, nullptr, cv::Mat(), hist0, 1, &histSize, &histRange, uniform, accumulate);
	cv::calcHist(&channels[1], 1, nullptr, cv::Mat(), hist1, 1, &histSize, &histRange, uniform, accumulate);
	cv::calcHist(&channels[2], 1, nullptr, cv::Mat(), hist2, 1, &histSize, &histRange, uniform, accumulate);

	hist0 = mean(hist0);
	hist1 = mean(hist1);
	hist2 = mean(hist2);

	float f0 = 0, f1 = 0, f2 = 0;

	for (auto i = 0; i < histSize; i++)
	{
		f0 += hist0.at<float>(i);
		f1 += hist1.at<float>(i);
		f2 += hist2.at<float>(i);
	}

	cv::Scalar e;
	e.val[0] = 0;
	e.val[1] = 0;
	e.val[2] = 0;

	for (auto i = 0; i < histSize; i++)
	{
		const auto p0 = abs(hist0.at<float>(i)) / f0;
		const auto p1 = abs(hist1.at<float>(i)) / f1;
		const auto p2 = abs(hist2.at<float>(i)) / f2;

		if (p0 != 0)
			e.val[0] += -p0*log10(p0);
		if (p1 != 0)
			e.val[1] += -p1*log10(p1);
		if (p2 != 0)
			e.val[2] += -p2*log10(p2);
	}

	SetEntropy(e);
}
