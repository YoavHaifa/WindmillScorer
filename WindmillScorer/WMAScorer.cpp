#include "pch.h"
#include "WMAScorer.h"
#include "..\..\yUtils\MyWindows.h"
#include "..\..\yUtils\XMLDump.h"
#include "..\..\yUtils\XMLParse.h"
#include "..\..\ImageRLib\MultiDataF.h"
#include "..\..\ImageRLib\Smoother.h"
#include "..\..\ImageRLib\Edger.h"
#include "..\..\ImageRLib\ImageRIF.h"
#include "DirectedDiff.h"

CWMAScorer gWMAScorer;

CWMAScorer::CWMAScorer()
	: mpHRImages(NULL)
	, mpLRImages(NULL)
	, mpOrig(NULL)
	, mpDiff(NULL)
	, mpNegDir(NULL)
	, mpPosDir(NULL)
	, mpDirAmp(NULL)
	, mpDirAmpSmooth(NULL)
	, mpDirAmpCons(NULL)
	, mpPrepDiff(NULL)
	, mpEdge(NULL)
	, mpAux(NULL)
	, mpImageRIF(NULL)
	, mpSharedImage(NULL)
	, mpSmoother(NULL)
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

	if (mpNegDir)
		delete mpNegDir;

	if (mpPosDir)
		delete mpPosDir;

	if (mpDirAmp)
		delete mpDirAmp;

	if (mpDirAmpCons)
		delete mpDirAmpCons;

	if (mpDirAmpSmooth)
		delete mpDirAmpSmooth;

	if (mpSharedImage)
		delete mpSharedImage;

	if (mpSmoother)
		delete mpSmoother;
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
		mpOrig = new CTImage<float>("Orig", mnLines, mnCols);
		mpDiff = new CTImage<float>("Diff", mnLines, mnCols);
		mpEdge = new CTImage<float>("Edge", mnLines, mnCols);
		mpAux = new CTImage<float>("Aux", mnLines, mnCols);
		mpPrepDiff = new CTImage<float>("PrepDiff", mnLines, mnCols);
		mpDirAmp = new CTImage<float>("DirAmp", mnLines, mnCols);
		mpDirAmpCons = new CTImage<float>("DirAmpCons", mnLines, mnCols);
		mpDirAmpSmooth = new CTImage<float>("DirAmpSmooth", mnLines, mnCols);

		mpSmoother = new CSmoother(mnLines, mnCols);
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
	MaskEdge();
	PrepDiff();
	if (!mpPosDir)
	{
		mpPosDir = new CDirectedDiff(mpPrepDiff, true, miCurrent);
		mpNegDir = new CDirectedDiff(mpPrepDiff, false, miCurrent);
	}
	else
	{
		mpPosDir->Update(mpPrepDiff, miCurrent);
		mpNegDir->Update(mpPrepDiff, miCurrent);
	}

	//SeparatePosAndNegDiff();
	//ComputeDiffDirs();
	ComputeDiffDirsAmp();
	float score = FindMax(mpDirAmpSmooth);

	float scoreOrig = ComputeAverageAbsDiff(mpDiff);
	mfLog.Flush("<ComputeScore> Original diff score", scoreOrig);

	float scorePrep  = ComputeAverageAbsDiff(mpPrepDiff);
	mfLog.Flush("<ComputeScore> Prep diff score", scorePrep);

	mfLog.Flush("<ComputeScore> score", score);

	Display(mpDirAmpSmooth);
	return score;
}
void CWMAScorer::ComputeDiff()
{
	float* pHR = mpHRImages->GetImageData(miCurrent);
	float* pLR = mpLRImages->GetImageData(miCurrent);
	float* pOrig = mpOrig->GetData();
	float* pDiff = mpDiff->GetData();

	for (int i = 0; i < mnPixelsPerImage; i++)
	{
		pOrig[i] = pLR[i];
		pDiff[i] = pLR[i] - pHR[i];
	}

	if (mDump)
	{
		mpOrig->Dump("WMA_Orig", miCurrent);
		mpDiff->Dump("WMA_Diff", miCurrent);
	}
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
void CWMAScorer::MaskEdge()
{
	CEdger edger(mnLines, mnCols, true /*bFloat*/);
	edger.ComputeEdge(mpEdge->GetData(), mpOrig->GetData(), mpAux->GetData(), 5, 5);

	float* pEdgeRaster = mpEdge->GetData();
	for (int i = 0; i < mnPixelsPerImage; i++)
	{
		pEdgeRaster[i] = abs(pEdgeRaster[i]);
	}

	mpSmoother->SetSmoothFactor(2);
	mpSmoother->SmoothFloat(*mpAux, *mpEdge);

	float threshold = 50;
	
	float* pDiffRaster = mpDiff->GetData();
	float* pSmoothEdgeRaster = mpAux->GetData();
	for (int i = 0; i < mnPixelsPerImage; i++)
	{
		if (pSmoothEdgeRaster[i] > threshold)
			pDiffRaster[i] = 0;
	}

	if (mDump)
	{
		mpEdge->Dump("WMA_Edge", miCurrent);
		mpAux->Dump("WMA_EdgeSmooth", miCurrent);
		mpDiff->Dump("WMA_DiffMasked", miCurrent);
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
/*
void CWMAScorer::SeparatePosAndNegDiff()
{
	mpNegDiff->Zero();
	mpPosDiff->Zero();

	float* pPrepRaster = mpPrepDiff->GetData();
	float* pNegRaster = mpNegDiff->GetData();
	float* pPosRaster = mpPosDiff->GetData();

	for (int i = 0; i < mnPixelsPerImage; i++)
	{
		float diff = pPrepRaster[i];
		if (diff < 0)
			pNegRaster[i] = -diff;
		else if (diff > 0)
			pPosRaster[i] = diff;
	}

	if (mDump)
	{
		mpNegDiff->Dump("WMA_NegDiff", miCurrent);
		mpPosDiff->Dump("WMA_PosDiff", miCurrent);
	}
}*/
/*
void CWMAScorer::ComputeDiffDirs()
{
	ComputeDiffDir(mpNegDiff, mpNegDir);
	ComputeDiffDir(mpPosDiff, mpPosDir);
	if (mDump)
	{
		mpNegDir->Dump("WMA_NegDir", miCurrent);
		mpPosDir->Dump("WMA_PosDir", miCurrent);
	}
}*/
/*
void CWMAScorer::ComputeDiffDir(CTImage<float>* pDiff, CTImage<float>* pDir)
{
	pDir->Zero();

	float* pDiffRaster = pDiff->GetData();
	float* pDirRaster = pDir->GetData();

	for (int iLine = 1; iLine < mnLines - 1; iLine++)
	{
		float* pDiffPrevLine = pDiffRaster + (iLine - 1) * mnCols;
		float* pDiffLine = pDiffPrevLine + mnCols;
		float* pDiffNextLine = pDiffLine + mnCols;
		float* pDirLine = pDirRaster + iLine * mnCols;

		for (int iCol = 1; iCol < mnCols - 1; iCol++)
		{
			if (pDiffLine[iCol] == 0)
				continue;

			int iBestDir = 0;
			float highest = 0;
			int iDir = 1;

			// Compare previous to next line
			for (int iTry = -1; iTry < 2; iTry++)
			{
				float value = min(pDiffPrevLine[iCol + iTry], pDiffNextLine[iCol - iTry]);
				if (value > highest)
				{
					highest = value;
					iBestDir = iDir;
				}
				iDir++;
			}
			// Compare horizontal (current line)
			float value = min(pDiffLine[iCol - 1], pDiffLine[iCol + 1]);
			if (value > highest)
			{
				highest = value;
				iBestDir = iDir;
			}
			pDirLine[iCol] = (float)iBestDir;
		}
	}
}*/
void CWMAScorer::ComputeDiffDirsAmp()
{
	//ComputeDiffDirAmp(mpNegDiff, mpNegDir, mpNegDirAmp, 1);
	//ComputeDiffDirAmp(mpPosDiff, mpPosDir, mpPosDirAmp, 1);
	Add(mpNegDir->GetAmp(), mpPosDir->GetAmp(), mpDirAmp);
	//mpSmoother->SmoothFloat(*mpDirAmpSmooth, *mpDirAmp);

	if (mDump)
	{
		mpDirAmp->Dump("WMA_DirAmp1", miCurrent);
		//mpDirAmpSmooth->Dump("WMA_DirAmp1Smooth1", miCurrent);
	}

	BoostConsistency();

	mpSmoother->SetSmoothFactor(5);
	mpSmoother->SmoothFloat(*mpDirAmpSmooth, *mpDirAmpCons);
	if (mDump)
	{
		mpDirAmpCons->Dump("WMA_DirAmp1Cons", miCurrent);
		mpDirAmpSmooth->Dump("WMA_DirAmp1Smooth5", miCurrent);
	}
}
void CWMAScorer::BoostConsistency()
{
	mpDirAmpCons->Zero();

	int delta = 4;
	float* pAmp = mpDirAmp->GetData();
	float *pCons = mpDirAmpCons->GetData();
	int envSide = delta * 2 + 1;
	int nInEnv = envSide * envSide;

	for (int iLine = delta; iLine < mnLines - delta; iLine++)
	{
		float* pAmpLine = pAmp + iLine * mnCols;
		float* pConsLine = pCons + iLine * mnCols;
		for (int iCol = delta; iCol < mnCols - delta; iCol++)
		{
			float amp = pAmpLine[iCol];
			if (amp == 0)
				continue;

			float low = amp / 2;
			float high = amp * 2;

			int n = 0;
			for (int iL = iLine - delta; iL <= iLine + delta; iL++)
			{
				float *pEnvLine = pAmp + iL * mnCols;
				for (int iC = iCol - delta; iC <= iCol + delta; iC++)
				{
					float ampEnv = pEnvLine[iC];
					if (ampEnv >= low && ampEnv <= high)
						n++;
				}
			}
			pConsLine[iCol] = amp * n / nInEnv;
		}
	}
}
/*
void CWMAScorer::ComputeDiffDirAmp(CTImage<float>* pDiff, CTImage<float>* pDir, CTImage<float>* pDirAmp, int iDir)
{
	pDirAmp->Zero();

	int iPrev = iDir - 1;
	if (iPrev < 1)
		iPrev = 4;
	int iNext = iDir + 1;
	if (iNext > 4)
		iNext = 1;

	float* pDiffRaster = pDiff->GetData();
	float* pDirRaster = pDir->GetData();
	float* pAmpRaster = pDirAmp->GetData();

	for (int i = 0; i < mnPixelsPerImage; i++)
	{
		float dir = pDirRaster[i];
		if (dir)
		{
			float diff = pDiffRaster[i];
			float amp = 0;
			if (dir == iDir)
				amp = diff * 3;
			else if (dir == iPrev || dir == iNext)
				amp = diff;
			else
				amp = -diff;
			pAmpRaster[i] = amp;
		}
	}
}*/
void CWMAScorer::Add(CTImage<float>* pIm1, CTImage<float>* pIm2, CTImage<float>* pRes)
{
	float* pRaster1 = pIm1->GetData();
	float* pRaster2 = pIm2->GetData();
	float* pResRaster = pRes->GetData();

	for (int i = 0; i < mnPixelsPerImage; i++)
		pResRaster[i] = pRaster1[i] + pRaster2[i];
}
float CWMAScorer::FindMax(CTImage<float>* pImage)
{
	float maxVal = -1000;

	float* pRaster = pImage->GetData();
	for (int i = 0; i < mnPixelsPerImage; i++)
	{
		float value = pRaster[i];
		if (value > maxVal)
			maxVal = value;
	}
	return maxVal;
}
void CWMAScorer::Display(CTImage<float>* pImage)
{
	if (!mpImageRIF)
		return;

	if (!mpSharedImage)
	{
		mpSharedImage = new CTSharedImage<float>("SharedImage", mnLines, mnCols);
		mpSharedImage->SetZoomName("SharedZoom");
	}

	mpSharedImage->StartWrite();
	mpSharedImage->CopyFromRaster(pImage->GetData());
	mpSharedImage->OnPageUpdate(0);
	mpImageRIF->DisplayShared(mpSharedImage);
}
