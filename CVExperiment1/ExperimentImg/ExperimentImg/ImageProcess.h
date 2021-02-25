#pragma once
#pragma once
#define NOISE 0.2
#include "MatCImage.h"
struct ThreadParam
{
	CImage* src;
	CImage* dst;
	int startIndex;
	int endIndex;
	int maxSpan;//为模板中心到边缘的距离
	int angle;
	int wParam;
	void* para;
	void* tppa;
	void(*callback)(ThreadParam*);

	//初始化
	ThreadParam(void) :
		wParam(1),
		callback(nullptr),
		src(nullptr),
		tppa(nullptr),
		para(nullptr) {}


};
struct ScaleParam
{
	//CImage *img;
	Mat img;
	float factor;
};

struct RotateParam
{
	CImage *img;
	double angle;
};

struct GaussParam
{
	CImage* img;
	double mean,sigma;
};
static bool GetValue(int p[], int size, int &value);

class ImageProcess
{
public:
	static ThreadParam* Multithread(CImage* img, int thread);

	static UINT medianFilter(LPVOID  param);
	static UINT addNoise(LPVOID param);
	static UINT scalePic(LPVOID param);
	static UINT rotatePic(LPVOID param);
	static UINT FourierTransform(LPVOID param);
	static UINT gaussNoise(LPVOID param);
	static UINT avgFilter(LPVOID  param);
	static UINT gaussFilter(LPVOID param);
	static UINT wienerFilter(LPVOID param);

}; 