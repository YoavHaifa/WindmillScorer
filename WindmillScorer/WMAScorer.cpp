#include "pch.h"
#include "WMAScorer.h"
#include "..\..\yUtils\MyWindows.h"
#include "..\..\yUtils\XMLDump.h"
#include "..\..\yUtils\XMLParse.h"
#include "..\..\ImageRLib\MultiDataF.h"
#include "..\..\ImageRLib\Smoother.h"
#include "..\..\ImageRLib\Edger.h"
#include "..\..\ImageRLib\ImageRIF.h"
#include "..\..\ImageRLib\DataRoi.h"
#include "Config.h"
#include "DirScore.h"
//#include "DirectedDiff.h"
#include "WindmillScorerDlg.h"

CWMAScorer gWMAScorer;

CWMAScorer::CWMAScorer()
	: CBaseScorer("CWMAScorer")
	, mpHRImages(NULL)
	, mpLRImages(NULL)
	, mpTarget(NULL)
	, mpOrig(NULL)
	, mpDiff(NULL)
	, mpPrepDiff(NULL)
	, mpEdge(NULL)
	, mpAux(NULL)
	, mRoiRadius(20)
	, miSelectedDir(-1)
	, mDebug(15)
	, mDump(15)
	, mfLog("WMAScorer")
{
	for (int i = 0; i < MAX_DIRS; i++)
		mapDirScore[i] = NULL;
}
CWMAScorer::~CWMAScorer()
{
	if (mpHRImages)
		delete mpHRImages;

	if (mpLRImages)
		delete mpLRImages;

	if (mpTarget)
		delete mpTarget;

	if (mpOrig)
		delete mpOrig;

	if (mpDiff)
		delete mpDiff;

	if (mpEdge)
		delete mpEdge;

	if (mpAux)
		delete mpAux;

	if (mpPrepDiff)
		delete mpPrepDiff;

	for (int i = 0; i < MAX_DIRS; i++)
	{
		if (mapDirScore[i])
			delete mapDirScore[i];
	}
}
void CWMAScorer::StartBackgroundThread()
{
	CMyWindows::CreateThread(CWMAScorer::StaticBGThread, NULL);
}
DWORD WINAPI CWMAScorer::StaticBGThread(LPVOID /*p*/)
{
	gWMAScorer.BGThread();
	return 0;
}
void CWMAScorer::BGThread()
{
	while (true)
	{
		if (umiCurrentImage != gConfig.miCurrentImage)
		{
			if (SetCurrent(gConfig.miCurrentImage))
			{
				SaveSelection();
				mfLog.Flush("<SetCurrent>", umiCurrentImage);
				ComputeScore();
			}
			else
				Sleep(100);
		}
		else
			Sleep(100);
	}
}
void CWMAScorer::SetHRVolume(const char* zfName)
{
	mpHRImages = new CMultiDataF(zfName);
	umnImages = mpHRImages->GetNPages();
	if (gConfig.miCurrentImage < 1 || gConfig.miCurrentImage > umnImages)
		gConfig.SetPosition(umnImages / 2);

	//mpHRImages->SetCurrent(gConfig.miCurrentImage);
	umnLines = mpHRImages->GetNLinesInPage();
	umnCols = mpHRImages->GetNCols();
	umnPixelsPerImage = umnLines * umnCols;
	SaveSelection();
	mfLog.Flush("<SetHRVolume>", zfName);
	mpHRImages->LogImage(mfLog, gConfig.miCurrentImage);
}

void CWMAScorer::SetLRVolume(const char* zfName)
{
	mpLRImages = new CMultiDataF(zfName);
	SaveSelection();

	if (!mpDiff)
	{
		mpOrig = new CTImage<float>("Orig", umnLines, umnCols);
		mpDiff = new CTImage<float>("Diff", umnLines, umnCols);
		mpEdge = new CTImage<float>("Edge", umnLines, umnCols);
		mpAux = new CTImage<float>("Aux", umnLines, umnCols);
		mpPrepDiff = new CTSharedImage<float>("PrepDiff", umnLines, umnCols);

		for (int i = 0; i < MAX_DIRS; i++)
		{
			if (!mapDirScore[i])
				mapDirScore[i] = new CDirScore(i+1);
		}

		umpSmoother = new CSmoother(umnLines, umnCols);

		mpPrepDiff->SetWindowName("Diff");

	}
	mfLog.Flush("<SetLRVolume>", zfName);
	mpLRImages->LogImage(mfLog, gConfig.miCurrentImage);
}
CMultiDataF* CWMAScorer::GetTarget()
{
	if (!mpHRImages)
		return NULL;

	if (!mpTarget)
		mpTarget = new CMultiDataF(*mpHRImages, "Target");
	return mpTarget;
}
void CWMAScorer::SaveSelection()
{
	if (!mpHRImages)
		return;
	CString sDir(CMyWindows::GetApplicationPath());
	CXMLDump dump(sDir, "LastCase", "selection");
	dump.Write("hr_volume", mpHRImages->Name());
	if (mpLRImages)
		dump.Write("lr_volume", mpLRImages->Name());
	dump.Write("current", gConfig.miCurrentImage);
}

