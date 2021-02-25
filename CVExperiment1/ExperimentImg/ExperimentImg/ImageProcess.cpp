#include "stdafx.h"
#include "ImageProcess.h"
#include"Display.h"
#include "MatCImage.h"
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <complex>


static bool GetValue(int p[], int size, int &value)
{
	//数组中间的值
	int zxy = p[(size - 1) / 2];
	//用于记录原数组的下标
	int *a = new int[size];
	int index = 0;
	for (int i = 0; i<size; ++i)
		a[index++] = i;

	for (int i = 0; i<size - 1; i++)
		for (int j = i + 1; j<size; j++)
			if (p[i]>p[j]) {
				int tempA = a[i];
				a[i] = a[j];
				a[j] = tempA;
				int temp = p[i];
				p[i] = p[j];
				p[j] = temp;

			}
	int zmax = p[size - 1];
	int zmin = p[0];
	int zmed = p[(size - 1) / 2];

	if (zmax>zmed&&zmin<zmed) {
		if (zxy>zmin&&zxy<zmax)
			value = (size - 1) / 2;
		else
			value = a[(size - 1) / 2];
		delete[]a;
		return true;
	}
	else {
		delete[]a;
		return false;
	}

}

//分线程工作
ThreadParam* ImageProcess::Multithread(CImage* img, int m_nThreadNum)
{
	ThreadParam* m_pThreadParam = new ThreadParam[m_nThreadNum];
	int subLength = img->GetWidth() * img->GetHeight() / m_nThreadNum;

	for (int i = 0; i < m_nThreadNum; ++i)
	{		
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : img->GetWidth() * img->GetHeight() - 1;
		m_pThreadParam[i].src = img;
		m_pThreadParam[i].tppa = m_pThreadParam;
	}
	return m_pThreadParam;
}

UINT ImageProcess::medianFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	int maxSpan = param->maxSpan;
	int maxLength = (maxSpan * 2 + 1) * (maxSpan * 2 + 1);


	byte* pRealData = (byte*)param->src->GetBits();
	int pit = param->src->GetPitch();
	int bitCount = param->src->GetBPP() / 8;

	int *pixel = new int[maxLength];//存储每个像素点的灰度
	int *pixelR = new int[maxLength];
	int *pixelB = new int[maxLength];
	int *pixelG = new int[maxLength];
	int index = 0;
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int Sxy = 1;
		int med = 0;
		int state = 0;
		int x = i % maxWidth;
		int y = i / maxWidth;
		while (Sxy <= maxSpan)
		{
			index = 0;
			for (int tmpY = y - Sxy; tmpY <= y + Sxy && tmpY <maxHeight; tmpY++)
			{
				if (tmpY < 0) continue;
				for (int tmpX = x - Sxy; tmpX <= x + Sxy && tmpX<maxWidth; tmpX++)
				{
					if (tmpX < 0) continue;
					if (bitCount == 1)
					{
						pixel[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixelR[index++] = pixel[index];

					}
					else
					{
						pixelR[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 2);
						pixelG[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 1);
						pixelB[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);

					}
				}

			}
			if (index <= 0)
				break;
			if ((state = GetValue(pixel, index, med)) == 1)
				break;

			Sxy++;
		};

		if (state)
		{
			if (bitCount == 1)
			{
				*(pRealData + pit*y + x*bitCount) = pixelR[med];

			}
			else
			{
				*(pRealData + pit*y + x*bitCount + 2) = pixelR[med];
				*(pRealData + pit*y + x*bitCount + 1) = pixelG[med];
				*(pRealData + pit*y + x*bitCount) = pixelB[med];

			}
		}

	}



	delete[]pixel;
	delete[]pixelR;
	delete[]pixelG;
	delete[]pixelB;

	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)param);
	return 0;
}

