#include "stdafx.h"
#include "Sample.h"
#include "static_data.h"
#include <opencv2/imgproc.hpp>

string Sample::ExtractPatch(cv::Mat & patch, const Coordinate & c)
{
	const auto startRow = c.Start()[0];
	const auto endRow = c.End()[0];
	const auto startColumn = c.Start()[1];
	const auto endColumn = c.End()[1];

	if (startRow > endRow || startColumn > endColumn)
	{
		const auto message = "Patch size (w and height) must be greater than zero. start_x: " + to_string(startRow) +
			"end_x: " + to_string(endRow) + "start_y: " + to_string(startColumn) + " end_y: " + to_string(endColumn);
		cout << message << endl;
		throw exception("Patch size (w and height) must be greater than zero.");
	}

	//cout << "Extracting patches ...";
	//common::show(mat_,"");
	const int rowBytes = static_cast<int>((endColumn - startColumn) * mat_.channels() * mat_.elemSize1());
	patch.create(endRow - startRow, endColumn - startColumn, mat_.type());
	auto pixelOriginal = static_cast<const uchar*>(mat_.data) + mat_.step[0] * startRow + mat_.step[1] * startColumn;
	auto pixelPatch = static_cast<uchar*>(patch.data);

	for (auto ri = startRow; ri < endRow; ri++, pixelPatch += patch.step[0], pixelOriginal += mat_.step[0])
	{ // copy row by row
		memcpy(pixelPatch, pixelOriginal, rowBytes);
	}

	//common::show(Patch, "");
	return Common::GeneratePatchName(c);
}

void Sample::ToCvMat(const cv::Size& size, bool round_up_to_nearest_power_of_2)
{
	mat_ = imread(input_file_);
	original_height_ = mat_.size().height;
	original_width_ = mat_.size().width;
	//if(!Common::IsSquareImage(mat_))
	//{
	//cout<< "Resizing to (" << size.width << "," << size.height << ")\n";
	cv::Mat temp;
	cv::resize(mat_, temp, size);
	mat_ = temp;
	//}

	if (!mat_.data)
	{
		cout << "Unable to read image from file, file: " << input_file_ << endl;
		exit(-1);
	}

	auto rows = mat_.rows;
	auto cols = mat_.cols;

	if (round_up_to_nearest_power_of_2 && (!Common::IsPower2(rows) || !Common::IsPower2(cols)))
	{
		cout << "Warn: input diamension is not power of 2.\n";

		//Round up to the next power of 2
		cout << "Rounding up width and height to the next powe of 2\n";
		rows--;
		rows |= rows >> 1;
		rows |= rows >> 2;
		rows |= rows >> 4;
		rows |= rows >> 8;
		rows |= rows >> 16;
		rows++;

		cols--;
		cols |= cols >> 1;
		cols |= cols >> 2;
		cols |= cols >> 4;
		cols |= cols >> 8;
		cols |= cols >> 16;
		cols++;

		cout << "Resizing input to " << rows << "X" << cols << endl;
		cv::Mat output;
		Resize(output, rows,cols);
		mat_.release();
		mat_ = output;
	}

	rows_ = mat_.rows;
	cols_ = mat_.cols;
	height_ = mat_.size().height;
	width_ = mat_.size().width;
	area_ = static_cast<int>(mat_.total());
	size_ = mat_.size();
}

bool Sample::Load()
{
	mat_ = imread(input_file_);

	if (!mat_.data)
	{
		cout << "Unable to read image from file, file: " << input_file_ << endl;
		return false;
	}

	return true;
}

