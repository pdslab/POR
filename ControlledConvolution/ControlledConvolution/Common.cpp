#include "stdafx.h"
#include "Common.h"
#include <fstream>
#include <opencv2/stitching.hpp>

using namespace cv;

string Common::GeneratePatchName(const int start_row, const int end_row, const int start_col, const int end_col)
{
	return to_string(start_row) + "." + to_string(end_row) + "_" + to_string(start_col) + "." + to_string(end_col);
}

string Common::GeneratePatchName(const Coordinate& c)
{
	/*
	 * A Patch name is its (x0,y0)_(x1,y1)
	 */
	return GeneratePatchName(c.Start()[0], c.Start()[1], c.End()[0], c.End()[1]);
}

bool Common::IsSquareImage(const Mat& mat)
{
	if (mat.rows != mat.cols) return false;

	return true;
}

bool Common::IsPower2(const int& number)
{
	return (number & (number - 1)) == 0;
}

void Common::Show(const Mat &image, const string& title = "")
{
	if (!image.data) // Check for invalid Sample
	{
		cout << "Could not open or find the image" << endl;
		return;
	}
	Mat resized;
	const cv::Size size(256, 256);
	cv::resize(image, resized, size);

	namedWindow(title, WINDOW_AUTOSIZE);// Create a window for display.
	imshow(title, resized); // Show our image inside it.

	waitKey(0);

	system("pause");
}

void Common::ShowMultiple(const string& title, const int nArgs, ...)
{
	int size;
	int i;
	int m, n;

	// w - Maximum number of images in a row
	// h - Maximum number of images in a column
	int w, h;

	// If the number of arguments is lesser than 0 or greater than 12
	// return without displaying
	if (nArgs <= 0) {
		printf("Number of arguments too small....\n");
		return;
	}
	else if (nArgs > 14) {
		printf("Number of arguments too large, can only handle maximally 12 images at a time ...\n");
		return;
	}
	// Determine the size of the image,
	// and the number of rows/cols
	// from number of arguments
	else if (nArgs == 1) {
		w = h = 1;
		size = 300;
	}
	else if (nArgs == 2) {
		w = 2; h = 1;
		size = 300;
	}
	else if (nArgs == 3 || nArgs == 4) {
		w = 2; h = 2;
		size = 300;
	}
	else if (nArgs == 5 || nArgs == 6) {
		w = 3; h = 2;
		size = 200;
	}
	else if (nArgs == 7 || nArgs == 8) {
		w = 4; h = 2;
		size = 200;
	}
	else if (nArgs == 9)
	{
		w = 3; h = 3;
		size = 200;
	}
	else if (nArgs == 10)
	{
		w = 5; h = 2;
		size = 200;
	}
	else if (nArgs == 11)
	{
		w = 5; h = 3;
		size = 200;
	}
	else if (nArgs == 12)
	{
		w = 4; h = 3;
		size = 200;
	}

	// Create a new 3 channel image
	const Mat dispImage = Mat::zeros(Size(100 + size*w, 60 + size*h), CV_8UC3);

	// Used to get the arguments passed
	va_list args;
	va_start(args, nArgs);

	// Loop for nArgs number of arguments
	for (i = 0, m = 20, n = 20; i < nArgs; i++, m += (20 + size)) {
		// Get the Pointer to the IplImage
		auto img = va_arg(args, Mat);

		// Check whether it is NULL or not
		// If it is NULL, release the image, and return
		if (img.empty()) {
			printf("Invalid arguments");
			return;
		}

		// Find the width and height of the image
		auto x = img.cols;
		auto y = img.rows;

		// Find whether height or width is greater in order to resize the image
		const auto max = (x > y) ? x : y;

		// Find the scaling factor to resize the image
		const auto scale = static_cast<float>(static_cast<float>(max) / size);

		// Used to Align the images
		if (i % w == 0 && m != 20) {
			m = 20;
			n += 20 + size;
		}

		// Set the image ROI to display the current image
		// Resize the input image and copy the it to the Single Big Image
		const Rect roi(m, n, static_cast<int>(x / scale), static_cast<int>(y / scale));
		Mat temp; resize(img, temp, Size(roi.width, roi.height));
		temp.copyTo(dispImage(roi));
	}

	// Create a new window, and show the Single Big Image
	namedWindow(title, 1);
	imshow(title, dispImage);
	waitKey();

	// End the number of arguments
	va_end(args);
}

