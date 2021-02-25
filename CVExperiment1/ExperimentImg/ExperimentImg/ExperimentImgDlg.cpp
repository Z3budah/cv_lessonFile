
// ExperimentImgDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "afxdialogex.h"
#include"Display.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CExperimentImgDlg �Ի���



CExperimentImgDlg::CExperimentImgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXPERIMENTIMG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);


}

void CExperimentImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_EDIT_INFO, mEditInfo);
	DDX_Control(pDX, IDC_PICTURE, mPictureControl);//ԭʼͼ��
	DDX_Control(pDX, IDC_CHECK_100, m_CheckCirculation);//ѭ��100
	DDX_Control(pDX, IDC_PICTURE1, mPictureControl1);//������ͼ��
	DDX_Control(pDX, IDC_TAB, m_tab);//tab��ҳ
	DDX_Control(pDX, IDC_OUTPUT, mOutput);//�������
	DDX_Control(pDX, IDC_COMBO_THREAD, mComboThreadType);//�߳�����
	DDX_Control(pDX, IDC_SLIDER_THREADNUM, mSliderThreadNum);//�߳���Ŀ
	DDX_Control(pDX, IDC_BUTTON_PROCESS, mButtonProcess);
}

BEGIN_MESSAGE_MAP(CExperimentImgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CExperimentImgDlg::OnBnClickedButtonOpen)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CExperimentImgDlg::OnTcnSelchangeTab)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_THREADNUM, &CExperimentImgDlg::OnNMCustomdrawSliderThreadnum)
	ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CExperimentImgDlg::OnBnClickedButtonProcess)
	ON_MESSAGE(WM_MSG_RECEIVED, &CExperimentImgDlg::OnMsgReceived)
//ON_MESSAGE(WM_NOISE, &CExperimentImgDlg::OnNoiseThreadMsgReceived)
//ON_MESSAGE(WM_MEDIAN_FILTER, &CExperimentImgDlg::OnMedianFilterThreadMsgReceived)
	ON_BN_CLICKED(IDC_BUTTON1, &CExperimentImgDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CExperimentImgDlg::OnBnClickedButton2)
//ON_EN_CHANGE(IDC_OUTPUT, &CExperimentImgDlg::OnEnChangeOutput)
END_MESSAGE_MAP()


// CExperimentImgDlg ��Ϣ�������

BOOL CExperimentImgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	//��ʼ��
	m_pImgSrc = NULL;
	m_pImgCpy = NULL;
	//Displayini
	auto disp = Display::GetInstance();
	disp->HWnd = this->GetSafeHwnd();
	disp->OutputArea = &this->mOutput;
	disp->PictureOrigin = &this->mPictureControl;
	disp->Picture = &this->mPictureControl1;
	disp->ThreadType = &this->mComboThreadType;
	disp->ThreadSlider = &this->mSliderThreadNum;
	disp->DImage = &this->m_pImgSrc;
	DISP->OutputLine(_T("��ʼ��...\n"));
	//m_nThreadNum = 1;
	//m_pThreadParam = new ThreadParam[MAX_THREAD];
	//srand(time(0));

	//tab��ҳ
	//ΪTab Control����3��ҳ��  
	m_tab.InsertItem(0, _T("��ֵ�븵��Ҷ"));
	m_tab.InsertItem(1, _T("������������ֵ�˲�"));
	m_tab.InsertItem(2, _T("��˹�������˲�"));
	//����3���Ի���  
	m_page1.Create(IDD_DIALOG1, &m_tab);
	m_page2.Create(IDD_DIALOG2, &m_tab);
	m_page3.Create(IDD_DIALOG3, &m_tab);

	//�趨��Tab����ʾ�ķ�Χ  
	CRect tabRect;
	m_tab.GetClientRect(&tabRect);
	tabRect.left += 1;
	tabRect.right -= 1;
	tabRect.top += 22;
	tabRect.bottom -= 1;

	m_page1.MoveWindow(&tabRect);
	m_page2.MoveWindow(&tabRect);	
	m_page3.MoveWindow(&tabRect);

	//��ʾ��ʼҳ��  
	m_page1.ShowWindow(SW_SHOW);
	m_tab.SetCurSel(0);

	//�����߳�
	mComboThreadType.InsertString(0, _T("WIN���߳�"));
	mComboThreadType.InsertString(1, _T("OpenMP"));
	mComboThreadType.InsertString(2, _T("CUDA"));
	mComboThreadType.SetCurSel(0);

	mSliderThreadNum.SetRange(1, MAX_THREAD, TRUE);
	mSliderThreadNum.SetPos(MAX_THREAD);


	AfxBeginThread((AFX_THREADPROC)&CExperimentImgDlg::Update, this);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CExperimentImgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CExperimentImgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		/*
		if (m_pImgSrc != NULL)
		{
			int height;
			int width;
			CRect rect;
			CRect rect1;
			height = m_pImgSrc->GetHeight();
			width = m_pImgSrc->GetWidth();

			mPictureControl.GetClientRect(&rect);
			CDC *pDC = mPictureControl.GetDC();
			SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

			if (width <= rect.Width() && height <= rect.Height())
			{
				rect1 = CRect(rect.TopLeft(), CSize(width, height));
				m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			else
			{
				float xScale = (float)rect.Width() / (float)width;
				float yScale = (float)rect.Height() / (float)height;
				float ScaleIndex = (xScale <= yScale ? xScale : yScale);
				rect1 = CRect(rect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
				//��picture control������ͼ�����ź�Ĵ�С
//				CWnd *pWnd;
//				pWnd = GetDlgItem(IDC_PICTURE);
//				pWnd->MoveWindow(CRect((int)rect.top, (int)rect.left, (int)width*ScaleIndex, (int)height*ScaleIndex));
				m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			ReleaseDC(pDC);
		}*/
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CExperimentImgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


