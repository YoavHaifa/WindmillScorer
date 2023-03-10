
// WindmillScorerDlg.h : header file
//

#pragma once
#include "..\..\yUtils\MyDialogEx.h"
#include "..\..\yUtils\LogFile.h"


// CWindmillScorerDlg dialog
class CWindmillScorerDlg : public CMyDialogEx
{
// Construction
public:
	CWindmillScorerDlg(CWnd* pParent = nullptr);	// standard constructor
	~CWindmillScorerDlg();

	static CWindmillScorerDlg* umpDlg;

	void DisplayScore(float score);

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
	CLogFile mfLog;
	int miPos;
	int miPos2d;
	bool mbInitializing;

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	bool LoadImageR();
	void OnLRVolumeSet();
	void DisplayPos();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileExit();
	afx_msg void OnFileOpenhighresvolume();
	afx_msg void OnFileOpenlowresvolume();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnFileOpenlastselection();
	afx_msg void OnDisplaySelectdirdif();
	afx_msg void OnHintsAddwmaroi();
	afx_msg void OnHintsAddcleanroi();
	afx_msg void OnHintsSaveallhints();
};
