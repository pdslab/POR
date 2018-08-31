#pragma once
#include "Reconstructor.h"

class ImageMeasure
{
public:
	explicit ImageMeasure(const cv::Mat mat);
	~ImageMeasure();
	void CalculateEntropy();
	void SetEntropy(const cv::Scalar e) { _entropy = e; }
	double Channel0Entropy() { return _entropy[0]; }
	double Channel1Entropy() { return _entropy[1]; }
	double Channel2Entropy() { return _entropy[2]; }
	double AverageEntropy() { return (_entropy[0] + _entropy[1] + _entropy[3]) / 3.0; }
private:
	cv::Mat _image;
	cv::Scalar _entropy;
};