UINT ImageProcess::addNoise(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();

	srand(time(NULL));
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		if ((rand() % 1000) * 0.001 < NOISE)
		{
			int value = 0;
			if (rand() % 1000 < 500)
			{
				value = 0;
			}
			else
			{
				value = 255;
			}
			if (bitCount == 1)
			{
				*(pRealData + pit * y + x * bitCount) = value;
			}
			else
			{
				*(pRealData + pit * y + x * bitCount) = value;
				*(pRealData + pit * y + x * bitCount + 1) = value;
				*(pRealData + pit * y + x * bitCount + 2) = value;
			}
		}
	}
	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)param);
	return 0;
}

//BiCubic函数
float BicubicWeight(float x)
{
	float abs_x = abs(x);
	float a = -0.5;
	if (abs_x <= 1.0)
	{
		return (a + 2) * pow(abs_x, 3) - (a + 3) * pow(abs_x, 2) + 1;
	}
	else if (abs_x < 2.0)
	{
		return a * pow(abs_x, 3) - 5 * a * pow(abs_x, 2) + 8 * a * abs_x - 4 * a;
	}
	else
		return 0.0;

}


UINT ImageProcess::scalePic(LPVOID  p)
{	
	auto params = (ThreadParam*)p;
	auto sp = (ScaleParam*)(params->para);
	Mat img = sp->img;
	float factor = sp->factor;
	//图像放大后的行列
	float row = img.rows * factor;
	float col = img.cols * factor;
	//CString str;
	//str.Format(_T("row %f,cols%f,factor %lf"), row, col, factor);
	//AfxMessageBox(str);
	//建立目标图像
	Mat dest(row, col, CV_8UC3);


	for (int i = 2; i < row - 4; i++) {
		for (int j = 2; j < col - 4; j++) {
			float x = i * (img.rows / row);//放大后的图像的像素位置相对于源图像的位置 
			float y = j * (img.cols / col);

			//取整数部分进行运算 x=X+u y=Y+v
			int X = (int)x;float u = x - X;
			int Y = (int)y; float v = y - Y;
			
			//行列方向的加权系数
			float row_i[4], col_j[4];
			//横坐标权重
			row_i[0] = BicubicWeight(1 + u);
			row_i[1] = BicubicWeight(u);
			row_i[2] = BicubicWeight(1 - u);
			row_i[3] = BicubicWeight(2 + u);
			//纵坐标权重
			col_j[0] = BicubicWeight(1 + v);
			col_j[1] = BicubicWeight(v);
			col_j[2] = BicubicWeight(1 - v);
			col_j[3] = BicubicWeight(2 + v);
		
			Vec3f stp = { 0, 0, 0 };
			for (int s = 0; s <= 3; s++) {
				for (int t = 0; t <= 3; t++) {
					stp = stp + (Vec3f)(img.at<Vec3b>(int(x) + s - 1, int(y) + t - 1)) * row_i[s] * col_j[t];
				}
			}
			
			dest.at<Vec3b>(i, j) = (Vec3b)stp;
		}
		//DISP->OutputLine(_T("EVERTHING ALLRIHT?1"));
	}	
	
	imshow("缩放后图像", dest);
	waitKey(0);
	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)params);
	return 0;


}

int normaliseXY(int x, int y, int max_x, int max_y) {
	if (x >= max_x || x <= 0 || y >= max_y || y <= 0) //超出边界的部分用黑色填充
		return 0;
	return x;
}

