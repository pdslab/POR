#pragma once
class Dataset
{
public:
	Dataset();
	explicit Dataset(const std::string &dir);
	~Dataset();

	void CalcualteEntropy();
	std::string GetDir() const { return _directory; }
	int TotalSamples() const { return _totalSamples; }
	friend ostream& operator <<(ostream& out, const Dataset& d);
private:
	std::string _directory;
	int _totalSamples;
	double _channel0Entropy;
	double _channel1Entropy;
	double _channel2Entropy;
	double _averageEntropy;
	vector<std::string> _sampleSet;
};