bool CWMAScorer::OpenLastSelection()
{
	CString sDir(CMyWindows::GetApplicationPath());
	CXMLParse parse(sDir, "LastCase", "selection");
	CXMLParseNode* pRoot = parse.GetRoot();
	if (!pRoot)
		return false;

	pRoot->GetValue("current", gConfig.miCurrentImage);

	CString s;
	if (pRoot->GetValue("hr_volume", s))
		SetHRVolume(s);
	else
		return false;

	if (pRoot->GetValue("lr_volume", s))
		SetLRVolume(s);
	else
		return false;

	return true;
}
float CWMAScorer::ComputeScore()
{
	mfLog.Flush("<ComputeScore> current", umiCurrentImage);
	mpHRImages->LogImage(mfLog, umiCurrentImage);
	mpLRImages->LogImage(mfLog, umiCurrentImage);

	mpPrepDiff->StartWrite();

	ComputeDiff();
	MaskEdge();
	PrepDiff();

	mScore = -1000000;
	for (int iDir = 0; iDir < MAX_DIRS; iDir++)
	{
		float score = mapDirScore[iDir]->Compute(mpPrepDiff);
		if (score > mScore)
		{
			miSelectedDir = iDir;
			mScore = score;
			mScoreCoo = mapDirScore[iDir]->GetScoreCoo();
			CString s;
			s.Format("<ComputeScore> dir %d, score %.2f", iDir + 1, score);
			mfLog.Flush(s);
			CMyWindows::PrintStatus(s);
		}
	}


	float scoreOrig = ComputeAverageAbsDiff(mpDiff);
	mfLog.Flush("<ComputeScore> Original diff score", scoreOrig);

	float scorePrep  = ComputeAverageAbsDiff(mpPrepDiff);
	mfLog.Flush("<ComputeScore> Prep diff score", scorePrep);

	mfLog.Flush("<ComputeScore> score", mScore);

	Display();
	if (CWindmillScorerDlg::umpDlg)
		CWindmillScorerDlg::umpDlg->DisplayScore(mScore);
	return mScore;
}
void CWMAScorer::ComputeDiff()
{
	float* pHR = mpHRImages->GetImageData(umiCurrentImage-1);
	float* pLR = mpLRImages->GetImageData(umiCurrentImage-1);
	float* pOrig = mpOrig->GetData();
	float* pDiff = mpDiff->GetData();

	for (int i = 0; i < umnPixelsPerImage; i++)
	{
		pOrig[i] = pLR[i];
		pDiff[i] = pLR[i] - pHR[i];
	}

	if (mDump)
	{
		mpOrig->Dump("WMA_Orig", umiCurrentImage);
		mpDiff->Dump("WMA_Diff", umiCurrentImage);
	}
}

float CWMAScorer::ComputeAverageAbsDiff(CTImage<float>* pDiffImage)
{
	float* pDiff = pDiffImage->GetData();

	float sum = 0;
	for (int i = 0; i < umnPixelsPerImage; i++)
		sum += abs(pDiff[i]);

	return sum / umnPixelsPerImage;
}

