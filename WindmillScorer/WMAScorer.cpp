#include "pch.h"
#include "WMAScorer.h"
#include "..\..\yUtils\MyWindows.h"
#include "..\..\yUtils\XMLDump.h"
#include "..\..\yUtils\XMLParse.h"
#include "..\..\ImageRLib\MultiDataF.h"

CWMAScorer gWMAScorer;

CWMAScorer::CWMAScorer()
	: mpHRImages(NULL)
	, mpLRImages(NULL)
	, mpDiff(NULL)
	, mpPrepDiff(NULL)
	, miCurrent(0)
	, mnCols(0)
	, mnLines(0)
	, mnPixelsPerImage(0)
	, mDebug(15)
	, mDump(15)
	, mfLog("WMAScorer")
{

}

CWMAScorer::~CWMAScorer()
{
	if (mpHRImages)
		delete mpHRImages;

	if (mpLRImages)
		delete mpLRImages;

	if (mpDiff)
		delete mpDiff;

	if (mpPrepDiff)
		delete mpPrepDiff;
}

void CWMAScorer::SetHRVolume(const char* zfName)
{
	mpHRImages = new CMultiDataF(zfName);
	miCurrent = mpHRImages->GetNPages() / 2;
	mpHRImages->SetCurrent(miCurrent);
	mnLines = mpHRImages->GetNLinesInPage();
	mnCols = mpHRImages->GetNCols();
	mnPixelsPerImage = mnLines * mnCols;
	SaveSelection();
	mfLog.Flush("<SetHRVolume>", zfName);
	mpHRImages->LogImage(mfLog, miCurrent);
}

void CWMAScorer::SetLRVolume(const char* zfName)
{
	mpLRImages = new CMultiDataF(zfName);
	SaveSelection();

	if (!mpDiff)
	{
		mpDiff = new CTImage<float>("Diff", mnLines, mnCols);
		mpPrepDiff = new CTImage<float>("PrepDiff", mnLines, mnCols);
	}
	mfLog.Flush("<SetLRVolume>", zfName);
	mpHRImages->LogImage(mfLog, miCurrent);
}

void CWMAScorer::SetCurrent(int i) 
{
	miCurrent = i;
	SaveSelection();
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
	dump.Write("current", miCurrent);
}

bool CWMAScorer::OpenLastSelection()
{
	CString sDir(CMyWindows::GetApplicationPath());
	CXMLParse parse(sDir, "LastCase", "selection");
	CXMLParseNode* pRoot = parse.GetRoot();
	if (!pRoot)
		return false;

	CString s;
	if (pRoot->GetValue("hr_volume", s))
		SetHRVolume(s);
	else
		return false;

	if (pRoot->GetValue("lr_volume", s))
		SetLRVolume(s);
	else
		return false;

	pRoot->GetValue("current", miCurrent);
	return true;
}
float CWMAScorer::ComputeScore()
{
	mfLog.Flush("<ComputeScore> current", miCurrent);
	mpHRImages->LogImage(mfLog, miCurrent);
	mpLRImages->LogImage(mfLog, miCurrent);

	ComputeDiff();
	PrepDiff();
	float scoreOrig = ComputeAverageAbsDiff(mpDiff);
	mfLog.Flush("<ComputeScore> Original diff score", scoreOrig);

	float score  = ComputeAverageAbsDiff(mpPrepDiff);
	mfLog.Flush("<ComputeScore> Prep diff score", score);
	return score;
}
void CWMAScorer::ComputeDiff()
{
	float* pHR = mpHRImages->GetImageData(miCurrent);
	float* pLR = mpLRImages->GetImageData(miCurrent);
	float* pDiff = mpDiff->GetData();

	for (int i = 0; i < mnPixelsPerImage; i++)
		pDiff[i] = pLR[i] - pHR[i];

	if (mDump)
		mpDiff->Dump("WMA_Diff", miCurrent);
}

float CWMAScorer::ComputeAverageAbsDiff(CTImage<float>* pDiffImage)
{
	float* pDiff = pDiffImage->GetData();

	float sum = 0;
	for (int i = 0; i < mnPixelsPerImage; i++)
		sum += abs(pDiff[i]);

	return sum / mnPixelsPerImage;
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

void CWMAScorer::PrepDiff()
{
	mpPrepDiff->Zero();

	float* pDiffRaster = mpDiff->GetData();
	float* pPrepRaster = mpPrepDiff->GetData();

	for (int iLine = 1; iLine < mnLines - 1; iLine++)
	{
		float* pDiffPrevLine = pDiffRaster + (iLine - 1) * mnCols;
		float* pDiffLine = pDiffRaster + iLine * mnCols;
		float* pPrepLine = pPrepRaster + iLine * mnCols;

		for (int iCol = 1; iCol < mnCols - 1; iCol++)
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
					pEnv += mnCols;
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
					pEnv += mnCols;
				}
				if (nextMinEnv >= 0)
					pPrepLine[iCol] = 0;
				else
					pPrepLine[iCol] = max(curDiff, nextMinEnv);
			}
		}
	}

	if (mDump)
		mpPrepDiff->Dump("WMA_PrepDiff", miCurrent);		
}
