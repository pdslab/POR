#pragma once
class Coordinate
{
public:
	Coordinate();
	Coordinate(const int& x0, const int& y0, const int& x1, const int& y1): width_(0), height_(0)
	{
		x_0_ = x0;
		y_0_ = y0;
		x_1_ = x1;
		y_1_ = y1;
	}

	~Coordinate();

	void SetStart(const int x0, const int y0)
	{
		x_0_ = x0;
		y_0_ = y0;
	}

	void Set(const int x, const int y)
	{
		x_0_ = x;
		y_0_ = y;
	}
	int GetWidth()
	{
		width_ = x_1_ - x_0_;
		return width_;
	}
	int GetHeight()
	{
		height_ = y_1_ - y_0_;
		return height_;
	}
	void SetEnd(const int x1, const int y1)
	{
		x_1_ = x1;
		y_1_ = y1;
	}
	int* Start() const
	{
		return new int [2]{ x_0_, y_0_ };
	}
	int* End() const
	{
		return new int[2] {x_1_, y_1_};
	}

	std::string ToStr() const { return "(" + std::to_string(x_0_) + "," + std::to_string(y_0_) + "," + std::to_string(x_1_) + "," + std::to_string(y_1_) + ")"; }

private:
	int x_0_;
	int x_1_;
	int y_0_;
	int y_1_;
	int width_;
	int height_;
};