UINT ImageProcess::rotatePic(LPVOID p)
{
	auto param = (ThreadParam*)p;
	auto rp = (RotateParam*)(param->para);

	//角度、弧度转换
	const double  degree = 3.14159 / 180.0;
	double theta = (rp->angle) * degree;
	short type = 2;

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	byte* pRealData = (byte*)param->src->GetBits();
	byte* pOriginData = (byte*)rp->img->GetBits();


	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();
	//绕中心旋转
	for (int i = startIndex; i <= endIndex; ++i)
	{

		int x = i % maxWidth;
		int y = i / maxWidth;


		double real_v = x * cos(theta) + y * sin(theta) + cos(theta) * (-1) * (maxWidth / 2) + sin(theta) * (-1) * (maxHeight / 2) - (-1) * maxWidth / 2;
		double real_w = (-1) * x * sin(theta) + y * cos(theta) + cos(theta) * (-1) * (maxHeight / 2) - sin(theta) * (-1) * (maxWidth / 2) - (-1) * maxHeight / 2;

		double value = 0;
		int v, w;
		for (int off = 0; off < bitCount; off++) {
			if (type == 0) {
				v = real_v;
				w = real_w;
				if ((value = normaliseXY(v, w, maxWidth, maxHeight)) != 0)
					value = *(pOriginData + pit * w + v * bitCount + off);
			}
			else if (type == 1) {
				v = floor(real_v);
				w = floor(real_w);

				double u = real_v - v;
				double k = real_w - w;
				if ((value = normaliseXY(v, w, maxWidth, maxHeight)) != 0)////(插值范围超出边界,但点没超出边界的话
					if (v >= 1 && v < maxWidth - 1 && w >= 1 && w < maxHeight - 1) {
						value = 0;
						value += (*(pOriginData + pit * (w)+(v)* bitCount + off)) * (1 - u) * (1 - k);
						value += (*(pOriginData + pit * (w + 1) + (v)* bitCount + off)) * (1 - u) * (k);
						value += (*(pOriginData + pit * (w)+(v + 1) * bitCount + off)) * (u) * (1 - k);
						value += (*(pOriginData + pit * (w + 1) + (v + 1) * bitCount + off)) * (u) * (k);
					}
			}
			else if (type == 2) {
				v = floor(real_v);
				w = floor(real_w);
				if ((value = normaliseXY(v, w, maxWidth, maxHeight)) != 0)
					if (v >= 2 && v < maxWidth - 2 && w >= 2 && w < maxHeight - 2) {
						//(插值范围在边界内)																
						value = 0;
						// (插值
						for (int i = -1; i < 3; i++)
							for (int j = -1; j < 3; j++)
								value += (*(pOriginData + pit * (w + i) + (v + j) * bitCount + off)) * BicubicWeight(real_v - v - j) * BicubicWeight(real_w - w - i);
					}
			}
			if (value < 0)value = 0;
			if (value > 255)value = 255;
			*(pRealData + pit * y + x * bitCount + off) = value;
		}
	}

	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)param);
	return 0;
}

constexpr auto FOURIER_FACTOR = 14.0;
UINT ImageProcess::FourierTransform(LPVOID p)
{
	auto param = (ThreadParam*)p;
	//auto source = (CImage*)(param->para);
	//CImage* img=source;
	//CImage* dest = param->src;
	const double PI = acos(-1);
	
	byte* pRealData = (byte*)param->dst->GetBits();
	byte* dstRealData = (byte*)param->src->GetBits();

	int bitCount = param->dst->GetBPP() / 8;
	int pit = param->dst->GetPitch();

	auto Width = param->dst->GetWidth();
	auto Height = param->dst->GetHeight();

	for (int i = param->startIndex; i < param->endIndex; ++i)
	{	
		int u = i % Width;
		int v = i / Width;
		double real = 0.0, imag = 0.0;
		for (int y = 0; y < Height; ++y)
		{
			for (int x = 0; x < Width; ++x)
			{
				auto pixel= (byte*)(pRealData + pit * y + bitCount * x);//auto pixel = img.GetPixel(x, y); 
				double gray = 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];

				if ((x + y) & 1) // centralize
					gray = -gray;
				double A = 2 * PI * ((double)u * (double)x / (double)Width + (double)v * (double)y / (double)Height);
				real += gray * cos(A);
				imag -= gray * sin(A);
			}
		}

		double mag = sqrt(real * real + imag * imag);
		mag = FOURIER_FACTOR * log(mag + 1);

		//mag = std::clamp(mag, 0.0, 255.0);
		if (mag > 255.0) mag = 255.0;
		else if (mag < 0.0) mag = 0.0;

		//dest.SetPixel(u, v, (byte)mag, (byte)mag, (byte)mag);
		*(dstRealData + pit * v + bitCount * u + 0) = (byte)mag;
		*(dstRealData + pit * v + bitCount * u + 1) = (byte)mag;
		*(dstRealData + pit * v + bitCount * u + 2) = (byte)mag;
	}
	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)param);
	return 0;
}

