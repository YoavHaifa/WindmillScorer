#include "pch.h"
#include "DirectedDiff.h"
#include "Config.h"

CDirectedDiff::CDirectedDiff(CTImage<float>* pPrepDiff, bool bPositive, int iDir)
	: CBaseScorer(bPositive ? "Pos" : "Neg", iDir)
	, mbPositive(bPositive)
	, miDir(iDir)
	, mpDiff(NULL)
	, mpDir(NULL)
{
	msName.Format("%s_Dir%d", bPositive ? "Pos" : "Neg", iDir);
	mpDiff = new CTImage<float>(msName + "Diff", umnLines, umnCols);
	mpDir = new CTImage<float>(msName + "Dir", umnLines, umnCols);
	Init(); // Start mpScores
	Update(pPrepDiff);
}
CDirectedDiff::~CDirectedDiff()
{
	if (mpDiff)
		delete mpDiff;
	if (mpDir)
		delete mpDir;
}
void CDirectedDiff::Update(CTImage<float>* pPrepDiff)
{
	GetSignedDiff(pPrepDiff);
	ComputeDiffDir();
	ComputeDiffDirAmp(miDir);
}
void CDirectedDiff::GetSignedDiff(CTImage<float>* pPrepDiff)
{
	mpDiff->Zero();

	float* pPrepRaster = pPrepDiff->GetData();
	float* pRaster = mpDiff->GetData();

	if (mbPositive)
	{
		for (int i = 0; i < umnPixelsPerImage; i++)
		{
			float diff = pPrepRaster[i];
			if (diff > 0)
				pRaster[i] = diff;
		}
	}
	else
	{
		for (int i = 0; i < umnPixelsPerImage; i++)
		{
			float diff = pPrepRaster[i];
			if (diff < 0)
				pRaster[i] = -diff;
		}
	}

	DumpImage(mpDiff, "Diff");
}
void CDirectedDiff::ComputeDiffDir()
{
	mpDir->Zero();

	float* pDiffRaster = mpDiff->GetData();
	float* pDirRaster = mpDir->GetData();

	for (int iLine = 1; iLine < umnLines - 1; iLine++)
	{
		float* pDiffPrevLine = pDiffRaster + (iLine - 1) * umnCols;
		float* pDiffLine = pDiffPrevLine + umnCols;
		float* pDiffNextLine = pDiffLine + umnCols;
		float* pDirLine = pDirRaster + iLine * umnCols;

		for (int iCol = 1; iCol < umnCols - 1; iCol++)
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

	DumpImage(mpDir, "Dir");
}
void CDirectedDiff::ComputeDiffDirAmp(int iDir)
{
	mpScores->Zero();

	int iPrev = iDir - 1;
	if (iPrev < 1)
		iPrev = 4;
	int iNext = iDir + 1;
	if (iNext > 4)
		iNext = 1;

	float* pDiffRaster = mpDiff->GetData();
	float* pDirRaster = mpDir->GetData();
	float* pAmpRaster = mpScores->GetData();

	for (int i = 0; i < umnPixelsPerImage; i++)
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

	DumpImage(mpScores, "DirAmp");
}
