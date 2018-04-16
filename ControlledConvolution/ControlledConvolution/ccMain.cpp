// ControlledConvolution.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <GL/glut.h>
#include <stdio.h>
#include <iterator>
#include "Sample.h"
#include "Common.h"
#include "Patch.h"
#include "Reconstructor.h"
#include <experimental/filesystem>

typedef std::vector<std::string> stringvec;

#pragma region inline
inline bool FileExists(const std::string& name) {
	struct stat buffer {};
	return (stat(name.c_str(), &buffer) == 0);
}
inline string Rename(std::string& file)
{
	auto i = 1;

	while (FileExists(file))
	{
		const auto lastindex = file.find_last_of(".");
		const auto rawname = file.substr(0, lastindex);
		file = rawname + "_" + to_string(i) + ".txt";
		i += 1;
	}

	return file;
}
void ReadDirectory(const std::string& name, stringvec& v)
{
	auto pattern(name);
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind;
	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		do {
			v.push_back(data.cFileName);
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
}
inline vector<string> GetSampleSet(const std::string dirPath)
{
	vector<string> samples;

	for (auto &file : fs::directory_iterator(dirPath))
	{
		std::ostringstream oss;
		oss << file;
		samples.push_back(oss.str());
	}

	return samples;
}

inline void GetSampleSet(const std::string dir, stringvec& v)
{
	ReadDirectory(dir, v);
}

inline bool CreateDirecoty(const std::string& dir)
{
	const fs::path p(dir);

	if (fs::exists(p))
		fs::remove_all(p);

	return fs::create_directories(p);
}

#pragma endregion

#pragma region constants
// constants ////////////////////////
//const char* file_name_default = "img.jpg";
//const char* file_name_default = "cifar_32_32.png";
const char* file_name_default = "retina_256_256.jpg";
//const char* file_name_default = "img_1024_1024.jpg";
const int MAX_NAME_DEFAULT = 1024;
#pragma endregion

static Order DetermineOrder(const int &i)
{
	switch (i)
	{
	case 0:
		return Order::increasing;
	case 1:
		return Order::decreasing;
	case 2:
		return Order::randomShuffle;
	case -1:
		return Order::none;
	default: return Order::unknown;
	}
}

static string ToString(const Order & order)
{
	switch (order)
	{
	case Order::increasing:
		return "increasing";
	case Order::decreasing:
		return "decreasing";
	case Order::none:
		return "none";
	default: return "UnknownOrder";
	}
}

int main(const int argc, char** argv)
{
	const auto sample = "D:\\dev\\NeuralNetworks\\reconstructor\\samples\\cat.jpg";
	auto s = new Sample(sample);
	const cv::Size size(256, 256);
	s->ToCvMat(size);
	auto mat = s->Mat();
	Common::Show(mat,"original");
	const auto sorted = Reconstructor::SortPixels(mat, Order::increasing);
	Common::Show(sorted,"original");

	return -1;
	cout << "Starting ...\n";
	const String keys =
		"{help h usage ?   |      | print this message}"
		"{input_dir i iDir |<none>| directory containing samples}"
		"{measure m        || measure to use for comparison}"
		"{output_dir oDir o|<none>| output directory}"
		"{format f         |jpeg| output format}"
		"{x patch_width pw |8| patch width }"
		"{patch_height ph y   |8| patch height}"
		"{height h         |32| resize input to this size before processing}"
		"{width w          |32| resize input to this size before processing}"
		"{order |-1| ordering of patches during sorting. Options(0=increasing, 1=decreasing, 2=randomShuffle)}";
	CommandLineParser parser(argc, argv, keys);
	parser.about("\nControlled Convoluion (CC) v1.0.0");

	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	if (!parser.check())
	{
		parser.printMessage();
		return -1;
	}

	const auto iDir = parser.get<string>("input_dir");
	const auto oDir = parser.get<string>("output_dir");
	const auto measure = parser.get<string>("measure");
	const auto format = parser.get<string>("format");
	const auto patchWidth = parser.get<int>("patch_width");
	const auto patchHeight = parser.get<int>("patch_height");
	const auto inputHeight = parser.get<int>("height");
	const auto inputWidth = parser.get<int>("width");
	const auto order = parser.get<int>("order");
	auto done = false;

	const fs::path path(iDir);

	if (!exists(path))
	{
		cerr << "Exit code: -1, Supplied input path \"" << iDir << "\"doesn't exist\n";
		parser.printMessage();
		return -1;
	}
	if (patchWidth != patchHeight || inputHeight != inputWidth)
	{
		cerr << "Exit code: -2, This version ony supports square size patches and inputs.\n";
		return -2;
	}

	auto samples = GetSampleSet(iDir);

	if (samples.empty())
	{
		cerr << "Exit code: -3, Directory contains no samples.\n";
		return -3;
	}

	cout << "\n\nCommand line parameters " << endl
		<< "\tDataset directory | " << iDir << endl
		<< "\tOutput directory  | " << oDir << endl
		<< "\tMeasure           | " << measure << endl
		<< "\tOrder				| " << order << endl
		<< "\tWidth		        | " << patchWidth << endl
		<< "\tHeight		    | " << patchHeight << endl
		<< "\tNumber of Samples | " << samples.size() << endl;

	const cv::Size patchSize(patchWidth, patchHeight);
	const cv::Size inputSize(inputWidth, inputHeight);

	cout << "\nContinue ... y (yes) or n (no)?\n";
	char userInput;
	cin >> userInput;

	if (userInput == 'n' || userInput == 'N') exit(-2);
	auto counter = 0;

	cv::TickMeter tm;
	tm.start();
	for (const auto& sample : samples)
	{
		counter++;
		const string title = "Original Image";

		//Read Sample
		auto s = new Sample(sample);
		s->ToCvMat(inputSize);

		while (!done)
		{
			Common::Show(s->Mat(), title);
			cin.ignore();
			done = true;
		}

		cv::TickMeter ts;
		ts.start();

		//STEP 1. Determine the minimum number of Patches ( assuming 8x8 patch is  the smallest patch)
		s->DetermineMinimumNumberOfPatchZones();
		cv::Mat img;

		//s->GetStat();

		//STEP 2. Generate patch proposals and coordinates
		s->GeneratePatchProposals(patchSize);

		//Extract patches
		cout << "Sample " << counter << " 0% [";
		for (const auto& patchCoordinate : s->PatchesCoordinates())
		{
			//STEP 3. Extract the patches
			s->ExtractPatch(img, patchCoordinate);
			Patch p(img, patchCoordinate);
			const auto name = Common::GeneratePatchName(patchCoordinate);
			p.SetName(name);
			p.ToPixel();
			//p.Save(saveOutput, "bmp");

			//STEP 4. Compute standalone image characterstics
			p.ComputeHisogram();

			s->AddPatch(p);
			img.release();
			cout << "#";
		}

		//Sort patches for reconstruction
		auto patches = s->Patches();
		s->SetSamplePatches(patches);
		Reconstructor sampleReconstructor;
		sampleReconstructor.SetSample(s);
		MeasureType mt = {};
		auto o = Order::none;

		if (measure == "l1Norm" || measure == "l1norm") mt = MeasureType::l1Norm;
		else if (measure == "l2norm" || measure == "l2Norm") mt = MeasureType::l2Norm;
		else if (measure == "hamming" || measure == "hamming") mt = MeasureType::hammingNorm;
		else if (measure == "c0e" || measure == "channel0_entropy") mt = MeasureType::channel0Entropy;
		else if (measure == "c1e" || measure == "channel1_entropy") mt = MeasureType::channel1Entropy;
		else if (measure == "c2e" || measure == "channel2_entropy") mt = MeasureType::channel2Entropy;
		else if (measure == "ae" || measure == "average_entropy") mt = MeasureType::averageEntropy;
		else if (measure == "psnr" || measure == "Psnr") mt = MeasureType::psnr;
		else if (measure == "ssim" || measure == "ssim_average") mt = MeasureType::ssimAverage;
		else if (measure == "ssim0" || measure == "channel0_ssim") mt = MeasureType::ssim0;
		else if (measure == "ssim1" || measure == "channel1_ssim") mt = MeasureType::ssim1;
		else if (measure == "ssim2" || measure == "channel2_ssim") mt = MeasureType::ssim2;
		else
		{
			cerr << "Exit code: -4, Unknown measure type. Aborting ...\n";
			return -4;
		}

		o = DetermineOrder(order);
		if (sampleReconstructor.SortPatches(patches, mt, o))
		{
			s->SetSortedSamplePatches(patches);
			const auto outputDir = oDir + "\\" + measure + "\\" + ToString(o) + "\\" + s->BaseName();
			CreateDirecoty(outputDir);
			s->SaveToDisc(outputDir, format);
		}
		else { throw exception("SortPatches failed, unable to save sorted patches"); }
		ts.stop();

		cout << "] 100%, Time = " << ts.getTimeMilli() << " ms\n";

		delete s;
	}

	tm.stop();

	cout << "Done processing " << samples.size() << "samples. Time: " << tm.getTimeSec() << " sec.";

	return 0;
}