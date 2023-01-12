
// WindmillScorerDlg.h : header file
//

#pragma once


// CWindmillScorerDlg dialog
class CWindmillScorerDlg : public CDialogEx
{
// Construction
public:
	CWindmillScorerDlg(CWnd* pParent = nullptr);	// standard constructor
	~CWindmillScorerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINDMILLSCORER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	class CImageRIF* mpImageRIF;
	class CArchivesImages* mpHRImages;
	class CArchivesImages* mpLRImages;
	int miCurrent;
	int mnLines;
	int mnCols;
	int mnPixelsPerImage;

	bool LoadImageR();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileExit();
	afx_msg void OnFileOpenhighresvolume();
};
