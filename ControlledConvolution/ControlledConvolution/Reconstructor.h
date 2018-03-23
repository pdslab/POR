#pragma once
#ifndef IMAGE_H
#define IMAGE_H
#include <iostream>
#include "stdafx.h"
#include "sample.h"


enum class MeasureType {hist, l1Norm, l2Norm, hammingNorm};

class Reconstructor
{
public:
	static bool CompareDouble(const double i, const double j) { return (i < j); }
	double L1Norm(const Patch &p1, const Patch &p2) const;
	static double L2Norm(const Patch &p1, const Patch &p2);
	static double HammingNorm(const Patch &p1, const Patch& p2);
	static double PSNR(const Patch &p);
	static double Entropy(const Patch &p);
	static double MutualInformation(const Patch& p1, const Patch& p2);
	static double Histogram(const Patch& p1);
	Reconstructor();
	explicit Reconstructor(Sample *s);
	~Reconstructor();

#pragma region operators
	void SortPatches(const Sample *s, MeasureType t);
	void SortPatches(vector<Patch> &v, MeasureType t) const;
	static void Stitch(Sample  *v);
	static void Reconstruct(Sample *s);
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
