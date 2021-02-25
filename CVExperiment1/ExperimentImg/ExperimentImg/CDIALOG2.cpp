// CDIALOG2.cpp: 实现文件
//

//#include "pch.h"
#include"stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "CDIALOG2.h"
#include "afxdialogex.h"
#include "Display.h"
#include"ImageProcess.h"


// CDIALOG2 对话框

IMPLEMENT_DYNAMIC(CDIALOG2, CDialogEx)

CDIALOG2::CDIALOG2(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{

}

CDIALOG2::~CDIALOG2()
{
}

void CDIALOG2::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_EXAMPLE, mCombeExample);

}


BEGIN_MESSAGE_MAP(CDIALOG2, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_EXAMPLE, &CDIALOG2::OnCbnSelchangeComboExample)
END_MESSAGE_MAP()


// CDIALOG2 消息处理程序

BOOL CDIALOG2::OnInitDialog()
{
	CDialog::OnInitDialog();
	mCombeExample.InsertString(0,_T("椒盐噪声"));
	mCombeExample.InsertString(1,_T("中值滤波"));
	mCombeExample.SetCurSel(0);
	return TRUE;
}

void CDIALOG2::DoProcess(CImage* image)
{
	switch (mCombeExample.GetCurSel())
	{
	case 0://椒盐噪声
	{
		this->DoSaltAndPepperNoise(image);
		break;
	}

	case 1://中值滤波
	{
		this->DoMedianFilter(image);
		break;
	}
	default:
		break;
	}

}

void CDIALOG2::DoSaltAndPepperNoise(CImage* image)
{
	DISP->OutputLine(_T("开始处理 椒盐噪声"));
	auto thread = DISP->GetThreadOption();//WIN or Openmp
	ThreadParam* m_pThreadParam = new ThreadParam[thread.threadnum];
	if (thread.type == ThreadOption::Win)
	{

		int subLength = image->GetWidth() * image->GetHeight() / thread.threadnum;
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : image->GetWidth() * image->GetHeight() - 1;
			m_pThreadParam[i].src = image;
			AfxBeginThread((AFX_THREADPROC)ImageProcess::addNoise, &m_pThreadParam[i]);
		}
	}
	else//Openmp
	{
		int subLength = image->GetWidth() * image->GetHeight() / thread.threadnum;

#pragma omp parallel for num_threads(thread.threadnum)

		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : image->GetWidth() * image->GetHeight() - 1;
			m_pThreadParam[i].src = image;
			ImageProcess::addNoise(&m_pThreadParam[i]);
		}

	}
}

void CDIALOG2::DoMedianFilter(CImage* image)
{
	DISP->OutputLine(_T("开始处理 中值滤波"));
	auto thread = DISP->GetThreadOption();//WIN or Openmp
	ThreadParam* m_pThreadParam = new ThreadParam[thread.threadnum];
	if (thread.type == ThreadOption::Win)
	{
		int subLength = image->GetWidth() * image->GetHeight() / thread.threadnum;
		int h = image->GetHeight() / thread.threadnum;
		int w = image->GetWidth() / thread.threadnum;
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : image->GetWidth() * image->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = image;
			AfxBeginThread((AFX_THREADPROC)ImageProcess::medianFilter, &m_pThreadParam[i]);
		}
	}
	else//Openmp
	{
		int subLength = image->GetWidth() * image->GetHeight() / thread.threadnum;

#pragma omp parallel for num_threads(thread.threadnum)

		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : image->GetWidth() * image->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = image;
			ImageProcess::medianFilter(&m_pThreadParam[i]);
		}

	}
}
void CDIALOG2::OnCbnSelchangeComboExample()
{
	// TODO: 在此添加控件通知处理程序代码
}
