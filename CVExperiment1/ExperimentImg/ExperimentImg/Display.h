#pragma once
#include"stdafx.h"

//线程操作
struct ThreadOption
{
	enum Type
	{
		Win, Openmp, Cuda
	};
	Type type;
	int threadnum;
};

class Display
{
private:
	CTime startTime;
	static Display display;
	Display() = default;
public:

	~Display() = default;
	static Display* GetInstance();
	HWND HWnd;
	CEdit* OutputArea;

	CStatic* PictureOrigin;
	CStatic* Picture;
	CComboBox* ThreadType;
	CSliderCtrl* ThreadSlider;//线程数
	CImage **DImage;

	void OutputLine(const CString& str);
	void ClearOutput(void);
	ThreadOption GetThreadOption(void);
	void PaintCImageToCStatic(CImage* i, CStatic* s);
	void StartTick(void);
	void PrintTimeElapsed(void);
};
#define DISP (Display::GetInstance())

