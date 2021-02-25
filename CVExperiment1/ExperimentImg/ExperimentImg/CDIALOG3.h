#pragma once


// CDIALOG3 对话框

class CDIALOG3 : public CDialogEx
{
	DECLARE_DYNAMIC(CDIALOG3)

public:
	CDIALOG3(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDIALOG3();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif
private:
	void DoGaussNoise(CImage* image);
	void DoAvgFilter(CImage* image);
	void DoGaussFilter(CImage* image);
	void DoWienerFilter(CImage* image);
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox mCombo;
	CEdit mGaussMean;
	CEdit mGaussSigma;
	afx_msg void OnCbnSelchangeComboExample();
	void DoProcess(CImage* image);
};
