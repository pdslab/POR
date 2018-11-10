#include "stdafx.h"
#include "ImageRegister.h"
#include "Common.h"
#undef max



ImageRegister::ImageRegister(string fixed_path, string moving_path)
	:histSize(2048)
{
	getImages(fixed_path, moving_path);
}

ImageRegister::ImageRegister(Mat fixed_mat, Mat moving_mat, cv::Size resize_to):histSize(2048)
{
	fixed = fixed_mat;
	moving = moving_mat;

	if (!fixed.data || !moving.data)
	{
		throw exception( "No image data available!");
	}


	resize(fixed, fixed, resize_to);
	resize(moving, moving, resize_to);

	auto fixed_size = fixed.size();
	auto moving_size = moving.size();
}

ImageRegister::~ImageRegister()
{

}

int ImageRegister::getImages(string fixed_path, string moving_path)
{
	fixed = imread(fixed_path);
	moving = imread(moving_path);

	if (!fixed.data || !moving.data)
	{
		cerr << "No image data available!" << endl;
		return FAILURE;
	}

	Size size_fixed = fixed.size();
	Size size_moving = moving.size();

	resize(fixed, fixed, cv::Size(32, 32));
	resize(moving, moving, cv::Size(32, 32));

	if ((size_fixed.height*size_fixed.width) > (size_moving.height*size_moving.width))
		resize(moving, moving, fixed.size());
	else
		resize(fixed, fixed, moving.size());
	auto fixed_size = fixed.size();
	auto moving_size = moving.size();

	cout << "Fixed image : (" << fixed_size.height<< "," << fixed_size.width<< ")\n";
	cout << "Moving image : (" << moving_size.height<< "," << moving_size.width<< ")\n";

	return OK;
}

Mat ImageRegister::calLog2(Mat mat_src)
{
	Mat mat_corr, loge_mat;
	cv::max(mat_src, Scalar::all(1e-10), mat_corr);
	log(mat_corr, loge_mat);
	Mat log2_mat = loge_mat / log(2);

	return log2_mat;
}

double ImageRegister::getNormalRandomNumber(double mean, double stddev, int type)
{
	/* TODO
	 * > Add different generator modes
	 */
	normal_distribution<double> distribution(mean, stddev);
	double na = distribution(generator);

	return na;
}

Mat ImageRegister::ComputeHistogram(Mat image)
{
	Mat hist = Mat::zeros(1, histSize, CV_32FC1);

	if (image.channels() == 3) cvtColor(image, image, CV_RGB2GRAY);

	float range[] = { 0, 255 };
	const float* hist_range = range;

	calcHist(&image, 1, 0, Mat(), hist, 1, &histSize, &hist_range);

	return hist;
}

Mat ImageRegister::ComputeJointHistogram(Mat image_1, Mat image_2)
{
	/*cvtColor(image_1, image_1, cv::COLOR_BGR2GRAY);
	cvtColor(image_2, image_2, cv::COLOR_BGR2GRAY);*/
	/* TODO
	 * > Verify this calculation
	 * > Search a better method to do this.
	 */
	Mat jpdf;
	Size image_size = image_1.size();
	jpdf = Mat::zeros(histSize, histSize, CV_32FC1);

	assert(image_size.height == image_2.size().height, "The two images have different sizes!!");

	for (int i = 0; i < image_size.height; i++)
	{
		for (int j = 0; j < image_size.width; j++)
		{
			try {
				jpdf.at<float>(image_1.at<uchar>(i, j), image_2.at<uchar>(i, j)) = (float)(jpdf.at<float>(image_1.at<uchar>(i, j), image_2.at<uchar>(i, j)) + 1);
			}
			catch (...) {
				continue;
			}
		}
	}

	return jpdf;
}
float ImageRegister::ComputeEntropy(Mat image)
{
	float entropy;
	Mat hist = ComputeHistogram(image);
	hist /= image.total();
	Mat logP = calLog2(hist);
	entropy = -1 * sum(hist.mul(logP)).val[0];

	return entropy;
}

float ImageRegister::ComputeRelativeEntropy(Mat image_1, Mat image_2)
{
	float re;

	auto hist_1 = ComputeHistogram(image_1);
	auto hist_2 = ComputeHistogram(image_2);

	re = sum(hist_1.mul(calLog2(hist_1 / hist_2))).val[0];

	return re;
}

float ImageRegister::ComputeJointEntropy(Mat image_1, Mat image_2)
{
	/* TODO
	 * > Verify this calculation
	 */
	float je;
	Mat jpdf = ComputeJointHistogram(image_1, image_2);
	jpdf = jpdf / (sum(jpdf).val[0]); // Normalized Joint Histogram       
	Mat logJP = calLog2(jpdf);
	je = -1 * sum(jpdf.mul(logJP)).val[0];

	return je;
}

float ImageRegister::ComputeMutualInformation(Mat image_1, Mat image_2)
{
	float mi;
	float entropy_1 = ComputeEntropy(image_1);
	float entropy_2 = ComputeEntropy(image_2);
	float joint_entropy = ComputeJointEntropy(image_1, image_2);
	mi = entropy_1 + entropy_2 - joint_entropy;

	return mi;
}

double ImageRegister::ComputeMaxMutualInformationValue(Mat image_1, Mat image_2, int points, int max_iterations)
{
	/* TODO
	 * > Develop image rotation and translation methods
	 * > Save matrices and vectors in debug mode
	 * > Develop the Simulated Annealing method
	 */
	double max_mi=0.0;
	double mi_init = ComputeMutualInformation(image_1, image_2);

	/* Initial values */
	double mi_test = mi_init;
	double th_accept = 0.0175; // degtorad(1)
	double tx_accept = 0;
	double ty_accept = 0;

	/* Gaussian window parameters */
	double w_radians_a = -(pi / 2)*pow(10, -8);
	double w_radians_b = (pi / 4);
	double w_pixels_a = -4 * pow(10, -8);
	double w_pixels_b = 2;
	int i = 1;
	int valid_points = 1;

	while (valid_points < points)
	{
		if (i > max_iterations); break;
		double th = getNormalRandomNumber(th_accept, w_radians_a*i + w_radians_b, STANDARD);
		double tx = getNormalRandomNumber(tx_accept, w_pixels_a*i + w_pixels_b, STANDARD);
		double ty = getNormalRandomNumber(ty_accept, w_pixels_a*i + w_pixels_b, STANDARD);

		i++;
	}
	return max_mi;
}