void Sample::DetermineMinimumNumberOfPatchZones(const int &patch_height=0, const int &patch_width=0)
{
	if (height_ < MIN_PATCH_SIZE_Y || width_ < MIN_PATCH_SIZE_X)
	{
		throw("Unable to determine number of Patch zones. Input diamensions don't match the minimum requirement!");
	}

	if (patch_height != 0 && patch_width != 0) {
		minimum_number_of_patches_y_ = height_ / patch_height;
		minimum_number_of_patches_x_ = width_ / patch_width;
	}
	else {
		minimum_number_of_patches_x_ = height_ / MIN_PATCH_SIZE_Y;
		minimum_number_of_patches_y_ = width_ / MIN_PATCH_SIZE_Y;
	}
}

void Sample::DetermineSampleFittness()
{
}

void Sample::GeneratePatchProposals(const cv::Size& size)
{
	if (!Common::IsPower2(size.area()))
	{
		auto message = "Unable to generate Patch proposals. Patch size is not square. input (" + to_string(size.height) + "," + to_string(size.width) + "), patch(" + to_string(height_) + "," + to_string(width_) + ")";
		cerr << message;
		throw(message);
	}

	if (size.height > height_ || size.width > width_)
	{
		auto message = "Unable to generate Patch proposals. Patch has larger dimensions than input. input (" + to_string(size.height) + "," + to_string(size.width) + "), patch(" + to_string(height_) + "," + to_string(width_) + ")";
		cerr << message;
		throw(message);
	}

	Coordinate c;

	for (auto x = 0; x < width_; x += size.width)
	{
		for (auto y = 0; y < height_; y += size.height)
		{
			c.SetStart(x, y);
			c.SetEnd(x + size.width, y + size.height);
			patch_proposal_coordinates_.push_back(c);

			//std::cout << "Successfully added Patch coordinates <" << x << "," << y << "," << x + size.width << "," << y + size.height << ">"<<endl;
		}
	}
}

bool Sample::operator<(const cv::Size& size) const
{
	return size_.height < size.height || size_.width < size.width;
}

void Sample::GetStat() const
{
	cout << "Path     : " << input_file_ << endl;
	cout << "Width    : " << width_ << endl;
	cout << "Height   : " << height_ << endl;
	cout << "Rows     : " << rows_ << endl;
	cout << "Columns  : " << cols_ << endl;
	cout << "Patchs x : " << minimum_number_of_patches_x_ << endl;
	cout << "Patchs y : " << minimum_number_of_patches_y_ << endl;
}

void Sample::SetTemplatePatch(const cv::Mat& p)
{
	template_patch_ = p;
}

void Sample::SaveReconstructedSample(const string format) const
{
	const auto outputFile = RONSTRUCTED_SAMPLES + "\\" + name_ + "_reconed." + format;
	cout << "Saving " << outputFile << endl;
	imwrite(outputFile, reconstructed_output_);
}

void Sample::SetName(const string & name)
{
	const auto lastIndex = name.find_last_of('.');
	name_ = name.substr(0, lastIndex);
}

string Sample::BaseName() const
{
	auto sep = '/';

#ifdef _WIN32
	sep = '\\';
#endif

	const auto i = name_.rfind(sep, name_.length());
	if (i != string::npos)
	{
		return (name_.substr(i + 1, name_.length() - i));
	}

	return ("");
}

void Sample::Resize(cv::Mat& output, const unsigned int & width, const unsigned int & height) const
{
	resize(mat_, output, cv::Size(width, height));
}

vector<Mat> Sample::PachesAsVectorOfMats()
{
	vector<cv::Mat> patches;

	for (const auto& p : sample_patches_sorted_)
	{
		patches.push_back(p.GetMat());
	}

	return patches;
}

void Sample::SaveToDisc(const string& outputDir, const string& format, const bool original)
{
	if (original) throw "Not implemented yet!";

	//cout << "Saving " << sample_patches_sorted_.size() << "patches to disc.";

	for (auto & i : sample_patches_sorted_)
	{
		//cout << "Saving patch .." << i.Name() << endl;
		const auto outputFileName = outputDir + "/" + i.Name();
		Common::SaveImage(i.GetMat(), outputFileName, format);
	}
}