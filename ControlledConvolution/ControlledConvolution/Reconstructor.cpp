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

double Reconstructor::PSNR(const Patch & p)
{
	return 0.0;
}

double Reconstructor::Entropy(const Patch & p)
{
	return 0.0;
}

double Reconstructor::MutualInformation(const Patch & p1, const Patch & p2)
{
	return 0.0;
}

double Reconstructor::Histogram(const Patch & p1)
{
	return 0.0;
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
{
}

void Reconstructor::SortPatches(const Sample *s, const MeasureType t)
{
	auto patches = s->Patches();
	SetPatchZero(patches[0]);
	if (t == MeasureType::l1Norm)
		if (t == MeasureType::l2Norm) std::sort(patches.begin(), patches.end(), L2Norm);
}

void Reconstructor::SortPatches(vector<Patch>& v, const MeasureType t) const
{
	auto p0 = v[0];
	v[0].SetName("0");
	auto lastSmallestNorm = 10e100;
	Patch mostSimiarPatch;
	auto norm1 = 0.0, norm2 = 0.0;

	//cout << "Sorting patches, size = "<<v.size() << endl;

	for (auto i = 0; i <= v.size() - 1; i++)
	{
		for (auto j = i + 1; j < v.size() && j + 1 <= v.size() - 1; j++)
		{
			switch (t)
			{
			case MeasureType::l1Norm:
				norm1 = L1Norm(v[i], v[j]);
				norm2 = L1Norm(v[j], v[j + 1]);
				break;
			case MeasureType::l2Norm:
				norm1 = L2Norm(v[i], v[j]);
				norm2 = L2Norm(v[j], v[j + 1]);
				break;
			case MeasureType::hammingNorm:
				norm1 = HammingNorm(v[i], v[j]);
				norm2 = HammingNorm(v[j], v[j + 1]);
				break;
			default: break;
			}
			/*const auto currentNorm = L2Norm(v[i], v[j]);
			const auto currentNorm2 = L2Norm(v[i], v[j + 1]);*/

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
		}
	}

	v[v.size() - 1].SetName(to_string(v.size() - 1));
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
		throw "Can't stitch images, error code ";
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

void Reconstructor::VisualizePatches(const int& howMany) const
{
	//(void)((!!(howMany <= 12)) || (_wassert(_CRT_WIDE("howMany <= 12"), _CRT_WIDE(__FILE__), static_cast<unsigned>(__LINE__)), 0));

	auto p = sample_->Patches();
	std::string  title;

	switch (howMany)
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