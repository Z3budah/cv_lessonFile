#pragma once
#include "afxwin.h"
#include "ImageProcess.h"
#include "afxcmn.h"
#include "MatCImage.h"


// CDIALOG1 对话框

class CDIALOG1 : public CDialogEx
{
	DECLARE_DYNAMIC(CDIALOG1)

public:
	CDIALOG1(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDIALOG1();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif
private:
	void DoScale(CImage* img);
	void DoRotate(CImage* img);
	void DoFourier(CImage* image);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	float factor;
	CImage temp;
	MatCImage szVert;
	Mat OriImage;
	Mat DestImage;

	DECLARE_MESSAGE_MAP()
public:

	CComboBox mSRFunction;
	CEdit mSRParameter;

	void DoProcess(CImage* img);
	afx_msg void OnNMCustomdrawSlider(NMHDR* pNMHDR, LRESULT* pResult);
	CSliderCtrl mscaleSlider;
	CEdit mAngle;
};
