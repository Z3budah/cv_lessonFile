
// ExperimentImgDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include "ImageProcess.h"

#include "CDIALOG1.h"
#include "CDIALOG2.h"
#include "CDIALOG3.h"

#define MAX_THREAD 8
#define MAX_SPAN 15
#define MAX_FACTOR 16
#define MAX_GAUSS_OFFSET 120
#define MAX_SIGMA 150
struct DrawPara
{
	CImage* pImgSrc;
	CDC* pDC;
	int oriX;
	int oriY;
	int width;
	int height;
};

// CExperimentImgDlg 对话框
class CExperimentImgDlg : public CDialogEx
{
// 构造
public:
	CExperimentImgDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENTIMG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	CImage* getImage() { return m_pImgSrc; }
	void ThreadDraw(DrawPara *p);
	static UINT Update(void* p);

	afx_msg LRESULT CExperimentImgDlg::OnMsgReceived(WPARAM wParam, LPARAM lParam);
// 实现
protected:
	HICON m_hIcon;
	CImage * m_pImgSrc;
	CImage * m_pImgCpy;
	int m_nThreadNum;
	ThreadParam* m_pThreadParam;
	CTime startTime;
//	ThreadParam * m_pThreadParam;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:	
	static void ImageCopy(CImage* pImgSrc, CImage* pImgDrt);

	CString strFilePath;
	afx_msg void OnBnClickedButtonOpen();
	CEdit mEditInfo;
	CStatic mPictureControl;

	afx_msg void OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonProcess();
	CButton m_CheckCirculation;

	CStatic mPictureControl1;
	CTabCtrl m_tab;
	afx_msg void OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	CDIALOG1 m_page1;
	CDIALOG2 m_page2;
	CDIALOG3 m_page3;
	
	CEdit mOutput;

	CComboBox mComboThreadType;
	CSliderCtrl mSliderThreadNum;
	afx_msg void OnBnClickedButton1();
	CButton mButtonProcess;
	afx_msg void OnBnClickedButton2();
	//afx_msg void OnEnChangeOutput();
};
