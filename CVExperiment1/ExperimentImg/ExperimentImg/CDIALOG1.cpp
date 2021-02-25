// CDIALOG1.cpp: 实现文件
//

//#include "pch.h"
#include"stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "CDIALOG1.h"
#include "afxdialogex.h"
#include"Display.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>
#include "MatCImage.h"

// CDIALOG1 对话框

IMPLEMENT_DYNAMIC(CDIALOG1, CDialogEx)

CDIALOG1::CDIALOG1(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CDIALOG1::~CDIALOG1()
{
}

void CDIALOG1::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO, mSRFunction);
	DDX_Control(pDX, IDC_SLIDER, mscaleSlider);
	DDX_Control(pDX, IDC_ANGLE, mAngle);
}


BEGIN_MESSAGE_MAP(CDIALOG1, CDialogEx)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER, &CDIALOG1::OnNMCustomdrawSlider)
END_MESSAGE_MAP()

BOOL CDIALOG1::OnInitDialog()
{
	CDialog::OnInitDialog();
	mSRFunction.InsertString(0, _T("缩放图像"));
	mSRFunction.InsertString(1, _T("旋转图像"));
	mSRFunction.InsertString(2, _T("傅里叶变换"));
	mSRFunction.SetCurSel(0);

	mscaleSlider.SetRange(4, MAX_FACTOR);
	mscaleSlider.SetPos(8);

	return 0;
}

// CDIALOG1 消息处理程序


void CDIALOG1::DoProcess(CImage *img)
{
	
	switch (mSRFunction.GetCurSel())
	{
	case 0://缩放
	{
		this->DoScale(img);
		break;
	}
	case 1://旋转
	{
		this->DoRotate(img);
		break;
	}
	case 2://傅里叶变换
	{
		this->DoFourier(img);
		break;
	}
	default:
		break;
	}
	
}

void CDIALOG1::DoScale(CImage *image)
{



	auto thread = DISP->GetThreadOption();//WIN or Openmp

	factor = (float)((float)(mscaleSlider.GetPos()) / 8.0);
	CString strf;
	strf.Format(_T("%f"), factor);
	DISP->OutputLine(_T("开始处理 缩放图像 比例参数为")+strf);
	CExperimentImgDlg::ImageCopy(image, &temp);
	szVert.CImageToMat(temp, OriImage);

	ThreadParam* m_pThreadParam = new ThreadParam[thread.threadnum];
	auto* sp = new ScaleParam;
	sp->img = OriImage;
	sp->factor = factor;

	if (thread.type == ThreadOption::Win)
	{
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].para = sp;
			//CString str;
			//str.Format(_T("factor is %f"), sp->factor);
			//DISP->OutputLine(str);
			AfxBeginThread((AFX_THREADPROC)ImageProcess::scalePic, &m_pThreadParam[i]);
		}
		
	}
	else//openmp
	{
#pragma omp parallel for num_threads(thread.threadnum)
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].para = sp;
			ImageProcess::scalePic(&m_pThreadParam[i]);
		}

	}
}

void CDIALOG1::DoRotate(CImage* image)
{
	CString str;
	mAngle.GetWindowTextW(str);
	double angle = _ttof(str);

	DISP->OutputLine(_T("开始处理 旋转图像 旋转角度为")+str+_T("°"));	
	CExperimentImgDlg::ImageCopy(image, &temp);
	auto* rp = new RotateParam;
	rp->img = &temp;
	rp->angle = angle;
	
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
			m_pThreadParam[i].para = rp;
			AfxBeginThread((AFX_THREADPROC)ImageProcess::rotatePic, &m_pThreadParam[i]);
		}

	}
	else//openmp
	{
		int subLength = image->GetWidth() * image->GetHeight() / thread.threadnum;
		int h = image->GetHeight() / thread.threadnum;
		int w = image->GetWidth() / thread.threadnum;
#pragma omp parallel for num_threads(thread.threadnum)
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : image->GetWidth() * image->GetHeight() - 1;
			m_pThreadParam[i].maxSpan = MAX_SPAN;
			m_pThreadParam[i].src = image;
			m_pThreadParam[i].para = rp;
			ImageProcess::rotatePic(&m_pThreadParam[i]);
		}

	}

}

void CDIALOG1::DoFourier(CImage* image)
{	auto thread = DISP->GetThreadOption();//WIN or Openmp
	DISP->OutputLine(_T("开始处理 傅里叶变换"));

	CImage* grayscale = new CImage;
	grayscale->Create(image->GetWidth(), image->GetHeight(), image->GetBPP());
	*(DISP->DImage) = grayscale;
	//CExperimentImgDlg::ImageCopy(grayscale, &temp);
	
	ThreadParam* m_pThreadParam = new ThreadParam[thread.threadnum];
	if (thread.type == ThreadOption::Win)
	{
		int subLength = grayscale->GetWidth() * grayscale->GetHeight() / thread.threadnum;
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : grayscale->GetWidth() * grayscale->GetHeight() - 1;
			m_pThreadParam[i].src = grayscale;
			m_pThreadParam[i].dst = image;
			AfxBeginThread(ImageProcess::FourierTransform, &m_pThreadParam[i]);
		}
	}
	else //openmp
	{
		int subLength = grayscale->GetWidth() * grayscale->GetHeight() / thread.threadnum;
#pragma omp parallel for num_threads(thread.threadnum)
		for (int i = 0; i < thread.threadnum; ++i)
		{
			m_pThreadParam[i].startIndex = i * subLength;
			m_pThreadParam[i].endIndex = i != thread.threadnum - 1 ?
				(i + 1) * subLength - 1 : grayscale->GetWidth() * grayscale->GetHeight() - 1;
			m_pThreadParam[i].src = grayscale;
			m_pThreadParam[i].dst = image;
			ImageProcess::FourierTransform(&m_pThreadParam[i]);
		}
	}


}


void CDIALOG1::OnNMCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString fact;

	factor = (float)((float)(mscaleSlider.GetPos()) / 8.0);
	fact.Format(_T("%f"), factor);
	GetDlgItem(IDC_STATIC_FACT)->SetWindowText(fact);
	*pResult = 0;
}




