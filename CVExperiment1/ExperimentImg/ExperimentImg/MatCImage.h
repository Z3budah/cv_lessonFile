#pragma once

#include "stdafx.h"
#include "afxwin.h"
#include <opencv2/core/core.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/highgui/highgui.hpp>  

using namespace cv;
using namespace std;
class MatCImage
{
public:
	/*MatToCImage
	*简介：
	*   OpenCV的Mat转ATL/MFC的CImage，仅支持单通道灰度或三通道彩色
	*参数：
	*   mat：OpenCV的Mat
	*   cimage：ATL/MFC的CImage
	*/
	void MatToCImage(Mat& mat, CImage& cimage);


	/*CImageToMat
	*简介：
	*   ATL/MFC的CImage转OpenCV的Mat，仅支持单通道灰度或三通道彩色
	*参数：
	*   cimage：ATL/MFC的CImage
	*   mat：OpenCV的Mat
	*/
	void CImageToMat(CImage& cimage, Mat& mat);


	// VS默认工程是Unicode编码（宽字节），有时需要ANSI，即单字节，实现宽到单的转化  
	string CString2StdString(const CString& cstr);
	// 显示图像到指定窗口  
	void DisplayImage(CWnd* m_pMyWnd, const CImage& image);

	// 格式转换，AWX云图转到可以显示的opencv支持的格式  
	Mat AWX2Mat(CString filePath);

	void DisplayImageEx(CWnd* pWnd, const CImage& image);
};