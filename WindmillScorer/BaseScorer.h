#pragma once
#include "..\..\ImageRLib\TSharedImage.h"

class CBaseScorer
{
public:
	CBaseScorer(const char *zName, int i=-1);
	~CBaseScorer();

	static void SetImageRIF(class CImageRIF* pImageRIF) { umpImageRIF = pImageRIF; }

	static int GetCurrent() { return umiCurrentImage; }
	bool SetCurrent(int iImage);
	bool Init();

	CDataCoordinates& GetScoreCoo() { return mScoreCoo; }

	float FindMax(CTImage<float>* pImage);
	void Add(CTImage<float>* pIm1, CTImage<float>* pIm2, CTImage<float>* pRes);

	static const int MAX_DIRS = 4;

protected:
	void DumpImage(CTImage<float>* pImage, const char* zText = NULL);
	CString msName;
	CTSharedImage<float>* mpScores;

	// Current Score
	float mScore;
	CDataCoordinates mScoreCoo;

	// Static for all scorers - should be set only once
	static class CImageRIF* umpImageRIF;
	static class CSmoother* umpSmoother;

	static int umiCurrentImage;
	static int umnLines;
	static int umnCols;
	static int umnPixelsPerImage;
	static int umnImages;

};

