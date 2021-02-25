#include "stdafx.h"
#include "Display.h"


Display Display::display;

Display* Display::GetInstance()
{
	return &Display::display;
}

//输出参数
void Display::OutputLine(const CString &str)
{
	this->OutputArea->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
	this->OutputArea->SetSel(this->OutputArea->GetWindowTextLength(), this->OutputArea->GetWindowTextLength(), FALSE);
	this->OutputArea->ReplaceSel(str + _T("\r\n"));
}

//清空输出参数面板
void Display::ClearOutput(void)
{
	this->OutputArea->SetWindowTextW(_T(""));
}

ThreadOption Display::GetThreadOption(void)
{
	ThreadOption ret;
	ret.threadnum = this->ThreadSlider->GetPos();
	switch (this->ThreadType->GetCurSel())
	{
	case 0:
		ret.type = ThreadOption::Win;
		break;
	case 1:
		ret.type = ThreadOption::Openmp;
		break;
	case 2:
		ret.type = ThreadOption::Cuda;
		break;
	}
	return ret;
}

//OnPaint
void Display::PaintCImageToCStatic(CImage* img, CStatic* s)
{
	int height, width;
	CRect rect, rect1;
	height = img->GetHeight();
	width = img->GetWidth();

	s->GetClientRect(&rect);
	CDC* pDC = s->GetDC();
	SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

	if (width <= rect.Width() && height <= rect.Height())
	{
		rect1 = CRect(rect.TopLeft(), CSize(width, height));
		img->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
	}
	else
	{
		float xScale = rect.Width() / (float)width;
		float yScale = rect.Height() / (float)height;
		float ScaleIndex = xScale <= yScale ? xScale : yScale;
		rect1 = CRect(rect.TopLeft(), CSize(static_cast<int>(width * ScaleIndex), static_cast<int>(height * ScaleIndex)));
		img->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
	}
	s->ReleaseDC(pDC);
}

//计算时间
void Display::StartTick(void)
{
	this->startTime = CTime::GetTickCount();
}

void Display::PrintTimeElapsed(void)
{
	CString str;
	const static CString threadstr[] = { _T("Windows AFX"), _T("OpenMP"), _T("Cuda") };
	auto to = this->GetThreadOption();
	if (to.type == ThreadOption::Cuda)
	{
		str.Format(_T("Time elapsed: %d ms (Cuda)"),
			CTime::GetTickCount() - this->startTime);
	}
	else
	{
		str.Format(_T("Time elapsed: %d ms (%d thread(s) by %s)"),
			CTime::GetTickCount() - this->startTime,
			to.threadnum, threadstr[(int)to.type]);
	}
	this->OutputLine(str);
}