
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
#include "..\..\ImageRLib\ArchivesImages.h"

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
	: CDialogEx(IDD_WINDMILLSCORER_DIALOG, pParent)
	, mpHRImages(NULL)
	, mpLRImages(NULL)
	, mpImageRIF(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CWindmillScorerDlg::~CWindmillScorerDlg()
{
	if (mpImageRIF)
		delete mpImageRIF;

	if (mpHRImages)
		delete mpHRImages;

	if (mpLRImages)
		delete mpLRImages;
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
	CString sPath("d:\\DLIR_V311\\Test Data");
	CMyFileDialog dlg(CMyFileDialog::FD_OPEN, "Select one image from High Resolution Volume", sPath);
	if (!dlg.DoModal())
		return;

	CString sHRImageName = dlg.mSelectedFileName;
	//CMyWindows::MessBox(sHRImageName, "Selected Folder");

	mpHRImages = new CArchivesImages(sHRImageName);
	miCurrent = mpHRImages->GetNFiles() / 2;
	mpHRImages->SetCurrent(miCurrent);
	mnLines = mpHRImages->GetNLinesInPage();
	mnCols = mpHRImages->GetNCols();
	mnPixelsPerImage = mpHRImages->GetNPixelsPerImage();

	LoadImageR();
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
	mpImageRIF->FileOpen(mpHRImages->GetCurrentImageName());
	return true;
}