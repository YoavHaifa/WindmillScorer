#pragma once
#include "..\..\yUtils\LogFile.h"
#include "..\..\ImageRLib\TImage.h"

class CWMAScorer
{
public:
	CWMAScorer();
	~CWMAScorer();

	void SetHRVolume(const char* zfName);
	void SetLRVolume(const char* zfName);

	int GetCurrent() { return miCurrent; }
	void SetCurrent(int i);

	class CMultiDataF* mpHRImages;
	class CMultiDataF* mpLRImages;
	CTImage<float>* mpDiff;
	CTImage<float>* mpPrepDiff;

private:
	void Sort2(float v1, float v2, float& oMin, float& oMax);
	int miCurrent;
	int mnLines;
	int mnCols;
	int mnPixelsPerImage;

	int mDebug;
	int mDump;
	CLogFile mfLog;

public:
	bool OpenLastSelection();
	void SaveSelection();
	float ComputeScore();
	void ComputeDiff();
	float ComputeAverageAbsDiff(CTImage<float>* pDiff);
	void PrepDiff();
};

extern CWMAScorer gWMAScorer;