UINT CExperimentImgDlg::Update(void* p)
{
	while (1)
	{
		Sleep(200);
		CExperimentImgDlg* dlg = (CExperimentImgDlg*)p;
		if (dlg->m_pImgSrc != NULL)
		{
			int height;
			int width;
			CRect rect;
			CRect rect1;
			height = dlg->m_pImgSrc->GetHeight();
			width = dlg->m_pImgSrc->GetWidth();

			dlg->mPictureControl1.GetClientRect(&rect);
			CDC *pDC = dlg->mPictureControl1.GetDC();
			SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

			if (width <= rect.Width() && height <= rect.Width())
			{
				rect1 = CRect(rect.TopLeft(), CSize(width, height));
				dlg->m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			else
			{
				float xScale = (float)rect.Width() / (float)width;
				float yScale = (float)rect.Height() / (float)height;
				float ScaleIndex = (xScale <= yScale ? xScale : yScale);
				rect1 = CRect(rect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex));
				dlg->m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			dlg->ReleaseDC(pDC);
		}
	}
	return 0;
}

void CExperimentImgDlg::ThreadDraw(DrawPara *p)
{
	CRect rect;
	GetClientRect(&rect);
	CDC     memDC;             // ���ڻ����ͼ���ڴ滭��  
	CBitmap memBitmap;         // ���ڻ����ͼ���ڴ滭��
	memDC.CreateCompatibleDC(p->pDC);  // ������ԭ���ʼ��ݵĻ���
	memBitmap.CreateCompatibleBitmap(p->pDC, p->width, p->height);  // ������ԭλͼ���ݵ��ڴ滭��
	memDC.SelectObject(&memBitmap);      // ���������뻭���Ĺ���
	memDC.FillSolidRect(rect, p->pDC->GetBkColor());
	p->pDC->SetStretchBltMode(HALFTONE);
	// ��pImgSrc���������Ż����ڴ滭����
	p->pImgSrc->StretchBlt(memDC.m_hDC, 0, 0, p->width, p->height);

	// ���ѻ��õĻ������Ƶ������Ļ�������
	p->pDC->BitBlt(p->oriX, p->oriY, p->width, p->height, &memDC, 0, 0, SRCCOPY);
	memBitmap.DeleteObject();
	memDC.DeleteDC();
}

 void CExperimentImgDlg::ImageCopy(CImage* pImgSrc, CImage* pImgDrt)
{
	int MaxColors = pImgSrc->GetMaxColorTableEntries();
	RGBQUAD* ColorTab;
	ColorTab = new RGBQUAD[MaxColors];

	CDC *pDCsrc, *pDCdrc;
	if (!pImgDrt->IsNull())
	{
		pImgDrt->Destroy();
	}
	pImgDrt->Create(pImgSrc->GetWidth(), pImgSrc->GetHeight(), pImgSrc->GetBPP(), 0);

	if (pImgSrc->IsIndexed())
	{
		pImgSrc->GetColorTable(0, MaxColors, ColorTab);
		pImgDrt->SetColorTable(0, MaxColors, ColorTab);
	}

	pDCsrc = CDC::FromHandle(pImgSrc->GetDC());
	pDCdrc = CDC::FromHandle(pImgDrt->GetDC());
	pDCdrc->BitBlt(0, 0, pImgSrc->GetWidth(), pImgSrc->GetHeight(), pDCsrc, 0, 0, SRCCOPY);
	pImgSrc->ReleaseDC();
	pImgDrt->ReleaseDC();
	delete ColorTab;

}

void CExperimentImgDlg::OnBnClickedButtonOpen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	TCHAR szFilter[] = _T("JPEG(*jpg)|*.jpg|*.bmp|*.png|TIFF(*.tif)|*.tif|All Files ��*.*��|*.*||");
	CString filePath("");
	
	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		strFilePath=filePath;
//		mEditInfo.SetWindowTextW(strFilePath);	//���ı�������ʾͼ��·��

		if (m_pImgSrc != NULL && m_pImgCpy != NULL)
		{
			m_pImgSrc->Destroy();
			delete m_pImgSrc;
			m_pImgCpy->Destroy();
			delete m_pImgCpy;
		}
		m_pImgSrc = new CImage();
		m_pImgSrc->Load(strFilePath);
		m_pImgCpy = new CImage();
		m_pImgCpy->Load(strFilePath);
		DISP->PaintCImageToCStatic(m_pImgSrc, &mPictureControl);
		DISP->OutputLine(_T("����ͼƬ") + strFilePath);
	
		//�ػ洰��this->Invalidate();
		

	}
}

