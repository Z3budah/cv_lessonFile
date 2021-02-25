// CDIALOG3.cpp: 实现文件
//

//#include "pch.h"
#include"stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "CDIALOG3.h"
#include "afxdialogex.h"
#include "Display.h"
#include"ImageProcess.h"


// CDIALOG3 对话框

IMPLEMENT_DYNAMIC(CDIALOG3, CDialogEx)

CDIALOG3::CDIALOG3(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG3, pParent)
{

}

CDIALOG3::~CDIALOG3()
{
}

void CDIALOG3::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, mCombo);
	DDX_Control(pDX, IDC_MEAN, mGaussMean);
	DDX_Control(pDX, IDC_SIGMA, mGaussSigma);
}


BEGIN_MESSAGE_MAP(CDIALOG3, CDialogEx)

END_MESSAGE_MAP()

// CDIALOG3 消息处理程序

BOOL CDIALOG3::OnInitDialog()
{
	CDialog::OnInitDialog();
	mCombo.InsertString(0, _T("高斯噪声"));
	mCombo.InsertString(1, _T("平滑线性滤波"));
	mCombo.InsertString(2, _T("高斯滤波"));
	mCombo.InsertString(3, _T("维纳滤波"));
	mCombo.SetCurSel(0);
	return TRUE;
}

void CDIALOG3::DoProcess(CImage* image)
{
	switch (mCombo.GetCurSel())
	{
	case 0://高斯噪声
	{
		this->DoGaussNoise(image);
		break;
	}
	case 1://平滑线性滤波
	{
		this->DoAvgFilter(image);
		break;
	}
	case 2://高斯滤波
	{
		this->DoGaussFilter(image);
		break;
	}
	case 3://维纳滤波
	{
		this->DoWienerFilter(image);
		break;
	}
	default:break;
	}

}

void CDIALOG3::DoGaussNoise(CImage* image)
{
	CString str;
	double mean, sigma;
	mGaussMean.GetWindowTextW(str);
	mean = _ttof(str);
	mGaussSigma.GetWindowTextW(str);
	sigma = _ttof(str);
	str.Format(_T("开始处理 高斯噪声 均值=%f 标准差=%f"), mean, sigma);
	DISP->OutputLine(str);

	auto thread = DISP->GetThreadOption();//WIN or Openmp
	auto gp = new GaussParam;
	gp->img = nullptr;
	gp->mean = mean;
	gp->sigma = sigma;
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
			m_pThreadParam[i].para = gp;
			AfxBeginThread((AFX_THREADPROC)ImageProcess::gaussNoise, &m_pThreadParam[i]);
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
			m_pThreadParam[i].para = gp;
			ImageProcess::gaussNoise(&m_pThreadParam[i]);
		}

	}
}

void CDIALOG3::DoAvgFilter(CImage* image)
{
	DISP->OutputLine(_T("开始处理 平滑线性滤波"));
	auto thread = DISP->GetThreadOption();//WIN or Openmp

	CImage* dest = new CImage;
	dest->Create(image->GetWidth(), image->GetHeight(), image->GetBPP());
	*(DISP->DImage) = dest;

	ThreadParam* m_pThreadParam = new ThreadParam[thread.threadnum];
	if (thread.type == ThreadOption::Win)
	{
		int subLength = dest->GetWidth() * dest->GetHeight() / thread.threadnum;
		int h = dest->GetHeight() / thread.threadnum;
		int w = dest->GetWidth() / thread.threadnum;
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : dest->GetWidth() * dest->GetHeight() - 1;
			m_pThreadParam[i].src = dest;
			m_pThreadParam[i].dst = image;
			AfxBeginThread((AFX_THREADPROC)ImageProcess::avgFilter, &m_pThreadParam[i]);
		}
	}
	else//Openmp
	{
		int subLength = dest->GetWidth() * dest->GetHeight() / thread.threadnum;
		int h = dest->GetHeight() / thread.threadnum;
		int w = dest->GetWidth() / thread.threadnum;

#pragma omp parallel for num_threads(thread.threadnum)

		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : dest->GetWidth() * dest->GetHeight() - 1;
			m_pThreadParam[i].src = dest;
			m_pThreadParam[i].dst = image;
			ImageProcess::avgFilter(&m_pThreadParam[i]);
		}

	}
}

/*void CDIALOG3::DoGaussFilter(CImage* image)
{
	CString str;
	double mean, sigma;
	mGaussMean.GetWindowTextW(str);
	mean = _ttof(str);
	mGaussSigma.GetWindowTextW(str);
	sigma = _ttof(str);
	str.Format(_T("开始处理 高斯滤波 均值=%f 标准差=%f"), mean, sigma);
	DISP->OutputLine(str);

	auto thread = DISP->GetThreadOption();//WIN or Openmp
	auto gp = new GaussParam;
	gp->img = image;
	gp->mean = mean;
	gp->sigma = sigma;

	CImage* dest = new CImage;
	dest->Create(image->GetWidth(), image->GetHeight(), image->GetBPP());
	*(DISP->DImage) = dest;

	ThreadParam* m_pThreadParam = new ThreadParam[thread.threadnum];
	if (thread.type == ThreadOption::Win)
	{
		int subLength = dest->GetWidth() * dest->GetHeight() / thread.threadnum;
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : dest->GetWidth() * dest->GetHeight() - 1;
			m_pThreadParam[i].src = dest;
			m_pThreadParam[i].para = gp;
			AfxBeginThread((AFX_THREADPROC)ImageProcess::gaussFilter, &m_pThreadParam[i]);
		}
	}
	else//Openmp
	{
		int subLength = dest->GetWidth() * dest->GetHeight() / thread.threadnum;
#pragma omp parallel for num_threads(thread.threadnum)


		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : dest->GetWidth() * dest->GetHeight() - 1;
			m_pThreadParam[i].src = dest;
			m_pThreadParam[i].para = gp;
			ImageProcess::gaussFilter(&m_pThreadParam[i]);
		}

	}
}*/
void CDIALOG3::DoGaussFilter(CImage* image)
{
	CString str;
	double mean, sigma;
	mGaussMean.GetWindowTextW(str);
	mean = _ttof(str);
	mGaussSigma.GetWindowTextW(str);
	sigma = _ttof(str);
	str.Format(_T("开始处理 高斯滤波 均值=%f 标准差=%f"), mean, sigma);
	DISP->OutputLine(str);
	auto thread = DISP->GetThreadOption();//WIN or Openmp
	auto gp = new GaussParam;
	gp->img = image;
	gp->mean = mean;
	gp->sigma = sigma;

	ThreadParam* m_pThreadParam = new ThreadParam[thread.threadnum];
	if (thread.type == ThreadOption::Win)
	{
		int subLength = image->GetWidth() * image->GetHeight() / thread.threadnum;

		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : image->GetWidth() * image->GetHeight() - 1;
			m_pThreadParam[i].src =image;
			m_pThreadParam[i].para = gp;
			AfxBeginThread((AFX_THREADPROC)ImageProcess::gaussFilter, &m_pThreadParam[i]);
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
			m_pThreadParam[i].para = gp;
			ImageProcess::gaussFilter(&m_pThreadParam[i]);
		}

	}
}

void CDIALOG3::DoWienerFilter(CImage* image)
{
	DISP->OutputLine(_T("开始处理 维纳滤波"));
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
			AfxBeginThread((AFX_THREADPROC)ImageProcess::wienerFilter, &m_pThreadParam[i]);
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
			ImageProcess::wienerFilter(&m_pThreadParam[i]);
		}

	}
}