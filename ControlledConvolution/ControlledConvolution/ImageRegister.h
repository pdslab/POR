#include <iostream>
#include <random>
#include <opencv2/opencv.hpp>

# define pi 3.14159265358979323846
using namespace std;
using namespace cv;


#pragma once
class ImageRegister
{
		/* TODO
		 * > Verify what methods are public.
		 */

	public:
		ImageRegister(string fixed_path, string moving_path);
		ImageRegister(Mat fixed_mat, Mat moving_mat, cv::Size resize_to);
		~ImageRegister();
		Mat ComputeHistogram(Mat image);
		Mat ComputeJointHistogram(Mat image_1, Mat image_2);
		float ComputeEntropy(Mat image);
		float ComputeRelativeEntropy(Mat image_1, Mat image_2);
		float ComputeJointEntropy(Mat image_1, Mat image_2);
		float ComputeMutualInformation(Mat image_1, Mat image_2);
		double ComputeMaxMutualInformationValue(Mat image_1, Mat image_2, int points, int max_iterations = 2500);

		/* Testing*/

	private:
		int getImages(string fixed_path, string moving_path);
		double getNormalRandomNumber(double mean, double stddev, int type);
		Mat calLog2(Mat image);

	public:
		static const int OK = 0;
		static const int FAILURE = -1;
		static const int STANDARD = 10;

	private:
		Mat fixed, moving;
		int histSize;
		default_random_engine generator;

	public:
		int getHistSize() const { return histSize; }
		Mat GetFixedImage() const { return fixed; }
		Mat GetMovingImage() const { return moving; }

		/* Testing*/
};
