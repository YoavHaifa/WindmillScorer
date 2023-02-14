#include "pch.h"
#include "Hints.h"
#include "..\..\yUtils\FilesList.h"
#include "..\..\yUtils\XMLParse.h"
#include "..\..\ImageRLib\ImageRIF.h"
#include "..\..\ImageRLib\DataRoi.h"
#include "Config.h"

CHints gHints;

CHints::CHints()
	: CBaseScorer("Hints")
{

}
void CHints::DisplayRoi(CDataRoi* pRoi)
{
	umpImageRIF->DisplayGraphic(pRoi);
	umpImageRIF->AddGraphicElementToWatch(pRoi);
}
void CHints::AddRoi(bool bWMA)
{
	if (!umpImageRIF)
		return;

	COLORREF color;
	int id;
	CString sType;
	if (bWMA)
	{
		color = 0x0000ff; // Red
		id = (int)mWMAHints.GetSize() + 1;
		sType = "WMA";
	}
	else
	{
		color = 0x00ff00; // Green
		id = (int)mCleanHints.GetSize() + 1;
		sType = "Clean";
	}

	CString sName;
	sName.Format("%s_Roi%d_im%d", (const char *)sType, id, umiCurrentImage);
	CDataRoi* pRoi = new CDataRoi(NULL, sName, color);
	CDataCoordinates center(umnCols / 2.0f, umnLines / 2.0f);
	pRoi->InitEllipse(center, 10.0f + id);
	pRoi->SetDisplayOnAll(true);
	pRoi->LockToPosition(umiCurrentImage);
	pRoi->SetSaveToXml(true);
	pRoi->mbReportClientOnActivation = true;
	DisplayRoi(pRoi);

	if (bWMA)
		mWMAHints.AddTail(pRoi);
	else
		mCleanHints.AddTail(pRoi);
}
void CHints::SaveAll()
{
	SaveAll(mWMAHints);
	SaveAll(mCleanHints);
}
void CHints::SaveAll(CList<CDataRoi*, CDataRoi*>& hints)
{
	POSITION pos = hints.GetHeadPosition();
	while (pos)
	{
		CDataRoi* pRoi = hints.GetNext(pos);
		pRoi->SaveToDir(gConfig.msHintsDir);
	}
}
void CHints::RestoreSavedHints()
{
	CFilesList fList;
	int n = CMyWindows::ListFilesInDir(gConfig.msHintsDir, "xml", fList);
	if (n < 1)
		return;

	POSITION pos = fList.GetHeadPosition();
	while (pos)
	{
		CString* psFileName = fList.GetNext(pos);
		CXMLParse xmlParse(*psFileName);
		CXMLParseNode* pRoot = xmlParse.GetRoot();
		if (!pRoot)
			break;

		CDataRoi *pRoi = new CDataRoi(NULL, pRoot);
		pRoi->SetFreezed(true);
		if (pRoi->msName.Left(3) == "WMA")
			mWMAHints.AddTail(pRoi);
		else if (pRoi->msName.Left(5) == "Clean")
			mCleanHints.AddTail(pRoi);
		else
		{
			CMyWindows::MessBox(pRoi->msName, "Unrecognized Hint");
			delete pRoi;
		}
	}
	DisplayRestoredHints();
}
void CHints::DisplayRestoredHints()
{
	Display(mWMAHints);
	Display(mCleanHints);
}
void CHints::Display(CList<CDataRoi*, CDataRoi*>& hints)
{
	POSITION pos = hints.GetHeadPosition();
	while (pos)
	{
		CDataRoi* pRoi = hints.GetNext(pos);
		pRoi->SaveToDir(gConfig.msHintsDir);
		DisplayRoi(pRoi);
	}
}
