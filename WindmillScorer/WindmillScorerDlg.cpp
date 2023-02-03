
// WindmillScorerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "WindmillScorer.h"
#include "WindmillScorerDlg.h"
#include "afxdialogex.h"
#include "..\..\yUtils\MyWindows.h"
#include "..\..\yUtils\MyFileDialog.h"
#include "..\..\ImageRLib\ImageRIF.h"
#include "..\..\ImageRLib\Position.h"
#include "..\..\ImageRLib\MultiDataF.h"
#include "Config.h"
#include "WMAScorer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CWindmillScorerDlg dialog



CWindmillScorerDlg::CWindmillScorerDlg(CWnd* pParent /*=nullptr*/)
	: CMyDialogEx(IDD_WINDMILLSCORER_DIALOG, pParent)
	, mpImageRIF(NULL)
	, mfLog("WindmillScorerDlg")
	, miPos(0)
	, miPos2d(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CWindmillScorerDlg::~CWindmillScorerDlg()
{
	if (mpImageRIF)
		delete mpImageRIF;
	if (mfLog)
	{
		mfLog.Flush("\n<~CWindmillScorerDlg>");
	}
}

void CWindmillScorerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWindmillScorerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_EXIT, &CWindmillScorerDlg::OnFileExit)
	ON_COMMAND(ID_FILE_OPENHIGHRESVOLUME, &CWindmillScorerDlg::OnFileOpenhighresvolume)
	ON_COMMAND(ID_FILE_OPENLOWRESVOLUME, &CWindmillScorerDlg::OnFileOpenlowresvolume)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CWindmillScorerDlg::OnBnClickedButtonUpdate)
	ON_COMMAND(ID_FILE_OPENLASTSELECTION, &CWindmillScorerDlg::OnFileOpenlastselection)
END_MESSAGE_MAP()


// CWindmillScorerDlg message handlers

BOOL CWindmillScorerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetStatusWindow(IDC_STATIC_STATUS);
	SetStatusWindow1(IDC_STATIC_STATUS2);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWindmillScorerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWindmillScorerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWindmillScorerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWindmillScorerDlg::OnFileExit()
{
	CMyWindows::MessBox("Exiting on users request", "Notice");
	exit(0);
}

void CWindmillScorerDlg::OnFileOpenhighresvolume()
{
	if (gWMAScorer.mpHRImages)
		return;

	CMyFileDialog dlg(CMyFileDialog::FD_OPEN, "Select one image from High Resolution Volume", gConfig.msDataRoot);
	if (!dlg.DoModal())
		return;

	gWMAScorer.SetHRVolume(dlg.mSelectedFileName);

	LoadImageR();
}

void CWindmillScorerDlg::OnFileOpenlowresvolume()
{
	if (!gWMAScorer.mpHRImages)
		return;
	if (gWMAScorer.mpLRImages)
		return;

	CMyFileDialog dlg(CMyFileDialog::FD_OPEN, "Select one image from Low Resolution Volume", gConfig.msDataRoot);
	if (!dlg.DoModal())
		return;

	gWMAScorer.SetLRVolume(dlg.mSelectedFileName);
	OnLRVolumeSet();
}
void CWindmillScorerDlg::OnLRVolumeSet()
{
	mpImageRIF->FileOpen(gWMAScorer.mpLRImages->Name());
	mpImageRIF->AddDiff();
}
bool CWindmillScorerDlg::LoadImageR()
{
	if (mpImageRIF)
	{
		delete mpImageRIF;
		Sleep(200);
	}

	//mpImageRIF = new CImageRIF(0, 0, false, gConfig.mDefaultImageRScreenX, gConfig.mDefaultImageRScreenY);
	mpImageRIF = new CImageRIF(0, 0, false);
	mpImageRIF->SetTitle("Windmill Scorer Viewer");
	CString sfName(gWMAScorer.mpHRImages->Name());
	mpImageRIF->FileOpen(sfName);
	mpImageRIF->SetPosition(sfName, gWMAScorer.GetCurrent());
	mpImageRIF->SetViewerBroadPos();
	mpImageRIF->SetIndicesToUpdatePosition2d(miPos, miPos2d);
	mpImageRIF->SetWindow(100, 500);

	gWMAScorer.SetImageRIF(mpImageRIF);
	return true;
}

LRESULT CWindmillScorerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	char zBuf[128];
	if (mpImageRIF)
	{
		mfLog.Printf("<WindowProc> Message %d (%3d %3d)\n", message, wParam, lParam);
		bool bChange = false;
		if (mpImageRIF->GetPositionMessage(message, wParam, lParam, bChange))
		{
			static int uCount = 0;
			sprintf_s(zBuf, sizeof(zBuf), "<WindowProc> Position Messgae(%d) %d %d", ++uCount, (int)wParam, (int)lParam);
			CMyWindows::PrintStatus1(zBuf);

			if (bChange)
			{
				if (message == CPosition::umaPositionMsg[0])
					mfLog.Printf("<WindowProc> PositionMsg 0 ID %d pos %d\n",
						(int)lParam, (int)wParam);
				else if (message == CPosition::umaPositionMsg[1])
					mfLog.Printf("<WindowProc> PositionMsg 1 ID %d pos %d\n",
						(int)lParam, (int)wParam);
				mfLog.Printf("<WindowProc> Pos (%3d %3d)\n", miPos, miPos2d);

				//DisplayPos();
				sprintf_s(zBuf, sizeof(zBuf), "<WindowProc> Position Messgae %d %d", (int)wParam, (int)lParam);
				CMyWindows::PrintStatus(zBuf);
				/*
				if (mpImages)
				{
					CPosition* pPosition = mpImages->GetPosition();
					if (miPos >= pPosition->miFirst && miPos <= pPosition->miLast)
						mpImages->SetCurrent(miPos);
				}*/
				//if (mpSmoother)
				//	UpdateSmooth();
			}
			return 0;
		}
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CWindmillScorerDlg::OnBnClickedButtonUpdate()
{
	if (!gWMAScorer.mpLRImages)
		return;

	int iImage = -1;
	if (GetParameter(IDC_EDIT_I_IMAGE, iImage))
	{
		mpImageRIF->SetPosition(gWMAScorer.mpHRImages->Name(), iImage);
		gWMAScorer.SetCurrent(iImage);
		float score = gWMAScorer.ComputeScore();
		SetParameter(IDC_EDIT_SCORE, score);
	}
}

void CWindmillScorerDlg::OnFileOpenlastselection()
{
	if (!gWMAScorer.OpenLastSelection())
		return;

	LoadImageR();
	OnLRVolumeSet();
}
