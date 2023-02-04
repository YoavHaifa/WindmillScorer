#pragma once
#include "..\..\yUtils\LogFile.h"
#include "..\..\ImageRLib\TImage.h"
#include "..\..\ImageRLib\TSharedImage.h"
#include "..\..\ImageRLib\DataCoordinates.h"

class CWMAScorer
{
public:
	CWMAScorer();
	~CWMAScorer();

	void SetImageRIF(class CImageRIF* pImageRIF) { mpImageRIF = pImageRIF; }

	void SetHRVolume(const char* zfName);
	void SetLRVolume(const char* zfName);

	void StartBackgroundThread();

	int GetCurrent() { return miCurrentImage; }
	bool SetCurrent(int i);

	bool OpenLastSelection();
	float ComputeScore();

	class CMultiDataF* mpHRImages;
	class CMultiDataF* mpLRImages;

private:
	static DWORD WINAPI StaticBGThread(LPVOID p);
	void BGThread();
	void SaveSelection();
	void ComputeDiff();
	float ComputeAverageAbsDiff(CTImage<float>* pDiffImage);
	void MaskEdge();
	void PrepDiff();
	void ComputeDiffDirsAmp();
	void BoostConsistency();
	float FindMax(CTImage<float>* pImage);

	void Add(CTImage<float>* pIm1, CTImage<float>* pIm2, CTImage<float>* pRes);
	void Sort2(float v1, float v2, float& oMin, float& oMax);

	void Display();
	void DisplayRoi();

	CTImage<float>* mpOrig;
	CTImage<float>* mpDiff;
	CTImage<float>* mpEdge;
	CTImage<float>* mpAux;
	CTSharedImage<float>* mpPrepDiff;
	CTSharedImage<float>* mpDirAmp;
	CTImage<float>* mpDirAmpCons;
	CTSharedImage<float>* mpDirAmpSmooth;

	class CDirectedDiff* mpPosDir;
	class CDirectedDiff* mpNegDir;

	class CImageRIF* mpImageRIF;
	//CTSharedImage<float>* mpSharedImage;


	class CSmoother* mpSmoother;

	int miCurrentImage;
	int mnLines;
	int mnCols;
	int mnPixelsPerImage;
	int mnImages;

	// Current Score
	float mScore;
	CDataCoordinates mScoreCoo;
	float mRoiRadius;

	int mDebug;
	int mDump;
	CLogFile mfLog;
};

extern CWMAScorer gWMAScorer;