#pragma once
#ifndef INPUT_H
#define INPUT_H
#include <iostream>
#include "Patch.h"
#include "stdafx.h"
#include <map>

class Sample
{
public: 
	Sample(): minimum_number_of_patches_x_(0), minimum_number_of_patches_y_(0), height_(0), width_(0), rows_(0), cols_(0),
	          area_(0)
	{
	}

	explicit Sample(const string filePath): minimum_number_of_patches_x_(0), minimum_number_of_patches_y_(0),
	                                         height_(0), width_(0),
	                                         rows_(0), cols_(0), area_(0)
	{
		input_file_ = filePath;
		patch_proposal_coordinates_ = {};
		SetName(filePath);
	}

	string ExtractPatch(Mat& patch, const Coordinate &c);
	string GetInput() const { return input_file_; }
	vector<Patch> Patches() const { return sample_patches_sorted_; }
	void ToCvMat(const cv::Size& size);
	bool Load();
	void DetermineMinimumNumberOfPatchZones();
	static void DetermineSampleFittness();
	void GeneratePatchProposals(const Size &s) ;
	void AddPatchCoordinates(const Coordinate& c) { patch_proposal_coordinates_.push_back(c); }
	void AddPatch(Patch &p)
	{
		sample_patches_sorted_.push_back(p);
		p.Release();
	}
	
	vector<Coordinate> PatchesCoordinates() const { return patch_proposal_coordinates_; }

	Mat Mat() const
	{
		return mat_;
	}

	bool operator < (const Size& size1) const;
	void Resize(cv::Mat& output, const unsigned int& width, const unsigned int& height) const;
	vector<cv::Mat> PachesAsVectorOfMats();
	void SaveToDisc(const string& output, const string& format, bool original = false);
	friend class Reconstructor;

#pragma  region Setters and Getters
	void SetSortedSamplePatches(const vector<Patch>& patch)
	{
		sample_patches_sorted_ = patch;
	}
	void SetSamplePatches(const vector<Patch>& patch)
	{
		sample_patches_original_ = patch;
	}
	void GetStat() const;
	void SetTemplatePatch(const cv::Mat& p);
	cv::Mat GetReconstructedOutput() const { return reconstructed_output_; }
	void SetStitchedOutput(cv::Mat& mat) { stitched_output_ = mat; }
	void SetReconstructedOutput(cv::Mat& mat) { reconstructed_output_ = mat; }
	void SaveReconstructedSample(string format) const;
	void SetName(const string& name);
	string Name() const { return name_; }
	string BaseName() const;
	Size Size() const { return size_; }
#pragma endregion 

private:
	string input_file_;
	vector<Patch> sample_patches_sorted_;
	vector<Patch> sample_patches_original_;
	vector<Coordinate> patch_proposal_coordinates_;
	int minimum_number_of_patches_x_;
	int minimum_number_of_patches_y_;
	int height_;
	int width_;
	int rows_;
	int cols_;
	int area_;
	cv::Mat mat_;
	cv::Size size_;
	vector<cv::Mat> sample_bgr_planes_;
	cv::Mat template_patch_;
	cv::Mat reconstructed_output_;
	cv::Mat stitched_output_;
	string name_;
};
#endif 