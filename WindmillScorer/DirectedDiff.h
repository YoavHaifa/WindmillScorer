#pragma once
#include "..\..\ImageRLib\TImage.h"

class CDirectedDiff
{
public:
	CDirectedDiff(CTImage<float>* pPrepDiff, bool bPositive, int iCurrent);
	~CDirectedDiff();

	void Update(CTImage<float>* pPrepDiff, int iCurrent);
	CTImage<float>* GetAmp() { return mpDirAmp; }

private:
	void GetSignedDiff(CTImage<float>* pPrepDiff);
	void ComputeDiffDir();
	void ComputeDiffDirAmp(int iDir);

	int mnLines;
	int mnCols;
	int mnPixelsPerImage;
	bool mbPositive;
	int miCurrent;
	CString msName;

	CTImage<float>* mpDiff; // Always positive, >= 0
	CTImage<float>* mpDir; // Directed diff
	CTImage<float>* mpDirAmp; // Directed and amplified

};