void Common::PrintVector(const vector<vector<float>>& vec)
{
	for (auto row = vec.begin(); row != vec.end(); ++row)
	{
		for (auto col = row->begin(); col != row->end(); ++col)
		{
			cout << *col << " ";
		}
		cout << endl;
	}
}

void Common::WriteToFile(const vector<vector<float>>& vec, const string& file)
{
	cout << "Entering write_to_file ... filename = " << file << endl;
	ofstream output;

	const auto f = PATCH_PIXELS + "\\" + file + ".txt";
	output.open(f, fstream::out);

	for (auto row = vec.begin(); row != vec.end(); ++row)
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

void Common::Resize(const Mat & input, Mat & output, const unsigned int & width, const unsigned int & height)
{
}

Mat ConcatenateMatC(vector<Mat> &vec)
{
	return {};
}

void Common::ReadCifar10(Mat &trainX, Mat &testX, Mat &trainY, Mat &testY)
{
	auto filename = CIFAR10 + "/cifar-10-batches-bin/data_batch_1.bin";
	vector<Mat> batch1;
	Mat label1 = Mat::zeros(1, 10000, CV_64FC1);
	ReadBatch(filename, batch1, label1);

	filename = CIFAR10 + "/cifar-10-batches-bin/data_batch_2.bin";
	vector<Mat> batch2;
	Mat label2 = Mat::zeros(1, 10000, CV_64FC1);
	ReadBatch(filename, batch2, label2);

	filename = CIFAR10 + "/cifar-10-batches-bin/data_batch_3.bin";
	vector<Mat> batch3;
	Mat label3 = Mat::zeros(1, 10000, CV_64FC1);
	ReadBatch(filename, batch3, label3);

	filename = CIFAR10 + "/cifar-10-batches-bin/data_batch_4.bin";
	vector<Mat> batch4;
	Mat label4 = Mat::zeros(1, 10000, CV_64FC1);
	ReadBatch(filename, batch4, label4);

	filename = CIFAR10 + "/cifar-10-batches-bin/data_batch_5.bin";
	vector<Mat> batch5;
	Mat label5 = Mat::zeros(1, 10000, CV_64FC1);
	ReadBatch(filename, batch5, label5);

	filename = CIFAR10 + "/cifar-10-batches-bin/test_batch.bin";
	vector<Mat> batcht;
	Mat labelt = Mat::zeros(1, 10000, CV_64FC1);
	ReadBatch(filename, batcht, labelt);

	Mat mt1 = ConcatenateMat(batch1);
	Mat mt2 = ConcatenateMat(batch2);
	Mat mt3 = ConcatenateMat(batch3);
	Mat mt4 = ConcatenateMat(batch4);
	Mat mt5 = ConcatenateMat(batch5);
	Mat mtt = ConcatenateMat(batcht);

	Rect roi = Rect(mt1.cols * 0, 0, mt1.cols, trainX.rows);
	Mat subView = trainX(roi);
	mt1.copyTo(subView);
	roi = Rect(label1.cols * 0, 0, label1.cols, 1);
	subView = trainY(roi);
	label1.copyTo(subView);

	roi = Rect(mt1.cols * 1, 0, mt1.cols, trainX.rows);
	subView = trainX(roi);
	mt2.copyTo(subView);
	roi = Rect(label1.cols * 1, 0, label1.cols, 1);
	subView = trainY(roi);
	label2.copyTo(subView);

	roi = Rect(mt1.cols * 2, 0, mt1.cols, trainX.rows);
	subView = trainX(roi);
	mt3.copyTo(subView);
	roi = Rect(label1.cols * 2, 0, label1.cols, 1);
	subView = trainY(roi);
	label3.copyTo(subView);

	roi = Rect(mt1.cols * 3, 0, mt1.cols, trainX.rows);
	subView = trainX(roi);
	mt4.copyTo(subView);
	roi = Rect(label1.cols * 3, 0, label1.cols, 1);
	subView = trainY(roi);
	label4.copyTo(subView);

	roi = Rect(mt1.cols * 4, 0, mt1.cols, trainX.rows);
	subView = trainX(roi);
	mt5.copyTo(subView);
	roi = Rect(label1.cols * 4, 0, label1.cols, 1);
	subView = trainY(roi);
	label5.copyTo(subView);

	mtt.copyTo(testX);
	labelt.copyTo(testY);
}

Mat Common::ConcatenateMat(vector<Mat>& vec)
{
	Show(vec[0], "");
	if (vec.size() < 2) { throw("Unable to stitch images. Need patches > 2"); }

	vector<uchar> array;

	for (auto i = 0; i < vec.size(); i++)
	{
		if (vec[i].data && vec[i].isContinuous())
		{
			array.assign(vec[i].datastart, vec[i].dataend);
		}
	}

	Mat tmp(Size(256, 256), CV_16UC1);

	memcpy(tmp.data, &array, array.size());

	return tmp;
}

void Common::ReadBatch(const string filename, vector<Mat>& vec, Mat & label)
{
	ifstream file(filename, ios::binary);

	if (file.is_open())
	{
		const auto numberOfImages = 10000;
		const auto nRows = 32;
		const auto nCols = 32;

		for (auto i = 0; i < numberOfImages; ++i)
		{
			unsigned char tplabel = 0;
			file.read(reinterpret_cast<char*>(&tplabel), sizeof(tplabel));
			vector<Mat> channels;
			Mat finImg = Mat::zeros(nRows, nCols, CV_8UC3);
			for (auto ch = 0; ch < 3; ++ch) {
				Mat tp = Mat::zeros(nRows, nCols, CV_8UC1);
				for (auto r = 0; r < nRows; ++r) {
					for (auto c = 0; c < nCols; ++c) {
						unsigned char temp = 0;
						file.read(reinterpret_cast<char*>(&temp), sizeof(temp));
						tp.at<uchar>(r, c) = static_cast<int>(temp);
					}
				}
				channels.push_back(tp);
			}
			merge(channels, finImg);
			vec.push_back(finImg);
			label.ATD(0, i) = static_cast<double>(tplabel);
		}
	}
}

void Common::SaveImage(const Mat& mat, const string& filename, const string& format)
{
	const auto outputFile = filename + "." + format;
	imwrite(outputFile, mat);
}

SemiRandomSortType Common::ToCustomType(const MeasureType &mt)
{
	const string message = "Measure type contains no custom sort order\n";
	switch (mt)
	{
	case MeasureType::l1Norm:
		return SemiRandomSortType::bubbleSortl1Norm;
	case MeasureType::l2Norm:
		return SemiRandomSortType::bubbleSortl2Norm;
	case MeasureType::ssimAverage:
		return SemiRandomSortType::bubbleSortSsimAverage;
	case MeasureType::ssim0:
		return SemiRandomSortType::bubbleSortSsim0;
	case MeasureType::ssim1:
		return SemiRandomSortType::bubbleSortSsim1;
	case MeasureType::ssim2:
		return SemiRandomSortType::bubbleSortSsim2;
	case MeasureType::psnr:
		return SemiRandomSortType::bubbleSrotPsnr;
	default: throw runtime_error(message);
	}
}