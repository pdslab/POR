#pragma once
#ifndef IMAGE_H
#define IMAGE_H
#include <iostream>
#include "stdafx.h"
#include "sample.h"


enum class MeasureType {hist, l1Norm, l2Norm, hammingNorm,channel0Entropy, channel1Entropy, channel2Entropy, averageEntropy, psnr,mi};

class Reconstructor
{
public:
	static bool CompareDouble(const double i, const double j) { return (i < j); }
	double L1Norm(const Patch &p1, const Patch &p2) const;
	static double L2Norm(const Patch &p1, const Patch &p2);
	static double HammingNorm(const Patch &p1, const Patch& p2);
	static double PeakSignalToNoiseRatio(const Patch &p);
	static cv::Scalar Entropy(const Patch& p);
	static double MutualInformation(const Patch& p1, const Patch& p2);
	static double Histogram(const Patch& p1);
	Reconstructor();
	explicit Reconstructor(Sample *s);
	~Reconstructor();

#pragma region operators
	void SortPatches(const Sample *s, MeasureType t);
	bool SortPatches(vector<Patch>& v, MeasureType t) const;
	void SortPatches(vector<Patch> &v, MeasureType t, bool verbos) const;
	static void Stitch(Sample  *v);
	static void Reconstruct(Sample *s);
	static bool CompareUsingAverageEntropy(const Patch& p1, const Patch& p2);
	static bool CompareUsingChannel0Entropy(const Patch& p1, const Patch& p2);
	static bool CompareUsingChannel1Entropy(const Patch& p1, const Patch& p2);
	static bool CompareUsingChannel2Entropy(const Patch& p1, const Patch& p2);
#pragma endregion
#pragma region setters and getters
	void SetSample(Sample *s) { sample_ = s; }
	void SetPatchZero(const Patch &p) { patch_zero_ = p; }
	Patch GetPatchZero() const { return patch_zero_; }
#pragma endregion

#pragma region utils
	void VisualizePatches(const int& number_to_visualize = 12) const;
#pragma endregion

private:
	Sample* sample_;
	Patch patch_zero_;
};
#endif
