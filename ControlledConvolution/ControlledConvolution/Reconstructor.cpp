#include "stdafx.h"
#include "Reconstructor.h"
#include "Common.h"
#include <iostream>
#include <opencv2/stitching.hpp>

double Reconstructor::L1Norm(const Patch& p1, const Patch& p2) const
{
	return cv::norm(p1.GetMat(), p2.GetMat(), NORM_L1);
}

double Reconstructor::L2Norm(const Patch& p1, const Patch& p2)
{
	return cv::norm(p1.GetMat(), p2.GetMat(), NORM_L2);
}

double Reconstructor::HammingNorm(const Patch& p1, const Patch& p2)
{
	return cv::norm(p1.GetMat(), p2.GetMat(), NORM_HAMMING);
}

double Reconstructor::PeakSignalToNoiseRatio(const Patch &p1, const Patch &p2)
{
	cv::Mat s1;

	absdiff(p1.GetMat(), p2.GetMat(), s1); //|p1-p2|
	s1.convertTo(s1, CV_32F);

	s1 = s1.mul(s1); //|p1-p2|^2

	const auto s = sum(s1);

	const auto sse = s.val[0] + s.val[1] + s.val[2];
	//if (sse <= 1e-10) return 0; //Too small return 0

	const auto mse = sse / static_cast<double>(p1.GetMat().channels()) * p1.GetMat().total(); //mean squred error 
	const auto psnr = 10.0*log10((255 * 255) / mse); //peaksignal to noise ratio In case of a simple single byte image per pixel per channel this is 255

	return psnr;
}

cv::Scalar Reconstructor::Entropy(const Patch& p)
{
	const auto image = p.GetMat();

	std::vector<cv::Mat> channels;

	cv::split(image, channels);

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

	for(auto i=0; i <histSize;i++)
	{
		f0 += hist0.at<float>(i);
		f1 += hist1.at<float>(i);
		f2 += hist2.at<float>(i);
	}

	cv::Scalar e;
	e.val[0] = 0;
	e.val[1] = 0;
	e.val[2] = 0;

	for (auto i=0; i < histSize; i++)
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

	return e;
}

double Reconstructor::JointEntropy(const Patch & p1, const Patch & p2)
{
	return 0.0;
}

double Reconstructor::MutualInformation(const Patch & p1, const Patch & p2)
{
	return 0.0;
}

cv::Scalar Reconstructor::StructuralSimilarityIndex(const Patch& p1, const Patch& p2)
{
	const double C1 = 6.5025, C2 = 58.5225;
	const auto d = CV_32F;

	//Init 
	Mat i1, i2;
	p1.GetMat().convertTo(i1, d);
	p2.GetMat().convertTo(i2, d);
	const auto i1_squred = i1.mul(i1);
	const auto i2_squred = i2.mul(i2);
	const auto i1_i2 = i1.mul(i2); // i1 * i2
	//Preliminary computing 
	Mat mu1, mu2;
	
	GaussianBlur(i1, mu1, Size(11, 11), 1.5);
	GaussianBlur(i2, mu2, Size(11, 11), 1.5);

	const Mat mu1_squared = mu1.mul(mu1);
	const Mat mu2_squared = mu2.mul(mu2);
	const Mat mu1_mu2= mu1.mul(mu2);

	Mat sigma1_2, sigma2_2;
	const Mat sigma12;
	GaussianBlur(i1_squred, sigma1_2, Size(11, 11), 1.5);
	sigma1_2 -= mu1_squared;

	GaussianBlur(i2_squred, sigma2_2, Size(11, 11), 1.5);
	sigma2_2 -= mu2_squared;

	//Formula 
	Mat t1 = 2 * mu1_mu2 + C1;
	Mat t2 = 2 * sigma12 + C2;
	const Mat t3 = t1.mul(t2); // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

	t1 = mu1_squared + mu2_squared + C1;
	t2 = sigma1_2 + sigma2_2 + C2; 
	t1 = t1.mul(t2);  // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

	Mat ssimMap;
	divide(t3, t1, ssimMap); // ssim_map =  t3./t1;

	auto mssim = mean(ssimMap); // mssim = average of ssim map

	return mssim;
}

Reconstructor::Reconstructor() : sample_(nullptr)
{
}

Reconstructor::Reconstructor(Sample* s)
{
	sample_ = s;
	patch_zero_ = s->Patches()[0];
}

Reconstructor::~Reconstructor()
= default;

void Reconstructor::SortPatches(const Sample *s, const MeasureType t)
{
	auto patches = s->Patches();
	SetPatchZero(patches[0]);
	if (t == MeasureType::l1Norm)
		if (t == MeasureType::l2Norm) std::sort(patches.begin(), patches.end(), L2Norm);
}

