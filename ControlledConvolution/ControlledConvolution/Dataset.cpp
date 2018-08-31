#include "stdafx.h"
#include "Dataset.h"
#include "ImageMeasure.h"


Dataset::Dataset(): _totalSamples(0), _channel0Entropy(0), _channel1Entropy(0), _channel2Entropy(0), _averageEntropy(0)
{
}

Dataset::Dataset(const std::string & dir): _totalSamples(0), _channel0Entropy(0), _channel1Entropy(0),
										   _channel2Entropy(0), _averageEntropy(0)
{
	_directory = dir;
	_sampleSet = Common::GetSampleSet(_directory);
}


Dataset::~Dataset()
{
}

void Dataset::CalcualteEntropy()
{
	auto counter = 0;

	cout << "[";
	for(const auto& sample : _sampleSet)
	{
		const auto image = cv::imread(sample, IMREAD_COLOR);
		ImageMeasure m(image);
		m.CalculateEntropy();
		cout << "#";

		_channel0Entropy += m.Channel0Entropy();
		_channel1Entropy += m.Channel1Entropy();
		_channel2Entropy += m.Channel2Entropy();
		_averageEntropy += m.AverageEntropy();
		counter += 1;
	}
	cout << counter;
	_channel0Entropy = _channel0Entropy / counter;
	_channel1Entropy = _channel1Entropy / counter;
	_channel2Entropy = _channel2Entropy / counter;
	_averageEntropy = _averageEntropy / counter;
}

ostream & operator<<(ostream & out, const Dataset & d)
{
	// TODO: insert return statement here
	out << "Dataset stat:\n";
	out << "\tDirectory: " << d._directory << endl;
	out << "\tTotal samples : " << d._totalSamples<< endl;
	out << "\tBlue Channel Entropy: " << d._channel0Entropy << endl;
	out << "\tGreen Channel Entropy: " << d._channel1Entropy << endl;
	out << "\tRed Channel Entropy: " << d._channel2Entropy << endl;

	return out;
}