static double BoxMullerGenerator(double mean, double sigma)
{
	static const double twopi = 2.0 * acos(-1);
	double u1, u2;
	static double z0, z1;
	static bool generate = false;
	generate = !generate;
	if (!generate)
		return z1 * sigma + mean;
	do
	{
		u1 = (double)rand() / RAND_MAX;
		u2 = (double)rand() / RAND_MAX;
	} while (u1 <= DBL_MIN);
	z0 = sqrt(-2.0 * log(u1)) * cos(twopi * u2);
	z1 = sqrt(-2.0 * log(u1)) * sin(twopi * u2);
	return z0 * sigma + mean;
}

UINT ImageProcess::gaussNoise(LPVOID  p)
{
	auto param = (ThreadParam*)p;
	auto gp = (GaussParam*)(param->para);

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();

	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();

	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;

		auto pixel = (byte*)(pRealData + pit * y + bitCount * x);//auto pixel = img.GetPixel(x, y); 
		for (int i = 0; i < 3; ++i)
		{
			double val = pixel[i] + BoxMullerGenerator(gp->mean, gp->sigma);
			if (val > 255.0)
				val = 255.0;
			if (val < 0.0)
				val = 0.0;
			pixel[i] = (byte)val;
		}
		
	}
	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)param);
	return 0;

}

UINT ImageProcess::avgFilter(LPVOID  p)
{
	auto param = (ThreadParam*)p;

	byte* pRealData = (byte*)param->src->GetBits();
	byte* dstRealData = (byte*)param->dst->GetBits();

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();

	const int TEMPLATE_SIZE = 3 * 3;

	for (int i = param->startIndex; i < param->endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		// 略过边界上的像素点
		if (x < 1 || y < 1 || x >= maxWidth - 1 || y >= maxHeight - 1)
		{
			auto pixel = (byte*)(dstRealData + pit * y + bitCount * x);//auto pixel = img.GetPixel(x, y); 
			//img.SetPixel(x, y, src.GetPixel(x, y));		
			*(pRealData + pit * y + bitCount * x + 0) = pixel[0];
			*(pRealData + pit * y + bitCount * x + 1) = pixel[1];
			*(pRealData + pit * y + bitCount * x + 2) = pixel[2];

			continue;
		}
		int r, g, b;
		r = g = b = 0;

		for (int i = -1; i < 2; i++)
		{		
			for (int j = -1; j < 2; j++)
			{
				int u = x + i;
				int v = y + j;
				auto _t = (byte*)(dstRealData + pit * v + bitCount * u);//auto _t = src.GetPixel(_x, _y)
				r += _t[0]; g += _t[1]; b += _t[2];
			}
		}
/*
#define ACCUMULATE(_x, _y) { \
auto _t = src.GetPixel(_x, _y); \
r += _t[0]; g += _t[1]; b += _t[2]; }
		ACCUMULATE(x - 1, y - 1); ACCUMULATE(x, y - 1); ACCUMULATE(x + 1, y - 1);
		ACCUMULATE(x - 1, y);     ACCUMULATE(x, y);     ACCUMULATE(x + 1, y);
		ACCUMULATE(x - 1, y + 1); ACCUMULATE(x, y + 1); ACCUMULATE(x + 1, y - 1);
#undef ACCUMULATE
*/

		//img.SetPixel(x, y, (byte)(r / TEMPLATE_SIZE), (byte)(g / TEMPLATE_SIZE), (byte)(b / TEMPLATE_SIZE));
		* (pRealData + pit * y + bitCount * x + 0) = (byte)(r / TEMPLATE_SIZE);
		*(pRealData + pit * y + bitCount * x + 1) = (byte)(g / TEMPLATE_SIZE);
		*(pRealData + pit * y + bitCount * x + 2) = (byte)(b / TEMPLATE_SIZE);
	}
	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)param);
	return 0;
}