bool Reconstructor::SortPatches(vector<Patch>& v, const MeasureType t) const
{
	auto p0 = v[0];
	v[0].SetName("0");
	auto lastSmallestNorm = 10e100;
	Patch mostSimiarPatch;
	auto norm1 = 0.0, norm2 = 0.0, psnr1=0.0, psnr2=0.0;

	//cout << "Sorting patches, size = "<<v.size() << endl;

	if (t == MeasureType::averageEntropy)
	{
		std::sort(v.begin(), v.end(), CompareUsingAverageEntropy);
		for (auto i = 0; i < v.size(); i++)
			v[i].SetName(to_string(i));

		return true;
	}
	if (t == MeasureType::channel0Entropy)
	{
		std::sort(v.begin(), v.end(), CompareUsingChannel0Entropy);
		for (auto i = 0; i < v.size(); i++)
			v[i].SetName(to_string(i));

		return true;
	}

	if (t == MeasureType::channel1Entropy)
	{
		std::sort(v.begin(), v.end(), CompareUsingChannel1Entropy);
		for (auto i = 0; i < v.size(); i++)
			v[i].SetName(to_string(i));

		return true;
	}

	if (t == MeasureType::channel2Entropy)
	{
		std::sort(v.begin(), v.end(), CompareUsingChannel2Entropy);
		for (auto i = 0; i < v.size(); i++)
			v[i].SetName(to_string(i));

		return true;
	}

	if (t == MeasureType::l1Norm || t == MeasureType::l2Norm || t == MeasureType::hammingNorm || t==MeasureType::psnr)
	{
		for (auto i = 0; i <= v.size() - 1; i++)
		{
			for (auto j = i + 1; j < v.size() && j + 1 <= v.size() - 1; j++)
			{
				switch (t)
				{
				case MeasureType::l1Norm:
					norm1 = L1Norm(v[i], v[j]);
					norm2 = L1Norm(v[j], v[j + 1]);
					if (norm1 != 0 && norm2 != 0)
					{
						if (norm1 > norm2)
						{
							lastSmallestNorm = norm2;
							v[j + 1].SetName(to_string(j));
							std::swap(v[j], v[j + 1]);
							p0 = v[j + 1];
						}
						else if (norm1 < norm2)
						{
							p0 = v[j];
							v[j].SetName(to_string(j));
						}
					}
					break;
				case MeasureType::l2Norm:
					norm1 = L2Norm(v[i], v[j]);
					norm2 = L2Norm(v[j], v[j + 1]);
					if (norm1 != 0 && norm2 != 0)
					{
						if (norm1 > norm2)
						{
							lastSmallestNorm = norm2;
							v[j + 1].SetName(to_string(j));
							std::swap(v[j], v[j + 1]);
							p0 = v[j + 1];
						}
						else if (norm1 < norm2)
						{
							p0 = v[j];
							v[j].SetName(to_string(j));
						}
					}
					break;
				case MeasureType::hammingNorm:
					norm1 = HammingNorm(v[i], v[j]);
					norm2 = HammingNorm(v[j], v[j + 1]);
					break;
				case MeasureType::psnr:
					psnr1 = PeakSignalToNoiseRatio(v[i], v[j]);
					psnr2 = PeakSignalToNoiseRatio(v[j], v[j+1]);
					if (psnr1 != 0 && psnr2!= 0)
					{
						if (psnr1 > psnr2)
						{
							lastSmallestNorm = psnr2;
							v[j + 1].SetName(to_string(j));
							std::swap(v[j], v[j + 1]);
							p0 = v[j + 1];
						}
						else if (psnr1 < psnr2)
						{
							p0 = v[j];
							v[j].SetName(to_string(j));
						}
					}
					break;
				case MeasureType::ssimAverage:

				default: break;
				}
			}
		}

		v[v.size() - 1].SetName(to_string(v.size() - 1));

		return true;
	}
	throw exception("SortPatches -> Unknown measure type!");
}

void Reconstructor::Stitch(Sample* s)
{
	const auto mode = Stitcher::PANORAMA;
	auto patches = s->PachesAsVectorOfMats();

	Mat output;
	auto stitcher = Stitcher::create(mode, true);
	std::cout << patches.size();

	const auto status = stitcher->stitch(patches, output);

	if (status != Stitcher::OK)
	{
		cout << "Can't stitch images, error code = " << int(status) << endl;
		throw exception("Can't stitch images, error code ");
	}

	Mat resized;
	cv::resize(output, resized, s->Size());
	s->SetStitchedOutput(resized);
}