void CExperimentImgDlg::OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString text("");
	m_nThreadNum = mSliderThreadNum.GetPos();
	text.Format(_T("%d"), m_nThreadNum);
	GetDlgItem(IDC_STATIC_THREADNUM)->SetWindowText(text);
	*pResult = 0;
}


void CExperimentImgDlg::OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	switch (m_tab.GetCurSel())
	{
	case 0:
		m_page1.ShowWindow(SW_SHOW);
		m_page2.ShowWindow(SW_HIDE);
		m_page3.ShowWindow(SW_HIDE);
		m_page1.SetFocus();
		break;
	case 1:
		m_page1.ShowWindow(SW_HIDE);
		m_page2.ShowWindow(SW_SHOW);
		m_page3.ShowWindow(SW_HIDE);
		m_page1.SetFocus();
		break;
	case 2:
		m_page1.ShowWindow(SW_HIDE);
		m_page2.ShowWindow(SW_HIDE);
		m_page3.ShowWindow(SW_SHOW);
		m_page1.SetFocus();
		break;
	default:break;
	}
	*pResult = 0;
}

void CExperimentImgDlg::OnBnClickedButtonProcess()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	this->mButtonProcess.EnableWindow(false);//����
	DISP->StartTick();//��ʱ
	switch (m_tab.GetCurFocus())
	{
	case 0:  //
		m_page1.DoProcess(m_pImgSrc);
		break;
	case 1: //tab2
		m_page2.DoProcess(m_pImgSrc);
		break;
	case 2: //tab3
		m_page3.DoProcess(m_pImgSrc);
		break;
	default:
		break;
	}


	//DISP->PaintCImageToCStatic(m_pImgSrc, &mPictureControl1);

}



LRESULT CExperimentImgDlg::OnMsgReceived(WPARAM wParam, LPARAM lParam)
{		
;

	static int cnt = 0;
	++cnt;

	if (Display::GetInstance()->GetThreadOption().threadnum == cnt || wParam == 0)
	{
		cnt = 0;
		auto p = (ThreadParam*)lParam;
		if (p->callback != nullptr)
		p->callback(p);
		DISP->PrintTimeElapsed();
		DISP->PaintCImageToCStatic(((ThreadParam*)lParam)->src, &mPictureControl1);
		if (p->para != nullptr)
			delete p->para;
		if (p->tppa != nullptr)
			delete[] p->tppa; // Ŀǰtppa��ThraedParam[]
	}	
	this->mButtonProcess.EnableWindow(true);
	return LRESULT();
}

void CExperimentImgDlg::OnBnClickedButton1()
{
	DISP->ClearOutput();
}


void CExperimentImgDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	delete m_pImgSrc;
	m_pImgSrc = new CImage;
	m_pImgSrc->Load(this->strFilePath);
}

