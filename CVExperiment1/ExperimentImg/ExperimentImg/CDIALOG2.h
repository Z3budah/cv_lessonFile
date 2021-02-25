#pragma once


// CDIALOG2 对话框

class CDIALOG2 : public CDialogEx
{
	DECLARE_DYNAMIC(CDIALOG2)

public:
	CDIALOG2(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDIALOG2();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif
private:
	void DoSaltAndPepperNoise(CImage* image);
	void DoMedianFilter(CImage* image);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CComboBox mCombeExample;
	afx_msg void OnCbnSelchangeComboExample();
	void DoProcess(CImage* image);

};