double Gaussian(int x1, int y1, int x2, int y2, double sigma)
{
	const double PI = 4.0 * atan(1.0);
	double absX = pow(abs(x1 - x2), 2);
	double absY = pow(abs(y1 - y2), 2);
	double k = (1 / (2 * PI * sigma * sigma)) * exp(-(absX + absY) / (2 * pow(sigma, 2)));
	return k;
}
UINT ImageProcess::gaussFilter(LPVOID p)
{
	auto param = (ThreadParam*)p;
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();

	double sigma = 1;
	int start = 3;
	int theFirstBit;
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		int SumofGuass = 0;
		int RGB[3] = { 0 };

		for (int PY = y - start; PY <= y + start && PY < maxHeight; ++PY)
		{
			if (PY < 0)continue;
			for (int PX = x - start; PX <= x + start && PX < maxWidth; ++PX)
			{
				if (PX < 0)continue;
				if (bitCount == 1)
					theFirstBit = *(pRealData + pit * (PY)+(PX)* bitCount);
				else
				{
					RGB[0] += *(pRealData + pit * (PY)+(PX)* bitCount) * Gaussian(y, x, PY, PX, sigma);
					RGB[1] += *(pRealData + pit * (PY)+(PX)* bitCount + 1) * Gaussian(y, x, PY, PX, sigma);
					RGB[2] += *(pRealData + pit * (PY)+(PX)* bitCount + 2) * Gaussian(y, x, PY, PX, sigma);
					SumofGuass += Gaussian(y, x, PY, PX, sigma);
				}
			}
		}
		if (bitCount == 1)
			* (pRealData + pit * y + x * bitCount) = theFirstBit;
		else
		{

			*(pRealData + pit * y + x * bitCount) = (RGB[0] < 0) ? 0 : ((RGB[0] > 255) ? 255 : RGB[0]);
			*(pRealData + pit * y + x * bitCount + 1) = (RGB[1] < 0) ? 0 : ((RGB[1] > 255) ? 255 : RGB[1]);
			*(pRealData + pit * y + x * bitCount + 2) = (RGB[2] < 0) ? 0 : ((RGB[2] > 255) ? 255 : RGB[2]);
		}
	}
	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)param);
	return 0;
}

