#pragma once
#include "BaseScorer.h"
#include "..\..\yUtils\LogFile.h"
#include "..\..\ImageRLib\TSharedImage.h"
#include "..\..\ImageRLib\DataCoordinates.h"

class CWMAScorer : public CBaseScorer
{
public:
	CWMAScorer();
	~CWMAScorer();


	void SetHRVolume(const char* zfName);
	void SetLRVolume(const char* zfName);

	void StartBackgroundThread();

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

	void Sort2(float v1, float v2, float& oMin, float& oMax);

	void Display();
	void DisplayRoi();

	CTImage<float>* mpOrig;
	CTImage<float>* mpDiff;
	CTImage<float>* mpEdge;
	CTImage<float>* mpAux;
	CTSharedImage<float>* mpPrepDiff;

	class CDirScore* mapDirScore[MAX_DIRS];
	int miSelectedDir;

	//CTSharedImage<float>* mpSharedImage;


	float mRoiRadius;

	int mDebug;
	int mDump;
	CLogFile mfLog;
public:
	void DisplayDir(int iDir);
};

extern CWMAScorer gWMAScorer;