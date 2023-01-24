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

	bool OpenLastSelection();
	float ComputeScore();

	class CMultiDataF* mpHRImages;
	class CMultiDataF* mpLRImages;

private:
	void SaveSelection();
	void ComputeDiff();
	float ComputeAverageAbsDiff(CTImage<float>* pDiffImage);
	void MaskEdge();
	void PrepDiff();
	void SeparatePosAndNegDiff();
	void ComputeDiffDirs();
	void ComputeDiffDirsAmp();
	void BoostConsistency();
	float FindMax(CTImage<float>* pImage);

	void ComputeDiffDir(CTImage<float>* pDiff, CTImage<float>* pDir);
	void ComputeDiffDirAmp(CTImage<float>* pDiff, CTImage<float>* pDir, CTImage<float>* pDirAmp, int iDir);
	void Add(CTImage<float>* pIm1, CTImage<float>* pIm2, CTImage<float>* pRes);
	void Sort2(float v1, float v2, float& oMin, float& oMax);

	CTImage<float>* mpOrig;
	CTImage<float>* mpDiff;
	CTImage<float>* mpEdge;
	CTImage<float>* mpAux;
	CTImage<float>* mpPrepDiff;
	CTImage<float>* mpNegDiff;
	CTImage<float>* mpPosDiff;
	CTImage<float>* mpNegDir;
	CTImage<float>* mpPosDir;
	CTImage<float>* mpNegDirAmp;
	CTImage<float>* mpPosDirAmp;
	CTImage<float>* mpDirAmp;
	CTImage<float>* mpDirAmpCons;
	CTImage<float>* mpDirAmpSmooth;

	class CSmoother* mpSmoother;

	int miCurrent;
	int mnLines;
	int mnCols;
	int mnPixelsPerImage;

	int mDebug;
	int mDump;
	CLogFile mfLog;
};

extern CWMAScorer gWMAScorer;