UINT ImageProcess::wienerFilter(LPVOID p)
{
	int radius = 1.5;
	int size = 9;
	auto param = (ThreadParam*)p;
	byte* pRealData = (byte*)param->src->GetBits();
	int channels = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();
	int rows = param->src->GetHeight();
	int cols = param->src->GetWidth();
	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	int ymin = startIndex / cols;
	int ymax = endIndex / cols;
	int length = (ymax - ymin + 1) * cols;

	//计算全部像素的邻域的均值和方差
	float* avgR = new float[length]();
	float* avgG = new float[length]();
	float* avgB = new float[length]();

	float* sigR = new float[length]();
	float* sigG = new float[length]();
	float* sigB = new float[length]();
	int index = 0;
	for (int y = ymin; y <= ymax; y++)
	{
		for (int x = 0; x < cols; x++) {

			float sumred = 0;
			float sumgreen = 0;
			float sumblue = 0;

			for (int i = -radius; i <= radius; i++) {
				int newy = y + i;
				if (newy < 0 || newy >= rows) { newy = y; }
				for (int j = -radius; j <= radius; j++) {
					int newx = x + j;
					if (newx < 0 || newx >= cols) { newx = x; }
					if (channels == 1)
					{
						sumred += *(pRealData + pit * (newy)+(newx)* channels);
					}
					else
					{
						sumred += *(pRealData + pit * (newy)+(newx)* channels + 2);
						sumgreen += *(pRealData + pit * (newy)+(newx)* channels + 1);
						sumblue += *(pRealData + pit * (newy)+(newx)* channels + 0);
					}
				}
			}
			if (channels == 1)
			{
				avgR[index] = sumred / size;
			}
			else
			{
				avgR[index] = sumred / size;
				avgG[index] = sumgreen / size;
				avgB[index] = sumblue / size;
			}
			sumred = 0, sumgreen = 0, sumblue = 0;
			for (int i = -radius; i <= radius; i++) {
				int newy = y + i;
				if (newy < 0 || newy >= rows) { newy = y; }
				for (int j = -radius; j <= radius; j++) {
					int newx = x + j;
					if (newx < 0 || newx >= cols) { newx = x; }
					if (channels == 1)
					{
						sumred += pow((*(pRealData + pit * (newy)+(newx)* channels)) - avgR[index], 2);
					}
					else
					{
						sumred += pow((*(pRealData + pit * (newy)+(newx)* channels) + 2) - avgR[index], 2);
						sumgreen += pow((*(pRealData + pit * (newy)+(newx)* channels) + 1) - avgG[index], 2);
						sumblue += pow((*(pRealData + pit * (newy)+(newx)* channels)) - avgB[index], 2);
					}
				}
			}
			if (channels == 1)
			{
				sigR[index] = sumred / size;
			}
			else
			{
				sigR[index] = sumred / size;
				sigG[index] = sumgreen / size;
				sigB[index] = sumblue / size;
			}
			index++;
		}
	}
	float miu_sigR = 0, miu_sigG = 0, miu_sigB = 0;
	for (int i = 0; i < index; i++) {
		miu_sigR += sigR[i];
		miu_sigG += sigG[i];
		miu_sigB += sigB[i];
	}
	miu_sigR /= length;
	miu_sigG /= length;
	miu_sigB /= length;
	index = 0;
	for (int y = ymin; y <= ymax; y++)
	{
		for (int x = 0; x < cols; x++) {
			if (channels == 1)
			{
				*(pRealData + pit * y + x * channels) = avgR[index] + (sigR[index] - miu_sigR > 0 ? sigR[index], miu_sigR : 0) / (max(sigR[index], miu_sigR)) * (*(pRealData + pit * y + x * channels) - avgR[index]);
			}
			else
			{
				*(pRealData + pit * y + x * channels + 2) = avgR[index] + (sigR[index] - miu_sigR > 0 ? sigR[index], miu_sigR : 0) / (max(sigR[index], miu_sigR)) * (*(pRealData + pit * y + x * channels + 2) - avgR[index]);
				*(pRealData + pit * y + x * channels + 1) = avgG[index] + (sigG[index] - miu_sigG > 0 ? sigG[index], miu_sigG : 0) / (max(sigG[index], miu_sigG)) * (*(pRealData + pit * y + x * channels + 1) - avgG[index]);
				*(pRealData + pit * y + x * channels) = avgB[index] + (sigB[index] - miu_sigB > 0 ? sigB[index], miu_sigB : 0) / (max(sigB[index], miu_sigB)) * (*(pRealData + pit * y + x * channels) - avgB[index]);
			}
			index++;
		}
	}
	delete[] avgR;
	delete[] avgG;
	delete[] avgB;
	delete[] sigR;
	delete[] sigG;
	delete[] sigB;
	avgR = nullptr;
	avgG = nullptr;
	avgB = nullptr;
	sigR = nullptr;
	sigG = nullptr;
	sigB = nullptr;
	::PostMessage(DISP->HWnd, WM_MSG_RECEIVED, 1, (LPARAM)param);
	return 0;
}