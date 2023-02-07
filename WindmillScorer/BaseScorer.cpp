#include "pch.h"
#include "BaseScorer.h"
#include "Config.h"

class CImageRIF* CBaseScorer::umpImageRIF = NULL;
class CSmoother* CBaseScorer::umpSmoother = NULL;

int CBaseScorer::umiCurrentImage = -1;
int CBaseScorer::umnLines = 0;
int CBaseScorer::umnCols = 0;
int CBaseScorer::umnPixelsPerImage = 0;
int CBaseScorer::umnImages = 0;

CBaseScorer::CBaseScorer(const char* zName, int i)
	: msName(zName)
	, mpScores(NULL)
	, mScore(0)
{
	if (i >= 0)
	{
		CString s;
		s.Format("%d", i);
		msName += s;
	}
	Init();
}
CBaseScorer::~CBaseScorer()
{
	if (mpScores)
		delete mpScores;
}
bool CBaseScorer::SetCurrent(int iImage)
{
	if (iImage < 1 || iImage > umnImages)
		return false;
	umiCurrentImage = iImage;
	return true;
}
bool CBaseScorer::Init()
{
	if (umnLines < 1 || umnCols < 1)
		return false;

	if (!mpScores)
		mpScores = new CTSharedImage<float>(msName, umnLines, umnCols);
	return true;
}
void CBaseScorer::DumpImage(CTImage<float>* pImage, const char* zText)
{
	if (!gConfig.mDump)
		return;

	CString sName(msName);
	if (zText)
		sName += zText;
	pImage->Dump(sName, umiCurrentImage);
}
float CBaseScorer::FindMax(CTImage<float>* pImage)
{
	float maxVal = -1000;

	for (int iLine = 0; iLine < umnLines; iLine++)
	{
		float* pLine = pImage->GetLine(iLine);

		for (int iCol = 0; iCol < umnCols; iCol++)
		{
			float value = pLine[iCol];
			if (value > maxVal)
			{
				maxVal = value;
				mScoreCoo.fy = (float)iLine;
				mScoreCoo.fx = (float)iCol;
			}
		}
	}
	mScore = maxVal;
	return maxVal;
}
void CBaseScorer::Add(CTImage<float>* pIm1, CTImage<float>* pIm2, CTImage<float>* pRes)
{
	float* pRaster1 = pIm1->GetData();
	float* pRaster2 = pIm2->GetData();
	float* pResRaster = pRes->GetData();

	for (int i = 0; i < umnPixelsPerImage; i++)
		pResRaster[i] = pRaster1[i] + pRaster2[i];
}