void CWMAScorer::Sort2(float v1, float v2, float& oMin, float& oMax)
{
	if (v1 > v2)
	{
		oMin = v2;
		oMax = v1;
	}
	else
	{
		oMin = v1;
		oMax = v2;
	}
}
void CWMAScorer::MaskEdge()
{
	CEdger edger(umnLines, umnCols, true /*bFloat*/);
	edger.ComputeEdge(mpEdge->GetData(), mpOrig->GetData(), mpAux->GetData(), 5, 5);

	float* pEdgeRaster = mpEdge->GetData();
	for (int i = 0; i < umnPixelsPerImage; i++)
	{
		pEdgeRaster[i] = abs(pEdgeRaster[i]);
	}

	umpSmoother->SetSmoothFactor(2);
	umpSmoother->SmoothFloat(*mpAux, *mpEdge);

	float threshold = 50;
	
	float* pDiffRaster = mpDiff->GetData();
	float* pSmoothEdgeRaster = mpAux->GetData();
	for (int i = 0; i < umnPixelsPerImage; i++)
	{
		if (pSmoothEdgeRaster[i] > threshold)
			pDiffRaster[i] = 0;
	}

	if (mDump)
	{
		mpEdge->Dump("WMA_Edge", umiCurrentImage);
		mpAux->Dump("WMA_EdgeSmooth", umiCurrentImage);
		mpDiff->Dump("WMA_DiffMasked", umiCurrentImage);
	}
}
void CWMAScorer::PrepDiff()
{
	mpPrepDiff->Zero();

	float* pDiffRaster = mpDiff->GetData();
	float* pPrepRaster = mpPrepDiff->GetData();

	for (int iLine = 1; iLine < umnLines - 1; iLine++)
	{
		float* pDiffPrevLine = pDiffRaster + (iLine - 1) * umnCols;
		float* pDiffLine = pDiffRaster + iLine * umnCols;
		float* pPrepLine = pPrepRaster + iLine * umnCols;

		for (int iCol = 1; iCol < umnCols - 1; iCol++)
		{
			float* pEnv = pDiffPrevLine + iCol - 1;
			float curDiff = pDiffLine[iCol];

			if (curDiff > 0)
			{
				float maxEnv, nextMaxEnv;
				Sort2(pEnv[0], pEnv[1], nextMaxEnv, maxEnv);

				// Search env
				for (int iL = 0; iL < 3; iL++)
				{
					for (int iC = 0; iC < 3; iC++)
					{
						if (iL == 1 && iC == 1) // The central pixel
							continue;

						float value = pEnv[iC];
						if ((iC > 1 || iL > 0) && value > 0)
						{
							if (value > maxEnv)
							{
								nextMaxEnv = maxEnv;
								maxEnv = value;
							}
							else if (value > nextMaxEnv)
							{
								nextMaxEnv = value;
							}
						}
					}
					pEnv += umnCols;
				}
				if (nextMaxEnv <= 0)
					pPrepLine[iCol] = 0;
				else
					pPrepLine[iCol] = min(curDiff, nextMaxEnv);
			}
			else if (curDiff < 0)
			{
				float minEnv, nextMinEnv;
				Sort2(pEnv[0], pEnv[1], minEnv, nextMinEnv);

				// Search env
				for (int iL = 0; iL < 3; iL++)
				{
					for (int iC = 0; iC < 3; iC++)
					{
						if (iL == 1 && iC == 1) // The central pixel
							continue;

						float value = pEnv[iC];
						if ((iC > 1 || iL > 0) && value < 0)
						{
							if (value < minEnv)
							{
								nextMinEnv = minEnv;
								minEnv = value;
							}
							else if (value < nextMinEnv)
							{
								nextMinEnv = value;
							}
						}
					}
					pEnv += umnCols;
				}
				if (nextMinEnv >= 0)
					pPrepLine[iCol] = 0;
				else
					pPrepLine[iCol] = max(curDiff, nextMinEnv);
			}
		}
	}

	DumpImage(mpPrepDiff);
}

void CWMAScorer::Display()
{
	mpPrepDiff->OnPageUpdate(0);

	umpImageRIF->DisplayShared(mpPrepDiff);

	mapDirScore[miSelectedDir]->Display();

	DisplayRoi();
}
void CWMAScorer::DisplayRoi()
{
	static int count = 0;
	count++;
	CString sName;
	sName.Format("DemoRoi%d_%d_%d", count, umiCurrentImage);
	CDataRoi* pRoi = new CDataRoi(NULL, sName, 0xff0080);

	pRoi->InitEllipse(mScoreCoo, mRoiRadius);
	pRoi->LockToPosition(umiCurrentImage);
	pRoi->SetDisplayOnAll(true);
	pRoi->SetFixedCenter();

	CString sScore;
	sScore.Format("WMA %.2f", mScore);
	pRoi->SetText(sScore);

	//pRoi->SetSaveToXml(true);
	//pRoi->mbReportClientOnActivation = true;
	umpImageRIF->DisplayGraphic(pRoi);
}

void CWMAScorer::DisplayDir(int iDir)
{
	mapDirScore[iDir-1]->Display();
}
