#include "pch.h"
#include "DirectedDiff.h"
#include "Config.h"

CDirectedDiff::CDirectedDiff(CTImage<float>* pPrepDiff, bool bPositive, int iCurrent)
	: mnLines(pPrepDiff->mnLines)
	, mnCols(pPrepDiff->mnCols)
	, mnPixelsPerImage (mnLines * mnCols)
	, mbPositive(bPositive)
	, miCurrent(iCurrent)
	, mpDiff(NULL)
	, mpDir(NULL)
	, mpDirAmp(NULL)
	, msName(bPositive? "Pos" : "Neg")
{
	mpDiff = new CTImage<float>(msName + "Diff", mnLines, mnCols);
	mpDir = new CTImage<float>(msName + "Dir", mnLines, mnCols);
	mpDirAmp = new CTImage<float>(msName + "DirAmp", mnLines, mnCols);
	Update(pPrepDiff, miCurrent);
}
CDirectedDiff::~CDirectedDiff()
{
	if (mpDiff)
		delete mpDiff;
	if (mpDir)
		delete mpDir;
	if (mpDirAmp)
		delete mpDirAmp;
}
void CDirectedDiff::Update(CTImage<float>* pPrepDiff, int iCurrent)
{
	int iDir = 1;
	miCurrent = iCurrent;
	GetSignedDiff(pPrepDiff);
	ComputeDiffDir();
	ComputeDiffDirAmp(iDir);
}
void CDirectedDiff::GetSignedDiff(CTImage<float>* pPrepDiff)
{
	mpDiff->Zero();

	float* pPrepRaster = pPrepDiff->GetData();
	float* pRaster = mpDiff->GetData();

	if (mbPositive)
	{
		for (int i = 0; i < mnPixelsPerImage; i++)
		{
			float diff = pPrepRaster[i];
			if (diff > 0)
				pRaster[i] = diff;
		}
	}
	else
	{
		for (int i = 0; i < mnPixelsPerImage; i++)
		{
			float diff = pPrepRaster[i];
			if (diff < 0)
				pRaster[i] = -diff;
		}
	}

	if (gConfig.mDump)
		mpDiff->Dump(msName + "Diff", miCurrent);
}
void CDirectedDiff::ComputeDiffDir()
{
	mpDir->Zero();

	float* pDiffRaster = mpDiff->GetData();
	float* pDirRaster = mpDir->GetData();

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

	if (gConfig.mDump)
		mpDir->Dump(msName + "Dir", miCurrent);
}
void CDirectedDiff::ComputeDiffDirAmp(int iDir)
{
	mpDirAmp->Zero();

	int iPrev = iDir - 1;
	if (iPrev < 1)
		iPrev = 4;
	int iNext = iDir + 1;
	if (iNext > 4)
		iNext = 1;

	float* pDiffRaster = mpDiff->GetData();
	float* pDirRaster = mpDir->GetData();
	float* pAmpRaster = mpDirAmp->GetData();

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

	if (gConfig.mDump)
		mpDirAmp->Dump(msName + "DirAmp", miCurrent);
}