void Reconstructor::Reconstruct(Sample* s)
{
	auto patches = s->PachesAsVectorOfMats();

	const auto reconstructedOutput = Common::ConcatenateMat(patches);

	Common::Show(reconstructedOutput, "");
}

bool Reconstructor::CompareUsingAverageEntropy(const Patch& p1, const Patch& p2)
{
	auto entropy1 = Entropy(p1);
	auto entropy2 = Entropy(p2);
	const float e1 = (entropy1[0] + entropy1[1] + entropy1[2]) / 3.0;
	const float e2 = (entropy2[0] + entropy2[1] + entropy2[2]) / 3.0;

	return e1 > e2;
}

bool Reconstructor::CompareUsingChannel0Entropy(const Patch& p1, const Patch& p2)
{
	const auto entropy1 = Entropy(p1)[0];
	const auto entropy2 = Entropy(p2)[0];

	return entropy1 > entropy2;
}

bool Reconstructor::CompareUsingChannel1Entropy(const Patch & p1, const Patch & p2)
{
	const auto entropy1 = Entropy(p1)[1];
	const auto entropy2 = Entropy(p2)[1];

	return entropy1 > entropy2;;
}

bool Reconstructor::CompareUsingChannel2Entropy(const Patch & p1, const Patch & p2)
{
	const auto entropy1 = Entropy(p1)[2];
	const auto entropy2 = Entropy(p2)[2];

	return entropy1 > entropy2;
}

bool Reconstructor::GreaterThan(const double i, const double j)
{
	return (i > j);
}

bool Reconstructor::GreaterThan(const float i, const float j)
{
	return (i > j);
}

void Reconstructor::SetSample(Sample* s)
{
	sample_ = s;
}

void Reconstructor::SetPatchZero(const Patch& p)
{
	patch_zero_ = p;
}

Patch Reconstructor::GetPatchZero() const
{
	return patch_zero_;
}

void Reconstructor::VisualizePatches(const int& numberToVisualize) const
{
	//(void)((!!(howMany <= 12)) || (_wassert(_CRT_WIDE("howMany <= 12"), _CRT_WIDE(__FILE__), static_cast<unsigned>(__LINE__)), 0));

	auto p = sample_->Patches();
	std::string  title;

	switch (numberToVisualize)
	{
	case 1:
		Common::Show(p[0].GetMat(), "Patch 0");
		return;
	case 2:
		title = p[0].GetPatchCoordinates().ToStr() + "_" + p[1].GetPatchCoordinates().ToStr();
		Common::ShowMultiple(title, 2, p[0].GetMat(), p[1].GetMat());
	case 3:
		title = p[0].GetPatchCoordinates().ToStr() + "_" + p[1].GetPatchCoordinates().ToStr() + p[2].GetPatchCoordinates().ToStr();
		Common::ShowMultiple(title, 3, p[0].GetMat(), p[1].GetMat(), p[2].GetMat());
		break;
	case 4:
		Common::ShowMultiple(title, 4, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat());
		break;
	case 5:
		Common::ShowMultiple(title, 5, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat(), p[4].GetMat());
		break;
	case 6:
		Common::ShowMultiple(title, 6, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat(), p[4].GetMat(),
			p[5].GetMat());
		break;
	case 7:
		Common::ShowMultiple(title, 7, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat(), p[4].GetMat(),
			p[5].GetMat(), p[6].GetMat());
		break;
	case 8:
		Common::ShowMultiple(title, 8, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat(), p[4].GetMat(),
			p[5].GetMat(), p[6].GetMat(), p[7].GetMat());
		break;
	case 9:
		Common::ShowMultiple(title, 9, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat(), p[4].GetMat(),
			p[5].GetMat(), p[6].GetMat(), p[7].GetMat(), p[8].GetMat());
		break;
	case 10:
		Common::ShowMultiple(title, 10, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat(), p[4].GetMat(),
			p[5].GetMat(), p[6].GetMat(), p[7].GetMat(), p[8].GetMat(), p[9].GetMat());
		break;
	case 11:
		Common::ShowMultiple(title, 11, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat(), p[4].GetMat(),
			p[5].GetMat(), p[6].GetMat(), p[7].GetMat(), p[8].GetMat(), p[9].GetMat(), p[10].GetMat());
		break;
	case 12:
		Common::ShowMultiple(title, 12, p[0].GetMat(), p[1].GetMat(), p[2].GetMat(), p[3].GetMat(), p[4].GetMat(),
			p[5].GetMat(), p[6].GetMat(), p[7].GetMat(), p[8].GetMat(), p[9].GetMat(), p[10].GetMat(), p[11].GetMat());
		break;
	default: break;
	}
}
