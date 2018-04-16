#pragma once
#ifndef IMAGE_H
#define IMAGE_H
#include <iostream>
#include "stdafx.h"
#include "sample.h"

/// <summary>
/// Similarity Measures
///psnr - peak signal to noise ratio
///mi - mutual information
///ssim - stuctural similarity index
///ji - joint entropy
/// </summary>
enum class MeasureType
{
	hist, l1Norm, l2Norm,
	hammingNorm,
	channel0Entropy,
	channel1Entropy,
	channel2Entropy,
	averageEntropy, ji,
	psnr, mi, pixel,
	ssimAverage, ssim0,
	ssim1, ssim2
};

enum class Order { decreasing, increasing, randomShuffle, none, unknown };

class Reconstructor  // NOLINT
{
public:
	/// <summary>
	/// Computes L1 norm between two patches
	///𝑆= ∑_(𝑖=0)^(𝑛=𝑡𝑜𝑡𝑎𝑙 𝑝𝑖𝑥𝑒𝑙𝑠)|𝑇_𝑖−𝑓(𝑅_𝑖)|
	/// </summary>
	/// <param name="p1">The p1.</param>
	/// <param name="p2">The p2.</param>
	/// <returns></returns>
	double L1Norm(const Patch &p1, const Patch &p2) const;
	/// <summary>
	/// Copmutes the L2 norm between two patches
	///𝑆=∑_(𝑖=0)^𝑛〖(𝑇_𝑖−𝑓(𝑅_𝑖 ))〗^2
	/// </summary>
	/// <param name="p1">patch 1.</param>
	/// <param name="p2">patch 2.</param>
	/// <returns></returns>
	static double L2Norm(const Patch &p1, const Patch &p2);
	/// <summary>
	/// Computes Hamming norm between two patches .
	/// </summary>
	/// <param name="p1">patch 1.</param>
	/// <param name="p2">patch 2.</param>
	/// <returns></returns>
	static double HammingNorm(const Patch &p1, const Patch& p2);
	/// <summary>
	/// Computes PSNR between two patches.
	///𝑀𝑆𝐸=  1/(𝑐∗𝑖∗𝑗) ∑〖(𝐼1−𝐼2)〗^2
	///𝑃𝑆𝑁𝑅 = 10.log_10⁡〖max_𝐼⁡2 / 𝑀𝑆𝐸〗
	/// </summary>
	/// <param name="p1">The p1.</param>
	/// <param name="p2">The p2.</param>
	/// <returns></returns>
	static double PeakSignalToNoiseRatio(const Patch &p1, const Patch &p2);
	/// <summary>
	/// Computes the Entropy of a given patch.
	///𝐻=−∑_(𝑘=0)^(𝑀−1)▒〖𝑝_𝑘 log⁡(𝑝_𝑘)〗
	/// </summary>
	/// <param name="p">The p.</param>
	/// <returns></returns>
	static cv::Scalar Entropy(const Patch& p);
	/// <summary>
	/// Computes the joint entropy of patches p1 and p2
	///𝐻(𝐴,𝐵)=−∑_(𝑎,𝑏)〖𝑝_𝑎𝑏 log⁡(𝑝_𝑎𝑏)〗
	/// </summary>
	/// <param name="p1">The p1.</param>
	/// <param name="p2">The p2.</param>
	/// <returns></returns>
	static double JointEntropy(const Patch& p1, const Patch& p2);
	/// <summary>
	/// Mutuals the information.
	/// </summary>
	/// <param name="p1">The p1.</param>
	/// <param name="p2">The p2.</param>
	/// <returns></returns>
	static double MutualInformation(const Patch& p1, const Patch& p2);
	/// <summary>
	/// Measures the structural similarity index (SSIM) of of the two patches
	/// </summary>
	/// <param name="p1">The p1.</param>
	/// <param name="p2">The p2.</param>
	/// <returns></returns>
	static cv::Scalar StructuralSimilarityIndex(const Patch& p1, const Patch& p2);

#pragma region constructors
	Reconstructor();
	explicit Reconstructor(Sample *s);
	~Reconstructor();
#pragma endregion

#pragma region operators
	void SortPatches(const Sample *s, MeasureType t);
	//bool SortPatches(vector<Patch>& v, const MeasureType t, const Order& o) const;
	bool SortPatches(vector<Patch>& v, MeasureType t, const Order &order) const;
	static bool SortPixels(Patch* in, const Order& order);
	static cv::Mat SortPixels(cv::Mat &mat, const Order& order);
	static void Stitch(Sample  *s);
	static void Reconstruct(Sample *s);
	static bool AverageEntropy(const Patch& p1, const Patch& p2, Order& order);
	static bool AverageEntropyAscending(const Patch& p1, const Patch& p2);
	static bool AverageEntropyDescending(const Patch& p1, const Patch& p2);
	static bool Channel0Entropy(const Patch& p1, const Patch& p2, Order& order);
	static bool Channel0EntropyAscending(const Patch& p1, const Patch& p2);
	static bool Channel0EntropyDescending(const Patch& p1, const Patch& p2);
	static bool Channel1Entropy(const Patch& p1, const Patch& p2, Order& order);
	static bool Channel1EntropyAscending(const Patch& p1, const Patch& p2);
	static bool Channel1EntropyDescending(const Patch& p1, const Patch& p2);
	static bool Channel2Entropy(const Patch& p1, const Patch& p2, Order& order);
	static bool Channel2EntropyAscending(const Patch& p1, const Patch& p2);
	static bool Channel2EntropyDescending(const Patch& p1, const Patch& p2);
	static bool GreaterThan(const double i, const double j);
	static bool GreaterThan(const float i, const float j);
	static bool LessThan(const double i, const double j) { return (i < j); }
	static bool LessThan(const float i, const float j) { return (i < j); }
#pragma endregion
#pragma region setters and getters
	void SetSample(Sample* s);
	void SetPatchZero(const Patch& p);
	Patch GetPatchZero() const;
#pragma endregion

#pragma region utils
	void VisualizePatches(const int& numberToVisualize = 12) const;
#pragma endregion

private:
	Sample* sample_;
	Patch patch_zero_;
};
#endif
